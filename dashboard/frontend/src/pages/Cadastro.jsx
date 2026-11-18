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
import { Link, useSearchParams } from 'react-router-dom'
import { api } from '../api/client.js'
import BotaoLed from '../components/ui/BotaoLed.jsx'
import CampoFormulario from '../components/ui/CampoFormulario.jsx'
import Carregando from '../components/ui/Carregando.jsx'
import ConfirmacaoInline from '../components/ui/ConfirmacaoInline.jsx'
import ErroMensagem from '../components/ui/ErroMensagem.jsx'
import PainelVidro from '../components/ui/PainelVidro.jsx'
import TituloPagina from '../components/ui/TituloPagina.jsx'
import { LOCAIS, locaisLivres } from '../domain/locais.js'
import { PERFIS_FIRMWARE } from '../domain/perfis.js'
import { useLeitura } from '../hooks/useLeitura.js'

// Cadastro dos C-Shields: o formulário provisiona o device no FIWARE (IoT Agent, Orion e as
// subscriptions do STH-Comet) e o coloca num ponto livre do mapa. O id precisa ser o ID_DEVICE
// do firmware (cshield001...). Remover apaga o device e a entidade; o histórico do STH fica.
const nomeDoLocal = (id) => LOCAIS.find((l) => l.id === id)?.nome ?? id

