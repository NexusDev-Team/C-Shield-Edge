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

// O mapa SVG da frota (adaptado de emulador/MapaCidade do site): a Central (FIWARE) no meio e os
// 5 pontos da cidade. Só os pontos com C-Shield ONLINE têm linha de dados até a central, e a
// linha vai só nesse sentido (o C-Shield envia, nunca recebe). Não há atacante nem linha entre
// pontos: aqui os dados são reais, vindos de /api/fleet.
import { LOCAIS } from '../../domain/locais.js'
import Central from '../emulador/Central.jsx'
import FundoMapa from '../emulador/FundoMapa.jsx'
import { VIEWBOX } from '../emulador/geometria.js'
import LinhaDados from '../emulador/LinhaDados.jsx'
import ListaLocais from './ListaLocais.jsx'
import LocalNoMapa from './LocalNoMapa.jsx'

export default function MapaFrota({ porLocal, selecionadoId, aoSelecionar, reduzirMovimento }) {
  return (
    <div className="relative">
      <svg
        viewBox={`${VIEWBOX.x} ${VIEWBOX.y} ${VIEWBOX.largura} ${VIEWBOX.altura}`}
        role="group"
        aria-label="Mapa da cidade com os cinco pontos da frota de C-Shields"
        className="h-auto w-full"
      >
        {/* O fundo: ruas, quarteirões, avenida e rio, como um mapa de verdade */}
        <FundoMapa />

        {/* A cidade: um contorno tracejado */}
        <rect
          x={30}
          y={96}
          width={940}
          height={484}
          rx={28}
          fill="none"
          stroke="var(--color-borda)"
          strokeWidth={2}
          strokeDasharray="10 10"
        />

        {LOCAIS.filter((l) => porLocal[l.id]?.status === 'online').map((def) => (
          <LinhaDados key={`dados-${def.id}`} local={def} reduzirMovimento={reduzirMovimento} />
        ))}

        <Central />
        {LOCAIS.map((def) => (
          <LocalNoMapa
            key={def.id}
            def={def}
            dispositivo={porLocal[def.id]}
            selecionado={selecionadoId === def.id}
            aoSelecionar={aoSelecionar}
          />
        ))}
      </svg>

      <ListaLocais porLocal={porLocal} selecionadoId={selecionadoId} aoSelecionar={aoSelecionar} />
    </div>
  )
}
