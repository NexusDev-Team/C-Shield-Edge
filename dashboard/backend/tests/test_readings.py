# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Testes da leitura atual (Orion) e do histórico (STH-Comet) de um C-Shield."""
import httpx
import pytest
import respx

from app.models.schemas import HistoryQuery
from app.services.readings import deslocamentos_paginas, reading_from_entity, to_float

pytestmark = pytest.mark.anyio

E1 = "urn:ngsi-ld:CShield:001"
URL_ENTIDADE = f"http://10.0.0.1:1026/v2/entities/{E1}"
URL_STH = ("http://10.0.0.1:8666/STH/v1/contextEntities/type/CShield"
           f"/id/{E1}/attributes/temperature")


def test_to_float_descarta_ausente_e_invalido():
    assert to_float("4.9") == 4.9
    assert to_float(5) == 5.0
    assert to_float(-999) is None          # sensor sem leitura no firmware
    assert to_float(None) is None
    assert to_float(True) is None
    assert to_float("abc") is None
    assert to_float(float("nan")) is None


def test_reading_from_entity_completa():
    leitura = reading_from_entity("cshield001", {
        "id": E1, "TimeInstant": "2026-11-18T12:00:00.000Z", "temperature": 4.9,
        "humidity": 80, "luminosity": 28, "setpoint": 5, "defrost": 0,
        "state": "BLOQUEIO", "reason": "SETPOINT_INVALIDO", "blocks": 1, "exposure": 0})
    assert leitura.temperature == 4.9
    assert leitura.defrost is False
    assert leitura.state == "BLOQUEIO"
    assert leitura.reason == "SETPOINT_INVALIDO"
    assert leitura.blocks == 1
    assert leitura.time_instant == "2026-11-18T12:00:00.000Z"


def test_reading_from_entity_vazia_e_sensor_sem_leitura():
    vazia = reading_from_entity("cshield001", {})
    assert vazia.temperature is None and vazia.state is None and vazia.blocks is None
    sem_sensor = reading_from_entity("cshield001", {"temperature": -999, "humidity": -999,
                                                    "state": "FALHA_SENSOR"})
    assert sem_sensor.temperature is None and sem_sensor.humidity is None
    assert sem_sensor.state == "FALHA_SENSOR"


def test_deslocamentos_paginas():
    assert deslocamentos_paginas(0, 100, 15) == []
    assert deslocamentos_paginas(250, 100, 15) == [0, 100, 200]
    espacadas = deslocamentos_paginas(10_000, 100, 15)
    assert len(espacadas) == 15 and espacadas[0] == 0 and espacadas[-1] == 9900


@respx.mock
async def test_current_le_o_orion(readings, cadastrar):
    cadastrar()
    rota = respx.get(URL_ENTIDADE).mock(return_value=httpx.Response(
        200, json={"id": E1, "type": "CShield", "temperature": 4.9, "state": "NORMAL"}))
    leitura = await readings.current("cshield001")
    assert leitura.temperature == 4.9 and leitura.state == "NORMAL"
    assert rota.calls.last.request.url.params["options"] == "keyValues"
    assert rota.calls.last.request.headers["fiware-service"] == "smart"


@respx.mock
async def test_current_sem_entidade_no_orion_devolve_vazio(readings, cadastrar):
    cadastrar()
    respx.get(URL_ENTIDADE).mock(return_value=httpx.Response(404, json={"error": "NotFound"}))
    assert (await readings.current("cshield001")).temperature is None


@respx.mock
async def test_history_last_n_normaliza_pontos(readings, cadastrar):
    cadastrar()
    valores = [{"recvTime": "2026-11-18T12:00:00.000Z", "attrValue": 4.9},
               {"recvTime": "2026-11-18T12:00:05.000Z", "attrValue": -999},   # descartado
               {"recvTime": "2026-11-18T12:00:10.000Z", "attrValue": "5.1"}]
    resposta = {"contextResponses": [{"contextElement": {
        "attributes": [{"name": "temperature", "values": valores}]}}]}
    rota = respx.get(URL_STH).mock(return_value=httpx.Response(200, json=resposta))
    pontos = await readings.history("cshield001", HistoryQuery(attr="temperature", last_n=30))
    assert [(p.ts, p.value) for p in pontos] == [("2026-11-18T12:00:00.000Z", 4.9),
                                                 ("2026-11-18T12:00:10.000Z", 5.1)]
    assert rota.calls.last.request.url.params["lastN"] == "30"