export default function Cadastro() {
  const [busca] = useSearchParams()
  const { dados: cadastrados, erro, carregando, recarregar } = useLeitura(api.listDevices)
  const livres = locaisLivres(cadastrados)

  const [deviceId, setDeviceId] = useState('')
  const [nome, setNome] = useState('')
  const [localEscolhido, setLocalEscolhido] = useState(busca.get('local') ?? '')
  const [perfil, setPerfil] = useState('')
  const [erroForm, setErroForm] = useState('')
  const [salvando, setSalvando] = useState(false)
  const [removendo, setRemovendo] = useState(null)
  const [ocupadoRemover, setOcupadoRemover] = useState(false)
  const [erroRemover, setErroRemover] = useState('')

  // Local escolhido que continua livre; senão, o primeiro livre. O perfil acompanha o local
  // até a pessoa escolher outro.
  const local = livres.find((l) => l.id === localEscolhido) ?? livres[0]
  const perfilAtual = perfil || local?.perfilSugerido || 'VACINA'

  async function cadastrar(evento) {
    evento.preventDefault()
    setErroForm('')
    setSalvando(true)
    try {
      await api.createDevice({ device_id: deviceId.trim(), name: nome.trim() || local.nome, local_id: local.id, profile: perfilAtual })
      setDeviceId('')
      setNome('')
      setPerfil('')
      recarregar()
    } catch (falha) {
      setErroForm(falha.message)
    } finally {
      setSalvando(false)
    }
  }

  async function remover(id) {
    setErroRemover('')
    setOcupadoRemover(true)
    try {
      await api.deleteDevice(id)
      setRemovendo(null)
      recarregar()
    } catch (falha) {
      setErroRemover(falha.message)
    } finally {
      setOcupadoRemover(false)
    }
  }

  return (
    <>
      <TituloPagina
        rotulo="Cadastro"
        titulo="C-Shields da frota"
        apoio="Cadastre cada aparelho num ponto do mapa. O id precisa ser o mesmo que o firmware usa para publicar."
      />
      <div className="conteudo grid gap-6 lg:grid-cols-[1fr_1.2fr]">
        <PainelVidro as="section" aria-labelledby="titulo-novo" className="p-6 sm:p-8">
          <h2 id="titulo-novo" className="text-xl font-semibold text-gelo-50">
            Novo C-Shield
          </h2>
          {livres.length === 0 && cadastrados ? (
            <p className="mt-4 text-gelo-100">Todos os pontos do mapa já têm um C-Shield. Remova um para cadastrar outro.</p>
          ) : (
            <form onSubmit={cadastrar} className="mt-6 flex flex-col gap-5" noValidate>
              <CampoFormulario id="device-id" rotulo="Id do dispositivo" dica="Três dígitos, como no firmware: cshield001, cshield002..." erro={erroForm.startsWith('device_id') ? erroForm : ''}>
                {(p) => (
                  <input {...p} value={deviceId} onChange={(e) => setDeviceId(e.target.value)} placeholder="cshield001" autoComplete="off" required />
                )}
              </CampoFormulario>
              <CampoFormulario id="nome" rotulo="Nome" dica={`Em branco usa o nome do local: ${local?.nome ?? ''}.`}>
                {(p) => <input {...p} value={nome} onChange={(e) => setNome(e.target.value)} maxLength={60} />}
              </CampoFormulario>
              <CampoFormulario id="local" rotulo="Ponto do mapa">
                {(p) => (
                  <select {...p} value={local?.id ?? ''} onChange={(e) => { setLocalEscolhido(e.target.value); setPerfil('') }}>
                    {livres.map((l) => (
                      <option key={l.id} value={l.id}>{l.nome}</option>
                    ))}
                  </select>
                )}
              </CampoFormulario>
              <CampoFormulario id="perfil" rotulo="Produto guardado" dica="Define a faixa segura mostrada no gráfico; a decisão em si vem do firmware.">
                {(p) => (
                  <select {...p} value={perfilAtual} onChange={(e) => setPerfil(e.target.value)}>
                    {Object.values(PERFIS_FIRMWARE).map((pf) => (
                      <option key={pf.id} value={pf.id}>{pf.nome}</option>
                    ))}
                  </select>
                )}
              </CampoFormulario>
              {erroForm && !erroForm.startsWith('device_id') ? <ErroMensagem mensagem={erroForm} /> : null}
              <BotaoLed type="submit" disabled={salvando || !local || !deviceId.trim()}>
                {salvando ? 'Provisionando no FIWARE...' : 'Cadastrar C-Shield'}
              </BotaoLed>
            </form>
          )}
        </PainelVidro>

        <PainelVidro as="section" aria-labelledby="titulo-lista" className="p-6 sm:p-8">
          <h2 id="titulo-lista" className="text-xl font-semibold text-gelo-50">
            Cadastrados
          </h2>
          {carregando && !cadastrados ? <div className="mt-4"><Carregando /></div> : null}
          {erro && !cadastrados ? <div className="mt-4"><ErroMensagem mensagem={erro} aoTentarDeNovo={recarregar} /></div> : null}
          {cadastrados?.length === 0 ? <p className="mt-4 text-gelo-100">Nenhum C-Shield cadastrado ainda.</p> : null}
          <ul className="mt-4 flex flex-col gap-3">
            {(cadastrados ?? []).map((d) => (
              <li key={d.device_id} className="rounded-xl border border-borda bg-noite-900 p-4">
                <div className="flex flex-wrap items-center justify-between gap-3">
                  <div>
                    <p className="font-semibold text-gelo-50">{d.name}</p>
                    <p className="text-sm text-gelo-300">
                      <span className="font-mono">{d.device_id}</span> · {nomeDoLocal(d.local_id)} · {PERFIS_FIRMWARE[d.profile]?.nome}
                    </p>
                  </div>
                  <div className="flex flex-wrap gap-2">
                    <BotaoLed as={Link} to={`/dispositivo/${d.device_id}`} variante="fantasma">
                      Abrir
                    </BotaoLed>
                    <BotaoLed variante="ataque" onClick={() => { setErroRemover(''); setRemovendo(d.device_id) }}>
                      Remover
                    </BotaoLed>
                  </div>
                </div>
                {removendo === d.device_id ? (
                  <div className="mt-3">
                    <ConfirmacaoInline
                      mensagem={`Remover ${d.name} do FIWARE? O histórico no STH-Comet permanece.`}
                      rotuloConfirmar="Sim, remover"
                      ocupado={ocupadoRemover}
                      erro={erroRemover}
                      aoConfirmar={() => remover(d.device_id)}
                      aoCancelar={() => setRemovendo(null)}
                    />
                  </div>
                ) : null}
              </li>
            ))}
          </ul>
        </PainelVidro>
      </div>
    </>
  )
}
