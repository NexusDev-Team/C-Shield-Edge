# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Constantes canônicas do FIWARE usadas pelo cliente.

Ficam num módulo só para os componentes (IoT Agent, Orion, STH-Comet)
compartilharem os mesmos nomes sem se importarem uns aos outros.
"""

# Atributos do C-Shield: (nome curto no UltraLight, nome longo no Orion/STH, tipo).
# É a mesma lista de edge/fiware/provisionar.sh; o C-Shield só envia dados, não há comandos.
SENSOR_ATTRS = (
    ("t", "temperature", "Float"),
    ("h", "humidity", "Float"),
    ("l", "luminosity", "Integer"),
    ("sp", "setpoint", "Float"),
    ("dg", "defrost", "Integer"),
    ("st", "state", "Text"),
    ("mt", "reason", "Text"),
    ("bq", "blocks", "Integer"),
    ("ex", "exposure", "Float"),
)
# Eventos do cartão NFC (liberação presencial). Só vão no payload no momento do evento.
EVENT_ATTRS = (
    ("lb", "releasedBy", "Text"),
    ("nl", "releaseCount", "Integer"),
    ("rj", "refusedBy", "Text"),
    ("rm", "refusedReason", "Text"),
    ("nr", "refusalCount", "Integer"),
)
# Atributos que ganham subscription própria para o STH-Comet guardar o histórico
# (os mesmos de ATRIBUTOS_STH em provisionar.sh).
ATRIBUTOS_HISTORICO = ("temperature", "humidity", "luminosity", "state", "exposure")
# Subscriptions de evento: o contador (sempre muda) dispara e leva o evento inteiro.
# nome -> (atributo que dispara, atributos notificados)
EVENTOS_HISTORICO = {
    "ataque": ("blocks", ("blocks", "reason", "setpoint")),
    "liberacao": ("releaseCount", ("releasedBy", "releaseCount")),
    "recusa": ("refusalCount", ("refusedBy", "refusedReason", "refusalCount")),
}
ENTITY_TYPE = "CShield"
# Máximo de itens por listagem no Orion (frota e subscriptions)
ORION_LIST_LIMIT = 1000
# Nomes longos aceitos em subscriptions e no histórico
ATRIBUTOS_LONGOS = tuple(longo for _, longo, _ in SENSOR_ATTRS + EVENT_ATTRS)
# Página máxima aceita pelo STH-Comet (config.maxPageSize padrão é 100;
# acima disso ele responde 400)
HISTORY_PAGE_LIMIT = 100
# Teto de páginas por consulta por datas (1.500 pontos, cobre 2 h de leituras);
# acima disso, as páginas são espaçadas ao longo da janela (amostra)
HISTORY_MAX_PAGES = 15
