# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""Dublês dos testes: FIWARE falso para o poller e relógio controlado."""
from datetime import datetime, timedelta, timezone

from app.services.fiware_errors import FiwareUnavailable


class Relogio:
    """Relógio parado em 2026-11-18 12:00:00 UTC, avançado pelo teste."""

    def __init__(self):
        self.agora = datetime(2026, 11, 18, 12, 0, 0, tzinfo=timezone.utc)

    def __call__(self) -> datetime:
        return self.agora

    def avancar(self, segundos: float) -> None:
        self.agora += timedelta(seconds=segundos)

    def iso(self, atras: float = 0) -> str:
        """Instante `atras` segundos antes de agora, no formato do TimeInstant."""
        return (self.agora - timedelta(seconds=atras)).strftime("%Y-%m-%dT%H:%M:%S.000Z")


class FiwareFalso:
    """Responde list_entities com o que o teste puser em `entidades`, ou falha."""

    def __init__(self):
        self.entidades: list[dict] = []
        self.fora = False
        self.chamadas = 0

    async def list_entities(self, entity_type: str = "CShield") -> list[dict]:
        self.chamadas += 1
        if self.fora:
            raise FiwareUnavailable("orion", "sem conexão")
        return self.entidades
