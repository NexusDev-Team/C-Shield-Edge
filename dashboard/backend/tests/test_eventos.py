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

from app.services.eventos import direcao_do_ataque, juntar_ataques, juntar_eventos
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


def test_direcao_do_ataque_segue_a_regra_do_firmware():
    assert direcao_do_ataque("RESFRIAMENTO_RAPIDO", None, "VACINA") == "frio"
    assert direcao_do_ataque("AQUECIMENTO_RAPIDO", None, "VACINA") == "calor"
    assert direcao_do_ataque("DEGELO_FORA_HORARIO", None, "VACINA") == "calor"
    assert direcao_do_ataque("SETPOINT_INVALIDO", 45.0, "VACINA") == "calor"    # acima de 8
    assert direcao_do_ataque("SETPOINT_INVALIDO", -5.0, "VACINA") == "frio"     # abaixo de 2
    assert direcao_do_ataque("SETPOINT_INVALIDO", -10.0, "CONGELADO") == "calor"  # acima de -18
    assert direcao_do_ataque("SETPOINT_INVALIDO", None, "VACINA") is None


def test_juntar_ataques_descarta_partida_e_duplicata_do_contador():
    ataques = juntar_ataques(
        contador=[_ponto("2026-11-18T12:00:00.000Z", 0),      # contador da partida, sem ataque
                  _ponto("2026-11-18T12:10:00.000Z", 1),
                  _ponto("2026-11-18T12:10:00.300Z", 1),      # subscription antiga repetindo
                  _ponto("2026-11-18T12:20:00.000Z", 2)],
        motivo=[_ponto("2026-11-18T12:00:00.000Z", "-"),
                _ponto("2026-11-18T12:10:00.000Z", "SETPOINT_INVALIDO"),
                _ponto("2026-11-18T12:20:00.000Z", "RESFRIAMENTO_RAPIDO")],
        setpoint=[_ponto("2026-11-18T12:10:00.000Z", 45)],
        perfil="VACINA")
    assert [(a.total, a.motivo, a.direcao, a.setpoint) for a in ataques] == [
        (1, "SETPOINT_INVALIDO", "calor", 45.0), (2, "RESFRIAMENTO_RAPIDO", "frio", None)]


@respx.mock
async def test_ataques_do_mais_novo_para_o_mais_antigo(eventos, cadastrar):
    cadastrar()
    respx.get(BASE_STH + "blocks").mock(return_value=_sth("blocks", [
        _ponto("2026-11-18T12:10:00.000Z", 1), _ponto("2026-11-18T12:30:00.000Z", 2)]))
    respx.get(BASE_STH + "reason").mock(return_value=_sth("reason", [
        _ponto("2026-11-18T12:10:00.000Z", "SETPOINT_INVALIDO"),
        _ponto("2026-11-18T12:30:00.000Z", "AQUECIMENTO_RAPIDO")]))
    respx.get(BASE_STH + "setpoint").mock(return_value=_sth("setpoint", [
        _ponto("2026-11-18T12:10:00.000Z", -8)]))
    lista = await eventos.ataques("cshield001")
    assert [(a.total, a.direcao) for a in lista] == [(2, "calor"), (1, "frio")]
    assert lista[1].setpoint == -8.0


@respx.mock
async def test_ataques_sem_historico_no_sth(eventos, cadastrar):
    cadastrar()
    respx.get(url__startswith=BASE_STH).mock(return_value=httpx.Response(404, json={}))
    assert await eventos.ataques("cshield001") == []
