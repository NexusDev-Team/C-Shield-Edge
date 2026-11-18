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
 * geometria.js — medidas do mapa SVG (viewBox 0 70 1000 530: o topo do mapa é a borda de baixo da zona do atacante). As posições dos locais vêm de
 * LOCAIS (domain/cidade.js); aqui ficam o atacante, a central e a conta das linhas.
 */

export const VIEWBOX = { x: 0, y: 70, largura: 1000, altura: 530 }
// As linhas de ataque saem do atacante, que fica FORA do mapa, na zona de cima do card: o ponto de
// partida é o meio da borda de cima do mapa. A central fica no meio.
export const ATACANTE = { x: 500, y: 70 }
export const CENTRAL = { x: 500, y: 300 }
export const RAIO_LOCAL = 34
export const RAIO_ESCUDO = 46
export const RAIO_CENTRAL = 40

// Ponto a `distancia` do destino, sobre a reta origem → destino (onde a linha deve parar).
export function pontoAntesDoDestino(origem, destino, distancia) {
  const dx = destino.x - origem.x
  const dy = destino.y - origem.y
  const tamanho = Math.hypot(dx, dy) || 1
  return { x: destino.x - (dx / tamanho) * distancia, y: destino.y - (dy / tamanho) * distancia }
}
