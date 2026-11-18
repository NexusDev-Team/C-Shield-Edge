# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Rotas do cadastro de C-Shields: cadastrar (com provisionamento no FIWARE),
listar, consultar e remover. Nenhuma rota manda ordem ao dispositivo.
"""
from fastapi import APIRouter, Depends, Response

from app.api.deps import get_registry
from app.models.schemas import Device, DeviceCreate
from app.services.device_registry import DeviceRegistry

router = APIRouter(prefix="/api/devices", tags=["c-shields"])


@router.post("", response_model=Device, status_code=201)
async def create_device(data: DeviceCreate,
                        registry: DeviceRegistry = Depends(get_registry)) -> Device:
    """Cadastra o C-Shield e o provisiona no FIWARE (re-provisiona se já existir lá)."""
    return await registry.create(data)


@router.get("", response_model=list[Device])
def list_devices(registry: DeviceRegistry = Depends(get_registry)) -> list[Device]:
    """C-Shields cadastrados, ordenados pelo id."""
    return registry.list()


@router.get("/{device_id}", response_model=Device)
def read_device(device_id: str, registry: DeviceRegistry = Depends(get_registry)) -> Device:
    """Um C-Shield cadastrado."""
    return registry.get(device_id)


@router.delete("/{device_id}", status_code=204)
async def delete_device(device_id: str,
                        registry: DeviceRegistry = Depends(get_registry)) -> Response:
    """Remove o C-Shield do FIWARE e do cadastro local (o histórico do STH fica)."""
    await registry.delete(device_id)
    return Response(status_code=204)
