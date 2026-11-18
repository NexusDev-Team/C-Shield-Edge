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

// Card de vidro da leitura atual (adaptado de inicio/WidgetLeitura do site): "Temperatura atual",
// a pílula ONLINE/OFFLINE, o número grande em fonte de instrumento e o estado que o PRÓPRIO
// C-Shield decidiu (o dashboard não calcula nada). O valor muda a cada poucos segundos, então
// não é anunciado por leitor de tela (aria-live desligado). Os dados são reais, do FIWARE.
import { ESTADOS, rotuloMotivo } from '../../domain/estados.js'
import { faixaParaMostrar } from '../../domain/perfis.js'
import PainelVidro from '../ui/PainelVidro.jsx'
import { formatarGraus } from '../ui/formato.js'

function textoFaixa(perfil) {
  const { minimo, maximo } = faixaParaMostrar(perfil)
  return minimo === null
    ? `até ${formatarGraus(maximo)} °C`
    : `${formatarGraus(minimo)} °C a ${formatarGraus(maximo)} °C`
}

export default function WidgetLeitura({ titulo, temperatura, estado, motivo, online, perfil }) {
  const info = ESTADOS[estado]
  const motivoTexto = rotuloMotivo(motivo)
  return (
    <PainelVidro as="article" className="flex h-full flex-col justify-between gap-8 p-6 sm:p-8">
      <div className="flex items-start justify-between gap-3">
        <div>
          <p className="rotulo-tecnico">Temperatura atual</p>
          <h3 className="mt-2 text-lg font-semibold text-gelo-50">{titulo}</h3>
        </div>
        {online ? (
          <p className="inline-flex items-center gap-2 rounded-full border border-ok/40 bg-ok/10 px-3 py-1 font-mono text-xs tracking-wider text-ok uppercase">
            <span aria-hidden="true" className="faisca-pulsa size-2 rounded-full bg-ok" />
            Online
          </p>
        ) : (
          <p className="inline-flex items-center gap-2 rounded-full border border-borda px-3 py-1 font-mono text-xs tracking-wider text-gelo-300 uppercase">
            <span aria-hidden="true" className="size-2 rounded-full bg-texto-mudo" />
            Offline
          </p>
        )}
      </div>

      <div className="flex flex-col items-start gap-5">
        <p className="font-titulo leading-none font-semibold tracking-tight text-gelo-50 tabular-nums" aria-live="off">
          <span className="text-6xl sm:text-7xl">{typeof temperatura === 'number' ? formatarGraus(temperatura) : '--'}</span>
          <span className="ml-2 text-3xl text-gelo-300">°C</span>
        </p>
        {estado === 'NORMAL' ? (
          <p className="inline-flex items-center rounded-full bg-menta px-5 py-2 text-base font-semibold text-menta-texto">
            Estado: normal
          </p>
        ) : (
          <p
            className="inline-flex items-center gap-2 rounded-full border px-5 py-2 text-base font-semibold text-gelo-50"
            style={{ borderColor: info.cor }}
          >
            <span aria-hidden="true" className="size-2.5 rounded-full" style={{ backgroundColor: info.cor }} />
            Estado: {info.rotulo.toLowerCase()}
          </p>
        )}
        {motivoTexto && online ? <p className="text-sm text-gelo-100">Motivo: {motivoTexto}</p> : null}
      </div>

      <div className="flex flex-col gap-1 text-sm text-gelo-300">
        <p>Faixa segura: {textoFaixa(perfil)} · gravada no dispositivo</p>
        <p>Dados reais do FIWARE · atualiza a cada 5 s.</p>
      </div>
    </PainelVidro>
  )
}
