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
 * usePrefereMenosMovimento — true quando a pessoa pediu menos movimento no sistema
 * (prefers-reduced-motion). Quem anima (linhas, ventoinha, pulsos) para quando vale true (W47).
 */
import { useSyncExternalStore } from 'react'

const CONSULTA = '(prefers-reduced-motion: reduce)'

function assinar(aoMudar) {
  if (typeof window === 'undefined' || !window.matchMedia) return () => {}
  const mq = window.matchMedia(CONSULTA)
  mq.addEventListener('change', aoMudar)
  return () => mq.removeEventListener('change', aoMudar)
}

function lerAgora() {
  return typeof window !== 'undefined' && Boolean(window.matchMedia) && window.matchMedia(CONSULTA).matches
}

export function usePrefereMenosMovimento() {
  return useSyncExternalStore(assinar, lerAgora, () => false)
}
