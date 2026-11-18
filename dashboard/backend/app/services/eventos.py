# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Histórico de cartões NFC: quem liberou (ou tentou liberar) o bloqueio e quando.

O firmware publica cada evento uma vez (`releasedBy`/`releaseCount` na liberação,
`refusedBy`/`refusedReason`/`refusalCount` na recusa), e as subscriptions do
Orion levam o evento inteiro ao STH-Comet numa só notificação. Aqui as séries
do STH são lidas e juntadas por instante, formando uma lista de eventos.
"""
import asyncio
from datetime import datetime, timedelta

from app.models.schemas import EventoCartao
from app.services.device_registry import DeviceRegistry
from app.services.fiware_client import FiwareClient
from app.services.fiware_constants import ENTITY_TYPE
from app.services.fiware_errors import FiwareNotFound
from app.services.operadores import papel_de
from app.services.readings import to_float

# Os atributos de um mesmo evento chegam na mesma notificação e têm o mesmo recvTime;
# a tolerância só protege contra diferença de milissegundos entre as linhas do STH.
TOLERANCIA = timedelta(seconds=2)


def _instante(texto: str) -> datetime | None:
    """recvTime do STH (ISO 8601, com Z) como datetime com fuso; None se inválido."""
    try:
        return datetime.fromisoformat(texto.replace("Z", "+00:00"))
    except (ValueError, AttributeError):
        return None


def _mais_proximo(alvo: str, serie: list[dict]) -> dict | None:
    """Ponto da série mais próximo de `alvo` (dentro da tolerância), ou None."""
    inicio = _instante(alvo)
    if inicio is None:
        return None
    melhor, menor = None, TOLERANCIA
    for ponto in serie:
        outro = _instante(ponto.get("recvTime", ""))
        if outro is not None and abs(outro - inicio) <= menor:
            melhor, menor = ponto, abs(outro - inicio)
    return melhor


def juntar_eventos(tipo: str, contador: list[dict], quem: list[dict],
                   motivo: list[dict] | None = None) -> list[EventoCartao]:
    """Monta os eventos de um tipo a partir das séries do STH.

    O contador dá um ponto por evento (ele sempre muda); `quem` e `motivo` são
    procurados no mesmo instante. Evento sem o `quem` correspondente é descartado,
    porque não diz de quem foi.
    """
    eventos = []
    for ponto in contador:
        instante = ponto.get("recvTime")
        achou = _mais_proximo(instante, quem)
        if achou is None or not isinstance(achou.get("attrValue"), str):
            continue
        codigo = achou["attrValue"]
        razao = None
        if motivo is not None:
            ponto_motivo = _mais_proximo(instante, motivo)
            razao = ponto_motivo.get("attrValue") if ponto_motivo else None
        total = to_float(ponto.get("attrValue"))
        eventos.append(EventoCartao(
            tipo=tipo, quem=codigo, papel=papel_de(codigo),
            motivo=razao if isinstance(razao, str) else None, quando=instante,
            total=None if total is None else int(total)))
    return eventos


class EventosService:
    """Lista os eventos de cartão de um C-Shield, do mais novo para o mais antigo."""

    def __init__(self, registry: DeviceRegistry, fiware: FiwareClient):
        self._registry = registry
        self._fiware = fiware

    async def _serie(self, entity_id: str, attr: str, last_n: int) -> list[dict]:
        """Últimos pontos de um atributo; sem histórico ainda no STH, lista vazia."""
        try:
            return await self._fiware.query_history(ENTITY_TYPE, entity_id, attr, last_n=last_n)
        except FiwareNotFound:
            return []

    async def listar(self, device_id: str, last_n: int = 20) -> list[EventoCartao]:
        """Até `last_n` eventos mais recentes (liberações e recusas juntas)."""
        device = self._registry.get(device_id)
        nomes = ("releaseCount", "releasedBy", "refusalCount", "refusedBy", "refusedReason")
        series = dict(zip(nomes, await asyncio.gather(
            *[self._serie(device.entity_id, nome, last_n) for nome in nomes])))
        eventos = juntar_eventos("liberacao", series["releaseCount"], series["releasedBy"])
        eventos += juntar_eventos("recusa", series["refusalCount"], series["refusedBy"],
                                  series["refusedReason"])
        eventos.sort(key=lambda evento: evento.quando, reverse=True)
        return eventos[:last_n]
