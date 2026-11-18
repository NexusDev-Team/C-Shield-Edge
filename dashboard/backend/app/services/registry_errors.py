# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Exceções do cadastro de C-Shields.

Como as do FIWARE, não dependem do FastAPI; a tradução para resposta HTTP
fica em app/api/errors.py (404 para "não encontrada", 409 para conflito).
"""


class RegistryError(Exception):
    """Falha de regra do cadastro de C-Shields."""
    service = "registry"

    def __init__(self, message: str):
        super().__init__(message)
        self.message = message


class DeviceNotFound(RegistryError):
    """O device_id não está cadastrado."""


class DeviceAlreadyExists(RegistryError):
    """O device_id já está cadastrado."""
