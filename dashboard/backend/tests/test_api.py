# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Testes das rotas HTTP (create_app inteiro, com o FIWARE interceptado pelo respx)."""
import httpx
import pytest
import respx
from fastapi.testclient import TestClient

from app.main import create_app

E1 = "urn:ngsi-ld:CShield:001"


@pytest.fixture
def client(settings):
    with TestClient(create_app(settings)) as c:
        yield c


def test_rotas_expostas_so_leem(client):
    """Nenhuma rota aceita comando para o C-Shield: só GET e o cadastro/remoção do device."""
    rotas = {(m, r.path) for r in client.app.routes if hasattr(r, "methods") for m in r.methods}
    assert not any("command" in caminho or "cmd" in caminho for _, caminho in rotas)
    assert ("PATCH", "/api/devices/{device_id}") not in rotas
    assert not any(m in ("PUT", "PATCH") and "devices" in caminho for m, caminho in rotas)


def test_config_le_o_ip_do_env(client):
    corpo = client.get("/api/config").json()
    assert corpo["ec2_ip"] == "10.0.0.1" and corpo["orion_port"] == 1026


def test_config_troca_o_ip_em_runtime(client):
    resposta = client.put("/api/config", json={"ec2_ip": " http://9.9.9.9:1026/ "})
    assert resposta.json()["ec2_ip"] == "9.9.9.9"


def test_fleet_vazia(client):
    corpo = client.get("/api/fleet").json()
    assert corpo["dispositivos"] == [] and corpo["fiware_online"] is True


def test_device_inexistente_da_404_com_service(client):
    resposta = client.get("/api/devices/cshield009")
    assert resposta.status_code == 404 and resposta.json()["service"] == "registry"


def test_cadastro_com_id_invalido_da_422(client):
    resposta = client.post("/api/devices", json={"device_id": "vinheria001", "name": "x",
                                                 "local_id": "farmacia-mooca", "profile": "VACINA"})
    assert resposta.status_code == 422


def test_history_exige_janela(client):
    assert client.get("/api/devices/cshield001/history?attr=temperature").status_code == 422


@respx.mock
def test_health_sem_ec2_ligada_nao_quebra(client):
    for porta, caminho in ((1026, "/version"), (4041, "/iot/about"), (8666, "/version")):
        respx.get(f"http://10.0.0.1:{porta}{caminho}").mock(side_effect=httpx.ConnectError("x"))
    corpo = client.get("/api/config/health").json()
    assert corpo["ok"] is False and corpo["orion"]["ok"] is False
