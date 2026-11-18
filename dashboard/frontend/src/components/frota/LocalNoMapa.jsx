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

// Um ponto da frota no mapa (adaptado de emulador/LocalNoMapa do site): o círculo com o ícone do
// que o local guarda, o escudo azul se há C-Shield online, a temperatura REAL e o estado em
// texto. Ponto sem C-Shield aparece apagado; offline, em cinza. É um botão de verdade
// (role="button", Tab, Enter e Espaço) e a cor sempre vem acompanhada de texto (W49).
import { RAIO_ESCUDO, RAIO_LOCAL } from '../emulador/geometria.js'
import { ESTADOS, situacaoDoDispositivo } from '../../domain/estados.js'
import { formatarGraus as graus } from '../ui/formato.js'

// Ícone de cada tipo de local, desenhado em torno de (0, 0), com cerca de 30 de largura.
const ICONES = {
  VACINA: <path d="M-5 -14 H5 M-3 -14 V-8 M3 -14 V-8 M-8 -8 H8 V12 Q8 16 4 16 H-4 Q-8 16 -8 12 Z M-8 0 H8" />,
  INSULINA: <path d="M-5 -14 H5 V-5 H14 V5 H5 V14 H-5 V5 H-14 V-5 H-5 Z" />,
  PLASMA: <path d="M0 -16 C10 -3 12 3 12 7 A12 12 0 0 1 -12 7 C-12 3 -10 -3 0 -16 Z" />,
  CONGELADO: (
    <path d="M-15 -12 H-9 L-5 8 H11 L14 -6 H-7 M-3 14 A3 3 0 1 0 -3 14.01 M9 14 A3 3 0 1 0 9 14.01" />
  ),
}

const HALO = { paintOrder: 'stroke', stroke: 'var(--color-noite-950)', strokeWidth: 6, strokeLinejoin: 'round' }

// Temperatura do ponto: "--" quando não há leitura (sem C-Shield, offline ou sensor sem dado).
export function textoTemperatura(dispositivo) {
  const t = dispositivo?.current?.temperature
  return typeof t === 'number' ? `${graus(t)} °C` : '-- °C'
}

export default function LocalNoMapa({ def, dispositivo, selecionado, aoSelecionar }) {
  const situacao = situacaoDoDispositivo(dispositivo)
  const { rotulo, cor } = ESTADOS[situacao]
  const vazio = !dispositivo
  const online = dispositivo?.status === 'online'
  const critico = situacao === 'CRITICO'
  const preenchimento = critico
    ? 'color-mix(in srgb, var(--color-critico) 25%, var(--color-noite-900))'
    : 'var(--color-noite-900)'
  const temperatura = textoTemperatura(dispositivo)

  function aoTeclar(e) {
    if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault()
      aoSelecionar(def.id)
    }
  }

  return (
    <g
      role="button"
      tabIndex={0}
      aria-pressed={selecionado}
      aria-label={`${def.nome}, ${rotulo.toLowerCase()}, ${vazio ? 'sem leitura' : temperatura}`}
      transform={`translate(${def.x} ${def.y})`}
      onClick={() => aoSelecionar(def.id)}
      onKeyDown={aoTeclar}
      className="group cursor-pointer outline-none"
      opacity={vazio ? 0.55 : 1}
    >
      {/* Área de toque grande e invisível */}
      <circle r={RAIO_ESCUDO + 12} fill="transparent" />
      {selecionado ? (
        <circle r={RAIO_ESCUDO + 10} fill="none" stroke="var(--color-gelo-50)" strokeWidth={2} strokeDasharray="6 6" />
      ) : null}
      {/* Anel redondo só para quem navega pelo teclado (no lugar do quadrado de foco do navegador) */}
      <circle
        r={RAIO_ESCUDO + 10}
        fill="none"
        stroke="var(--color-gelo-50)"
        strokeWidth={3}
        className="opacity-0 group-focus-visible:opacity-100"
      />
      {online ? (
        <circle
          r={RAIO_ESCUDO}
          fill="none"
          stroke="var(--color-led-forte)"
          strokeWidth={4}
          style={{ filter: 'drop-shadow(0 0 8px var(--color-led-forte))' }}
        />
      ) : null}
      <g className={critico ? 'led-falha' : undefined}>
        <circle r={RAIO_LOCAL} fill={preenchimento} stroke={cor} strokeWidth={4} strokeDasharray={vazio ? '6 6' : undefined} />
        <g fill="none" stroke="var(--color-gelo-50)" strokeWidth={3} strokeLinecap="round" strokeLinejoin="round">
          {ICONES[def.icone]}
        </g>
      </g>
      {/* No celular os rótulos ficariam pequenos demais: quem mostra o texto é a ListaLocais */}
      <g textAnchor="middle" style={{ pointerEvents: 'none' }} className="max-sm:hidden">
        <text
          y={RAIO_ESCUDO + 30}
          fontSize={28}
          fontWeight={600}
          fill="var(--color-gelo-50)"
          textDecoration={selecionado ? 'underline' : undefined}
          style={HALO}
        >
          {def.bairro}
        </text>
        <text y={RAIO_ESCUDO + 60} fontSize={28} fill="var(--color-gelo-50)" fontFamily="var(--font-mono)" style={HALO}>
          {temperatura}
        </text>
        <text y={RAIO_ESCUDO + 88} fontSize={24} fill="var(--color-gelo-50)" style={HALO}>
          <tspan fill={cor}>● </tspan>
          {rotulo}
        </text>
      </g>
    </g>
  )
}
