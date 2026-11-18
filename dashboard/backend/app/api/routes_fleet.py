# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Rota da visão geral da frota: estado da EC2 e de todos os C-Shields."""
from fastapi import APIRouter, Depends

from app.api.deps import get_fleet, get_registry
from app.models.schemas_fleet import FleetReport
from app.services.device_registry import DeviceRegistry
from app.services.fleet_state import FleetState

router = APIRouter(prefix="/api/fleet", tags=["frota"])


@router.get("", response_model=FleetReport)
async def read_fleet(fleet: FleetState = Depends(get_fleet),
                     registry: DeviceRegistry = Depends(get_registry)) -> FleetReport:
    """Retrato do último ciclo do poller, só das C-Shields ainda cadastrados.

    Um C-Shield recém-cadastrado aparece no ciclo seguinte (até poll_seconds).
    """
    return fleet.report([device.device_id for device in registry.list()])
