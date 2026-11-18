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

// Testes da lógica pura do dashboard: estados, locais, gráfico e cliente HTTP.
import { describe, expect, it } from 'vitest'
import { errorFromResponse } from '../api/client.js'
import { ESTADOS, rotuloMotivo, rotuloRecusa, situacaoDoDispositivo } from './estados.js'
import { dominioDoGrafico, pontosParaGrafico, rotuloTempo } from './grafico.js'
import { LOCAIS, dispositivosPorLocal, locaisLivres } from './locais.js'

describe('situacaoDoDispositivo', () => {
  it('ponto sem C-Shield', () => {
    expect(situacaoDoDispositivo(undefined)).toBe('SEM_CSHIELD')
  })
  it('offline vale mais que o último estado conhecido', () => {
    expect(situacaoDoDispositivo({ status: 'offline', state: 'BLOQUEIO' })).toBe('OFFLINE')
  })
  it('online mostra o estado decidido pelo firmware', () => {
    for (const estado of ['NORMAL', 'ATENCAO', 'BLOQUEIO', 'CRITICO', 'FALHA_SENSOR']) {
      expect(situacaoDoDispositivo({ status: 'online', state: estado })).toBe(estado)
    }
  })
  it('online sem estado ainda é tratado como normal', () => {
    expect(situacaoDoDispositivo({ status: 'online', state: null })).toBe('NORMAL')
  })
  it('todo estado tem rótulo e cor', () => {
    for (const info of Object.values(ESTADOS)) {
      expect(info.rotulo).toBeTruthy()
      expect(info.cor).toMatch(/^var\(--color-/)
    }
  })
})

describe('rótulos em português', () => {
  it('motivo do bloqueio', () => {
    expect(rotuloMotivo('SETPOINT_INVALIDO')).toBe('Setpoint fora da faixa')
    expect(rotuloMotivo('-')).toBeNull()
    expect(rotuloMotivo(null)).toBeNull()
    expect(rotuloMotivo('NOVO_MOTIVO')).toBe('NOVO_MOTIVO')
  })
  it('motivo da recusa de cartão', () => {
    expect(rotuloRecusa('NAO_CADASTRADO')).toBe('Cartão não cadastrado')
    expect(rotuloRecusa('TRAVA')).toBe('Trava mínima ainda ativa')
    expect(rotuloRecusa(undefined)).toBe('')
  })
})

describe('locais', () => {
  it('são os cinco pontos do emulador do site', () => {
    expect(LOCAIS.map((l) => l.id)).toEqual([
      'ubs-vila-mariana', 'ubs-brasilandia', 'hemocentro-consolacao', 'farmacia-mooca', 'mercado-liberdade',
    ])
  })
  it('indexa os dispositivos por ponto e lista os pontos livres', () => {
    const frota = [{ device_id: 'cshield001', local_id: 'ubs-vila-mariana' }]
    expect(dispositivosPorLocal(frota)['ubs-vila-mariana'].device_id).toBe('cshield001')
    expect(locaisLivres(frota)).toHaveLength(4)
    expect(locaisLivres(frota).some((l) => l.id === 'ubs-vila-mariana')).toBe(false)
    expect(locaisLivres(undefined)).toHaveLength(5)
  })
})

describe('gráfico térmico', () => {
  it('rótulo de tempo', () => {
    expect(rotuloTempo(1000, 1000)).toBe('agora')
    expect(rotuloTempo(0, 45_000)).toBe('há 45 s')
    expect(rotuloTempo(0, 600_000)).toBe('há 10 min')
  })
  it('ordena do mais antigo ao mais novo e conta a partir do ponto mais novo', () => {
    const pontos = pontosParaGrafico([
      { ts: '2026-11-18T12:00:30.000Z', value: 5.2 },
      { ts: '2026-11-18T12:00:00.000Z', value: 4.9 },
      { ts: 'invalido', value: 1 },
    ])
    expect(pontos.map((p) => p.v)).toEqual([4.9, 5.2])
    expect(pontos.map((p) => p.t)).toEqual(['há 30 s', 'agora'])
  })
  it('série vazia', () => {
    expect(pontosParaGrafico([])).toEqual([])
    expect(pontosParaGrafico(undefined)).toEqual([])
  })
  it('domínio inclui o limite', () => {
    expect(dominioDoGrafico([4.9, 5.2], null)).toEqual([4, 6])
    expect(dominioDoGrafico([4.9, 5.2], 8)).toEqual([4, 9])
  })
})

describe('errorFromResponse', () => {
  it('detail em texto vira a mensagem, com o service', () => {
    const erro = errorFromResponse(404, { detail: 'cshield009 não está cadastrado', service: 'registry' })
    expect(erro.status).toBe(404)
    expect(erro.message).toBe('cshield009 não está cadastrado')
    expect(erro.service).toBe('registry')
  })
  it('validação do Pydantic vira "campo: mensagem"', () => {
    const erro = errorFromResponse(422, { detail: [{ loc: ['body', 'device_id'], msg: 'padrão inválido' }] })
    expect(erro.message).toBe('device_id: padrão inválido')
  })
  it('5xx sem corpo aponta o túnel e o uvicorn', () => {
    expect(errorFromResponse(502, null).message).toMatch(/túnel/)
  })
})
