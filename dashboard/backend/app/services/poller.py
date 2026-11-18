# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Poller da frota: um retrato de todos os C-Shields a cada ciclo.

A cada `poll_seconds` faz UMA chamada list_entities ao Orion e, para cada
C-Shield cadastrado, decide entre online (mandou leitura nos últimos
`offline_seconds`) e offline. O estado (NORMAL, BLOQUEIO...) vem pronto do
firmware no atributo `state`: o dashboard não decide nada, só mostra.

Se a EC2 não responde por mais de `offline_seconds`, a frota inteira aparece
offline com motivo "fiware". Falhas curtas não mudam o retrato.
"""
import asyncio
import logging
from datetime import datetime, timezone

from app.core.config import ConfigStore
from app.models.schemas import Device
from app.models.schemas_fleet import DispositivoStatus, SensorValues
from app.services.device_registry import DeviceRegistry
from app.services.fiware_client import FiwareClient
from app.services.fiware_errors import FiwareError
from app.services.fleet_state import FleetState
from app.services.readings import reading_from_entity

log = logging.getLogger(__name__)


def _agora_utc() -> datetime:
    """Relógio padrão do poller (os testes injetam outro)."""
    return datetime.now(timezone.utc)


def _iso(instante: datetime) -> str:
    """Instante em ISO 8601, segundos."""
    return instante.isoformat(timespec="seconds")


def parse_instant(texto) -> datetime | None:
    """Converte o TimeInstant do IoT Agent (ex.: 2026-11-18T11:59:58.000Z).

    Sem fuso, assume UTC. Vazio, inválido ou não texto devolve None.
    """
    if not isinstance(texto, str) or not texto.strip():
        return None
    try:
        instante = datetime.fromisoformat(texto.strip().replace("Z", "+00:00"))
    except ValueError:
        return None
    if instante.tzinfo is None:
        instante = instante.replace(tzinfo=timezone.utc)
    return instante


class Poller:
    """Monta o retrato da frota a cada ciclo e o entrega ao FleetState."""

    def __init__(self, registry: DeviceRegistry, fiware: FiwareClient, config: ConfigStore,
                 fleet: FleetState, clock=_agora_utc, sleep=asyncio.sleep):
        self._registry = registry
        self._fiware = fiware
        self._config = config
        self._fleet = fleet
        self._clock = clock
        self._sleep = sleep
        # Estado da EC2: instante da primeira falha seguida e se já está dada como fora
        self._falha_desde: datetime | None = None
        self._fiware_fora = False

    async def run(self) -> None:
        """Laço em fundo: um tick a cada poll_seconds (lido a cada volta)."""
        while True:
            try:
                await self.tick()
            except Exception:  # o poller não pode morrer por causa de um ciclo ruim
                log.exception("falha inesperada no ciclo do poller; sigo no próximo")
            await self._sleep(self._config.get().poll_seconds)

    async def tick(self) -> None:
        """Uma passada pela frota inteira (testável sem o laço)."""
        agora = self._clock()
        offline_seconds = self._config.get().offline_seconds
        devices = self._registry.list()
        if not devices and not self._fiware_fora:
            self._publicar([], agora)
            return
        try:
            entidades = await self._fiware.list_entities()
        except FiwareError as exc:
            log.warning("poller sem resposta do FIWARE (%s)", exc.message)
            self._registrar_falha(devices, agora, offline_seconds)
            return
        self._falha_desde = None
        self._fiware_fora = False
        por_id = {entidade.get("id"): entidade for entidade in entidades}
        self._publicar([self._retrato(device, por_id.get(device.entity_id, {}), agora,
                                      offline_seconds) for device in devices], agora)

    def _retrato(self, device: Device, entidade: dict, agora: datetime,
                 offline_seconds: int) -> DispositivoStatus:
        """Retrato de um C-Shield: online se a última leitura é recente."""
        leitura = reading_from_entity(device.device_id, entidade)
        visto = parse_instant(leitura.time_instant)
        # Sem leitura, a referência é o cadastro: um C-Shield novo tem offline_seconds
        # para mandar a primeira leitura antes de aparecer offline.
        referencia = visto or parse_instant(device.created_at)
        passou = None if visto is None else max(0, int((agora - visto).total_seconds()))
        offline = referencia is None or (agora - referencia).total_seconds() > offline_seconds
        return DispositivoStatus(
            device_id=device.device_id, name=device.name, local_id=device.local_id,
            profile=device.profile, status="offline" if offline else "online",
            offline_reason="sem_leitura" if offline else None,
            # Valor antigo de C-Shield offline não vira estado atual
            state=None if offline else leitura.state,
            reason=None if offline else leitura.reason,
            blocks=leitura.blocks, last_seen=leitura.time_instant, seconds_since=passou,
            current=SensorValues(temperature=leitura.temperature, humidity=leitura.humidity,
                                 luminosity=leitura.luminosity, setpoint=leitura.setpoint))

    def _registrar_falha(self, devices: list[Device], agora: datetime,
                         offline_seconds: int) -> None:
        """FIWARE sem resposta: falha curta não muda nada; longa marca a frota offline."""
        if self._falha_desde is None:
            self._falha_desde = agora
        if not self._fiware_fora:
            if (agora - self._falha_desde).total_seconds() <= offline_seconds:
                return
            self._fiware_fora = True
        self._publicar([self._retrato_sem_fiware(device) for device in devices], agora)

    def _retrato_sem_fiware(self, device: Device) -> DispositivoStatus:
        """C-Shield offline por causa da EC2, com os últimos valores conhecidos."""
        anterior = self._fleet.get(device.device_id)
        return DispositivoStatus(
            device_id=device.device_id, name=device.name, local_id=device.local_id,
            profile=device.profile, status="offline", offline_reason="fiware",
            blocks=anterior.blocks if anterior else None,
            last_seen=anterior.last_seen if anterior else None,
            seconds_since=None, current=anterior.current if anterior else SensorValues())

    def _publicar(self, statuses: list[DispositivoStatus], agora: datetime) -> None:
        """Entrega o retrato do ciclo ao FleetState, com o estado da EC2."""
        desde = _iso(self._falha_desde) if self._fiware_fora and self._falha_desde else None
        self._fleet.replace(statuses, _iso(agora), fiware_online=not self._fiware_fora,
                            fiware_offline_since=desde)
