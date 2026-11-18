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

// Rotas do dashboard. Navbar e rodapé vêm de components/ui; a atmosfera é a do site.
import { Route, Routes } from 'react-router-dom'
import FundoAtmosfera from './components/ui/FundoAtmosfera.jsx'
import Navbar from './components/ui/Navbar.jsx'
import Rodape from './components/ui/Rodape.jsx'
import Avancado from './pages/Avancado.jsx'
import Cadastro from './pages/Cadastro.jsx'
import Dispositivo from './pages/Dispositivo.jsx'
import Frota from './pages/Frota.jsx'
import NaoEncontrada from './pages/NaoEncontrada.jsx'

// Endereço do site do projeto (opcional): VITE_URL_SITE no .env do build.
const URL_SITE = import.meta.env.VITE_URL_SITE || ''

export default function App() {
  return (
    <>
      <FundoAtmosfera />
      <a
        href="#conteudo"
        className="sr-only focus:not-sr-only focus:fixed focus:left-4 focus:top-4 focus:z-50 focus:rounded-lg focus:bg-led-forte focus:px-4 focus:py-2 focus:text-noite-950"
      >
        Pular para o conteúdo
      </a>
      <Navbar urlSite={URL_SITE} />
      <main id="conteudo" tabIndex={-1} className="outline-none">
        <Routes>
          <Route path="/" element={<Frota />} />
          <Route path="/dispositivo/:id" element={<Dispositivo />} />
          <Route path="/dispositivos" element={<Cadastro />} />
          <Route path="/avancado" element={<Avancado />} />
          <Route path="*" element={<NaoEncontrada />} />
        </Routes>
      </main>
      <Rodape />
    </>
  )
}
