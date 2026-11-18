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

// Lógica pura da chuva de binários do fundo (sem DOM, sem canvas): criar as colunas, avançá-las no
// tempo e calcular o brilho de cada dígito do rastro. O sorteio entra por parâmetro (`aleatorio`)
// para os testes serem determinísticos. Quem desenha é o ChuvaBinaria.jsx.

// Altura de cada dígito no rastro, em px (o canvas usa a mesma medida para espaçar as linhas).
export const LINHA_PX = 18

const VELOCIDADE_MIN = 25
const VELOCIDADE_MAX = 60

function sortearDigitos(quantidade, aleatorio) {
  return Array.from({ length: quantidade }, () => (aleatorio() < 0.5 ? '0' : '1'))
}

function sortearComprimento(aleatorio) {
  return 8 + Math.floor(aleatorio() * 12)
}

// Uma coluna por faixa de `espaco` px; só uma fração (`densidade`, de 0 a 1) fica ativa. Cada coluna
// começa acima do topo, com velocidade e comprimento sorteados. `y` é a posição da cabeça do rastro.
export function criarColunas(largura, { espaco = 28, densidade = 0.35, aleatorio = Math.random } = {}) {
  const colunas = []
  const faixas = Math.floor(largura / espaco)
  for (let k = 0; k < faixas; k += 1) {
    if (aleatorio() >= densidade) continue
    const comprimento = sortearComprimento(aleatorio)
    colunas.push({
      x: k * espaco,
      y: -aleatorio() * 400,
      velocidade: VELOCIDADE_MIN + aleatorio() * (VELOCIDADE_MAX - VELOCIDADE_MIN),
      comprimento,
      digitos: sortearDigitos(comprimento, aleatorio),
    })
  }
  return colunas
}

// Avança `dtMs` milissegundos: cada coluna desce `velocidade` px/s. Quando a cabeça passa da
// altura, a coluna recomeça acima do topo com dígitos novos. Não muda a lista de entrada.
export function avancarColunas(colunas, dtMs, altura, aleatorio = Math.random) {
  return colunas.map((coluna) => {
    const y = coluna.y + (coluna.velocidade * dtMs) / 1000
    if (y <= altura) return { ...coluna, y }
    const comprimento = sortearComprimento(aleatorio)
    return {
      ...coluna,
      y: -aleatorio() * 200,
      comprimento,
      digitos: sortearDigitos(comprimento, aleatorio),
    }
  })
}

// Brilho (0 a 1) do dígito `indice` de um rastro de `comprimento`: o último (a cabeça) vale 1 e a
// cauda apaga em curva, de modo que cada dígito é mais forte que o anterior.
export function alfaDoRastro(indice, comprimento) {
  return ((indice + 1) / comprimento) ** 2
}
