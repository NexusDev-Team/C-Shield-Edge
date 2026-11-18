# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Testes do cadastro de C-Shields: provisionamento no FIWARE e SQLite local."""
import json

import httpx
import pytest
import respx

from app.models.schemas import DeviceCreate
from app.services.device_registry import entity_id_for
from app.services.fiware_errors import FiwareUnavailable
from app.services.registry_errors import DeviceAlreadyExists, DeviceNotFound

pytestmark = pytest.mark.anyio

IOTA = "http://10.0.0.1:4041"
ORION = "http://10.0.0.1:1026"
E1 = "urn:ngsi-ld:CShield:001"


def _novo(numero="001", local="ubs-vila-mariana"):
    return DeviceCreate(device_id=f"cshield{numero}", name="UBS Vila Mariana",
                        local_id=local, profile="VACINA")


def _fiware_ok():
    """Intercepta as chamadas do cadastro e devolve as rotas que os testes inspecionam."""
    rotas = {
        "grupo": respx.post(f"{IOTA}/iot/services").mock(return_value=httpx.Response(201)),
        "apagar_device": respx.delete(url__startswith=f"{IOTA}/iot/devices/").mock(
            return_value=httpx.Response(204)),
        "device": respx.post(f"{IOTA}/iot/devices").mock(return_value=httpx.Response(201)),
        "listar_subs": respx.get(f"{ORION}/v2/subscriptions").mock(
            return_value=httpx.Response(200, json=[])),
        "criar_sub": respx.post(f"{ORION}/v2/subscriptions").mock(
            return_value=httpx.Response(201, headers={"Location": "/v2/subscriptions/abc"})),
    }
    return rotas


def test_entity_id_for():
    assert entity_id_for("cshield001") == E1
    assert entity_id_for("cshield012") == "urn:ngsi-ld:CShield:012"


@respx.mock
async def test_create_provisiona_sem_comandos_e_assina_o_historico(registry):
    rotas = _fiware_ok()
    device = await registry.create(_novo())
    assert device.entity_id == E1 and registry.get("cshield001").local_id == "ubs-vila-mariana"

    corpo = json.loads(rotas["device"].calls.last.request.content)["devices"][0]
    assert "commands" not in corpo                          # o C-Shield não recebe ordens
    assert corpo["entity_type"] == "CShield" and corpo["protocol"] == "PDI-IoTA-UltraLight"
    nomes = {a["name"] for a in corpo["attributes"]}
    assert {"temperature", "state", "releasedBy", "releaseCount", "refusedBy",
            "refusedReason", "refusalCount"} <= nomes

    subs = [json.loads(c.request.content) for c in rotas["criar_sub"].calls]
    assert len(subs) == 8                                   # 5 de telemetria + 3 de evento (ataque, liberação, recusa)
    por_descricao = {s["description"]: s for s in subs}
    liberacao = por_descricao["C-Shield STH evento liberacao"]
    assert liberacao["subject"]["condition"]["attrs"] == ["releaseCount"]
    assert liberacao["notification"]["attrs"] == ["releasedBy", "releaseCount"]
    assert liberacao["notification"]["http"]["url"] == "http://sth-comet:8666/notify"
    ataque = por_descricao["C-Shield STH evento ataque"]
    assert ataque["subject"]["condition"]["attrs"] == ["blocks"]
    assert ataque["notification"]["attrs"] == ["blocks", "reason", "setpoint"]


@respx.mock
async def test_create_repetido_e_recusado(registry):
    _fiware_ok()
    await registry.create(_novo())
    with pytest.raises(DeviceAlreadyExists):
        await registry.create(_novo())


@respx.mock
async def test_um_c_shield_por_local_do_mapa(registry):
    _fiware_ok()
    await registry.create(_novo("001"))
    with pytest.raises(DeviceAlreadyExists):
        await registry.create(_novo("002", local="ubs-vila-mariana"))
    await registry.create(_novo("002", local="farmacia-mooca"))
    assert [d.device_id for d in registry.list()] == ["cshield001", "cshield002"]


@respx.mock
async def test_fiware_fora_do_ar_nao_deixa_cadastro_pela_metade(registry):
    respx.post(f"{IOTA}/iot/services").mock(side_effect=httpx.ConnectError("sem rota"))
    with pytest.raises(FiwareUnavailable):
        await registry.create(_novo())
    assert registry.list() == []


@respx.mock
async def test_delete_limpa_fiware_e_sqlite(registry, cadastrar):
    cadastrar()
    respx.get(f"{ORION}/v2/subscriptions").mock(return_value=httpx.Response(200, json=[
        {"id": "s1", "subject": {"entities": [{"id": E1}]}},
        {"id": "s2", "subject": {"entities": [{"id": "outra"}]}}]))
    apaga_sub = respx.delete(f"{ORION}/v2/subscriptions/s1").mock(return_value=httpx.Response(204))
    respx.delete(f"{IOTA}/iot/devices/cshield001").mock(return_value=httpx.Response(204))
    respx.delete(f"{ORION}/v2/entities/{E1}").mock(return_value=httpx.Response(404))
    await registry.delete("cshield001")
    assert apaga_sub.called and registry.list() == []


def test_get_inexistente(registry):
    with pytest.raises(DeviceNotFound):
        registry.get("cshield009")
