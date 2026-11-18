# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Dependências do FastAPI.

As rotas recebem os services por injeção, lidos de app.state, em vez de
importar instâncias globais. Assim os testes trocam qualquer peça sem
mexer nas rotas.
"""
from fastapi import Request

from app.core.config import ConfigStore
from app.services.device_registry import DeviceRegistry
from app.services.eventos import EventosService
from app.services.fiware_client import FiwareClient
from app.services.fleet_state import FleetState
from app.services.readings import ReadingsService


def get_config_store(request: Request) -> ConfigStore:
    """ConfigStore da aplicação."""
    return request.app.state.config_store


def get_fiware(request: Request) -> FiwareClient:
    """Cliente FIWARE da aplicação."""
    return request.app.state.fiware


def get_registry(request: Request) -> DeviceRegistry:
    """Cadastro de C-Shields da aplicação."""
    return request.app.state.registry


def get_readings(request: Request) -> ReadingsService:
    """Serviço de leituras da aplicação."""
    return request.app.state.readings


def get_eventos(request: Request) -> EventosService:
    """Serviço do histórico de cartões da aplicação."""
    return request.app.state.eventos


def get_fleet(request: Request) -> FleetState:
    """Retrato da frota da aplicação."""
    return request.app.state.fleet
