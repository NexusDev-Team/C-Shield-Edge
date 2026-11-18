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

// Cabeçalho de página no estilo do site: rótulo técnico em caixa alta espaçada, título grande e
// uma linha de apoio. `acao` abre espaço para um botão ou link à direita.
export default function TituloPagina({ rotulo, titulo, apoio, acao = null }) {
  return (
    <header className="conteudo flex flex-wrap items-end justify-between gap-x-6 gap-y-4 pt-8 pb-8 sm:pt-12">
      <div className="max-w-3xl">
        <p className="rotulo-tecnico">{rotulo}</p>
        <h1 className="mt-3 text-4xl leading-tight font-semibold tracking-tight text-gelo-50 sm:text-5xl">{titulo}</h1>
        {apoio ? <p className="mt-4 text-lg text-gelo-100">{apoio}</p> : null}
      </div>
      {acao}
    </header>
  )
}
