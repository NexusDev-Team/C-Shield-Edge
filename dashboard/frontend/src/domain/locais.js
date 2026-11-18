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
 * locais.js — os 5 pontos do mapa da frota: os mesmos do emulador do site (WebFront-GS), com as
 * mesmas posições (viewBox 0 0 1000 600), para o mapa daqui ficar igual ao de lá. O C-Shield
 * cadastrado num ponto aparece ali; ponto sem C-Shield aparece apagado.
 * `icone` é o tipo de produto que o local guarda (desenha o ícone); o `perfil` do dispositivo
 * (faixa de temperatura) é escolhido no cadastro e pode ser outro.
 */

export const LOCAIS = [
  { id: 'ubs-vila-mariana', nome: 'UBS Vila Mariana', bairro: 'Vila Mariana', icone: 'VACINA', perfilSugerido: 'VACINA', x: 300, y: 380 },
  { id: 'ubs-brasilandia', nome: 'UBS Brasilândia', bairro: 'Brasilândia', icone: 'VACINA', perfilSugerido: 'VACINA', x: 140, y: 170 },
  { id: 'hemocentro-consolacao', nome: 'Hemocentro Consolação', bairro: 'Consolação', icone: 'PLASMA', perfilSugerido: 'PLASMA', x: 860, y: 430 },
  { id: 'farmacia-mooca', nome: 'Farmácia Mooca', bairro: 'Mooca', icone: 'INSULINA', perfilSugerido: 'VACINA', x: 900, y: 210 },
  { id: 'mercado-liberdade', nome: 'Mercado Liberdade', bairro: 'Liberdade', icone: 'CONGELADO', perfilSugerido: 'CONGELADO', x: 640, y: 410 },
]

// Os dispositivos de /api/fleet indexados pelo ponto do mapa em que estão: { localId: dispositivo }.
export function dispositivosPorLocal(dispositivos) {
  const mapa = {}
  for (const dispositivo of dispositivos ?? []) {
    mapa[dispositivo.local_id] = dispositivo
  }
  return mapa
}

// Os pontos do mapa em que ainda não há C-Shield cadastrado.
export function locaisLivres(cadastrados) {
  const usados = new Set((cadastrados ?? []).map((d) => d.local_id))
  return LOCAIS.filter((local) => !usados.has(local.id))
}
