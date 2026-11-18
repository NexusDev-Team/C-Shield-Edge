# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Rotas de dados de um C-Shield: estado atual, histórico térmico, histórico de ataques
(bloqueios) e de cartões (quem liberou ou tentou liberar o bloqueio). Todas só leem.
"""
from typing import Annotated

from fastapi import APIRouter, Depends, Query

from app.api.deps import get_eventos, get_readings
from app.models.schemas import AtaqueRegistrado, CurrentReading, EventoCartao, HistoryPoint, HistoryQuery
from app.services.eventos import EventosService
from app.services.readings import ReadingsService

router = APIRouter(prefix="/api/devices", tags=["dados"])


@router.get("/{device_id}/current", response_model=CurrentReading)
async def read_current(device_id: str,
                       readings: ReadingsService = Depends(get_readings)) -> CurrentReading:
    """Valores atuais; null quando o C-Shield ainda não mandou leitura."""
    return await readings.current(device_id)


@router.get("/{device_id}/history", response_model=list[HistoryPoint])
async def read_history(device_id: str, query: Annotated[HistoryQuery, Query()],
                       readings: ReadingsService = Depends(get_readings)) -> list[HistoryPoint]:
    """Histórico de um atributo: last_n (1–100) ou date_from/date_to."""
    return await readings.history(device_id, query)


@router.get("/{device_id}/ataques", response_model=list[AtaqueRegistrado])
async def read_ataques(device_id: str, last_n: Annotated[int, Query(ge=1, le=100)] = 20,
                       eventos: EventosService = Depends(get_eventos)) -> list[AtaqueRegistrado]:
    """Ataques barrados (bloqueios novos), do mais novo para o mais antigo."""
    return await eventos.ataques(device_id, last_n)


@router.get("/{device_id}/eventos", response_model=list[EventoCartao])
async def read_eventos(device_id: str, last_n: Annotated[int, Query(ge=1, le=100)] = 20,
                       eventos: EventosService = Depends(get_eventos)) -> list[EventoCartao]:
    """Liberações e recusas de cartão NFC, da mais nova para a mais antiga."""
    return await eventos.listar(device_id, last_n)
