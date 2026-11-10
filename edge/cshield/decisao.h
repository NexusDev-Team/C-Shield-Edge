/*
 * C-Shield — Edge Computing do Guardião (GS 2026.2 "CitizenTech")
 * Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
 * Equipe Nexus Vision. Integrantes:
 *   Isac Nilton Fernandes de Oliveira — RM573282
 *   João Benedito de Oliveira Simplício — RM570206
 *   Leonardo Formigari — RM573291
 *   Luis Fernando — RM570959
 *   Nicolas Cinel — RM572940
 */

/*
 * decisao.h — lógica PURA de decisão do C-Shield.
 *
 * Não depende do Arduino (nem de millis(), Serial ou pinos): recebe uma Leitura,
 * a memória entre ciclos (Contexto) e o relógio em milissegundos, e devolve uma
 * Decisao. Por isso compila com g++ e é testada no computador, sem placa
 * (edge/test/test_decisao.cpp).
 *
 * A política aqui é fixa: ela nunca é alterada pela rede. O firmware só envia dados.
 * Prioridade dos estados: CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL.
 */
#ifndef DECISAO_H
#define DECISAO_H

#include "config.h"

// ---------------------------------------------------------------------------
// Tipos
// ---------------------------------------------------------------------------

enum Estado { ST_NORMAL, ST_ATENCAO, ST_BLOQUEIO, ST_CRITICO, ST_FALHA_SENSOR };

enum Motivo {
  MT_NENHUM,
  MT_PORTA_ABERTA,
  MT_DERIVA,
  MT_SETPOINT_INVALIDO,
  MT_DEGELO_FORA_HORARIO,
  MT_DEGELO_LONGO,
  MT_AQUECIMENTO_RAPIDO,
  MT_RESFRIAMENTO_RAPIDO,
  MT_TEMP_FORA_TOLERANCIA,
  MT_SENSOR_SEM_LEITURA
};

// Entrada de um ciclo de decisão (montada por sensores.h; nos testes, preenchida à mão).
struct Leitura {
  bool  valida;       // o DHT22 devolveu valor utilizável neste ciclo
  float temperatura;  // °C, já suavizada pela média móvel (só vale se valida)
  float umidade;      // %
  float luzPct;       // % do LDR (independe do DHT22)
  bool  temSetpoint;  // false na maquete (sem chave de setpoint)
  float setpoint;     // °C que o controlador comercial está mandando
  bool  degelo;       // degelo comandado pelo controlador
  bool  horaValida;   // hora do NTP disponível
  int   hora;         // 0..23, só vale se horaValida
  bool  liberacao;    // botão segurado por LIBERACAO_MS (calculado em sensores.h)
};

// Memória entre ciclos. Zere com iniciarContexto().
struct Contexto {
  bool          iniciado;          // já houve um ciclo (para calcular o intervalo)
  unsigned long ultimoMs;          // instante do ciclo anterior
  bool          bloqueado;         // condição persistente de bloqueio
  unsigned long inicioBloqueioMs;  // base da trava mínima
  Motivo        motivoBloqueio;
  bool          bloqueioFrio;      // o ataque ESFRIA (setpoint baixo ou resfriamento rápido)
  int           bloqueios;         // contador publicado em `bq`
  float         exposicao;         // °C·min acumulados acima de T_MAX (`ex`)
  bool          degeloAtivo;       // o degelo está comandado desde inicioDegeloMs
  unsigned long inicioDegeloMs;
  int           contCritico;       // ciclos seguidos além da tolerância
  int           contInvalido;      // ciclos seguidos com sensor inválido
  float         histTemp[N_AMOSTRAS_TAXA];       // histórico da janela da taxa
  unsigned long histMs[N_AMOSTRAS_TAXA];
  int           histN;
  float         ultimoSetpoint;    // para registrar mudanças ignoradas na trava
  bool          ultimoDegelo;
};

