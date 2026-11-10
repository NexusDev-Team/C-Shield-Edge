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

// Prioridade, da mais alta para a mais baixa: CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL.
enum Estado { ST_NORMAL, ST_ATENCAO, ST_BLOQUEIO, ST_CRITICO, ST_FALHA_SENSOR };

enum Motivo {
  MT_NENHUM, MT_PORTA_ABERTA, MT_DERIVA, MT_DEGELO, MT_DEGELO_FORCADO, MT_AQUECEDOR_LONGO,
  MT_SUBIDA_RAPIDA, MT_TEMP_FORA_TOLERANCIA, MT_SENSOR_SEM_LEITURA
};

// Entrada de um ciclo de decisão (montada por sensores.h; nos testes, preenchida à mão).
struct Leitura {
  bool  valida;        // o DHT22 devolveu valor utilizável neste ciclo
  float temperatura;   // °C, já suavizada pela média móvel (só vale se valida)
  float umidade;       // %
  float luzPct;        // % do LDR (independe do DHT22)
  bool  aquecedorCmd;  // o controlador está mandando ligar o aquecedor/degelo
  bool  ventoinhaCmd;  // o controlador está mandando ligar a ventoinha/compressor
  bool  horaValida;    // hora do NTP disponível
  int   minutoDoDia;   // 0..1439, só vale se horaValida
  bool  liberacao;     // botão segurado por LIBERACAO_MS (calculado em sensores.h)
};

// Memória entre ciclos. Zere com iniciarContexto().
struct Contexto {
  bool          iniciado;          // já houve um ciclo (para calcular o intervalo)
  unsigned long ultimoMs;          // instante do ciclo anterior
  bool          bloqueado;         // condição persistente de bloqueio (não expira)
  Motivo        motivoBloqueio;
  bool          aquecedorAtivo;    // o aquecedor está comandado desde inicioAquecedorMs
  unsigned long inicioAquecedorMs;
  bool          termostatoLigado;  // ventoinha do C-Shield (relé 2) durante bloqueio/crítico
  int           bloqueios;         // contador publicado em `bq`
  int           ignorados;         // comandos do controlador ignorados, publicado em `ci`
  float         exposicao;         // °C·min acumulados acima de T_MAX (`ex`)
  int           contCritico;       // ciclos seguidos além da tolerância
  int           contInvalido;      // ciclos seguidos com sensor inválido
  float         histTemp[N_AMOSTRAS_TAXA];       // histórico da janela da taxa
  unsigned long histMs[N_AMOSTRAS_TAXA];
  int           histN;
  bool          ultimoAq;          // comandos do ciclo anterior, para detectar mudança
  bool          ultimoVt;
};

// Saída de um ciclo de decisão.
struct Decisao {
  Estado estado;
  Motivo motivo;
  bool   cortarAquecedor;     // relé 1 aciona: abre o NF e corta o aquecedor
  bool   ligarVentoinha;      // relé 2 aciona: o C-Shield liga a ventoinha direto
  bool   novoBloqueio;        // true só no ciclo em que um bloqueio começou (gera evento)
  bool   mudancaIgnorada;     // true quando o bloqueio descartou uma mudança do controlador
  bool   liberou;            // true no ciclo em que a liberação presencial foi aceita
  bool   liberacaoRecusada;  // true quando o botão foi segurado mas a temperatura está fora da faixa
  float  taxaCPorMin;         // para a linha do Serial
  float  exposicao;           // °C·min acumulados, para `ex`
  unsigned long degeloRestanteMs;   // contagem do degelo legítimo (0 fora dele)
  unsigned long aquecedorLigadoMs;  // há quanto tempo o aquecedor está comandado (linha de evento)
};

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
