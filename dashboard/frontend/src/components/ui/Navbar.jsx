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

import { useState } from 'react'
import { Menu, X } from 'lucide-react'
import { NavLink, Link } from 'react-router-dom'
import Logo from './Logo.jsx'

// Navegação do dashboard: mesma barra do site (80 px, links em texto, rota atual em ciano com
// sublinhado). No celular vira um botão de menu (aria-expanded). Só quatro destinos: o mapa da
// frota, o cadastro dos C-Shields, a saúde da EC2 e o site do projeto.
const LINKS = [
  { para: '/', rotulo: 'Mapa da frota', fim: true },
  { para: '/dispositivos', rotulo: 'C-Shields' },
  { para: '/avancado', rotulo: 'Avançado' },
]

function classeDoLink({ isActive }) {
  const base =
    'relative flex min-h-11 w-fit items-center px-1 text-base font-medium transition-colors duration-300 ease-suave after:absolute after:inset-x-0 after:bottom-1.5 after:h-0.5 after:rounded-full after:transition-colors'
  return isActive
    ? `${base} text-ciano after:bg-ciano`
    : `${base} text-gelo-100 after:bg-transparent hover:text-gelo-50 hover:after:bg-gelo-azul/40`
}

export default function Navbar({ urlSite }) {
  const [aberto, setAberto] = useState(false)
  const fechar = () => setAberto(false)

  return (
    <header className="sticky top-0 z-40 bg-noite-950/90">
      <div
        aria-hidden="true"
        className="pointer-events-none absolute inset-x-0 top-full h-10 bg-linear-to-b from-noite-950/90 to-transparent"
      />
      <nav
        aria-label="Principal"
        className="conteudo flex min-h-20 flex-wrap items-center justify-between gap-x-6 gap-y-2 py-2"
      >
        <Link to="/" aria-label="C-Shield Central, mapa da frota" className="rounded-xl" onClick={fechar}>
          <Logo variante="compacta" />
        </Link>

        <button
          type="button"
          aria-expanded={aberto}
          aria-controls="menu-principal"
          aria-label={aberto ? 'Fechar menu' : 'Abrir menu'}
          onClick={() => setAberto((valor) => !valor)}
          className="inline-flex min-h-11 min-w-11 cursor-pointer items-center justify-center rounded-full border border-borda text-gelo-50 transition-colors duration-300 hover:border-gelo-azul md:hidden"
        >
          {aberto ? <X aria-hidden="true" className="size-5" /> : <Menu aria-hidden="true" className="size-5" />}
        </button>

        <ul
          id="menu-principal"
          className={`${aberto ? 'flex' : 'hidden'} w-full flex-col gap-1 pb-3 md:flex md:w-auto md:flex-row md:items-center md:gap-8 md:pb-0`}
        >
          {LINKS.map((link) => (
            <li key={link.para}>
              <NavLink to={link.para} end={link.fim} className={classeDoLink} onClick={fechar}>
                {link.rotulo}
              </NavLink>
            </li>
          ))}
          {urlSite ? (
            <li>
              <a
                href={urlSite}
                className="relative flex min-h-11 w-fit items-center px-1 text-base font-medium text-gelo-100 transition-colors duration-300 hover:text-gelo-50"
              >
                Site do projeto
              </a>
            </li>
          ) : null}
        </ul>
      </nav>
    </header>
  )
}
