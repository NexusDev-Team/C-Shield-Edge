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
import { useNavigate } from 'react-router-dom'
import { api } from '../api/client.js'
import MapaFrota from '../components/frota/MapaFrota.jsx'
import Carregando from '../components/ui/Carregando.jsx'
import ErroMensagem from '../components/ui/ErroMensagem.jsx'
import PainelVidro from '../components/ui/PainelVidro.jsx'
import TituloPagina from '../components/ui/TituloPagina.jsx'
import { FLEET_POLL_MS } from '../constants.js'
import { dispositivosPorLocal } from '../domain/locais.js'
import { useLeitura } from '../hooks/useLeitura.js'
import { usePrefereMenosMovimento } from '../hooks/usePrefereMenosMovimento.js'

// Página inicial: o mapa com todos os C-Shields cadastrados, no estilo do mapa do emulador do
// site, mas com temperatura e estado REAIS (/api/fleet, atualizado a cada 5 s). Clicar num ponto
// com C-Shield abre o monitoramento dele; num ponto vazio, leva ao cadastro já naquele local.
function Contador({ rotulo, valor }) {
  return (
    <div className="flex flex-col">
      <dt className="rotulo-tecnico">{rotulo}</dt>
      <dd className="mt-1 font-titulo text-3xl font-semibold text-gelo-50 tabular-nums">{valor}</dd>
    </div>
  )
}

export default function Frota() {
  const navegar = useNavigate()
  const reduzirMovimento = usePrefereMenosMovimento()
  const [selecionadoId, setSelecionadoId] = useState(null)
  const { dados: frota, erro, carregando, recarregar } = useLeitura(api.getFleet, FLEET_POLL_MS)

  const dispositivos = frota?.dispositivos ?? []
  const porLocal = dispositivosPorLocal(dispositivos)
  const online = dispositivos.filter((d) => d.status === 'online').length
  const emBloqueio = dispositivos.filter((d) => d.status === 'online' && d.state === 'BLOQUEIO').length
  const criticos = dispositivos.filter((d) => d.status === 'online' && d.state === 'CRITICO').length

  function aoSelecionar(localId) {
    setSelecionadoId(localId)
    const dispositivo = porLocal[localId]
    navegar(dispositivo ? `/dispositivo/${dispositivo.device_id}` : `/dispositivos?local=${localId}`)
  }

  return (
    <>
      <TituloPagina
        rotulo="Frota"
        titulo="Todos os C-Shields"
        apoio="Cada ponto do mapa mostra a temperatura e o estado que o aparelho enviou à central. Nada aqui manda ordens a ele."
      />
      <div className="conteudo flex flex-col gap-6">
        {erro && !frota ? <ErroMensagem mensagem={erro} aoTentarDeNovo={recarregar} /> : null}
        {frota && !frota.fiware_online ? (
          <p role="status" className="rounded-xl border border-atencao/50 bg-noite-900 p-4 text-gelo-50">
            A central FIWARE (EC2) não está respondendo. Os valores abaixo são os últimos conhecidos.
          </p>
        ) : null}

        <PainelVidro as="section" aria-label="Resumo da frota" className="p-6">
          <dl className="grid grid-cols-2 gap-6 sm:grid-cols-4">
            <Contador rotulo="Cadastrados" valor={frota ? dispositivos.length : '--'} />
            <Contador rotulo="Online" valor={frota ? online : '--'} />
            <Contador rotulo="Em bloqueio" valor={frota ? emBloqueio : '--'} />
            <Contador rotulo="Críticos" valor={frota ? criticos : '--'} />
          </dl>
        </PainelVidro>

        <PainelVidro as="section" aria-label="Mapa da frota" className="p-3 sm:p-5">
          {carregando && !frota ? <Carregando texto="Lendo a frota na central..." /> : null}
          <MapaFrota
            porLocal={porLocal}
            selecionadoId={selecionadoId}
            aoSelecionar={aoSelecionar}
            reduzirMovimento={reduzirMovimento}
          />
        </PainelVidro>
      </div>
    </>
  )
}
