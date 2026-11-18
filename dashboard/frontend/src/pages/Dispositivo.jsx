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

import { ArrowLeft } from 'lucide-react'
import { useCallback } from 'react'
import { Link, useParams } from 'react-router-dom'
import { api } from '../api/client.js'
import CartoesSensores from '../components/dispositivo/CartoesSensores.jsx'
import GraficoTermico from '../components/dispositivo/GraficoTermico.jsx'
import HistoricoCartoes from '../components/dispositivo/HistoricoCartoes.jsx'
import WidgetLeitura from '../components/dispositivo/WidgetLeitura.jsx'
import Carregando from '../components/ui/Carregando.jsx'
import ErroMensagem from '../components/ui/ErroMensagem.jsx'
import PainelVidro from '../components/ui/PainelVidro.jsx'
import TituloPagina from '../components/ui/TituloPagina.jsx'
import { EVENTOS_TABELA, LEITURA_POLL_MS, PONTOS_GRAFICO } from '../constants.js'
import { LOCAIS } from '../domain/locais.js'
import { situacaoDoDispositivo } from '../domain/estados.js'
import { PERFIS_FIRMWARE } from '../domain/perfis.js'
import { useLeitura } from '../hooks/useLeitura.js'

// Monitoramento de um C-Shield, no layout da seção "Monitoramento de temperatura" do site: card
// da temperatura atual à esquerda e histórico térmico à direita, agora com dados reais; abaixo,
// os números de apoio e o histórico de liberações por cartão NFC. Tudo atualiza a cada 5 s.
export default function Dispositivo() {
  const { id } = useParams()
  const carregarFrota = api.getFleet
  const carregarAtual = useCallback(() => api.getCurrent(id), [id])
  const carregarHistorico = useCallback(() => api.getHistory(id, { attr: 'temperature', lastN: PONTOS_GRAFICO }), [id])
  const carregarEventos = useCallback(() => api.getEventos(id, EVENTOS_TABELA), [id])

  const frota = useLeitura(carregarFrota, LEITURA_POLL_MS)
  const atual = useLeitura(carregarAtual, LEITURA_POLL_MS)
  const historico = useLeitura(carregarHistorico, LEITURA_POLL_MS)
  const eventos = useLeitura(carregarEventos, LEITURA_POLL_MS)

  const dispositivo = frota.dados?.dispositivos.find((d) => d.device_id === id)
  const local = LOCAIS.find((l) => l.id === dispositivo?.local_id)
  const perfil = PERFIS_FIRMWARE[dispositivo?.profile] ?? PERFIS_FIRMWARE.VACINA

  // Falha de leitura do cadastro (404 quando o id não existe) vale para a página inteira.
  if (atual.falha?.status === 404) {
    return (
      <div className="conteudo pt-12">
        <ErroMensagem mensagem={`O C-Shield ${id} não está cadastrado.`} />
      </div>
    )
  }

  const online = dispositivo?.status === 'online'
  const estado = situacaoDoDispositivo(dispositivo)
  const titulo = dispositivo?.name ?? local?.nome ?? id

  return (
    <>
      <TituloPagina
        rotulo={`Monitoramento · ${id}`}
        titulo={titulo}
        apoio="O que o aparelho mostra no local e envia à central. Nada aqui manda ordens a ele."
        acao={
          <Link to="/" className="inline-flex min-h-11 items-center gap-2 text-gelo-100 hover:text-gelo-50">
            <ArrowLeft aria-hidden="true" className="size-5" />
            Mapa da frota
          </Link>
        }
      />

      <div className="conteudo flex flex-col gap-6">
        {atual.erro && !atual.dados ? <ErroMensagem mensagem={atual.erro} aoTentarDeNovo={atual.recarregar} /> : null}
        {atual.carregando && !atual.dados ? <Carregando texto="Lendo o C-Shield na central..." /> : null}

        <div className="grid gap-6 lg:grid-cols-[1fr_2fr]">
          <WidgetLeitura
            titulo={titulo}
            temperatura={atual.dados?.temperature ?? null}
            estado={estado}
            motivo={dispositivo?.reason}
            online={online}
            perfil={perfil}
          />
          <PainelVidro as="section" aria-labelledby="titulo-historico" className="p-6 sm:p-8">
            <div className="flex items-start justify-between gap-3">
              <h2 id="titulo-historico" className="text-xl font-semibold text-gelo-50">
                Histórico térmico
              </h2>
              <p className="inline-flex items-center gap-2 rounded-full border border-borda px-3 py-1 font-mono text-xs tracking-wider text-gelo-50 uppercase">
                <span aria-hidden="true" className="size-2 rounded-full bg-ciano" />
                Ao vivo
              </p>
            </div>
            <div className="mt-6">
              {historico.erro && !historico.dados ? (
                <ErroMensagem mensagem={historico.erro} aoTentarDeNovo={historico.recarregar} />
              ) : (
                <GraficoTermico
                  pontos={historico.dados ?? []}
                  titulo={`Histórico térmico de ${titulo}`}
                  limite={{ valor: perfil.tMax, rotulo: `limite ${perfil.tMax.toLocaleString('pt-BR', { minimumFractionDigits: 1 })} °C` }}
                />
              )}
            </div>
          </PainelVidro>
        </div>

        <CartoesSensores atual={atual.dados} />

        {eventos.erro && !eventos.dados ? (
          <ErroMensagem mensagem={eventos.erro} aoTentarDeNovo={eventos.recarregar} />
        ) : (
          <HistoricoCartoes eventos={eventos.dados ?? []} />
        )}
      </div>
    </>
  )
}
