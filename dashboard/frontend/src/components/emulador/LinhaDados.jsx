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

// A linha de dados de um local com C-Shield: ele só ENVIA para a central (nunca recebe). Pulsos
// andam só em direção à central e uma seta fixa marca o sentido, então a promessa "só envia"
// aparece desenhada. Nunca há linha entre locais nem da central para fora. Sem movimento
// (prefers-reduced-motion), ficam a linha e a seta, sem pulsos.
import { CENTRAL, RAIO_CENTRAL, RAIO_ESCUDO, pontoAntesDoDestino } from './geometria.js'

export default function LinhaDados({ local, reduzirMovimento }) {
  const inicio = pontoAntesDoDestino(CENTRAL, local, RAIO_ESCUDO + 3) // sai da borda do escudo
  const fim = pontoAntesDoDestino(local, CENTRAL, RAIO_CENTRAL + 4) // chega na borda da central
  const angulo = Math.atan2(fim.y - inicio.y, fim.x - inicio.x)
  const ponta = (desvio) => `${fim.x - Math.cos(angulo + desvio) * 16},${fim.y - Math.sin(angulo + desvio) * 16}`
  const caminho = `M ${inicio.x} ${inicio.y} L ${fim.x} ${fim.y}`

  return (
    <g aria-hidden="true">
      <line
        x1={inicio.x}
        y1={inicio.y}
        x2={fim.x}
        y2={fim.y}
        stroke="var(--color-led-fraco)"
        strokeWidth={3}
        strokeLinecap="round"
      />
      <polygon points={`${fim.x},${fim.y} ${ponta(0.45)} ${ponta(-0.45)}`} fill="var(--color-led-forte)" />
      {reduzirMovimento ? null : (
        <>
          <circle r={5} fill="var(--color-led-forte)">
            <animateMotion dur="2.4s" repeatCount="indefinite" path={caminho} />
          </circle>
          <circle r={5} fill="var(--color-led-forte)">
            <animateMotion dur="2.4s" begin="1.2s" repeatCount="indefinite" path={caminho} />
          </circle>
        </>
      )}
    </g>
  )
}
