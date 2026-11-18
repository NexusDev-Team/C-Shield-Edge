# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Modelos de dados do retrato da frota (GET /api/fleet).

Ficam separados de schemas.py para nenhum arquivo passar de ~250 linhas.
"""
from typing import Literal

from pydantic import BaseModel

from app.models.schemas import LocalId, Perfil

# online = mandou leitura nos últimos offline_seconds; offline = parou, ou a EC2 caiu
FleetStatus = Literal["online", "offline"]
# Por que está offline: o C-Shield parou de mandar leituras, ou a EC2 caiu
OfflineReason = Literal["sem_leitura", "fiware"]


class SensorValues(BaseModel):
    """Valores atuais; None quando falta a leitura."""
    temperature: float | None = None
    humidity: float | None = None
    luminosity: float | None = None
    setpoint: float | None = None


class DispositivoStatus(BaseModel):
    """Retrato de um C-Shield no último ciclo do poller."""
    device_id: str
    name: str
    local_id: LocalId
    profile: Perfil
    status: FleetStatus
    offline_reason: OfflineReason | None = None
    state: str | None = None        # NORMAL, ATENCAO, BLOQUEIO, CRITICO ou FALHA_SENSOR
    reason: str | None = None       # motivo do estado, como em `reason` no Orion
    blocks: int | None = None
    last_seen: str | None = None
    seconds_since: int | None = None
    current: SensorValues


class FleetReport(BaseModel):
    """Resposta de GET /api/fleet: estado da EC2 e de cada C-Shield."""
    fiware_online: bool
    fiware_offline_since: str | None = None
    updated_at: str | None = None
    dispositivos: list[DispositivoStatus]
