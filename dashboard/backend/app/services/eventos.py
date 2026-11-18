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

from app.models.schemas import AtaqueRegistrado, EventoCartao
from app.services.device_registry import DeviceRegistry
from app.services.fiware_client import FiwareClient
from app.services.fiware_constants import ENTITY_TYPE
from app.services.fiware_errors import FiwareNotFound
from app.services.operadores import papel_de
from app.services.perfis import FAIXAS
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


# Motivos que indicam um ataque barrado (o que abre um bloqueio no firmware).
MOTIVOS_DE_ATAQUE = ("SETPOINT_INVALIDO", "DEGELO_FORA_HORARIO", "DEGELO_LONGO",
                     "AQUECIMENTO_RAPIDO", "RESFRIAMENTO_RAPIDO")


def direcao_do_ataque(motivo: str, setpoint: float | None, perfil: str) -> str | None:
    """"frio" ou "calor", com a mesma regra do firmware (bloqueioFrio em decisao.h).

    Resfriamento rápido e setpoint abaixo do mínimo da faixa são ataques de frio; setpoint
    acima do máximo, aquecimento rápido e degelo indevido são de calor. Setpoint sem valor
    (ou dentro da faixa) não permite dizer.
    """
    if motivo == "RESFRIAMENTO_RAPIDO":
        return "frio"
    if motivo in ("AQUECIMENTO_RAPIDO", "DEGELO_FORA_HORARIO", "DEGELO_LONGO"):
        return "calor"
    if motivo == "SETPOINT_INVALIDO" and setpoint is not None and perfil in FAIXAS:
        minimo, maximo = FAIXAS[perfil]
        if setpoint < minimo:
            return "frio"
        if setpoint > maximo:
            return "calor"
    return None


def juntar_ataques(contador: list[dict], motivo: list[dict], setpoint: list[dict],
                   perfil: str) -> list[AtaqueRegistrado]:
    """Monta os ataques a partir das séries do STH.

    `blocks` só sobe quando um bloqueio começa, então cada ponto é um ataque; motivo e
    setpoint são procurados no mesmo instante. Pontos sem motivo de ataque (o 0 da partida,
    por exemplo) são descartados, e o mesmo contador não repete (uma subscription antiga
    de `blocks` ainda pode existir na EC2).
    """
    vistos, ataques = set(), []
    for ponto in contador:
        instante = ponto.get("recvTime")
        total = to_float(ponto.get("attrValue"))
        razao = _mais_proximo(instante, motivo)
        codigo = razao.get("attrValue") if razao else None
        if codigo not in MOTIVOS_DE_ATAQUE or total in vistos:
            continue
        vistos.add(total)
        ponto_sp = _mais_proximo(instante, setpoint)
        valor_sp = to_float(ponto_sp.get("attrValue")) if ponto_sp else None
        ataques.append(AtaqueRegistrado(
            quando=instante, motivo=codigo, setpoint=valor_sp,
            direcao=direcao_do_ataque(codigo, valor_sp, perfil),
            total=None if total is None else int(total)))
    return ataques


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

    async def ataques(self, device_id: str, last_n: int = 20) -> list[AtaqueRegistrado]:
        """Até `last_n` ataques mais recentes, do mais novo para o mais antigo."""
        device = self._registry.get(device_id)
        nomes = ("blocks", "reason", "setpoint")
        series = dict(zip(nomes, await asyncio.gather(
            *[self._serie(device.entity_id, nome, last_n) for nome in nomes])))
        ataques = juntar_ataques(series["blocks"], series["reason"], series["setpoint"],
                                 device.profile)
        ataques.sort(key=lambda ataque: ataque.quando, reverse=True)
        return ataques[:last_n]

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
