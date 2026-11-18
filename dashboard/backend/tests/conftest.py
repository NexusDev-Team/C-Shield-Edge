# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Fixtures compartilhadas pelos testes do backend."""
import pytest

from app.core.config import ConfigStore, Settings
from app.core.db import connect
from app.models.schemas import Device
from app.services.device_registry import DeviceRegistry
from app.services.eventos import EventosService
from app.services.fiware_client import FiwareClient
from app.services.fleet_state import FleetState
from app.services.poller import Poller
from app.services.readings import ReadingsService
from tests.fakes import FiwareFalso, Relogio


@pytest.fixture
def anyio_backend():
    """Roda os testes assíncronos (marcados com anyio) no asyncio."""
    return "asyncio"


@pytest.fixture
def settings(tmp_path):
    """Settings de teste: IP fictício e banco num diretório temporário."""
    return Settings(fiware_host="10.0.0.1", database_path=str(tmp_path / "teste.db"))


@pytest.fixture
def conn(settings):
    """Conexão SQLite já com o schema criado."""
    conexao = connect(settings.database_path)
    yield conexao
    conexao.close()


@pytest.fixture
def store(conn, settings):
    """ConfigStore sobre o banco de teste."""
    return ConfigStore(conn, settings)


@pytest.fixture
def fiware(store, settings):
    """FiwareClient apontando para o IP fictício 10.0.0.1 (respx intercepta)."""
    return FiwareClient(store, settings)


@pytest.fixture
def registry(conn, fiware):
    """DeviceRegistry com FIWARE interceptado pelo respx e banco de teste."""
    return DeviceRegistry(conn, fiware)


@pytest.fixture
def cadastrar(conn):
    """Grava C-Shields direto no SQLite, sem FIWARE (para testes de leitura e poller)."""
    def _cadastrar(numero="001", name="UBS Vila Mariana", local_id="ubs-vila-mariana",
                   profile="VACINA", created_at="2026-11-18T11:00:00+00:00"):
        device = Device(device_id=f"cshield{numero}",
                        entity_id=f"urn:ngsi-ld:CShield:{numero}",
                        name=name, local_id=local_id, profile=profile, created_at=created_at)
        with conn:
            conn.execute(
                "INSERT INTO devices (device_id, entity_id, name, local_id, profile, created_at) "
                "VALUES (?, ?, ?, ?, ?, ?)",
                (device.device_id, device.entity_id, device.name, device.local_id,
                 device.profile, device.created_at),
            )
        return device
    return _cadastrar


@pytest.fixture
def readings(registry, fiware):
    """ReadingsService com FIWARE interceptado pelo respx."""
    return ReadingsService(registry, fiware)


@pytest.fixture
def eventos(registry, fiware):
    """EventosService com FIWARE interceptado pelo respx."""
    return EventosService(registry, fiware)


@pytest.fixture
def fleet():
    """Retrato da frota vazio."""
    return FleetState()


@pytest.fixture
def fiware_falso():
    """FIWARE falso para o poller (sem respx)."""
    return FiwareFalso()


@pytest.fixture
def relogio():
    """Relógio parado em 2026-11-18 12:00:00 UTC, avançado pelo teste."""
    return Relogio()


@pytest.fixture
def poller(registry, fiware_falso, store, fleet, relogio):
    """Poller com FIWARE falso, banco de teste e relógio controlado."""
    return Poller(registry, fiware_falso, store, fleet, clock=relogio)
