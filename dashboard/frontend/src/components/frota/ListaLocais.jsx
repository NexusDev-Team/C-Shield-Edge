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

// Lista dos pontos para telas estreitas (só no celular), adaptada de emulador/ListaLocais. O mapa
// continua ali como visão geral, mas os rótulos dele ficariam pequenos demais; aqui cada ponto é
// um botão de 44 px com nome, estado em texto e temperatura.
import { LOCAIS } from '../../domain/locais.js'
import { ESTADOS, situacaoDoDispositivo } from '../../domain/estados.js'
import { textoTemperatura } from './LocalNoMapa.jsx'

export default function ListaLocais({ porLocal, selecionadoId, aoSelecionar }) {
  return (
    <ul className="mt-3 flex flex-col gap-2 sm:hidden" aria-label="Pontos da frota">
      {LOCAIS.map((def) => {
        const dispositivo = porLocal[def.id]
        const { rotulo, cor } = ESTADOS[situacaoDoDispositivo(dispositivo)]
        const ativo = selecionadoId === def.id
        return (
          <li key={def.id}>
            <button
              type="button"
              aria-pressed={ativo}
              onClick={() => aoSelecionar(def.id)}
              className="flex min-h-11 w-full cursor-pointer items-center justify-between gap-3 rounded-lg border bg-noite-900 px-3 py-2 text-left text-gelo-50"
              style={{ borderColor: ativo ? 'var(--color-gelo-50)' : 'var(--color-borda)' }}
            >
              <span>
                <span className="block font-semibold">{def.nome}</span>
                <span className="flex items-center gap-2 text-sm text-gelo-300">
                  <span aria-hidden="true" className="size-2.5 rounded-full" style={{ backgroundColor: cor }} />
                  {rotulo}
                </span>
              </span>
              <span className="shrink-0 font-mono text-lg whitespace-nowrap">{textoTemperatura(dispositivo)}</span>
            </button>
          </li>
        )
      })}
    </ul>
  )
}
