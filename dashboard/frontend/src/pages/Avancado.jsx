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
import { api } from '../api/client.js'
import BotaoLed from '../components/ui/BotaoLed.jsx'
import CampoFormulario from '../components/ui/CampoFormulario.jsx'
import Carregando from '../components/ui/Carregando.jsx'
import ErroMensagem from '../components/ui/ErroMensagem.jsx'
import PainelVidro from '../components/ui/PainelVidro.jsx'
import TituloPagina from '../components/ui/TituloPagina.jsx'
import { HEALTH_POLL_MS } from '../constants.js'
import { useLeitura } from '../hooks/useLeitura.js'

// Avançado (copiado em espírito da Vinheria): o IP da EC2, que muda quando a instância é
// reiniciada sem IP elástico, e a saúde dos três componentes do FIWARE. O IP salvo vale na hora
// e sobrevive ao reinício do backend.
function Componente({ nome, saude }) {
  const ok = saude?.ok
  return (
    <li className="flex items-center justify-between gap-3 rounded-lg border border-borda bg-noite-900 px-4 py-3">
      <span className="text-gelo-50">{nome}</span>
      <span className="flex items-center gap-2 text-sm text-gelo-100">
        <span aria-hidden="true" className="size-2.5 rounded-full" style={{ backgroundColor: ok ? 'var(--color-ok)' : 'var(--color-critico)' }} />
        {ok ? `no ar · ${saude.latency_ms} ms` : saude?.error ?? 'sem resposta'}
      </span>
    </li>
  )
}

export default function Avancado() {
  const config = useLeitura(api.getConfig)
  const saude = useLeitura(api.getHealth, HEALTH_POLL_MS)
  const [ip, setIp] = useState(null)
  const [erro, setErro] = useState('')
  const [salvando, setSalvando] = useState(false)

  const ipAtual = ip ?? config.dados?.ec2_ip ?? ''

  async function salvar(evento) {
    evento.preventDefault()
    setErro('')
    setSalvando(true)
    try {
      await api.saveConfig({ ec2_ip: ipAtual })
      setIp(null)
      config.recarregar()
      saude.recarregar()
    } catch (falha) {
      setErro(falha.message)
    } finally {
      setSalvando(false)
    }
  }

  return (
    <>
      <TituloPagina
        rotulo="Avançado"
        titulo="Central FIWARE"
        apoio="Endereço da EC2 e saúde do Orion, do IoT Agent e do STH-Comet."
      />
      <div className="conteudo grid gap-6 lg:grid-cols-2">
        <PainelVidro as="section" aria-labelledby="titulo-ip" className="p-6 sm:p-8">
          <h2 id="titulo-ip" className="text-xl font-semibold text-gelo-50">
            Endereço da EC2
          </h2>
          {config.carregando && !config.dados ? <div className="mt-4"><Carregando /></div> : null}
          {config.erro && !config.dados ? <div className="mt-4"><ErroMensagem mensagem={config.erro} aoTentarDeNovo={config.recarregar} /></div> : null}
          <form onSubmit={salvar} className="mt-6 flex flex-col gap-5" noValidate>
            <CampoFormulario id="ec2-ip" rotulo="IP da EC2" dica="Só o endereço, sem http:// nem porta." erro={erro}>
              {(p) => <input {...p} value={ipAtual} onChange={(e) => setIp(e.target.value)} autoComplete="off" />}
            </CampoFormulario>
            <BotaoLed type="submit" disabled={salvando || !ipAtual.trim()}>
              {salvando ? 'Salvando...' : 'Salvar endereço'}
            </BotaoLed>
          </form>
        </PainelVidro>

        <PainelVidro as="section" aria-labelledby="titulo-saude" className="p-6 sm:p-8">
          <h2 id="titulo-saude" className="text-xl font-semibold text-gelo-50">
            Saúde dos componentes
          </h2>
          {saude.carregando && !saude.dados ? <div className="mt-4"><Carregando texto="Testando a EC2..." /></div> : null}
          {saude.erro && !saude.dados ? <div className="mt-4"><ErroMensagem mensagem={saude.erro} aoTentarDeNovo={saude.recarregar} /></div> : null}
          {saude.dados ? (
            <ul className="mt-6 flex flex-col gap-3">
              <Componente nome="Orion (contexto, 1026)" saude={saude.dados.orion} />
              <Componente nome="IoT Agent MQTT (4041)" saude={saude.dados.iota} />
              <Componente nome="STH-Comet (histórico, 8666)" saude={saude.dados.sth} />
            </ul>
          ) : null}
        </PainelVidro>
      </div>
    </>
  )
}
