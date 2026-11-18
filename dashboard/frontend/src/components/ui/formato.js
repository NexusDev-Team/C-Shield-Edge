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

// Formatos de tela de datas e números (só apresentação; as regras ficam em src/domain).

// "11/11/26, 14:30"; se o texto não for uma data, mostra como veio.
export function formatarDataHora(texto) {
  const data = new Date(texto)
  if (Number.isNaN(data.getTime())) return String(texto ?? '')
  return data.toLocaleString('pt-BR', { dateStyle: 'short', timeStyle: 'short' })
}

// Temperatura com uma casa decimal e vírgula, como se escreve em pt-BR: -19,2.
export function formatarGraus(n) {
  return n.toLocaleString('pt-BR', { minimumFractionDigits: 1, maximumFractionDigits: 1 })
}