// Saída de um ciclo de decisão.
struct Decisao {
  Estado estado;
  Motivo motivo;
  bool   cortaAquecedor;   // relé 1 abre o NF
  bool   ligaVentoinha;    // relé 2 fecha o NA
  bool   novoBloqueio;     // true só no ciclo em que um bloqueio começou (gera evento)
  bool   mudancaIgnorada;  // true quando a trava descartou uma ordem do controlador
  bool   liberou;          // true no ciclo em que a liberação presencial foi aceita
  bool   liberacaoRecusada;// true quando o botão foi segurado mas a trava mínima não acabou
  bool   bloqueado;        // o bloqueio está ativo (mesmo que o estado mostrado seja CRITICO)
  bool   alarme;           // o apito deve tocar: bloqueio ou crítico com a temperatura ainda fora da faixa
  float  taxaCPorMin;      // para a linha do Serial
  float  exposicao;        // °C·min acumulados, para `ex`
  unsigned long travaRestanteMs; // 0 quando a trava mínima já passou
};

// ---------------------------------------------------------------------------
// Funções auxiliares (puras)
// ---------------------------------------------------------------------------

// Perfil de produto escolhido em config.h.
static inline Perfil perfilAtivo() { return PERFIS[PERFIL_ATIVO]; }

// true se a temperatura é utilizável: não é NaN e está na faixa física do DHT22.
static inline bool temperaturaValida(float t) {
  if (t != t) return false;  // NaN é o único valor diferente de si mesmo
  return t >= DHT_TEMP_MIN_FISICA && t <= DHT_TEMP_MAX_FISICA;
}

// true se a luminosidade indica porta aberta.
static inline bool portaAberta(float luzPct) { return luzPct > LIMIAR_PORTA_PCT; }

