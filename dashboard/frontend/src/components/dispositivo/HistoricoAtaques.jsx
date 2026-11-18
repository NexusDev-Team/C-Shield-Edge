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

import { DIRECAO_ATAQUE, descreverAtaque } from '../../domain/estados.js'
import PainelVidro from '../ui/PainelVidro.jsx'
import { formatarDataHora } from '../ui/formato.js'

// Histórico de ataques: cada vez que o C-Shield abriu um bloqueio por uma ordem perigosa do
// controlador, quando foi, se esquentava ou esfriava e como foi feito (setpoint alterado,
// subida ou queda brusca de temperatura, degelo indevido). A direção vem escrita, nunca só na
// cor (W49). `ataques` vem de /api/devices/<id>/ataques.
function Linha({ ataque }) {
  const direcao = DIRECAO_ATAQUE[ataque.direcao] ?? DIRECAO_ATAQUE.indefinida
  return (
    <tr className="border-t border-borda align-top">
      <td className="py-3 pr-4 font-mono text-sm whitespace-nowrap text-gelo-100">{formatarDataHora(ataque.quando)}</td>
      <td className="py-3 pr-4">
        <span
          className="inline-flex items-center gap-2 rounded-full border px-3 py-1 text-sm text-gelo-50"
          style={{ borderColor: direcao.cor }}
        >
          <span aria-hidden="true" className="size-2 rounded-full" style={{ backgroundColor: direcao.cor }} />
          {direcao.rotulo}
        </span>
      </td>
      <td className="py-3 pr-4 text-gelo-50">{descreverAtaque(ataque)}</td>
      <td className="py-3 text-gelo-100">Barrado: aquecimento cortado ou ventoinha ligada, ordens ignoradas</td>
    </tr>
  )
}

export default function HistoricoAtaques({ ataques }) {
  return (
    <PainelVidro as="section" aria-labelledby="titulo-ataques" className="p-6 sm:p-8">
      <h2 id="titulo-ataques" className="text-xl font-semibold text-gelo-50">
        Histórico de ataques
      </h2>
      <p className="mt-2 text-gelo-300">
        Cada ordem perigosa do controlador que o C-Shield barrou, com o jeito que ela veio: setpoint alterado,
        temperatura subindo ou caindo rápido, ou degelo indevido.
      </p>
      {ataques.length === 0 ? (
        <p className="mt-6 text-gelo-100">Nenhum ataque registrado neste C-Shield ainda.</p>
      ) : (
        <div className="mt-4 overflow-x-auto">
          <table className="w-full min-w-[40rem] text-left">
            <thead>
              <tr className="text-sm text-gelo-300">
                <th scope="col" className="pb-2 pr-4 font-medium">Quando</th>
                <th scope="col" className="pb-2 pr-4 font-medium">Tipo</th>
                <th scope="col" className="pb-2 pr-4 font-medium">Como foi feito</th>
                <th scope="col" className="pb-2 font-medium">Resposta</th>
              </tr>
            </thead>
            <tbody>
              {ataques.map((ataque) => (
                <Linha key={`${ataque.quando}-${ataque.total}`} ataque={ataque} />
              ))}
            </tbody>
          </table>
        </div>
      )}
    </PainelVidro>
  )
}
