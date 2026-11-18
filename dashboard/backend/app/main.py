# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Ponto de entrada do backend FastAPI do dashboard do C-Shield.

create_app() monta a aplicação: lê a configuração, abre o SQLite, cria o
cliente FIWARE e os services (cadastro, leituras, eventos de cartão e
frota) e registra CORS, tradução de erros e rotas. O lifespan sobe o poller
em fundo quando Settings.run_background está ligado. Os testes chamam
create_app() com Settings próprios; o uvicorn usa o objeto `app` do fim do
arquivo (uvicorn app.main:app --reload, rodando de dentro de dashboard/backend/).

O backend só LÊ o FIWARE (e cadastra o device): não há rota que mande ordem
ao C-Shield.
"""
import asyncio
import logging
from contextlib import asynccontextmanager, suppress

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app.api import routes_config, routes_data, routes_devices, routes_fleet
from app.api.errors import register_error_handlers
from app.core.config import ConfigStore, Settings, load_settings
from app.core.db import connect
from app.services.device_registry import DeviceRegistry
from app.services.eventos import EventosService
from app.services.fiware_client import FiwareClient
from app.services.fleet_state import FleetState
from app.services.poller import Poller
from app.services.readings import ReadingsService

log = logging.getLogger(__name__)


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Ciclo de vida do app.

    Na partida, com run_background ligado, sobe o poller. No desligamento,
    cancela a tarefa e fecha o pool HTTP e o SQLite.
    """
    tarefas = []
    if app.state.settings.run_background:
        tarefas.append(asyncio.create_task(app.state.poller.run()))
    yield
    for tarefa in tarefas:
        tarefa.cancel()
    for tarefa in tarefas:
        with suppress(asyncio.CancelledError):
            await tarefa
    await app.state.fiware.aclose()
    app.state.conn.close()


def create_app(settings: Settings | None = None) -> FastAPI:
    """Cria a aplicação com todas as dependências ligadas."""
    settings = settings or load_settings()
    conn = connect(settings.database_path)
    store = ConfigStore(conn, settings)

    app = FastAPI(
        title="C-Shield — API do Dashboard",
        description="Backend de leitura do dashboard do C-Shield, integrado ao FIWARE.",
        version="0.1.0",
        lifespan=lifespan,
    )
    app.state.settings = settings
    app.state.conn = conn
    app.state.config_store = store
    fiware = FiwareClient(store, settings)
    registry = DeviceRegistry(conn, fiware)
    fleet = FleetState()
    app.state.fiware = fiware
    app.state.registry = registry
    app.state.readings = ReadingsService(registry, fiware)
    app.state.eventos = EventosService(registry, fiware)
    app.state.fleet = fleet
    app.state.poller = Poller(registry, fiware, store, fleet)

    app.add_middleware(
        CORSMiddleware,
        allow_origins=list(settings.cors_origins),
        allow_methods=["*"],
        allow_headers=["*"],
    )
    register_error_handlers(app)
    app.include_router(routes_config.router)
    app.include_router(routes_devices.router)
    app.include_router(routes_data.router)
    app.include_router(routes_fleet.router)
    return app


def __getattr__(nome: str):
    """Cria o `app` global só no primeiro acesso (PEP 562).

    O uvicorn pede `app.main:app` e dispara a criação; importar o módulo nos
    testes só para usar `create_app` não abre banco nem lê o .env.
    """
    if nome == "app":
        globals()["app"] = create_app()
        return globals()["app"]
    raise AttributeError(f"module {__name__!r} has no attribute {nome!r}")
