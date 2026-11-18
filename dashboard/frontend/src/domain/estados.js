/*
 * C-Shield — Dashboard (terceira camada: lê o FIWARE)
 * GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
 * Equipe Nexus Vision. Integrantes:
 *   Isac Nilton Fernandes de Oliveira — RM573282
 *   João Benedito de Oliveira Simplício — RM570206
 *   Leonardo Formigari — RM573291
 *   Luis Fernando — RM570959
 *   Nicolas Cinel — RM572940
 */

/*
 * estados.js — as palavras e as cores do dashboard (só apresentação; quem decide o estado é o
 * firmware). Cor nunca vem sozinha (W49): sempre acompanha o texto.
 */

// Estado do C-Shield em cor e texto. Nomes canônicos, iguais ao Edge. BLOQUEIO é azul, como o
// LED do aparelho. OFFLINE não é estado do firmware: o dashboard marca assim o C-Shield que
// parou de mandar leituras. SEM_CSHIELD é um ponto do mapa em que ainda não há dispositivo.
export const ESTADOS = {
  NORMAL: { rotulo: 'Normal', cor: 'var(--color-ok)' },
  ATENCAO: { rotulo: 'Atenção', cor: 'var(--color-atencao)' },
  BLOQUEIO: { rotulo: 'Bloqueio', cor: 'var(--color-led-forte)' },
  CRITICO: { rotulo: 'Crítico', cor: 'var(--color-critico)' },
  FALHA_SENSOR: { rotulo: 'Falha de sensor', cor: 'var(--color-atencao)' },
  OFFLINE: { rotulo: 'Offline', cor: 'var(--color-texto-mudo)' },
  SEM_CSHIELD: { rotulo: 'Sem C-Shield', cor: 'var(--color-texto-mudo)' },
}

// Estado a mostrar de um item de /api/fleet (ou nenhum, se o ponto do mapa está vazio).
export function situacaoDoDispositivo(dispositivo) {
  if (!dispositivo) return 'SEM_CSHIELD'
  if (dispositivo.status === 'offline') return 'OFFLINE'
  return dispositivo.state in ESTADOS ? dispositivo.state : 'NORMAL'
}

// Motivo do bloqueio (atributo `reason`), em português.
const ROTULO_MOTIVO = {
  PORTA_ABERTA: 'Porta aberta',
  DERIVA: 'Temperatura fora da faixa',
  SETPOINT_INVALIDO: 'Setpoint fora da faixa',
  DEGELO_FORA_HORARIO: 'Degelo fora do horário',
  DEGELO_LONGO: 'Degelo longo demais',
  AQUECIMENTO_RAPIDO: 'Aquecimento rápido',
  RESFRIAMENTO_RAPIDO: 'Resfriamento rápido',
  TEMP_FORA_TOLERANCIA: 'Temperatura além da tolerância',
  SENSOR_SEM_LEITURA: 'Sensor sem leitura',
}

export function rotuloMotivo(motivo) {
  if (!motivo || motivo === '-') return null
  return ROTULO_MOTIVO[motivo] ?? motivo
}

// Motivo de uma recusa de cartão (atributo `refusedReason`), em português.
const ROTULO_RECUSA = {
  TRAVA: 'Trava mínima ainda ativa',
  ORDEM: 'Ordem do controlador ainda inválida',
  TEMPERATURA: 'Temperatura ainda além da tolerância',
  NAO_CADASTRADO: 'Cartão não cadastrado',
}

export function rotuloRecusa(motivo) {
  return ROTULO_RECUSA[motivo] ?? motivo ?? ''
}
