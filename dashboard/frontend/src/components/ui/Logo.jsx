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

import imagemLogo from '../../assets/logo-c-shield.png'
import { NOME_PRODUTO } from '../../domain/produto.js'

// Logo do C-Shield: o escudo com floco de neve (src/assets/logo-c-shield.png, a logo oficial) e o
// wordmark, que sai de NOME_PRODUTO. A imagem é decorativa (alt vazio): o nome vem escrito ao lado,
// e na variante 'escudo' (card de defesa do emulador) o texto vizinho já diz o que é.
// Variantes: 'completa' (escudo + nome + slogan), 'compacta' (escudo + nome) e 'escudo'.
function Escudo({ className = '' }) {
  return <img src={imagemLogo} alt="" width="280" height="250" className={className} />
}

export default function Logo({ variante = 'compacta', className = '' }) {
  if (variante === 'escudo') {
    return <Escudo className={`h-10 w-auto ${className}`} />
  }

  return (
    <span className={`inline-flex items-center gap-3 ${className}`}>
      <Escudo className="h-10 w-auto shrink-0" />
      <span className="flex flex-col leading-none">
        <span className="font-titulo text-xl font-semibold tracking-[0.06em] text-gelo-50 sm:text-2xl">
          {NOME_PRODUTO.toUpperCase()}
        </span>
        {variante === 'completa' && (
          <span className="mt-1.5 max-w-52 text-sm leading-snug text-gelo-300">
            Segurança inteligente para câmaras frias.
          </span>
        )}
      </span>
    </span>
  )
}
