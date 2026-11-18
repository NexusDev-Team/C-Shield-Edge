# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Testes do poller da frota: online, offline e EC2 fora do ar."""
import pytest

pytestmark = pytest.mark.anyio

E1 = "urn:ngsi-ld:CShield:001"


def _entidade(relogio, atras=2, **extras):
    dados = {"id": E1, "type": "CShield", "TimeInstant": relogio.iso(atras), "temperature": 4.9,
             "humidity": 80, "luminosity": 28, "setpoint": 5, "state": "NORMAL", "reason": "-",
             "blocks": 0}
    dados.update(extras)
    return dados


async def test_c_shield_com_leitura_recente_fica_online(poller, cadastrar, fiware_falso, relogio, fleet):
    cadastrar()
    fiware_falso.entidades = [_entidade(relogio, atras=2, state="BLOQUEIO",
                                        reason="SETPOINT_INVALIDO", blocks=1)]
    await poller.tick()
    status = fleet.get("cshield001")
    assert status.status == "online" and status.offline_reason is None
    assert status.state == "BLOQUEIO" and status.reason == "SETPOINT_INVALIDO"
    assert status.current.temperature == 4.9 and status.blocks == 1
    assert status.seconds_since == 2
    assert fleet.fiware_online


async def test_leitura_antiga_fica_offline_sem_estado(poller, cadastrar, fiware_falso, relogio, fleet):
    cadastrar()
    fiware_falso.entidades = [_entidade(relogio, atras=120)]
    await poller.tick()
    status = fleet.get("cshield001")
    assert status.status == "offline" and status.offline_reason == "sem_leitura"
    assert status.state is None            # estado velho não é mostrado como atual


async def test_recem_cadastrado_sem_leitura_tem_carencia(poller, cadastrar, fiware_falso, relogio, fleet):
    cadastrar(created_at=relogio.iso(10).replace(".000Z", "+00:00"))
    await poller.tick()
    assert fleet.get("cshield001").status == "online"     # ainda dentro de offline_seconds
    relogio.avancar(60)
    await poller.tick()
    assert fleet.get("cshield001").status == "offline"


async def test_ec2_fora_so_derruba_a_frota_depois_do_limite(poller, cadastrar, fiware_falso, relogio, fleet):
    cadastrar()
    fiware_falso.entidades = [_entidade(relogio, atras=1)]
    await poller.tick()
    assert fleet.get("cshield001").status == "online"
    fiware_falso.fora = True
    relogio.avancar(10)
    await poller.tick()                                    # falha curta: nada muda
    assert fleet.fiware_online and fleet.get("cshield001").status == "online"
    relogio.avancar(60)
    await poller.tick()                                    # passou de offline_seconds
    status = fleet.get("cshield001")
    assert not fleet.fiware_online and fleet.fiware_offline_since is not None
    assert status.status == "offline" and status.offline_reason == "fiware"
    assert status.current.temperature == 4.9               # mantém os últimos valores conhecidos


async def test_ec2_volta_e_a_frota_se_recupera(poller, cadastrar, fiware_falso, relogio, fleet):
    cadastrar()
    fiware_falso.fora = True
    await poller.tick()
    relogio.avancar(60)
    await poller.tick()
    assert not fleet.fiware_online
    fiware_falso.fora = False
    fiware_falso.entidades = [_entidade(relogio, atras=1)]
    await poller.tick()
    assert fleet.fiware_online and fleet.get("cshield001").status == "online"


async def test_sem_c_shields_nao_consulta_o_orion(poller, fiware_falso, fleet):
    await poller.tick()
    assert fiware_falso.chamadas == 0 and fleet.snapshot() == []
