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

import { rotuloRecusa } from '../../domain/estados.js'
import PainelVidro from '../ui/PainelVidro.jsx'
import { formatarDataHora } from '../ui/formato.js'

// Histórico de liberações: cada vez que um cartão NFC foi encostado no aparelho, quem foi e o
// que aconteceu. Liberação aceita em verde, recusa em âmbar; o tipo vem sempre escrito (W49).
// Cartão desconhecido aparece pelo UID, sem papel. `eventos` vem de /api/devices/<id>/eventos.
function Linha({ evento }) {
  const liberacao = evento.tipo === 'liberacao'
  const cor = liberacao ? 'var(--color-ok)' : 'var(--color-atencao)'
  return (
    <tr className="border-t border-borda align-top">
      <td className="py-3 pr-4 font-mono text-sm whitespace-nowrap text-gelo-100">{formatarDataHora(evento.quando)}</td>
      <td className="py-3 pr-4">
        <span
          className="inline-flex items-center gap-2 rounded-full border px-3 py-1 text-sm text-gelo-50"
          style={{ borderColor: cor }}
        >
          <span aria-hidden="true" className="size-2 rounded-full" style={{ backgroundColor: cor }} />
          {liberacao ? 'Liberado' : 'Recusado'}
        </span>
      </td>
      <td className="py-3 pr-4 text-gelo-50">
        {evento.papel ?? 'Cartão desconhecido'}
        <span className="ml-2 font-mono text-sm text-gelo-300">{evento.quem}</span>
      </td>
      <td className="py-3 text-gelo-100">{liberacao ? 'Bloqueio encerrado no local' : rotuloRecusa(evento.motivo)}</td>
    </tr>
  )
}

export default function HistoricoCartoes({ eventos }) {
  return (
    <PainelVidro as="section" aria-labelledby="titulo-liberacoes" className="p-6 sm:p-8">
      <h2 id="titulo-liberacoes" className="text-xl font-semibold text-gelo-50">
        Histórico de liberações
      </h2>
      <p className="mt-2 text-gelo-300">
        Só o cartão cadastrado, encostado no aparelho, encerra um bloqueio. Cada tentativa fica registrada aqui.
      </p>
      {eventos.length === 0 ? (
        <p className="mt-6 text-gelo-100">Nenhum cartão foi usado neste C-Shield ainda.</p>
      ) : (
        <div className="mt-4 overflow-x-auto">
          <table className="w-full min-w-[34rem] text-left">
            <thead>
              <tr className="text-sm text-gelo-300">
                <th scope="col" className="pb-2 pr-4 font-medium">Quando</th>
                <th scope="col" className="pb-2 pr-4 font-medium">Resultado</th>
                <th scope="col" className="pb-2 pr-4 font-medium">Quem</th>
                <th scope="col" className="pb-2 font-medium">Detalhe</th>
              </tr>
            </thead>
            <tbody>
              {eventos.map((evento) => (
                <Linha key={`${evento.tipo}-${evento.quando}-${evento.total}`} evento={evento} />
              ))}
            </tbody>
          </table>
        </div>
      )}
    </PainelVidro>
  )
}
