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

import { useEffect, useRef } from 'react'
import { usePrefereMenosMovimento } from '../../hooks/usePrefereMenosMovimento.js'
import { LINHA_PX, alfaDoRastro, avancarColunas, criarColunas } from './chuvaBinaria.js'

// Chuva de binários do fundo: um canvas fixo, discreto, com colunas de 0 e 1 que descem devagar
// e deixam rastro. Mais forte nas bordas e quase apagada atrás do texto (máscara .chuva-binaria).
// Desempenho: ~30 quadros por segundo, sem filtros, pausa com a aba oculta e redesenha só no
// resize (com debounce). Com prefers-reduced-motion desenha um quadro parado, sem animar.
const ESPACO = 28
const QUADRO_MS = 1000 / 30
const COR = '100, 207, 255' // gelo-azul

function desenhar(ctx, colunas, largura, altura) {
  ctx.clearRect(0, 0, largura, altura)
  ctx.font = '14px "JetBrains Mono", ui-monospace, monospace'
  ctx.textAlign = 'center'
  for (const coluna of colunas) {
    for (let i = 0; i < coluna.comprimento; i += 1) {
      const y = coluna.y - (coluna.comprimento - 1 - i) * LINHA_PX
      if (y < -LINHA_PX || y > altura + LINHA_PX) continue
      ctx.fillStyle = `rgba(${COR}, ${(alfaDoRastro(i, coluna.comprimento) * 0.35).toFixed(3)})`
      ctx.fillText(coluna.digitos[i], coluna.x + ESPACO / 2, y)
    }
  }
}

export default function ChuvaBinaria() {
  const canvasRef = useRef(null)
  const parado = usePrefereMenosMovimento()

  useEffect(() => {
    const canvas = canvasRef.current
    const ctx = canvas?.getContext?.('2d')
    if (!canvas || !ctx) return undefined

    let colunas = []
    let largura = 0
    let altura = 0
    let quadro = 0
    let ultimo = 0
    let debounce = 0

    function medir() {
      const dpr = Math.min(window.devicePixelRatio || 1, 1.5)
      largura = window.innerWidth
      altura = window.innerHeight
      canvas.width = Math.round(largura * dpr)
      canvas.height = Math.round(altura * dpr)
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
      const densidade = largura < 768 ? 0.2 : 0.35
      colunas = criarColunas(largura, { espaco: ESPACO, densidade })
      // Espalha as colunas pela altura para o primeiro quadro já parecer uma chuva em andamento.
      colunas = colunas.map((c) => ({ ...c, y: Math.random() * altura }))
      desenhar(ctx, colunas, largura, altura)
    }

    function passo(agora) {
      quadro = requestAnimationFrame(passo)
      if (agora - ultimo < QUADRO_MS) return
      const dt = Math.min(agora - ultimo, 100)
      ultimo = agora
      colunas = avancarColunas(colunas, dt, altura)
      desenhar(ctx, colunas, largura, altura)
    }

    function iniciar() {
      if (parado || document.hidden || quadro) return
      ultimo = performance.now()
      quadro = requestAnimationFrame(passo)
    }

    function pausar() {
      cancelAnimationFrame(quadro)
      quadro = 0
    }

    function aoMudarVisibilidade() {
      if (document.hidden) pausar()
      else iniciar()
    }

    function aoRedimensionar() {
      clearTimeout(debounce)
      debounce = setTimeout(medir, 150)
    }

    medir()
    iniciar()
    document.addEventListener('visibilitychange', aoMudarVisibilidade)
    window.addEventListener('resize', aoRedimensionar)
    return () => {
      pausar()
      clearTimeout(debounce)
      document.removeEventListener('visibilitychange', aoMudarVisibilidade)
      window.removeEventListener('resize', aoRedimensionar)
    }
  }, [parado])

  return <canvas ref={canvasRef} aria-hidden="true" className="chuva-binaria absolute inset-0 size-full opacity-50" />
}