// true se o degelo é permitido nessa hora (janelas de HORA_DEGELO_1 e HORA_DEGELO_2).
static inline bool degeloPermitidoNaHora(int hora) {
  return (hora >= HORA_DEGELO_1 && hora < HORA_DEGELO_1 + JANELA_DEGELO_H) ||
         (hora >= HORA_DEGELO_2 && hora < HORA_DEGELO_2 + JANELA_DEGELO_H);
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

// Motivo curto, sem espaços, para o payload e para o Serial ("-" quando não há).
static inline const char* nomeMotivo(Motivo m) {
  switch (m) {
    case MT_NENHUM:              return "-";
    case MT_PORTA_ABERTA:        return "PORTA_ABERTA";
    case MT_DERIVA:              return "DERIVA";
    case MT_SETPOINT_INVALIDO:   return "SETPOINT_INVALIDO";
    case MT_DEGELO_FORA_HORARIO: return "DEGELO_FORA_HORARIO";
    case MT_DEGELO_LONGO:        return "DEGELO_LONGO";
    case MT_AQUECIMENTO_RAPIDO:  return "AQUECIMENTO_RAPIDO";
    case MT_RESFRIAMENTO_RAPIDO: return "RESFRIAMENTO_RAPIDO";
    case MT_TEMP_FORA_TOLERANCIA:return "TEMP_FORA_TOLERANCIA";
    case MT_SENSOR_SEM_LEITURA:  return "SENSOR_SEM_LEITURA";
  }
  return "-";
}

// Zera toda a memória entre ciclos.
static inline void iniciarContexto(Contexto& ctx) {
  ctx = Contexto();                 // valor-inicializa: tudo zero/false
  ctx.motivoBloqueio = MT_NENHUM;
  ctx.ultimoSetpoint = SETPOINT_AUSENTE;
}

// Guarda a temperatura no histórico e descarta o que saiu da janela de JANELA_TAXA_MS.
static inline void registrarAmostra(Contexto& ctx, float temp, unsigned long agoraMs) {
  if (ctx.histN >= N_AMOSTRAS_TAXA) {            // cheio: descarta a mais antiga
    for (int i = 1; i < ctx.histN; i++) {
      ctx.histTemp[i - 1] = ctx.histTemp[i];
      ctx.histMs[i - 1] = ctx.histMs[i];
    }
    ctx.histN--;
  }
  ctx.histTemp[ctx.histN] = temp;
  ctx.histMs[ctx.histN] = agoraMs;
  ctx.histN++;
  // Remove amostras mais velhas que a janela, sempre mantendo ao menos a mais nova.
  while (ctx.histN > 1 && agoraMs - ctx.histMs[0] > JANELA_TAXA_MS) {
    for (int i = 1; i < ctx.histN; i++) {
      ctx.histTemp[i - 1] = ctx.histTemp[i];
      ctx.histMs[i - 1] = ctx.histMs[i];
    }
    ctx.histN--;
  }
}

// Taxa de variação da temperatura em °C/min (positiva = aquecendo), medida entre a
// amostra mais antiga e a mais nova da janela. 0 se ainda não há intervalo suficiente.
static inline float calcularTaxa(const Contexto& ctx) {
  if (ctx.histN < 2) return 0.0f;
  unsigned long dt = ctx.histMs[ctx.histN - 1] - ctx.histMs[0];
  if (dt < TAXA_MIN_INTERVALO_MS) return 0.0f;
  return (ctx.histTemp[ctx.histN - 1] - ctx.histTemp[0]) / ((float)dt / 60000.0f);
}

// ---------------------------------------------------------------------------
// Decisão de um ciclo
// ---------------------------------------------------------------------------

// Avalia as ordens do controlador e a temperatura, e devolve o estado e as ações.
//  1. Atualiza sensor, histórico, exposição térmica e contador de CRITICO.
//  2. Trata a liberação presencial (depois da trava mínima).
//  3. Se não há bloqueio ativo, julga a ordem do controlador (setpoint, degelo, taxa).
//     Se há bloqueio, mudanças novas do controlador são ignoradas e registradas.
//  4. Escolhe o estado pela prioridade e define os relés.
static inline Decisao avaliar(const Leitura& l, Contexto& ctx, unsigned long agoraMs) {
  Decisao d = Decisao();
  const Perfil p = perfilAtivo();

  // Intervalo desde o ciclo anterior, em minutos (para integrar a exposição).
  float dtMin = 0.0f;
  if (ctx.iniciado) dtMin = (float)(agoraMs - ctx.ultimoMs) / 60000.0f;

  // --- 1. Sensor, histórico, exposição e CRITICO ---
  if (l.valida) {
    ctx.contInvalido = 0;
    registrarAmostra(ctx, l.temperatura, agoraMs);
    if (l.temperatura > p.tMax) ctx.exposicao += (l.temperatura - p.tMax) * dtMin;
    bool alemTolerancia = (l.temperatura > p.tMax + p.tolerancia) ||
                          (l.temperatura < p.tMin - p.tolerancia);
    ctx.contCritico = alemTolerancia ? ctx.contCritico + 1 : 0;
  } else {
    ctx.contInvalido++;
    ctx.contCritico = 0;   // sem leitura não há como confirmar temperatura ruim
  }
  const bool falhaSensor = ctx.contInvalido >= CONFIRMACOES;
  const bool critico = ctx.contCritico >= CONFIRMACOES;
  const bool porta = portaAberta(l.luzPct);

  // Acompanha o tempo do degelo comandado.
  if (l.degelo) {
    if (!ctx.degeloAtivo) { ctx.degeloAtivo = true; ctx.inicioDegeloMs = agoraMs; }
  } else {
    ctx.degeloAtivo = false;
  }

  // --- 2. Liberação presencial (só depois da trava mínima) ---
  if (ctx.bloqueado && l.liberacao && (agoraMs - ctx.inicioBloqueioMs) < TRAVA_MS) {
    d.liberacaoRecusada = true;   // avisa o usuário (Serial e LCD) em vez de falhar em silêncio
  }
  if (ctx.bloqueado && l.liberacao && (agoraMs - ctx.inicioBloqueioMs) >= TRAVA_MS) {
    d.liberou = true;
    ctx.bloqueado = false;
    ctx.motivoBloqueio = MT_NENHUM;
    // O histórico de temperatura da janela de 60 s pertence ao episódio que acabou de ser
    // bloqueado. Sem descartá-lo, a subida antiga reabriria o bloqueio por "aquecimento
    // rápido" no mesmo instante da liberação, mesmo com a temperatura já estável.
    ctx.histN = 0;
    if (l.valida) registrarAmostra(ctx, l.temperatura, agoraMs);
  }

  // --- 3. Julgar a ordem do controlador (ou ignorá-la durante a trava) ---
  const float taxa = calcularTaxa(ctx);
  d.taxaCPorMin = taxa;

  if (ctx.bloqueado) {
    bool mudouSp = l.temSetpoint && (l.setpoint != ctx.ultimoSetpoint);
    d.mudancaIgnorada = mudouSp || (l.degelo != ctx.ultimoDegelo);
  } else {
    Motivo violada = MT_NENHUM;
    // Regra 1: setpoint fora da faixa do produto (julga a ordem antes do efeito).
    if (l.temSetpoint && (l.setpoint < p.tMin || l.setpoint > p.tMax)) {
      violada = MT_SETPOINT_INVALIDO;
    // Regra 2: degelo por tempo demais, ou fora dos horários permitidos (hora válida).
    } else if (l.degelo && ctx.degeloAtivo && (agoraMs - ctx.inicioDegeloMs) > DEGELO_MAX_MS) {
      violada = MT_DEGELO_LONGO;
    } else if (l.degelo && l.horaValida && !degeloPermitidoNaHora(l.hora)) {
      violada = MT_DEGELO_FORA_HORARIO;
    // Regra 3: aquecimento rápido com a porta fechada (porta aberta é só ATENCAO;
    // com o sensor em falha a regra não é avaliada).
    } else if (l.valida && !falhaSensor && !porta && taxa > TAXA_MAX_C_POR_MIN) {
      violada = MT_AQUECIMENTO_RAPIDO;
    // Regra 4, o espelho da 3: resfriamento rápido. Frio demais também estraga (sangue, vacina).
    // A porta aberta não explica esfriar, então não alivia a regra. Só vale abaixo do centro da
    // faixa: voltar do calor até o centro (a ventoinha esfriando) não é ataque.
    } else if (l.valida && !falhaSensor && l.temperatura < (p.tMin + p.tMax) / 2.0f &&
               taxa < -TAXA_MAX_RESFRIAMENTO_C_POR_MIN) {
      violada = MT_RESFRIAMENTO_RAPIDO;
    }
    if (violada != MT_NENHUM) {
      ctx.bloqueado = true;
      // Direção do ataque: um setpoint abaixo da faixa e o resfriamento rápido são ataques de FRIO.
      ctx.bloqueioFrio = (violada == MT_RESFRIAMENTO_RAPIDO) ||
                         (violada == MT_SETPOINT_INVALIDO && l.setpoint < p.tMin);
      ctx.inicioBloqueioMs = agoraMs;
      ctx.motivoBloqueio = violada;
      ctx.bloqueios++;
      d.novoBloqueio = true;
    }
  }

  // --- 4. Estado final pela prioridade e ações nos relés ---
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
  } else if (deriva) {
    d.estado = ST_ATENCAO;
    d.motivo = MT_DERIVA;
  } else {
    d.estado = ST_NORMAL;
    d.motivo = MT_NENHUM;
  }

  // Resposta nos relés, sempre para o lado que protege o produto:
  //   calor: corta o aquecedor e liga a ventoinha de emergência;
  //   frio:  deixa o aquecedor liberado e NÃO liga a ventoinha (esfriar mais estragaria o produto).
  // A temperatura medida manda; com ela dentro da faixa vale a direção do ataque.
  // Nunca mexe no compressor: o C-Shield só veta o que aumenta o risco.
  const bool atua = ctx.bloqueado || critico;
  const bool muitoQuente = l.valida && l.temperatura > p.tMax;
  const bool muitoFrio = l.valida && l.temperatura < p.tMin;
  const bool modoCalor = muitoQuente || (!muitoFrio && !ctx.bloqueioFrio);
  d.cortaAquecedor = atua && modoCalor;
  d.ligaVentoinha = atua && modoCalor;

  // Apito: toca enquanto o bloqueio (ou o crítico) estiver com a temperatura fora da faixa, e
  // para quando ela regulariza. O bloqueio NÃO acaba com o apito: só a liberação presencial.
  // Sem leitura válida não dá para saber se regularizou, então o apito continua. Nos primeiros
  // ALARME_INICIAL_MS de um bloqueio o apito toca de qualquer jeito (ataque com temperatura boa).
  const bool inicioDoBloqueio = ctx.bloqueado && (agoraMs - ctx.inicioBloqueioMs) < ALARME_INICIAL_MS;
  d.bloqueado = ctx.bloqueado;
  d.alarme = (ctx.bloqueado || critico) && (!l.valida || deriva || inicioDoBloqueio);

  d.exposicao = ctx.exposicao;
  if (ctx.bloqueado) {
    unsigned long decorrido = agoraMs - ctx.inicioBloqueioMs;
    d.travaRestanteMs = decorrido >= TRAVA_MS ? 0 : TRAVA_MS - decorrido;
  }

  // Memória para o próximo ciclo.
  ctx.iniciado = true;
  ctx.ultimoMs = agoraMs;
  if (l.temSetpoint) ctx.ultimoSetpoint = l.setpoint;
  ctx.ultimoDegelo = l.degelo;
  return d;
}

#endif
