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

import { Link } from 'react-router-dom'
import BotaoLed from '../components/ui/BotaoLed.jsx'
import TituloPagina from '../components/ui/TituloPagina.jsx'

// Rota que não existe: diz o que houve e leva de volta ao mapa da frota.
export default function NaoEncontrada() {
  return (
    <TituloPagina
      rotulo="404"
      titulo="Página não encontrada"
      apoio="Este endereço não existe na central."
      acao={<BotaoLed as={Link} to="/" seta>Voltar ao mapa</BotaoLed>}
    />
  )
}
