# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Faixas de temperatura dos perfis de produto (°C), espelho de PERFIS em edge/cshield/config.h.

O dashboard não decide nada, mas precisa da faixa para dizer se um setpoint perigoso era de
FRIO (abaixo do mínimo) ou de CALOR (acima do máximo), do mesmo jeito que o firmware.
"""

# perfil -> (mínimo, máximo)
FAIXAS = {
    "VACINA": (2.0, 8.0),
    "CONGELADO": (-30.0, -18.0),
    "RESFRIADO": (0.0, 4.0),
    "PLASMA": (-40.0, -25.0),
}
