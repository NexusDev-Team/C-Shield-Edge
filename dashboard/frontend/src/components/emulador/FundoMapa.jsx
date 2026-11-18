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

import { useId } from 'react'

// O fundo do mapa da cidade: quarteirões separados por ruas, uma avenida na diagonal, um rio e uma
// praça, tudo em tons escuros e com pouco contraste, só para parecer um mapa de verdade atrás dos
// locais. Desenha no mesmo sistema de coordenadas do MapaCidade (a cidade vai de x 30 a 970 e de
// y 96 a 580) e é puramente decorativo (aria-hidden). Os números são fixos, sem sorteio: o mapa
// é igual em toda visita.
const CIDADE = { x: 30, y: 96, largura: 940, altura: 484, raio: 28 }
// Linhas das ruas: os quarteirões ficam entre uma linha e a seguinte, com 9 px de calçada de cada lado.
const COLUNAS = [30, 170, 310, 430, 580, 700, 830, 970]
const LINHAS = [96, 200, 300, 400, 500, 580]
const CALCADA = 9
// Quarteirões que viram praça (índices coluna-linha).
const PRACAS = new Set(['3-1', '6-3'])

function quarteiroes() {
  const lista = []
  for (let c = 0; c < COLUNAS.length - 1; c += 1) {
    for (let l = 0; l < LINHAS.length - 1; l += 1) {
      lista.push({
        chave: `${c}-${l}`,
        x: COLUNAS[c] + CALCADA,
        y: LINHAS[l] + CALCADA,
        largura: COLUNAS[c + 1] - COLUNAS[c] - CALCADA * 2,
        altura: LINHAS[l + 1] - LINHAS[l] - CALCADA * 2,
        // Três tons de quarteirão, em padrão fixo, para a grade não parecer carimbada.
        tom: (c * 7 + l * 3) % 3,
      })
    }
  }
  return lista
}

const BLOCOS = quarteiroes()
const OPACIDADE_BLOCO = [0.5, 0.7, 0.35]
const RIO = 'M30 540 C 220 500, 330 590, 520 540 S 800 470, 970 500'
const AVENIDA = 'M30 330 L 970 150'

export default function FundoMapa() {
  const idCorte = `corte-${useId().replace(/:/g, '')}`
  return (
    <g aria-hidden="true">
      <defs>
        <clipPath id={idCorte}>
          <rect x={CIDADE.x} y={CIDADE.y} width={CIDADE.largura} height={CIDADE.altura} rx={CIDADE.raio} />
        </clipPath>
      </defs>

      <g clipPath={`url(#${idCorte})`}>
        {/* O chão: as ruas são o que sobra entre os quarteirões */}
        <rect x={CIDADE.x} y={CIDADE.y} width={CIDADE.largura} height={CIDADE.altura} fill="var(--color-noite-950)" />

        {BLOCOS.map((b) => (
          <rect
            key={b.chave}
            x={b.x}
            y={b.y}
            width={b.largura}
            height={b.altura}
            rx={8}
            fill={PRACAS.has(b.chave) ? 'var(--color-led-fraco)' : 'var(--color-superficie)'}
            fillOpacity={PRACAS.has(b.chave) ? 0.45 : OPACIDADE_BLOCO[b.tom]}
            stroke="var(--color-borda)"
            strokeOpacity={0.35}
            strokeWidth={1}
          />
        ))}

        {/* A avenida na diagonal corta os quarteirões e leva uma faixa tracejada no meio */}
        <path d={AVENIDA} stroke="var(--color-noite-950)" strokeWidth={22} strokeLinecap="round" fill="none" />
        <path d={AVENIDA} stroke="var(--color-gelo-300)" strokeOpacity={0.22} strokeWidth={2} strokeDasharray="14 12" fill="none" />

        {/* O rio: um leito escuro com a água azul por cima */}
        <path d={RIO} stroke="var(--color-noite-950)" strokeWidth={44} strokeLinecap="round" fill="none" />
        <path d={RIO} stroke="var(--color-ciano)" strokeOpacity={0.14} strokeWidth={30} strokeLinecap="round" fill="none" />
        <path d={RIO} stroke="var(--color-gelo-azul)" strokeOpacity={0.22} strokeWidth={2} strokeLinecap="round" fill="none" />
      </g>
    </g>
  )
}
