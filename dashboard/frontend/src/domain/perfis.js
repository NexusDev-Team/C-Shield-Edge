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
 * perfis.js — perfis de produto (faixa ideal de temperatura).
 * Formato único: { id, nome, tMin, tMax, tAlvo, tolerancia }.
 */

// Perfis do firmware (config.h). Usados pelo /simulador-real e pelos testes portados.
// Tolerância de 1,0 °C: margem curta o bastante para avisar antes de o produto estragar, e longa
// o bastante para não disparar CRITICO por oscilação do sensor. Nos congelados e no plasma o
// mínimo é só o piso prático do sensor, pois frio extra não estraga. O tAlvo (centro da faixa,
// ou perto do máximo nos congelados) só serve à física do emulador; o firmware não o usa.
export const PERFIS_FIRMWARE = {
  VACINA: { id: 'VACINA', nome: 'Vacina', tMin: 2, tMax: 8, tAlvo: 5, tolerancia: 1 },
  CONGELADO: { id: 'CONGELADO', nome: 'Congelado', tMin: -30, tMax: -18, tAlvo: -20, tolerancia: 1 },
  RESFRIADO: { id: 'RESFRIADO', nome: 'Resfriado', tMin: 0, tMax: 4, tAlvo: 2, tolerancia: 1 },
  PLASMA: { id: 'PLASMA', nome: 'Plasma', tMin: -40, tMax: -25, tAlvo: -30, tolerancia: 1 },
}

// A faixa como aparece na tela. Nos produtos congelados (máximo abaixo de zero) o mínimo do perfil
// é só o piso prático do sensor, e frio extra não estraga: a regra real é "no máximo tMax". Por
// isso o mínimo vem nulo ali, e quem desenha mostra "até X °C". Em vacina e resfriado vale a faixa
// inteira.
export function faixaParaMostrar({ tMin, tMax }) {
  return { minimo: tMax < 0 ? null : tMin, maximo: tMax }
}

// Perfil ativo do firmware (PERFIL_ATIVO em config.h): vacina.
export const PERFIL_ATIVO = 'VACINA'

export function perfilAtivo() {
  return PERFIS_FIRMWARE[PERFIL_ATIVO]
}

// Perfis do emulador: valores ILUSTRATIVOS para a cidade de 5 locais (D1–D3); não são os do
// firmware. A faixa de plasma e congelado tem só o máximo como regra real; o mínimo é o piso.
export const PERFIS_EMULADOR = {
  VACINA: { id: 'VACINA', nome: 'Vacina', tMin: 2, tMax: 8, tAlvo: 5, tolerancia: 1 },
  INSULINA: { id: 'INSULINA', nome: 'Insulina', tMin: 2, tMax: 8, tAlvo: 5, tolerancia: 1 },
  PLASMA: { id: 'PLASMA', nome: 'Plasma', tMin: -40, tMax: -25, tAlvo: -30, tolerancia: 1 },
  CONGELADO: { id: 'CONGELADO', nome: 'Congelado', tMin: -30, tMax: -18, tAlvo: -20, tolerancia: 1 },
}
