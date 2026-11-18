# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Exceções do cliente FIWARE.

Os services levantam estas exceções em vez de HTTPException, para não
depender do FastAPI (o poller da Task 4 também usa o cliente). A tradução
para resposta HTTP fica em app/api/errors.py.
"""


class FiwareError(Exception):
    """Falha ao falar com um componente do FIWARE (orion, iota, sth ou config)."""

    def __init__(self, service: str, message: str, status_code: int | None = None):
        super().__init__(f"{service}: {message}")
        self.service = service
        self.message = message
        self.status_code = status_code


class FiwareUnavailable(FiwareError):
    """Sem resposta: EC2 desligada, IP errado, timeout ou IP não configurado."""


class FiwareNotFound(FiwareError):
    """O recurso pedido não existe (HTTP 404)."""


class FiwareConflict(FiwareError):
    """O recurso já existe (HTTP 409)."""
