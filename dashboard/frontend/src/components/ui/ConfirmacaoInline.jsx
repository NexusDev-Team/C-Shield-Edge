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

// Confirmação feita na própria página, sem confirm() (W34). Ao abrir, o foco vai para "Cancelar"
// (o caminho seguro). Quem usa decide quando mostrar e devolve o foco ao botão de origem ao fechar.
import { useEffect, useRef } from 'react'
import BotaoLed from './BotaoLed.jsx'

export default function ConfirmacaoInline({
  mensagem,
  rotuloConfirmar = 'Sim, excluir',
  ocupado = false,
  erro = '',
  aoConfirmar,
  aoCancelar,
}) {
  const cancelar = useRef(null)

  useEffect(() => {
    cancelar.current?.focus()
  }, [])

  return (
    <div
      role="group"
      aria-label="Confirmar"
      className="flex flex-col gap-3 rounded-lg border border-ataque bg-noite-900 p-3"
    >
      <p className="text-gelo-50">{mensagem}</p>
      {erro ? (
        <p role="alert" className="text-gelo-50">
          <span aria-hidden="true" className="mr-2 font-mono text-ataque">
            [erro]
          </span>
          {erro}
        </p>
      ) : null}
      <div className="flex flex-wrap gap-2">
        <BotaoLed variante="ataque" onClick={aoConfirmar} disabled={ocupado}>
          {ocupado ? 'Aguarde...' : rotuloConfirmar}
        </BotaoLed>
        <BotaoLed ref={cancelar} variante="fantasma" onClick={aoCancelar} disabled={ocupado}>
          Cancelar
        </BotaoLed>
      </div>
    </div>
  )
}
