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

// Campo de formulário: rótulo sempre visível, dica opcional e mensagem de erro junto do campo,
// ligados ao controle por aria-describedby e aria-invalid (W32). O controle vem como função,
// que recebe as props prontas (id, aria-*, className) para espalhar no input, select ou textarea:
//   <CampoFormulario id="x" rotulo="Nome" erro={erros.nome}>{(p) => <input {...p} />}</CampoFormulario>
const CONTROLE =
  'min-h-11 w-full rounded-lg border bg-noite-900 px-3 py-2 text-gelo-50 aria-[invalid=true]:border-ataque border-borda'

export default function CampoFormulario({ id, rotulo, erro, dica, children }) {
  const ids = [dica ? `${id}-dica` : null, erro ? `${id}-erro` : null].filter(Boolean).join(' ')
  return (
    <div className="flex flex-col gap-1">
      <label htmlFor={id} className="text-gelo-50">
        {rotulo}
      </label>
      {children({
        id,
        'aria-invalid': erro ? 'true' : 'false',
        'aria-describedby': ids || undefined,
        className: CONTROLE,
      })}
      {dica ? (
        <p id={`${id}-dica`} className="text-sm text-gelo-300">
          {dica}
        </p>
      ) : null}
      {erro ? (
        <p id={`${id}-erro`} className="text-sm text-gelo-50">
          <span aria-hidden="true" className="mr-1 font-mono text-ataque">
            [erro]
          </span>
          {erro}
        </p>
      ) : null}
    </div>
  )
}
