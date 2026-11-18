# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Testes do histórico de cartões: quem liberou ou tentou liberar o bloqueio."""
import httpx
import pytest
import respx

from app.services.eventos import juntar_eventos
from app.services.operadores import papel_de

pytestmark = pytest.mark.anyio

E1 = "urn:ngsi-ld:CShield:001"
BASE_STH = f"http://10.0.0.1:8666/STH/v1/contextEntities/type/CShield/id/{E1}/attributes/"


def _ponto(instante, valor):
    return {"recvTime": instante, "attrValue": valor}


def _sth(atributo, valores):
    return httpx.Response(200, json={"contextResponses": [{"contextElement": {
        "attributes": [{"name": atributo, "values": valores}]}}]})


def test_papel_de():
    assert papel_de("RT01") == "Responsável técnico"
    assert papel_de("AABBCCDD") is None       # UID de cartão não cadastrado


def test_juntar_liberacoes_pelo_mesmo_instante():
    eventos = juntar_eventos(
        "liberacao",
        contador=[_ponto("2026-11-18T12:00:10.000Z", 1), _ponto("2026-11-18T12:05:00.000Z", 2)],
        quem=[_ponto("2026-11-18T12:00:10.000Z", "RT01"), _ponto("2026-11-18T12:05:00.000Z", "RT01")])
    assert [(e.quem, e.papel, e.total) for e in eventos] == [
        ("RT01", "Responsável técnico", 1), ("RT01", "Responsável técnico", 2)]


def test_mesmo_papel_duas_vezes_gera_dois_eventos():
    # O valor de releasedBy repetido não conta como mudança; o contador garante as duas linhas
    eventos = juntar_eventos(
        "liberacao",
        contador=[_ponto("2026-11-18T12:00:10.000Z", 1), _ponto("2026-11-18T12:30:00.000Z", 2)],
        quem=[_ponto("2026-11-18T12:00:10.000Z", "ENF01"), _ponto("2026-11-18T12:30:00.000Z", "ENF01")])
    assert len(eventos) == 2


def test_tolera_diferenca_de_milissegundos_e_descarta_evento_sem_quem():
    eventos = juntar_eventos(
        "liberacao",
        contador=[_ponto("2026-11-18T12:00:10.100Z", 1), _ponto("2026-11-18T13:00:00.000Z", 2)],
        quem=[_ponto("2026-11-18T12:00:10.000Z", "MAN01")])   # o evento 2 não tem quem
    assert [(e.quem, e.total) for e in eventos] == [("MAN01", 1)]


def test_juntar_recusas_com_motivo_e_cartao_desconhecido():
    eventos = juntar_eventos(
        "recusa",
        contador=[_ponto("2026-11-18T12:01:00.000Z", 1), _ponto("2026-11-18T12:02:00.000Z", 2)],
        quem=[_ponto("2026-11-18T12:01:00.000Z", "RT01"), _ponto("2026-11-18T12:02:00.000Z", "AABBCCDD")],
        motivo=[_ponto("2026-11-18T12:01:00.000Z", "TRAVA"),
                _ponto("2026-11-18T12:02:00.000Z", "NAO_CADASTRADO")])
    assert [(e.quem, e.papel, e.motivo) for e in eventos] == [
        ("RT01", "Responsável técnico", "TRAVA"), ("AABBCCDD", None, "NAO_CADASTRADO")]


@respx.mock
async def test_listar_junta_liberacoes_e_recusas_da_mais_nova_para_a_mais_antiga(eventos, cadastrar):
    cadastrar()
    respx.get(BASE_STH + "releaseCount").mock(
        return_value=_sth("releaseCount", [_ponto("2026-11-18T12:10:00.000Z", 1)]))
    respx.get(BASE_STH + "releasedBy").mock(
        return_value=_sth("releasedBy", [_ponto("2026-11-18T12:10:00.000Z", "RT01")]))
    respx.get(BASE_STH + "refusalCount").mock(
        return_value=_sth("refusalCount", [_ponto("2026-11-18T12:05:00.000Z", 1)]))
    respx.get(BASE_STH + "refusedBy").mock(
        return_value=_sth("refusedBy", [_ponto("2026-11-18T12:05:00.000Z", "AABBCCDD")]))
    respx.get(BASE_STH + "refusedReason").mock(
        return_value=_sth("refusedReason", [_ponto("2026-11-18T12:05:00.000Z", "NAO_CADASTRADO")]))
    lista = await eventos.listar("cshield001", last_n=10)
    assert [(e.tipo, e.quem) for e in lista] == [("liberacao", "RT01"), ("recusa", "AABBCCDD")]


@respx.mock
async def test_listar_sem_historico_no_sth_devolve_lista_vazia(eventos, cadastrar):
    cadastrar()
    respx.get(url__startswith=BASE_STH).mock(return_value=httpx.Response(404, json={}))
    assert await eventos.listar("cshield001") == []
