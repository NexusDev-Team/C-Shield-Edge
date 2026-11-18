# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Cadastro de C-Shields: SQLite local + provisionamento no FIWARE.

O FIWARE vem primeiro e o SQLite por último. Cada etapa no FIWARE pode ser
repetida sem efeito colateral (409/404 aceitos, subscriptions recriadas),
então, se algo falha no meio, nada fica gravado localmente e basta tentar
de novo. Device que já existe no IoT Agent é removido e provisionado de
novo, para ganhar a lista de atributos atual.
"""
import logging
import sqlite3
from datetime import datetime, timezone

from app.core.db import Database
from app.models.schemas import Device, DeviceCreate
from app.services.fiware_client import FiwareClient
from app.services.fiware_constants import ATRIBUTOS_HISTORICO, EVENTOS_HISTORICO
from app.services.fiware_errors import FiwareConflict
from app.services.registry_errors import DeviceAlreadyExists, DeviceNotFound

ENTITY_PREFIX = "urn:ngsi-ld:CShield:"

log = logging.getLogger(__name__)


def entity_id_for(device_id: str) -> str:
    """Deriva o id da entidade no Orion: cshield001 -> urn:ngsi-ld:CShield:001."""
    return ENTITY_PREFIX + device_id.removeprefix("cshield")


def _linha_para_device(linha: sqlite3.Row) -> Device:
    """Converte uma linha da tabela devices no modelo Device."""
    return Device(device_id=linha["device_id"], entity_id=linha["entity_id"],
                  name=linha["name"], local_id=linha["local_id"], profile=linha["profile"],
                  created_at=linha["created_at"])


class DeviceRegistry:
    """Cadastro, consulta e remoção dos C-Shields."""

    def __init__(self, conn: Database, fiware: FiwareClient):
        self._conn = conn
        self._fiware = fiware

    async def create(self, data: DeviceCreate) -> Device:
        """Provisiona o C-Shield no FIWARE e só então grava o cadastro no SQLite."""
        if self._exists(data.device_id):
            raise DeviceAlreadyExists(f"{data.device_id} já está cadastrado")
        self._conferir_local_livre(data.local_id)
        device = Device(
            device_id=data.device_id,
            entity_id=entity_id_for(data.device_id),
            name=data.name,
            local_id=data.local_id,
            profile=data.profile,
            created_at=datetime.now(timezone.utc).isoformat(timespec="seconds"),
        )
        await self._fiware.provision_service_group()
        # Remove antes de provisionar: re-provisiona quem já existe (adoção)
        await self._fiware.delete_device(device.device_id)
        await self._provisionar(device)
        await self._assinar_historico(device)
        try:
            with self._conn:
                self._conn.execute(
                    "INSERT INTO devices (device_id, entity_id, name, local_id, profile, created_at) "
                    "VALUES (?, ?, ?, ?, ?, ?)",
                    (device.device_id, device.entity_id, device.name, device.local_id,
                     device.profile, device.created_at),
                )
        except sqlite3.IntegrityError as exc:
            # Outro cadastro do mesmo id gravou entre a checagem e o INSERT
            raise DeviceAlreadyExists(f"{data.device_id} já está cadastrado") from exc
        return device

    def list(self) -> list[Device]:
        """Todos os C-Shields cadastrados, ordenados pelo id."""
        linhas = self._conn.execute("SELECT * FROM devices ORDER BY device_id")
        return [_linha_para_device(linha) for linha in linhas]

    def get(self, device_id: str) -> Device:
        """Um C-Shield pelo id; DeviceNotFound se não estiver cadastrado."""
        linha = self._conn.execute(
            "SELECT * FROM devices WHERE device_id = ?", (device_id,)).fetchone()
        if linha is None:
            raise DeviceNotFound(f"{device_id} não está cadastrado")
        return _linha_para_device(linha)

    async def delete(self, device_id: str) -> None:
        """Remove o C-Shield do FIWARE e depois do SQLite.

        Ordem: subscriptions, device no IoT Agent e entidade no Orion (404
        aceito). Se o FIWARE falhar, o cadastro local fica para uma nova
        tentativa. O histórico no STH-Comet permanece.
        """
        device = self.get(device_id)
        await self._fiware.delete_subscriptions(device.entity_id)
        await self._fiware.delete_device(device.device_id)
        await self._fiware.delete_entity(device.entity_id)
        with self._conn:
            self._conn.execute("DELETE FROM devices WHERE device_id = ?", (device_id,))

    async def _assinar_historico(self, device: Device) -> None:
        """Recria as subscriptions do STH-Comet (as mesmas de edge/fiware/provisionar.sh)."""
        await self._fiware.delete_subscriptions(device.entity_id)
        for attr in ATRIBUTOS_HISTORICO:
            await self._fiware.subscribe(device.entity_id, attr, (attr,), f"C-Shield STH {attr}")
        for nome, (condicao, notificados) in EVENTOS_HISTORICO.items():
            await self._fiware.subscribe(device.entity_id, condicao, notificados,
                                         f"C-Shield STH evento {nome}")

    async def _provisionar(self, device: Device) -> None:
        """Provisiona o device; se o IoT Agent responder 409, tenta de novo uma vez.

        O device acabou de ser removido, então um 409 aqui significa que algo
        o recriou no intervalo. Remover de novo e provisionar resolve; um
        segundo 409 propaga. O aviso no log deixa o caso visível.
        """
        try:
            await self._fiware.provision_device(device)
        except FiwareConflict:
            log.warning("IoT Agent já tinha %s logo após a remoção; removendo e "
                        "provisionando de novo", device.device_id)
            await self._fiware.delete_device(device.device_id)
            await self._fiware.provision_device(device)

    def _conferir_local_livre(self, local_id: str) -> None:
        """Cada ponto do mapa recebe um C-Shield só: o mapa mostra um por local."""
        if self._conn.execute(
                "SELECT 1 FROM devices WHERE local_id = ?", (local_id,)).fetchone() is not None:
            raise DeviceAlreadyExists(f"o local {local_id} já tem um C-Shield cadastrado")

    def _exists(self, device_id: str) -> bool:
        """True se o id já está no SQLite."""
        return self._conn.execute(
            "SELECT 1 FROM devices WHERE device_id = ?", (device_id,)).fetchone() is not None
