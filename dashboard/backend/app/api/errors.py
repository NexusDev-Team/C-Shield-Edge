# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Tradução das exceções dos services em respostas HTTP.

FiwareError (falhas do FIWARE) e RegistryError (regras do cadastro) viram
JSON { "detail", "service" } com um status coerente, para o front-end mostrar
uma mensagem clara em vez de um erro 500 genérico.
"""
from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse

from app.services.fiware_errors import (
    FiwareConflict,
    FiwareError,
    FiwareNotFound,
    FiwareUnavailable,
)
from app.services.registry_errors import DeviceAlreadyExists, RegistryError


def _status_para(erro: FiwareError) -> int:
    """Status HTTP da resposta para cada tipo de falha do FIWARE."""
    if isinstance(erro, FiwareUnavailable):
        return 503
    if isinstance(erro, FiwareNotFound):
        return 404
    if isinstance(erro, FiwareConflict):
        return 409
    return 502


def _status_registry(erro: RegistryError) -> int:
    """Cadastro repetido dá 409; C-Shield não encontrado dá 404."""
    return 409 if isinstance(erro, DeviceAlreadyExists) else 404


def register_error_handlers(app: FastAPI) -> None:
    """Registra no app os tradutores de FiwareError e RegistryError."""

    @app.exception_handler(FiwareError)
    async def _trata_fiware(request: Request, erro: FiwareError) -> JSONResponse:
        return JSONResponse(status_code=_status_para(erro),
                            content={"detail": erro.message, "service": erro.service})

    @app.exception_handler(RegistryError)
    async def _trata_registry(request: Request, erro: RegistryError) -> JSONResponse:
        return JSONResponse(status_code=_status_registry(erro),
                            content={"detail": erro.message, "service": erro.service})
