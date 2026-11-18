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

/*
 * grafico.js — prepara os pontos do histórico (STH-Comet) para o gráfico térmico. Lógica pura,
 * testada em grafico.test.js.
 */

// Rótulo do eixo X: "agora" para o ponto mais novo, "há N s" até 2 min, "há N min" até 2 h,
// "há N h" até 2 dias e "há N d" depois (a série do STH pode cobrir dias quando o aparelho para).
export function rotuloTempo(instanteMs, referenciaMs) {
  const segundos = Math.max(0, Math.round((referenciaMs - instanteMs) / 1000))
  if (segundos < 5) return 'agora'
  if (segundos < 120) return `há ${segundos} s`
  const minutos = Math.round(segundos / 60)
  if (minutos < 120) return `há ${minutos} min`
  const horas = Math.round(minutos / 60)
  return horas < 48 ? `há ${horas} h` : `há ${Math.round(horas / 24)} d`
}

// Pontos do backend ({ ts, value }) em { v, t }, do mais antigo ao mais novo. O rótulo conta a
// partir do ponto mais novo da série (não do relógio do navegador), para a série parada de um
// C-Shield offline continuar legível.
export function pontosParaGrafico(pontos) {
  const validos = (pontos ?? [])
    .map((p) => ({ instante: Date.parse(p.ts), v: p.value }))
    .filter((p) => Number.isFinite(p.instante) && Number.isFinite(p.v))
    .sort((a, b) => a.instante - b.instante)
  if (validos.length === 0) return []
  const referencia = validos[validos.length - 1].instante
  return validos.map((p) => ({ v: p.v, t: rotuloTempo(p.instante, referencia), instante: p.instante }))
}

// Faixa de temperatura que o gráfico precisa mostrar: os pontos e o limite, com folga.
export function dominioDoGrafico(valores, limite) {
  const todos = limite === null || limite === undefined ? valores : [...valores, limite]
  return [Math.floor(Math.min(...todos) - 0.5), Math.ceil(Math.max(...todos) + 0.5)]
}
