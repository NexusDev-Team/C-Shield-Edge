/*
 * C-Shield — Edge Computing (GS 2026.2 "CitizenTech")
 * Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
 * Equipe Nexus Vision. Integrantes:
 *   Isac Nilton Fernandes de Oliveira — RM573282
 *   João Benedito de Oliveira Simplício — RM570206
 *   Leonardo Formigari — RM573291
 *   Luis Fernando — RM570959
 *   Nicolas Cinel — RM572940
 */

/*
 * decisao.h — lógica pura de decisão do C-Shield.
 *
 * NÃO inclui Arduino.h de propósito: o tempo entra como parâmetro (agoraMs) e todo o estado
 * fica no Contexto. Assim o mesmo código roda na placa e nos testes do computador (g++).
 *
 * O C-Shield reconhece o ataque a um controlador de refrigeração por três sinais:
 *   D1  aquecedor ligado com a ventoinha desligada fora do degelo permitido
 *   D2  aquecedor ligado com a ventoinha ligada por tempo demais
 *   D3  temperatura subindo rápido com a porta fechada
 * Ao detectar, entra em BLOQUEIO: corta o aquecedor, ignora o controlador e regula a
 * ventoinha sozinho (termostato de emergência) até a liberação presencial. O bloqueio não
 * expira. O C-Shield nunca tira refrigeração: só corta o aquecedor e acrescenta ventoinha.
 */
#ifndef DECISAO_H
#define DECISAO_H

#include "config.h"
#include "decisao_tipos.h"
#include "decisao_regras.h"

// ---------------------------------------------------------------------------
// Liberação e decisão
// ---------------------------------------------------------------------------

// Liberação presencial: só vale com o botão segurado, a temperatura válida e dentro da faixa
// com tolerância. Aceita: encerra o bloqueio, solta os relés e descarta o histórico da taxa
// (senão a subida do episódio antigo reabriria o bloqueio na hora). Devolve true se liberou.
static inline bool liberar(const Leitura& l, Contexto& ctx, unsigned long agoraMs) {
  if (!ctx.bloqueado || !l.liberacao) return false;
  const Perfil p = perfilAtivo();
  if (!l.valida || l.temperatura < p.tMin - p.tolerancia || l.temperatura > p.tMax + p.tolerancia) return false;
  ctx.bloqueado = false;
  ctx.termostatoLigado = false;
  ctx.histN = 0;
  registrarAmostra(ctx, l.temperatura, agoraMs);
  return true;
}

// ---------------------------------------------------------------------------
// Decisão de um ciclo
// ---------------------------------------------------------------------------

