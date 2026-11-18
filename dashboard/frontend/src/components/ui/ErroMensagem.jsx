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

import BotaoLed from './BotaoLed.jsx'

// Mensagem de erro que diz o que houve e como seguir. role="alert" anuncia na hora.
// Só o ícone e a borda usam a cor de erro; o texto fica em gelo-50.
export default function ErroMensagem({ mensagem, aoTentarDeNovo }) {
  return (
    <div
      role="alert"
      className="flex flex-col items-start gap-3 rounded-xl border border-critico/40 bg-noite-900 p-4"
    >
      <p className="flex items-start gap-3 text-gelo-50">
        <svg
          aria-hidden="true"
          viewBox="0 0 24 24"
          className="mt-0.5 size-5 shrink-0 text-critico"
          fill="none"
          stroke="currentColor"
          strokeWidth="2"
          strokeLinecap="round"
          strokeLinejoin="round"
        >
          <path d="M12 3 2.5 20h19L12 3Z" />
          <path d="M12 10v4M12 17.2v.01" />
        </svg>
        <span>{mensagem}</span>
      </p>
      {aoTentarDeNovo && (
        <BotaoLed variante="fantasma" onClick={aoTentarDeNovo}>
          Tentar de novo
        </BotaoLed>
      )}
    </div>
  )
}
