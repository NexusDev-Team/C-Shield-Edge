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

import { ArrowRight } from 'lucide-react'

// Botão (ou link, com as=...) em pílula, com alvo de toque de pelo menos 44 x 44 px (W48).
// `ariaLabel` serve aos textos em leet (4T4QU3 = "Ataque").
// `seta` põe a seta no fim do texto ("Simular ataque →"); `icone` põe um ícone antes do texto
// (no secundário ele ganha um círculo, como o "Como funciona" do hero). Hover: brilho e 2 px para
// cima, só com movimento permitido.
const VARIANTES = {
  primario:
    'border-ciano bg-ciano/10 text-gelo-50 hover:bg-ciano/20 hover:shadow-glow-ciano',
  ataque:
    'border-critico bg-transparent text-gelo-50 hover:bg-critico/15',
  fantasma:
    'border-borda bg-transparent text-gelo-50 hover:border-gelo-azul hover:bg-superficie',
  secundario:
    'border-transparent bg-transparent px-2! text-gelo-300 hover:text-gelo-50',
}

export default function BotaoLed({
  as: Tag = 'button',
  variante = 'primario',
  ariaLabel,
  type = 'button',
  seta = false,
  icone = null,
  className = '',
  children,
  ...resto
}) {
  const circulo = variante === 'secundario'
  return (
    <Tag
      type={Tag === 'button' ? type : undefined}
      aria-label={ariaLabel}
      className={`inline-flex min-h-12 min-w-11 cursor-pointer items-center justify-center gap-3 rounded-full border-[1.5px] px-6 py-2 text-base font-semibold transition duration-300 ease-suave motion-safe:hover:-translate-y-0.5 disabled:cursor-not-allowed disabled:opacity-50 disabled:hover:translate-y-0 ${VARIANTES[variante]} ${className}`}
      {...resto}
    >
      {icone ? (
        <span
          aria-hidden="true"
          className={circulo ? 'inline-flex size-10 items-center justify-center rounded-full border border-gelo-300' : 'inline-flex'}
        >
          {icone}
        </span>
      ) : null}
      {children}
      {seta ? <ArrowRight aria-hidden="true" className="size-5 shrink-0" /> : null}
    </Tag>
  )
}
