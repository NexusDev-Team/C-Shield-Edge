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

// Intervalos de atualização das telas (ms). O C-Shield publica a cada 5 s e na hora em cada
// evento, então consultar com mais frequência não traz dado novo.
export const FLEET_POLL_MS = 5000       // mapa da frota
export const LEITURA_POLL_MS = 5000     // temperatura atual, histórico e eventos de um C-Shield
export const HEALTH_POLL_MS = 30000     // saúde da EC2

// Quantos pontos do histórico térmico (STH) o gráfico pede, quantos eventos de cartão e quantos ataques as tabelas.
export const PONTOS_GRAFICO = 40
export const EVENTOS_TABELA = 20
export const ATAQUES_TABELA = 20
