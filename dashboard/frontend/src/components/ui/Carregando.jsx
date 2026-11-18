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

// Aviso de carregamento. role="status" anuncia a espera a leitores de tela.
// O giro do ícone é lento e contínuo (sem piscar) e para com prefers-reduced-motion.
export default function Carregando({ texto = 'Carregando...' }) {
  return (
    <p role="status" className="flex items-center gap-3 text-gelo-300">
      <svg
        aria-hidden="true"
        viewBox="0 0 24 24"
        className="gira size-5 text-led-forte"
        fill="none"
        stroke="currentColor"
        strokeWidth="2.5"
        strokeLinecap="round"
      >
        <path d="M12 3a9 9 0 1 0 9 9" />
      </svg>
      {texto}
    </p>
  )
}
