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

import { ESTADOS } from '../../domain/estados.js'

// Estado do C-Shield em cor E texto (W49): nunca só pela cor.
// O texto fica em gelo-50 (contraste alto); a cor do estado vai no ponto e na borda.
export default function StatusBadge({ estado, className = '' }) {
  const info = ESTADOS[estado] ?? { rotulo: 'Sem leitura', cor: 'var(--color-gelo-300)' }
  return (
    <span
      className={`inline-flex items-center gap-2 rounded-full border bg-noite-900 px-3 py-1 font-mono text-sm text-gelo-50 ${className}`}
      style={{ borderColor: info.cor }}
    >
      <span aria-hidden="true" className="size-2.5 rounded-full" style={{ backgroundColor: info.cor }} />
      {info.rotulo}
    </span>
  )
}
