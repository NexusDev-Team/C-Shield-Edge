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

// A central (FIWARE), no meio do mapa: para onde os locais com C-Shield enviam os dados. Ela só
// recebe e mostra; nunca manda nada de volta.
import { CENTRAL, RAIO_CENTRAL } from './geometria.js'

const HALO = { paintOrder: 'stroke', stroke: 'var(--color-noite-950)', strokeWidth: 6, strokeLinejoin: 'round' }

export default function Central() {
  return (
    <g transform={`translate(${CENTRAL.x} ${CENTRAL.y})`} role="img" aria-label="Central, que só recebe os dados dos locais com C-Shield">
      <circle r={RAIO_CENTRAL} fill="var(--color-noite-900)" stroke="var(--color-led-forte)" strokeWidth={3} />
      <g fill="none" stroke="var(--color-gelo-50)" strokeWidth={3} strokeLinecap="round">
        <rect x={-14} y={-18} width={28} height={10} rx={3} />
        <rect x={-14} y={-5} width={28} height={10} rx={3} />
        <rect x={-14} y={8} width={28} height={10} rx={3} />
        <path d="M-8 -13 H-6 M-8 0 H-6 M-8 13 H-6" />
      </g>
      <text y={RAIO_CENTRAL + 32} textAnchor="middle" fontSize={26} fontWeight={600} fill="var(--color-gelo-50)" style={HALO}>
        Central (FIWARE)
      </text>
    </g>
  )
}
