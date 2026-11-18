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

import PainelVidro from '../ui/PainelVidro.jsx'
import { formatarGraus } from '../ui/formato.js'

// Acima de 40% de luz no LDR a porta está aberta (mesmo limite do firmware, PORTA_ABERTA_PCT).
const PORTA_ABERTA_PCT = 40

function Cartao({ rotulo, valor, apoio }) {
  return (
    <PainelVidro as="article" className="flex flex-col gap-2 p-5">
      <p className="rotulo-tecnico">{rotulo}</p>
      <p className="font-titulo text-3xl font-semibold text-gelo-50 tabular-nums">{valor}</p>
      <p className="text-sm text-gelo-300">{apoio}</p>
    </PainelVidro>
  )
}

// Os quatro números de apoio do C-Shield: umidade, porta, bloqueios desde que ligou e exposição
// térmica acumulada. `atual` é o /api/devices/<id>/current (valores null quando não há leitura).
export default function CartoesSensores({ atual }) {
  const luz = atual?.luminosity
  const porta = typeof luz === 'number' ? (luz > PORTA_ABERTA_PCT ? 'Aberta' : 'Fechada') : '--'
  return (
    <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-4">
      <Cartao
        rotulo="Umidade"
        valor={typeof atual?.humidity === 'number' ? `${Math.round(atual.humidity)} %` : '--'}
        apoio="Medida pelo DHT22 do aparelho."
      />
      <Cartao rotulo="Porta" valor={porta} apoio="Pela luz no LDR dentro da câmara." />
      <Cartao
        rotulo="Bloqueios"
        valor={typeof atual?.blocks === 'number' ? atual.blocks : '--'}
        apoio="Desde que o aparelho ligou."
      />
      <Cartao
        rotulo="Exposição"
        valor={typeof atual?.exposure === 'number' ? `${formatarGraus(atual.exposure)} °C·min` : '--'}
        apoio="Calor acumulado acima do máximo da faixa."
      />
    </div>
  )
}
