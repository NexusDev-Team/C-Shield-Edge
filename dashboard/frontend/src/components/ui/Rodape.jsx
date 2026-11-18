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

import Logo from './Logo.jsx'

// Rodapé do dashboard: a promessa do produto, que vale também para esta tela (só leitura).
export default function Rodape() {
  return (
    <footer className="mt-20 border-t border-borda py-10">
      <div className="conteudo flex flex-wrap items-center justify-between gap-6">
        <Logo variante="compacta" />
        <p className="max-w-xl text-sm text-gelo-300">
          Alarmes avisam. O C-Shield age. Esta central só lê o que o C-Shield envia: nada aqui manda ordens
          a ele, e o bloqueio só é liberado no local, com um cartão cadastrado.
        </p>
      </div>
    </footer>
  )
}
