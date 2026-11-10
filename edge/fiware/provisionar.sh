#!/usr/bin/env bash
# =============================================================================
# C-Shield — Edge Computing do Guardião (GS 2026.2 "CitizenTech")
# Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940
# =============================================================================
#
# provisionar.sh — cadastra o C-Shield no FIWARE (IoT Agent MQTT + Orion + STH-Comet).
#
# O device é provisionado SÓ COM ATRIBUTOS, SEM COMANDOS: o C-Shield envia dados e não
# recebe ordens. Por isso não há registration de comandos no Orion.
#
# Uso:
#   bash edge/fiware/provisionar.sh                       # usa o IP padrão abaixo
#   FIWARE_HOST=1.2.3.4 bash edge/fiware/provisionar.sh   # outro IP
#   DRY_RUN=1 bash edge/fiware/provisionar.sh             # só mostra o que faria
#
# Pode ser rodado mais de uma vez: service group e device existentes (HTTP 409) contam
# como sucesso, e a subscription só é criada se ainda não existir.

set -u

# --- Configuração (o IP elástico da EC2 vem de uma variável, nunca fixo no meio do script) ---
FIWARE_HOST="${FIWARE_HOST:-3.215.3.216}"   # placeholder: confirmar se o IP elástico ainda existe
IOTA_PORT="${IOTA_PORT:-4041}"              # IoT Agent MQTT (porta de administração)
ORION_PORT="${ORION_PORT:-1026}"            # Orion Context Broker
STH_PORT="${STH_PORT:-8666}"                # STH-Comet

SERVICE="smart"            # header fiware-service
SERVICE_PATH="/"           # header fiware-servicepath
APIKEY="TEF"               # apikey do service group (a mesma da Vinheria)
DEVICE_ID="cshield001"
ENTITY_ID="urn:ngsi-ld:CShield:001"
ENTITY_TYPE="CShield"

IOTA_URL="http://${FIWARE_HOST}:${IOTA_PORT}"
ORION_URL="http://${FIWARE_HOST}:${ORION_PORT}"

# Atributos do device: object_id curto (vai no payload UltraLight) -> nome longo (Orion/STH).
# Os de telemetria e estado são os que vão para o histórico do STH-Comet (seção abaixo).
ATRIBUTOS_STH=(temperature humidity luminosity state blocks exposure)

# --- Funções ---

# Envia uma requisição e devolve o código HTTP. Em DRY_RUN só imprime.
# $1 método, $2 URL, $3 corpo JSON (opcional)
chamar() {
  local metodo="$1" url="$2" corpo="${3:-}"
  if [ "${DRY_RUN:-0}" = "1" ]; then
    echo "  [dry-run] $metodo $url" >&2
    [ -n "$corpo" ] && echo "$corpo" >&2
    echo "201"
    return 0
  fi
  if [ -n "$corpo" ]; then
    curl -s -o /dev/null -w "%{http_code}" --max-time 10 -X "$metodo" "$url" \
      -H "fiware-service: ${SERVICE}" -H "fiware-servicepath: ${SERVICE_PATH}" \
      -H "Content-Type: application/json" -d "$corpo"
  else
    curl -s -o /dev/null -w "%{http_code}" --max-time 10 -X "$metodo" "$url" \
      -H "fiware-service: ${SERVICE}" -H "fiware-servicepath: ${SERVICE_PATH}"
  fi
}

# Confere o código: 2xx e 409 (já existe) são sucesso; qualquer outra coisa para o script.
conferir() {
  local codigo="$1" descricao="$2"
  case "$codigo" in
    2??) echo "ok    ${descricao} (HTTP ${codigo})" ;;
    409) echo "ok    ${descricao} (já existia, HTTP 409)" ;;
    000) echo "ERRO  ${descricao}: sem resposta de ${FIWARE_HOST}. A EC2 está ligada?" >&2; exit 1 ;;
    *)   echo "ERRO  ${descricao}: HTTP ${codigo}" >&2; exit 1 ;;
  esac
}

# --- 1. Service group (idempotente; a Vinheria já usa o mesmo apikey TEF) ---
corpo_service=$(cat <<EOF
{ "services": [ { "apikey": "${APIKEY}", "cbroker": "${ORION_URL}", "entity_type": "Thing", "resource": "", "timestamp": true } ] }
EOF
)
conferir "$(chamar POST "${IOTA_URL}/iot/services" "$corpo_service")" "service group ${APIKEY}"

# --- 2. Device: só atributos, SEM "commands" ---
corpo_device=$(cat <<EOF
{ "devices": [ {
  "device_id": "${DEVICE_ID}",
  "entity_name": "${ENTITY_ID}",
  "entity_type": "${ENTITY_TYPE}",
  "protocol": "PDI-IoTA-UltraLight",
  "transport": "MQTT",
  "attributes": [
    { "object_id": "t",  "name": "temperature", "type": "Float" },
    { "object_id": "h",  "name": "humidity",    "type": "Float" },
    { "object_id": "l",  "name": "luminosity",  "type": "Integer" },
    { "object_id": "sp", "name": "setpoint",    "type": "Float" },
    { "object_id": "dg", "name": "defrost",     "type": "Integer" },
    { "object_id": "st", "name": "state",       "type": "Text" },
    { "object_id": "mt", "name": "reason",      "type": "Text" },
    { "object_id": "bq", "name": "blocks",      "type": "Integer" },
    { "object_id": "ex", "name": "exposure",    "type": "Float" }
  ]
} ] }
EOF
)
conferir "$(chamar POST "${IOTA_URL}/iot/devices" "$corpo_device")" "device ${DEVICE_ID} (sem comandos)"

# --- 3. Subscriptions do Orion para o STH-Comet (uma por atributo, como na Vinheria) ---
# Só cria se ainda não existir uma com a mesma descrição, para rodar o script sem duplicar.
if [ "${DRY_RUN:-0}" = "1" ]; then
  existentes=""
else
  existentes=$(curl -s --max-time 10 "${ORION_URL}/v2/subscriptions?limit=1000" \
    -H "fiware-service: ${SERVICE}" -H "fiware-servicepath: ${SERVICE_PATH}")
fi

for attr in "${ATRIBUTOS_STH[@]}"; do
  descricao="C-Shield STH ${attr}"
  if echo "$existentes" | grep -qF "\"${descricao}\""; then
    echo "ok    subscription ${attr} (já existia)"
    continue
  fi
  corpo_sub=$(cat <<EOF
{
  "description": "${descricao}",
  "subject": {
    "entities": [ { "id": "${ENTITY_ID}", "type": "${ENTITY_TYPE}" } ],
    "condition": { "attrs": [ "${attr}" ] }
  },
  "notification": {
    "http": { "url": "http://${FIWARE_HOST}:${STH_PORT}/notify" },
    "attrs": [ "${attr}" ],
    "attrsFormat": "legacy"
  }
}
EOF
)
  conferir "$(chamar POST "${ORION_URL}/v2/subscriptions" "$corpo_sub")" "subscription ${attr} -> STH-Comet"
done

echo
echo "Pronto. Conferir a entidade:"
echo "  curl -s ${ORION_URL}/v2/entities/${ENTITY_ID} -H 'fiware-service: ${SERVICE}' -H 'fiware-servicepath: ${SERVICE_PATH}'"
