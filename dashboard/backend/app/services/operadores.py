# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Papéis dos cartões NFC de liberação.

O firmware publica só o CÓDIGO do papel (`releasedBy`, por exemplo RT01). Esta
tabela é o espelho de OPERADORES em edge/cshield/config.h e serve para o
dashboard mostrar o nome do papel. Se um código novo for cadastrado no
firmware, acrescente-o aqui também.
"""

PAPEIS = {
    "RT01": "Responsável técnico",
    "ENF01": "Enfermagem",
    "MAN01": "Manutenção",
    "SUP01": "Supervisão",
    "GER01": "Gerência",
}


def papel_de(codigo: str) -> str | None:
    """Nome do papel de um código; None quando não é de um cartão cadastrado.

    Numa recusa por cartão desconhecido, o firmware publica o UID sem ":" no
    lugar do código, e esse valor não está na tabela.
    """
    return PAPEIS.get(codigo)
