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
 * decisao_regras.h — funções auxiliares puras da decisão: validação da temperatura, janela de
 * degelo, nomes de estado e motivo, histórico e taxa de aquecimento. Sem Arduino.h.
 */
#ifndef DECISAO_REGRAS_H
#define DECISAO_REGRAS_H

#include "decisao_tipos.h"

// ---------------------------------------------------------------------------
// Funções auxiliares (puras)
// ---------------------------------------------------------------------------

static inline Perfil perfilAtivo() { return PERFIS[PERFIL_ATIVO]; }

// true se a temperatura é utilizável: não é NaN e está na faixa física do DHT22.
static inline bool temperaturaValida(float t) {
  if (t != t) return false;  // NaN é o único valor diferente de si mesmo
  return t >= DHT_TEMP_MIN_FISICA && t <= DHT_TEMP_MAX_FISICA;
}

static inline bool portaAberta(float luzPct) { return luzPct > LIMIAR_PORTA_PCT; }

static inline bool dentroDaJanela(int minuto, int horaInicio) {
  return minuto >= horaInicio * 60 && minuto < horaInicio * 60 + JANELA_DEGELO_MIN;
}

// true se um degelo é permitido agora: no modo demonstração e sem hora válida, sempre (vale só
// a duração); em produção, só nas janelas das 6h e das 18h.
static inline bool degeloNoHorario(const Leitura& l) {
#if MODO_DEMO
  (void)l;
  return true;
#else
  if (!l.horaValida) return true;
  return dentroDaJanela(l.minutoDoDia, HORA_DEGELO_1) || dentroDaJanela(l.minutoDoDia, HORA_DEGELO_2);
#endif
}

// Ventoinha de fato ligada: a do controlador OU a do C-Shield. Nunca menor que a comandada.
static inline bool ventoinhaEfetiva(const Decisao& d, const Leitura& l) {
  return d.ligarVentoinha || l.ventoinhaCmd;
}

static inline const char* nomeEstado(Estado e) {
  switch (e) {
    case ST_NORMAL:       return "NORMAL";
    case ST_ATENCAO:      return "ATENCAO";
    case ST_BLOQUEIO:     return "BLOQUEIO";
    case ST_CRITICO:      return "CRITICO";
    case ST_FALHA_SENSOR: return "FALHA_SENSOR";
  }
  return "?";
}

static inline const char* nomeMotivo(Motivo m) {
  switch (m) {
    case MT_NENHUM:                return "-";
    case MT_PORTA_ABERTA:          return "PORTA_ABERTA";
    case MT_DERIVA:                return "DERIVA";
    case MT_DEGELO:                return "DEGELO";
    case MT_DEGELO_FORCADO:        return "DEGELO_FORCADO";
    case MT_AQUECEDOR_LONGO:       return "AQUECEDOR_LONGO";
    case MT_SUBIDA_RAPIDA:         return "SUBIDA_RAPIDA";
    case MT_TEMP_FORA_TOLERANCIA:  return "TEMP_FORA_TOLERANCIA";
    case MT_SENSOR_SEM_LEITURA:    return "SENSOR_SEM_LEITURA";
  }
  return "?";
}

static inline void iniciarContexto(Contexto& ctx) { ctx = Contexto(); }

// Guarda a temperatura no histórico e descarta o que saiu da janela de JANELA_TAXA_MS.
static inline void registrarAmostra(Contexto& ctx, float temp, unsigned long agoraMs) {
  if (ctx.histN >= N_AMOSTRAS_TAXA) {            // cheio: descarta a mais antiga
    for (int i = 1; i < ctx.histN; i++) { ctx.histTemp[i - 1] = ctx.histTemp[i]; ctx.histMs[i - 1] = ctx.histMs[i]; }
    ctx.histN--;
  }
  ctx.histTemp[ctx.histN] = temp;
  ctx.histMs[ctx.histN] = agoraMs;
  ctx.histN++;
  // Remove amostras mais velhas que a janela, sempre mantendo ao menos a mais nova.
  while (ctx.histN > 1 && agoraMs - ctx.histMs[0] > JANELA_TAXA_MS) {
    for (int i = 1; i < ctx.histN; i++) { ctx.histTemp[i - 1] = ctx.histTemp[i]; ctx.histMs[i - 1] = ctx.histMs[i]; }
    ctx.histN--;
  }
}

// Taxa de variação da temperatura em °C/min (positiva = aquecendo), entre a amostra mais
// antiga e a mais nova da janela. 0 se ainda não há intervalo suficiente.
static inline float calcularTaxa(const Contexto& ctx) {
  if (ctx.histN < 2) return 0.0f;
  unsigned long dt = ctx.histMs[ctx.histN - 1] - ctx.histMs[0];
  if (dt < TAXA_MIN_INTERVALO_MS) return 0.0f;
  return (ctx.histTemp[ctx.histN - 1] - ctx.histTemp[0]) / ((float)dt / 60000.0f);
}

#endif
