# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Leituras de um C-Shield: estado atual (Orion) e histórico (STH-Comet).

Normaliza o que vem do FIWARE: valores viram float (ou None quando ausentes
ou inválidos, como o -999 de sensor sem leitura) e o histórico vira
[{ts, value}]. Um C-Shield recém-cadastrado ainda não tem atributos no
Orion; isso aparece como None, não como erro.
"""
import asyncio
import math

from app.models.schemas import CurrentReading, HistoryPoint, HistoryQuery
from app.services.device_registry import DeviceRegistry
from app.services.fiware_client import FiwareClient
from app.services.fiware_constants import ENTITY_TYPE, HISTORY_MAX_PAGES, HISTORY_PAGE_LIMIT
from app.services.fiware_errors import FiwareNotFound

# O firmware publica -999 quando o sensor está sem leitura (SETPOINT_AUSENTE em config.h)
VALOR_AUSENTE = -999.0


def deslocamentos_paginas(total: int, pagina: int, max_paginas: int) -> list[int]:
    """hOffsets das páginas a buscar para cobrir `total` pontos.

    Até `max_paginas` páginas cobrem tudo (0, pagina, 2*pagina...); acima disso,
    `max_paginas` páginas espaçadas igualmente do início ao fim da janela.
    """
    if total <= 0:
        return []
    if total <= pagina * max_paginas:
        return list(range(0, total, pagina))
    passo = (total - pagina) / (max_paginas - 1)
    return [round(i * passo) for i in range(max_paginas)]


def to_float(valor) -> float | None:
    """Converte um valor do FIWARE para float; None se ausente ou inválido.

    O Orion pode mandar número, texto ("14.2"), vazio (" ") ou outro tipo;
    booleanos, NaN, infinito e o -999 do firmware também viram None.
    """
    if valor is None or isinstance(valor, bool):
        return None
    try:
        numero = float(valor)
    except (TypeError, ValueError):
        return None
    if math.isnan(numero) or math.isinf(numero) or numero == VALOR_AUSENTE:
        return None
    return numero


def _texto(valor) -> str | None:
    """Texto não vazio ou None (o Orion devolve "-" quando não há motivo, que fica como está)."""
    return valor.strip() if isinstance(valor, str) and valor.strip() else None


def _inteiro(valor) -> int | None:
    """Inteiro a partir de um valor do FIWARE; None se ausente ou inválido."""
    numero = to_float(valor)
    return None if numero is None else int(numero)


def reading_from_entity(device_id: str, entidade: dict) -> CurrentReading:
    """Converte uma entidade do Orion (keyValues) no estado atual do C-Shield.

    Usada na consulta de um C-Shield e pelo poller da frota. Entidade vazia
    ({}) vira todos os valores None.
    """
    degelo = _inteiro(entidade.get("defrost"))
    return CurrentReading(
        device_id=device_id,
        temperature=to_float(entidade.get("temperature")),
        humidity=to_float(entidade.get("humidity")),
        luminosity=to_float(entidade.get("luminosity")),
        setpoint=to_float(entidade.get("setpoint")),
        defrost=None if degelo is None else degelo == 1,
        state=_texto(entidade.get("state")),
        reason=_texto(entidade.get("reason")),
        blocks=_inteiro(entidade.get("blocks")),
        exposure=to_float(entidade.get("exposure")),
        time_instant=_texto(entidade.get("TimeInstant")),
    )


def normalizar_pontos(brutos: list[dict]) -> list[HistoryPoint]:
    """Pontos do STH para HistoryPoint, descartando valor ou instante inválido."""
    pontos = []
    for ponto in brutos:
        valor = to_float(ponto.get("attrValue"))
        instante = ponto.get("recvTime")
        if valor is not None and isinstance(instante, str):
            pontos.append(HistoryPoint(ts=instante, value=valor))
    return pontos


class ReadingsService:
    """Consulta de estado atual e histórico por C-Shield."""

    def __init__(self, registry: DeviceRegistry, fiware: FiwareClient):
        self._registry = registry
        self._fiware = fiware

    async def current(self, device_id: str) -> CurrentReading:
        """Estado atual no Orion; sem entidade ou sem leitura, valores None."""
        device = self._registry.get(device_id)
        try:
            entidade = await self._fiware.get_entity(device.entity_id)
        except FiwareNotFound:
            entidade = {}
        return reading_from_entity(device.device_id, entidade)

    async def history(self, device_id: str, query: HistoryQuery) -> list[HistoryPoint]:
        """Histórico de um atributo, normalizado e sem pontos inválidos.

        Por datas, o STH limita cada página a 100 pontos: conta o total da
        janela e busca as páginas em paralelo (amostrando se passar do teto).
        """
        device = self._registry.get(device_id)
        if query.last_n is not None:
            brutos = await self._fiware.query_history(
                ENTITY_TYPE, device.entity_id, query.attr, last_n=query.last_n)
            return normalizar_pontos(brutos)
        janela = {"date_from": query.date_from, "date_to": query.date_to}
        total = await self._fiware.count_history(
            ENTITY_TYPE, device.entity_id, query.attr, **janela)
        offsets = deslocamentos_paginas(total, HISTORY_PAGE_LIMIT, HISTORY_MAX_PAGES)
        paginas = await asyncio.gather(*[
            self._fiware.query_history(ENTITY_TYPE, device.entity_id, query.attr,
                                       h_offset=offset, **janela)
            for offset in offsets])
        return normalizar_pontos([ponto for pagina in paginas for ponto in pagina])
