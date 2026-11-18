# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Modelos de dados (Pydantic) compartilhados pelo backend.

Aqui ficam os contratos de entrada e saída da API e dos services: configuração
e saúde do FIWARE, cadastro dos C-Shields, leituras, histórico e eventos de
cartão (quem liberou o bloqueio). O dashboard só lê; nenhum modelo descreve
comando para o dispositivo.
"""
from datetime import datetime
from typing import Literal

from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator

from app.services.fiware_constants import HISTORY_PAGE_LIMIT

_PORTA = {"ge": 1, "le": 65535}
_POLL = {"ge": 1, "le": 300}
_OFFLINE = {"ge": 5, "le": 3600}

# Atributos numéricos aceitos no histórico (nomes longos do Orion/STH)
HistoryAttr = Literal["temperature", "humidity", "luminosity", "exposure"]
# Pontos do mapa da cidade (os mesmos do emulador do site) e perfis de produto do firmware
LocalId = Literal["ubs-vila-mariana", "ubs-brasilandia", "hemocentro-consolacao",
                  "farmacia-mooca", "mercado-liberdade"]
Perfil = Literal["VACINA", "CONGELADO", "RESFRIADO", "PLASMA"]


def normalize_host(value: str) -> str:
    """Limpa um endereço digitado pelo usuário e devolve só o host.

    Tira espaços, o esquema (http:// ou https://), qualquer caminho e a porta.
    Ex.: " http://3.3.3.3:1026/ " -> "3.3.3.3". IPv6 não é suportado.
    """
    host = value.strip()
    for prefixo in ("http://", "https://"):
        if host.lower().startswith(prefixo):
            host = host[len(prefixo):]
    host = host.split("/")[0]
    host = host.split(":")[0]
    return host.strip()


class RuntimeConfig(BaseModel):
    """Configuração em uso agora (o .env com o que o usuário mudou por cima)."""
    ec2_ip: str
    orion_port: int = Field(**_PORTA)
    iota_port: int = Field(**_PORTA)
    sth_port: int = Field(**_PORTA)
    poll_seconds: int = Field(**_POLL)
    offline_seconds: int = Field(**_OFFLINE)


class ConfigUpdate(BaseModel):
    """Alteração parcial da configuração: só os campos enviados mudam."""
    ec2_ip: str | None = None
    orion_port: int | None = Field(default=None, **_PORTA)
    iota_port: int | None = Field(default=None, **_PORTA)
    sth_port: int | None = Field(default=None, **_PORTA)
    poll_seconds: int | None = Field(default=None, **_POLL)
    offline_seconds: int | None = Field(default=None, **_OFFLINE)

    @field_validator("ec2_ip")
    @classmethod
    def _limpa_ip(cls, valor: str | None) -> str | None:
        """Normaliza o IP e recusa endereço vazio."""
        if valor is None:
            return None
        host = normalize_host(valor)
        if not host:
            raise ValueError("ec2_ip não pode ser vazio")
        return host


class Device(BaseModel):
    """Um C-Shield cadastrado: device no IoT Agent, entidade no Orion e ponto no mapa."""
    device_id: str = Field(pattern=r"^cshield\d{3}$")
    entity_id: str
    name: str
    local_id: LocalId
    profile: Perfil
    created_at: str | None = None


class DeviceCreate(BaseModel):
    """Entrada do cadastro de um C-Shield (o id precisa bater com o ID_DEVICE do firmware)."""
    model_config = ConfigDict(str_strip_whitespace=True)

    device_id: str = Field(pattern=r"^cshield\d{3}$")
    name: str = Field(min_length=1, max_length=60)
    local_id: LocalId
    profile: Perfil


class ServiceHealth(BaseModel):
    """Resultado do teste de um componente do FIWARE."""
    ok: bool
    status_code: int | None = None
    latency_ms: int | None = None
    error: str | None = None


class HealthReport(BaseModel):
    """Saúde dos três componentes do FIWARE no IP configurado."""
    ok: bool
    ec2_ip: str
    orion: ServiceHealth
    iota: ServiceHealth
    sth: ServiceHealth


class CurrentReading(BaseModel):
    """Estado atual de um C-Shield; None quando ainda não há valor válido."""
    device_id: str
    temperature: float | None = None
    humidity: float | None = None
    luminosity: float | None = None
    setpoint: float | None = None
    defrost: bool | None = None
    state: str | None = None
    reason: str | None = None
    blocks: int | None = None
    exposure: float | None = None
    time_instant: str | None = None


class HistoryPoint(BaseModel):
    """Um ponto do histórico, já normalizado a partir do STH-Comet."""
    ts: str
    value: float


class HistoryQuery(BaseModel):
    """Janela de consulta do histórico: OU last_n OU intervalo de datas (ISO 8601)."""
    attr: HistoryAttr
    last_n: int | None = Field(default=None, ge=1, le=HISTORY_PAGE_LIMIT)
    date_from: str | None = None
    date_to: str | None = None

    @field_validator("date_from", "date_to")
    @classmethod
    def _data_iso(cls, valor: str | None) -> str | None:
        """Aceita só datas ISO 8601 (ex.: 2026-11-18T00:00:00)."""
        if valor is None:
            return None
        try:
            datetime.fromisoformat(valor)
        except ValueError as exc:
            raise ValueError("data deve estar em ISO 8601, ex.: 2026-11-18T00:00:00") from exc
        return valor

    @model_validator(mode="after")
    def _um_modo(self) -> "HistoryQuery":
        """Exige exatamente um modo de janela."""
        por_datas = self.date_from is not None or self.date_to is not None
        if self.last_n is None and not por_datas:
            raise ValueError("informe last_n ou date_from/date_to")
        if self.last_n is not None and por_datas:
            raise ValueError("use last_n ou date_from/date_to, não os dois")
        return self


class AtaqueRegistrado(BaseModel):
    """Um ataque (bloqueio novo) que o C-Shield barrou, do histórico do STH-Comet."""
    quando: str                          # instante do bloqueio (recvTime do STH)
    motivo: str                          # SETPOINT_INVALIDO, AQUECIMENTO_RAPIDO, RESFRIAMENTO_RAPIDO...
    direcao: Literal["calor", "frio"] | None = None   # o ataque esquentava ou esfriava
    setpoint: float | None = None        # setpoint que o controlador mandou nesse instante
    total: int | None = None             # contador de bloqueios do dispositivo (bq)


class EventoCartao(BaseModel):
    """Uma liberação ou uma recusa de cartão NFC, do histórico do STH-Comet."""
    tipo: Literal["liberacao", "recusa"]
    quem: str                      # código do papel (RT01...) ou UID sem ":" se desconhecido
    papel: str | None = None       # nome do papel; None quando o cartão não é cadastrado
    motivo: str | None = None      # só nas recusas: TRAVA, ORDEM, TEMPERATURA, NAO_CADASTRADO
    quando: str                    # instante do evento (recvTime do STH)
    total: int | None = None       # contador do dispositivo no momento do evento