// Avalia as ordens do controlador e a temperatura, e devolve o estado e as ações:
//  1. sensor, histórico, exposição e contadores   2. tempo do aquecedor
//  3. liberação presencial   4. julgar o controlador (D1 a D3) ou ignorá-lo no bloqueio
//  5. termostato de emergência   6. estado pela prioridade   7. relés
static inline Decisao avaliar(const Leitura& l, Contexto& ctx, unsigned long agoraMs) {
  Decisao d = Decisao();
  const Perfil p = perfilAtivo();

  float dtMin = 0.0f;   // intervalo desde o ciclo anterior, para integrar a exposição
  if (ctx.iniciado && agoraMs >= ctx.ultimoMs) dtMin = (float)(agoraMs - ctx.ultimoMs) / 60000.0f;

  // --- 1. Sensor, histórico, exposição e contadores de confirmação ---
  if (l.valida) {
    ctx.contInvalido = 0;
    registrarAmostra(ctx, l.temperatura, agoraMs);
    if (l.temperatura > p.tMax) ctx.exposicao += (l.temperatura - p.tMax) * dtMin;
    const bool alem = (l.temperatura > p.tMax + p.tolerancia) || (l.temperatura < p.tMin - p.tolerancia);
    ctx.contCritico = alem ? ctx.contCritico + 1 : 0;
  } else {
    ctx.contInvalido++;
    ctx.contCritico = 0;   // sem leitura não há como confirmar temperatura ruim
  }
  const bool falhaSensor = ctx.contInvalido >= CONFIRMACOES;
  const bool critico = ctx.contCritico >= CONFIRMACOES;
  const bool porta = portaAberta(l.luzPct);

  // --- 2. Há quanto tempo o aquecedor está comandado (conta também durante o bloqueio) ---
  if (l.aquecedorCmd) {
    if (!ctx.aquecedorAtivo) { ctx.aquecedorAtivo = true; ctx.inicioAquecedorMs = agoraMs; }
  } else {
    ctx.aquecedorAtivo = false;
  }

  // --- 3. Liberação presencial: aceita ou recusada ---
  if (ctx.bloqueado && l.liberacao) {
    if (liberar(l, ctx, agoraMs)) {
      d.liberou = true;
      if (ctx.aquecedorAtivo) ctx.inicioAquecedorMs = agoraMs;   // o degelo recomeça do zero
    } else {
      d.liberacaoRecusada = true;
    }
  }
  const unsigned long aqMs = ctx.aquecedorAtivo ? agoraMs - ctx.inicioAquecedorMs : 0;
  d.aquecedorLigadoMs = aqMs;
  d.taxaCPorMin = calcularTaxa(ctx);

  // --- 4. Julgar o controlador (ou ignorá-lo durante o bloqueio) ---
  const bool degeloAtivo = l.aquecedorCmd && !l.ventoinhaCmd;   // padrão do degelo, legítimo ou forçado
  if (ctx.bloqueado) {
    if (l.aquecedorCmd != ctx.ultimoAq) ctx.ignorados++;
    if (l.ventoinhaCmd != ctx.ultimoVt) ctx.ignorados++;
    d.mudancaIgnorada = (l.aquecedorCmd != ctx.ultimoAq) || (l.ventoinhaCmd != ctx.ultimoVt);
  } else {
    Motivo violada = MT_NENHUM;
    if (degeloAtivo && (!degeloNoHorario(l) || aqMs > DEGELO_MAX_MS)) {
      violada = MT_DEGELO_FORCADO;                                       // D1
    } else if (l.aquecedorCmd && l.ventoinhaCmd && aqMs > DEGELO_MAX_MS) {
      violada = MT_AQUECEDOR_LONGO;                                      // D2
    } else if (l.valida && !falhaSensor && !porta && d.taxaCPorMin > TAXA_MAX_C_POR_MIN) {
      violada = MT_SUBIDA_RAPIDA;                                        // D3 (porta aberta é só ATENCAO)
    }
    if (violada != MT_NENHUM) {
      ctx.bloqueado = true;
      ctx.motivoBloqueio = violada;
      ctx.termostatoLigado = true;   // refrigerar é o lado seguro: começa ligado
      ctx.bloqueios++;
      d.novoBloqueio = true;
    }
  }

  // --- 5. Termostato de emergência (bloqueio ou crítico) ---
  const bool atua = ctx.bloqueado || critico;
  if (atua) {
    if (falhaSensor) ctx.termostatoLigado = true;   // sem temperatura, mantém refrigerando
    else if (l.valida && l.temperatura >= p.tAlvo + HISTERESE_C) ctx.termostatoLigado = true;
    else if (l.valida && l.temperatura <= p.tAlvo - HISTERESE_C) ctx.termostatoLigado = false;
  } else {
    ctx.termostatoLigado = false;
  }

  // --- 6. Estado final pela prioridade ---
  const bool degeloLegitimo = degeloAtivo && !ctx.bloqueado;
  const bool deriva = l.valida && (l.temperatura < p.tMin || l.temperatura > p.tMax);
  if (critico) {
    d.estado = ST_CRITICO;
    d.motivo = ctx.bloqueado ? ctx.motivoBloqueio : MT_TEMP_FORA_TOLERANCIA;
  } else if (ctx.bloqueado) {
    d.estado = ST_BLOQUEIO;
    d.motivo = ctx.motivoBloqueio;
  } else if (falhaSensor) {
    d.estado = ST_FALHA_SENSOR;
    d.motivo = MT_SENSOR_SEM_LEITURA;
  } else if (porta) {
    d.estado = ST_ATENCAO;
    d.motivo = MT_PORTA_ABERTA;
  } else if (degeloLegitimo) {
    d.estado = ST_ATENCAO;
    d.motivo = MT_DEGELO;
    d.degeloRestanteMs = aqMs >= DEGELO_MAX_MS ? 0 : DEGELO_MAX_MS - aqMs;
  } else if (deriva) {
    d.estado = ST_ATENCAO;
    d.motivo = MT_DERIVA;
  } else {
    d.estado = ST_NORMAL;
    d.motivo = MT_NENHUM;
  }

  // --- 7. Relés: fora do bloqueio e do crítico ficam em repouso (o controlador manda) ---
  d.cortarAquecedor = atua;
  d.ligarVentoinha = atua && ctx.termostatoLigado;
  d.exposicao = ctx.exposicao;

  ctx.iniciado = true;
  ctx.ultimoMs = agoraMs;
  ctx.ultimoAq = l.aquecedorCmd;
  ctx.ultimoVt = l.ventoinhaCmd;
  return d;
}

#endif
