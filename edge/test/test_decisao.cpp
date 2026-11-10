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
 * test_decisao.cpp — testes da lógica pura (decisao.h), sem placa.
 * Um caso por requisito de decisão (R5, R7, R9, R13 a R32, R41); o número do
 * requisito aparece no nome de cada teste.
 *
 * Compilar e rodar:
 *   g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_decisao.cpp -o /tmp/t && /tmp/t
 */
#include <cmath>
#include <cstdio>
#include <string>
#include "decisao.h"

static int falhas = 0;
static int total = 0;

#define CHECK(cond)                                                              \
  do {                                                                           \
    total++;                                                                     \
    if (!(cond)) {                                                               \
      falhas++;                                                                  \
      std::printf("  FALHOU (linha %d): %s\n", __LINE__, #cond);                 \
    }                                                                            \
  } while (0)

static bool quase(float a, float b, float eps = 0.001f) { return std::fabs(a - b) < eps; }

// Leitura "tudo bem" no perfil de vacina: 5 °C, porta fechada, setpoint 5 °C, sem degelo.
static Leitura base() {
  Leitura l = Leitura();
  l.valida = true;
  l.temperatura = 5.0f;
  l.umidade = 80.0f;
  l.luzPct = 10.0f;
  l.temSetpoint = true;
  l.setpoint = 5.0f;
  l.degelo = false;
  l.horaValida = true;
  l.hora = 12;
  l.liberacao = false;
  return l;
}

static const unsigned long T0 = 100000;   // instante inicial dos testes
static const unsigned long PASSO = 2000;  // um ciclo de leitura

// Provoca um bloqueio por setpoint inválido e devolve o instante do ciclo.
static unsigned long bloquearPorSetpoint(Contexto& ctx) {
  Leitura l = base();
  l.setpoint = 40.0f;
  avaliar(l, ctx, T0);
  return T0;
}

static void teste_perfis() {  // R41
  std::printf("perfis (R41)\n");
  CHECK(quase(PERFIS[PERFIL_VACINA].tMin, 2.0f) && quase(PERFIS[PERFIL_VACINA].tMax, 8.0f));
  CHECK(quase(PERFIS[PERFIL_RESFRIADO].tMin, 0.0f) && quase(PERFIS[PERFIL_RESFRIADO].tMax, 4.0f));
  CHECK(quase(PERFIS[PERFIL_CONGELADO].tMax, -18.0f));
  CHECK(quase(PERFIS[PERFIL_PLASMA].tMax, -25.0f));
  CHECK(quase(PERFIS[PERFIL_VACINA].tolerancia, 1.0f));
  CHECK(PERFIL_ATIVO == PERFIL_VACINA);
  CHECK(quase(perfilAtivo().tMax, 8.0f));
}

static void teste_validacao_temperatura() {  // R7
  std::printf("validacao da temperatura (R7)\n");
  CHECK(temperaturaValida(4.8f));
  CHECK(!temperaturaValida(NAN));
  CHECK(!temperaturaValida(85.0f));
  CHECK(!temperaturaValida(-50.0f));
  CHECK(temperaturaValida(-40.0f) && temperaturaValida(80.0f));
}

static void teste_normal() {  // R14
  std::printf("NORMAL (R14)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d = avaliar(base(), ctx, T0);
  CHECK(d.estado == ST_NORMAL);
  CHECK(d.motivo == MT_NENHUM);
  CHECK(!d.cortaAquecedor && !d.ligaVentoinha);
}

static void teste_porta_aberta() {  // R15
  std::printf("ATENCAO por porta aberta (R15)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.luzPct = 80.0f;
  Decisao d = avaliar(l, ctx, T0);
  CHECK(d.estado == ST_ATENCAO && d.motivo == MT_PORTA_ABERTA);
  CHECK(!d.cortaAquecedor && !d.ligaVentoinha);
}

static void teste_deriva() {  // R16
  std::printf("ATENCAO por deriva (R16)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.temperatura = 8.5f;  // acima de 8, dentro da tolerância (até 9)
  Decisao d = avaliar(l, ctx, T0);
  d = avaliar(l, ctx, T0 + PASSO);
  CHECK(d.estado == ST_ATENCAO && d.motivo == MT_DERIVA);
}

static void teste_setpoint_invalido() {  // R20, R17
  std::printf("BLOQUEIO por setpoint invalido (R20, R17)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.setpoint = 40.0f;
  Decisao d = avaliar(l, ctx, T0);
  CHECK(d.estado == ST_BLOQUEIO);
  CHECK(d.motivo == MT_SETPOINT_INVALIDO);
  CHECK(d.novoBloqueio);
  CHECK(ctx.bloqueios == 1);
  CHECK(d.cortaAquecedor && d.ligaVentoinha);  // R5, R17
  CHECK(d.travaRestanteMs == TRAVA_MS);
}

static void teste_sem_setpoint() {  // R11
  std::printf("sem setpoint observado nao bloqueia (R11)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.temSetpoint = false; l.setpoint = SETPOINT_AUSENTE;
  Decisao d = avaliar(l, ctx, T0);
  CHECK(d.estado == ST_NORMAL);
}

static void teste_degelo_longo() {  // R21
  std::printf("BLOQUEIO por degelo longo (R21)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.degelo = true; l.hora = 6;  // 6h: horário permitido
  Decisao d;
  unsigned long t = T0;
  d = avaliar(l, ctx, t);
  CHECK(d.estado == ST_NORMAL);
  // Dentro dos 30 min: não bloqueia (a cada 60 s; 30 passos = exatamente 30 min).
  for (int i = 1; i <= 30; i++) { t = T0 + (unsigned long)i * 60000UL; d = avaliar(l, ctx, t); }
  CHECK(d.estado == ST_NORMAL);
  // Passou de 30 min: bloqueia.
  t = T0 + 31UL * 60000UL;
  d = avaliar(l, ctx, t);
  CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_DEGELO_LONGO);
}

static void teste_degelo_fora_horario() {  // R22, R13
  std::printf("BLOQUEIO por degelo fora do horario (R22, R13)\n");
  {
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.degelo = true; l.hora = 12;  // 12h: fora de 6h e 18h
    Decisao d = avaliar(l, ctx, T0);
    CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_DEGELO_FORA_HORARIO);
  }
  {
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.degelo = true; l.hora = 18;  // 18h: janela permitida
    CHECK(avaliar(l, ctx, T0).estado == ST_NORMAL);
  }
  {
    // Offline: sem hora válida, só vale a duração máxima.
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.degelo = true; l.horaValida = false; l.hora = 0;
    CHECK(avaliar(l, ctx, T0).estado == ST_NORMAL);
  }
}

static void teste_aquecimento_rapido_porta_fechada() {  // R23
  std::printf("BLOQUEIO por aquecimento rapido com porta fechada (R23)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base();
  Decisao d;
  bool bloqueou = false;
  for (int i = 0; i < 10; i++) {          // +0,1 °C a cada 2 s = 3 °C/min
    l.temperatura = 5.0f + 0.1f * (float)i;
    d = avaliar(l, ctx, T0 + (unsigned long)i * PASSO);
    if (d.estado == ST_BLOQUEIO) { bloqueou = true; break; }
  }
  CHECK(bloqueou);
  CHECK(d.motivo == MT_AQUECIMENTO_RAPIDO);
  CHECK(d.taxaCPorMin > TAXA_MAX_C_POR_MIN);
}

static void teste_aquecimento_rapido_porta_aberta() {  // R24
  std::printf("aquecimento rapido com porta aberta e so ATENCAO (R24, R15)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.luzPct = 80.0f;
  Decisao d;
  for (int i = 0; i < 10; i++) {
    l.temperatura = 5.0f + 0.1f * (float)i;
    d = avaliar(l, ctx, T0 + (unsigned long)i * PASSO);
  }
  CHECK(d.estado == ST_ATENCAO && d.motivo == MT_PORTA_ABERTA);
  CHECK(d.taxaCPorMin > TAXA_MAX_C_POR_MIN);  // a taxa estava alta, mas não bloqueou
  CHECK(ctx.bloqueios == 0);
}

static void teste_sensor_invalido() {  // R7, R9, R19a
  std::printf("FALHA_SENSOR (R7, R9, R19a)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.valida = false;
  Decisao d = avaliar(l, ctx, T0);
  CHECK(d.estado == ST_NORMAL);              // 1 ciclo inválido ainda não confirma
  d = avaliar(l, ctx, T0 + PASSO);
  CHECK(d.estado == ST_FALHA_SENSOR && d.motivo == MT_SENSOR_SEM_LEITURA);
  CHECK(!d.cortaAquecedor && !d.ligaVentoinha);
  // Leitura válida volta: sai da falha.
  d = avaliar(base(), ctx, T0 + 2 * PASSO);
  CHECK(d.estado == ST_NORMAL);
}

static void teste_falha_sensor_mantem_regras_1_e_2() {  // R29
  std::printf("em FALHA_SENSOR as regras 1 e 2 continuam (R29)\n");
  {
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.valida = false; l.setpoint = 40.0f;
    avaliar(l, ctx, T0);
    Decisao d = avaliar(l, ctx, T0 + PASSO);
    CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_SETPOINT_INVALIDO);
  }
  {
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.valida = false; l.degelo = true; l.hora = 12;
    avaliar(l, ctx, T0);
    Decisao d = avaliar(l, ctx, T0 + PASSO);
    CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_DEGELO_FORA_HORARIO);
  }
}

static void teste_trava_ignora_ordem() {  // R25
  std::printf("trava ignora nova ordem (R25)\n");
  Contexto ctx; iniciarContexto(ctx);
  bloquearPorSetpoint(ctx);
  Leitura l = base(); l.setpoint = 50.0f;   // controlador muda de novo
  Decisao d = avaliar(l, ctx, T0 + PASSO);
  CHECK(d.estado == ST_BLOQUEIO);
  CHECK(d.mudancaIgnorada);
  CHECK(!d.novoBloqueio);
  CHECK(ctx.bloqueios == 1);
  // Sem nova mudança no ciclo seguinte: nada a registrar.
  d = avaliar(l, ctx, T0 + 2 * PASSO);
  CHECK(!d.mudancaIgnorada);
}

static void teste_liberacao_antes_da_trava() {  // R26, R30
  std::printf("liberacao antes da trava minima nao libera (R26, R30)\n");
  Contexto ctx; iniciarContexto(ctx);
  bloquearPorSetpoint(ctx);
  Leitura l = base(); l.liberacao = true;   // setpoint de volta a 5
  Decisao d = avaliar(l, ctx, T0 + TRAVA_MS - PASSO);
  CHECK(d.estado == ST_BLOQUEIO);
  CHECK(ctx.bloqueado);
  CHECK(d.travaRestanteMs == PASSO);
  CHECK(d.liberacaoRecusada && !d.liberou);   // o usuário precisa ser avisado
}

static void teste_liberacao_temperatura_boa() {  // R26
  std::printf("liberacao com temperatura boa volta ao NORMAL (R26)\n");
  Contexto ctx; iniciarContexto(ctx);
  bloquearPorSetpoint(ctx);
  Leitura l = base(); l.liberacao = true;
  Decisao d = avaliar(l, ctx, T0 + TRAVA_MS);
  CHECK(d.estado == ST_NORMAL);
  CHECK(!ctx.bloqueado);
  CHECK(!d.cortaAquecedor && !d.ligaVentoinha);
  CHECK(d.travaRestanteMs == 0);
  CHECK(d.liberou && !d.liberacaoRecusada);
}

static void teste_liberacao_temperatura_ruim() {  // R27, R18
  std::printf("liberacao com temperatura ruim vai para CRITICO (R27, R18)\n");
  Contexto ctx; iniciarContexto(ctx);
  bloquearPorSetpoint(ctx);
  Leitura l = base(); l.temperatura = 12.0f;  // além da tolerância (acima de 9)
  Decisao d;
  unsigned long t = T0 + PASSO;
  for (; t < T0 + TRAVA_MS + 4000; t += PASSO) d = avaliar(l, ctx, t);
  CHECK(d.estado == ST_CRITICO);              // crítico já durante o bloqueio
  l.liberacao = true;
  d = avaliar(l, ctx, t);                     // depois da trava, com o botão
  CHECK(!ctx.bloqueado);
  CHECK(d.estado == ST_CRITICO);
  CHECK(d.cortaAquecedor && d.ligaVentoinha);
}

static void teste_liberacao_descarta_historico_da_taxa() {  // R26
  std::printf("liberacao nao reabre o bloqueio por subida antiga (R26)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.temperatura = 2.0f; l.setpoint = 40.0f;
  avaliar(l, ctx, T0);                          // bloqueia por setpoint com 2,0 °C
  l.temperatura = 5.0f; l.setpoint = 5.0f;      // a temperatura sobe durante a trava
  for (unsigned long t = T0 + PASSO; t < T0 + TRAVA_MS; t += PASSO) avaliar(l, ctx, t);
  l.liberacao = true;
  Decisao d = avaliar(l, ctx, T0 + TRAVA_MS);   // a subida antiga já é história
  CHECK(d.estado == ST_NORMAL);
  CHECK(!ctx.bloqueado);
  CHECK(ctx.bloqueios == 1);
}

static void teste_liberacao_ordem_ainda_invalida() {  // R28
  std::printf("liberacao com ordem ainda invalida volta a BLOQUEIO (R28)\n");
  Contexto ctx; iniciarContexto(ctx);
  bloquearPorSetpoint(ctx);                   // setpoint 40
  Leitura l = base(); l.setpoint = 40.0f; l.liberacao = true;
  Decisao d = avaliar(l, ctx, T0 + TRAVA_MS);
  CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_SETPOINT_INVALIDO);
  CHECK(d.novoBloqueio);
  CHECK(ctx.bloqueios == 2);
  CHECK(d.travaRestanteMs == TRAVA_MS);       // a trava recomeça
}

static void teste_critico() {  // R18, R9
  std::printf("CRITICO (R18, R9)\n");
  {
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.temperatura = 10.0f;   // acima de 9 (8 + tolerância)
    Decisao d = avaliar(l, ctx, T0);
    CHECK(d.estado != ST_CRITICO);               // 1 ciclo ainda não confirma
    d = avaliar(l, ctx, T0 + PASSO);
    CHECK(d.estado == ST_CRITICO && d.motivo == MT_TEMP_FORA_TOLERANCIA);
    CHECK(d.cortaAquecedor && d.ligaVentoinha);
  }
  {
    Contexto ctx; iniciarContexto(ctx);          // frio: abaixo de 1 (2 - tolerância)
    Leitura l = base(); l.temperatura = 0.0f;
    avaliar(l, ctx, T0);
    Decisao d = avaliar(l, ctx, T0 + PASSO);
    CHECK(d.estado == ST_CRITICO);
    CHECK(!d.cortaAquecedor && !d.ligaVentoinha);   // frio: aquecedor liberado, ventoinha desligada (R52)
  }
  {
    Contexto ctx; iniciarContexto(ctx);          // pico isolado não gera CRITICO
    Leitura l = base(); l.temperatura = 10.0f;
    avaliar(l, ctx, T0);
    l.temperatura = 5.0f;
    Decisao d = avaliar(l, ctx, T0 + PASSO);
    d = avaliar(l, ctx, T0 + 2 * PASSO);
    CHECK(d.estado == ST_NORMAL);
  }
}

static void teste_exposicao() {  // R31
  std::printf("exposicao termica acumulada (R31)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.temperatura = 10.0f;     // 2 °C acima de T_MAX (8)
  Decisao d;
  for (int i = 0; i < 10; i++) d = avaliar(l, ctx, T0 + (unsigned long)i * PASSO);
  // 9 intervalos de 2 s = 18 s = 0,3 min; 2 °C acima => 0,6 °C·min.
  CHECK(quase(d.exposicao, 0.6f));
  // Abaixo de T_MAX não acumula.
  l.temperatura = 5.0f;
  Decisao d2 = avaliar(l, ctx, T0 + 10 * PASSO);
  CHECK(quase(d2.exposicao, d.exposicao));
}

static void teste_prioridade() {  // R19
  std::printf("prioridade entre estados (R19)\n");
  {   // CRITICO vence BLOQUEIO
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.setpoint = 40.0f; l.temperatura = 12.0f;
    avaliar(l, ctx, T0);
    Decisao d = avaliar(l, ctx, T0 + PASSO);
    CHECK(d.estado == ST_CRITICO);
    CHECK(ctx.bloqueado);
    CHECK(d.motivo == MT_SETPOINT_INVALIDO);     // o motivo do ataque é preservado
  }
  {   // BLOQUEIO vence FALHA_SENSOR
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.valida = false; l.setpoint = 40.0f;
    avaliar(l, ctx, T0);
    CHECK(avaliar(l, ctx, T0 + PASSO).estado == ST_BLOQUEIO);
  }
  {   // FALHA_SENSOR vence ATENCAO
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.valida = false; l.luzPct = 80.0f;
    avaliar(l, ctx, T0);
    CHECK(avaliar(l, ctx, T0 + PASSO).estado == ST_FALHA_SENSOR);
  }
  {   // ATENCAO vence NORMAL
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base(); l.luzPct = 80.0f;
    CHECK(avaliar(l, ctx, T0).estado == ST_ATENCAO);
  }
}

static void teste_reles_por_estado() {  // R5, R17, R18
  std::printf("rele so corta aquecimento e liga ventoinha em BLOQUEIO/CRITICO (R5, R17, R18)\n");
  Estado estados[] = { ST_NORMAL, ST_ATENCAO, ST_FALHA_SENSOR, ST_BLOQUEIO, ST_CRITICO };
  for (Estado alvo : estados) {
    Contexto ctx; iniciarContexto(ctx);
    Leitura l = base();
    Decisao d;
    switch (alvo) {
      case ST_NORMAL:  d = avaliar(l, ctx, T0); break;
      case ST_ATENCAO: l.luzPct = 80.0f; d = avaliar(l, ctx, T0); break;
      case ST_FALHA_SENSOR:
        l.valida = false; avaliar(l, ctx, T0); d = avaliar(l, ctx, T0 + PASSO); break;
      case ST_BLOQUEIO: l.setpoint = 40.0f; d = avaliar(l, ctx, T0); break;
      case ST_CRITICO:
        l.temperatura = 12.0f; avaliar(l, ctx, T0); d = avaliar(l, ctx, T0 + PASSO); break;
    }
    CHECK(d.estado == alvo);
    bool atua = (alvo == ST_BLOQUEIO || alvo == ST_CRITICO);
    CHECK(d.cortaAquecedor == atua);
    CHECK(d.ligaVentoinha == atua);
  }
}

static void teste_nomes() {  // R34 (nomes usados no payload)
  std::printf("nomes de estado e motivo (R34)\n");
  CHECK(std::string(nomeEstado(ST_NORMAL)) == "NORMAL");
  CHECK(std::string(nomeEstado(ST_FALHA_SENSOR)) == "FALHA_SENSOR");
  CHECK(std::string(nomeMotivo(MT_NENHUM)) == "-");
  CHECK(std::string(nomeMotivo(MT_SETPOINT_INVALIDO)) == "SETPOINT_INVALIDO");
}

static void teste_alarme_ate_regularizar() {  // R50, R51
  std::printf("alarme sonoro ate a temperatura regularizar, bloqueio continua (R50, R51)\n");
  Contexto ctx; iniciarContexto(ctx);
  const unsigned long t = bloquearPorSetpoint(ctx);        // bloqueio com a temperatura boa (5 °C)
  Leitura l = base();
  Decisao d = avaliar(l, ctx, t + PASSO);
  CHECK(d.bloqueado && d.alarme);                          // aviso inicial, mesmo com temperatura boa
  d = avaliar(l, ctx, t + ALARME_INICIAL_MS + PASSO);
  CHECK(d.bloqueado && !d.alarme);                         // temperatura boa: silêncio, bloqueio fica

  l.temperatura = 8.5f;                                    // acima da faixa: o alarme volta
  d = avaliar(l, ctx, t + ALARME_INICIAL_MS + 2 * PASSO);
  CHECK(d.bloqueado && d.alarme);
  l.temperatura = 5.0f;                                    // regularizou: silêncio, bloqueio fica
  d = avaliar(l, ctx, t + ALARME_INICIAL_MS + 3 * PASSO);
  CHECK(d.bloqueado && !d.alarme && d.estado == ST_BLOQUEIO);
  d = avaliar(l, ctx, t + 2UL * 3600UL * 1000UL);          // horas depois, sem botão
  CHECK(d.bloqueado && !d.alarme);

  Contexto c2; iniciarContexto(c2);                        // CRITICO: alarme até voltar à faixa
  unsigned long u = bloquearPorSetpoint(c2) + ALARME_INICIAL_MS + PASSO;
  Leitura q = base(); q.temperatura = 12.0f;
  avaliar(q, c2, u += PASSO);
  d = avaliar(q, c2, u += PASSO);
  CHECK(d.estado == ST_CRITICO && d.alarme && d.bloqueado);
  q.temperatura = 5.0f;
  d = avaliar(q, c2, u += PASSO);
  CHECK(d.estado == ST_BLOQUEIO && !d.alarme && d.bloqueado);

  Contexto c3; iniciarContexto(c3);                        // sem leitura não dá para saber: alarme
  unsigned long v = bloquearPorSetpoint(c3) + ALARME_INICIAL_MS + PASSO;
  Leitura cega = base(); cega.valida = false;
  avaliar(cega, c3, v += PASSO);
  d = avaliar(cega, c3, v += PASSO);
  CHECK(d.bloqueado && d.alarme);

  Contexto c4; iniciarContexto(c4);                        // sem bloqueio nem crítico: nunca alarme
  d = avaliar(base(), c4, T0);
  CHECK(!d.bloqueado && !d.alarme);
  Leitura quente = base(); quente.temperatura = 8.5f;      // deriva é só ATENCAO
  avaliar(quente, c4, T0 + PASSO);
  d = avaliar(quente, c4, T0 + 2 * PASSO);
  CHECK(d.estado == ST_ATENCAO && !d.alarme);

  Contexto c5; iniciarContexto(c5);                        // liberar apaga o bloqueio
  unsigned long w = bloquearPorSetpoint(c5);
  Leitura lib = base(); lib.liberacao = true;
  d = avaliar(lib, c5, w + TRAVA_MS);
  CHECK(d.liberou && !d.bloqueado && !d.alarme);
}

// Ataque que ESFRIA (sangue, vacina: frio demais também estraga). O bloqueio é o espelho do
// aquecimento: o C-Shield libera o aquecedor e NÃO liga a ventoinha de emergência.
static void teste_ataque_frio() {  // R52, R53, R54
  std::printf("ataque que esfria: aquecedor liberado e ventoinha desligada (R52 a R54)\n");
  {
    Contexto ctx; iniciarContexto(ctx);        // resfriamento rápido: 5 °C -> 3 °C em 10 s
    Leitura l = base();
    Decisao d;
    for (int i = 0; i <= 5; i++) {
      l.temperatura = 5.0f - 0.4f * (float)i;
      d = avaliar(l, ctx, T0 + (unsigned long)i * PASSO);
    }
    CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_RESFRIAMENTO_RAPIDO);
    CHECK(d.bloqueado && d.novoBloqueio && ctx.bloqueios == 1);   // o bloqueio abriu neste último ciclo
    CHECK(d.taxaCPorMin < -TAXA_MAX_RESFRIAMENTO_C_POR_MIN);
    CHECK(!d.cortaAquecedor && !d.ligaVentoinha);       // frio: nada de cortar nem de ventilar
  }
  {
    Contexto ctx; iniciarContexto(ctx);        // porta aberta não explica esfriar: também bloqueia
    Leitura l = base(); l.luzPct = 80.0f;
    Decisao d;
    for (int i = 0; i <= 5; i++) {
      l.temperatura = 5.0f - 0.4f * (float)i;
      d = avaliar(l, ctx, T0 + (unsigned long)i * PASSO);
    }
    CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_RESFRIAMENTO_RAPIDO);
  }
  {
    Contexto ctx; iniciarContexto(ctx);        // setpoint baixo demais (0 °C): frio, relés em repouso
    Leitura l = base(); l.setpoint = 0.0f;
    Decisao d = avaliar(l, ctx, T0);
    CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_SETPOINT_INVALIDO && d.bloqueado);
    CHECK(!d.cortaAquecedor && !d.ligaVentoinha);
    l.temperatura = 9.5f;                      // se passar do calor, a resposta vira a do calor
    d = avaliar(l, ctx, T0 + PASSO);
    CHECK(d.cortaAquecedor && d.ligaVentoinha);
    l.temperatura = 5.0f;                      // voltou à faixa: segue a direção do bloqueio (frio)
    d = avaliar(l, ctx, T0 + 2 * PASSO);
    CHECK(!d.cortaAquecedor && !d.ligaVentoinha && d.bloqueado);
  }
  {
    Contexto ctx; iniciarContexto(ctx);        // bloqueio quente cuja temperatura cai abaixo da faixa
    const unsigned long t = bloquearPorSetpoint(ctx);
    Leitura l = base(); l.temperatura = 1.5f;
    Decisao d = avaliar(l, ctx, t + PASSO);
    CHECK(d.bloqueado && !d.cortaAquecedor && !d.ligaVentoinha);
    l.temperatura = 5.0f;                      // e de volta: retoma a direção original (calor)
    d = avaliar(l, ctx, t + 2 * PASSO);
    CHECK(d.cortaAquecedor && d.ligaVentoinha);
  }
  {
    Contexto ctx; iniciarContexto(ctx);        // alarme no frio: toca fora da faixa, para ao regularizar
    Leitura l = base(); l.setpoint = 0.0f;
    avaliar(l, ctx, T0);
    l.temperatura = 0.5f;
    Decisao d = avaliar(l, ctx, T0 + ALARME_INICIAL_MS + PASSO);
    CHECK(d.alarme && d.bloqueado);
    l.temperatura = 5.0f;
    d = avaliar(l, ctx, T0 + ALARME_INICIAL_MS + 2 * PASSO);
    CHECK(!d.alarme && d.bloqueado);
  }
  {
    Contexto ctx; iniciarContexto(ctx);        // recuperação do calor não é ataque frio
    Leitura l = base();
    l.temperatura = 12.0f;
    avaliar(l, ctx, T0);
    Decisao d = avaliar(l, ctx, T0 + PASSO);
    CHECK(d.estado == ST_CRITICO && d.cortaAquecedor && d.ligaVentoinha);
    unsigned long t = T0 + PASSO;
    for (float temp = 9.0f; temp >= 5.0f; temp -= 1.0f) {   // cai 30 °C/min até o centro da faixa
      l.temperatura = temp;
      d = avaliar(l, ctx, t += PASSO);
    }
    CHECK(d.estado == ST_NORMAL && !d.bloqueado);
  }
}

int main() {
  teste_perfis();
  teste_validacao_temperatura();
  teste_normal();
  teste_porta_aberta();
  teste_deriva();
  teste_setpoint_invalido();
  teste_sem_setpoint();
  teste_degelo_longo();
  teste_degelo_fora_horario();
  teste_aquecimento_rapido_porta_fechada();
  teste_aquecimento_rapido_porta_aberta();
  teste_sensor_invalido();
  teste_falha_sensor_mantem_regras_1_e_2();
  teste_trava_ignora_ordem();
  teste_liberacao_antes_da_trava();
  teste_liberacao_temperatura_boa();
  teste_liberacao_temperatura_ruim();
  teste_liberacao_descarta_historico_da_taxa();
  teste_liberacao_ordem_ainda_invalida();
  teste_critico();
  teste_exposicao();
  teste_prioridade();
  teste_reles_por_estado();
  teste_nomes();
  teste_alarme_ate_regularizar();
  teste_ataque_frio();
  std::printf("\n%d verificacoes, %d falhas\n", total, falhas);
  return falhas == 0 ? 0 : 1;
}
