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
 * test_decisao.cpp — testes da lógica pura (decisao.h), sem placa.
 * Um caso por requisito de lógica; o número do requisito (R..) aparece no nome de cada teste.
 *
 * Compilar e rodar (duas vezes: modo demonstração e modo de produção):
 *   g++ -std=c++17 -Wall -Wextra -static -I edge/cshield edge/test/test_decisao.cpp -o t.exe && ./t.exe
 *   g++ -std=c++17 -Wall -Wextra -static -DMODO_DEMO=0 -I edge/cshield edge/test/test_decisao.cpp -o t0.exe && ./t0.exe
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

// Leitura "tudo bem" no perfil de vacina: 5 °C, porta fechada, o controlador mandando só a
// ventoinha (operação normal), 06:05 (dentro da janela de degelo, em qualquer modo).
static Leitura base() {
  Leitura l = Leitura();
  l.valida = true;
  l.temperatura = 5.0f;
  l.umidade = 80.0f;
  l.luzPct = 10.0f;
  l.aquecedorCmd = false;
  l.ventoinhaCmd = true;
  l.horaValida = true;
  l.minutoDoDia = 365;
  l.liberacao = false;
  return l;
}

// Degelo comandado: aquecedor ligado e ventoinha desligada.
static Leitura degelo() {
  Leitura l = base();
  l.aquecedorCmd = true;
  l.ventoinhaCmd = false;
  return l;
}

static const unsigned long T0 = 100000;   // instante inicial dos testes
static const unsigned long PASSO = 2000;  // um ciclo de leitura

// Provoca um bloqueio por degelo forçado (aquecedor ligado com a ventoinha desligada por mais
// de DEGELO_MAX_MS) e devolve o instante em que o bloqueio abriu.
static unsigned long bloquearPorDegelo(Contexto& ctx) {
  avaliar(degelo(), ctx, T0);
  const unsigned long t = T0 + DEGELO_MAX_MS + PASSO;
  avaliar(degelo(), ctx, t);
  return t;
}

// Ciclo de temperatura qualquer, com as chaves do controlador em operação normal.
static Decisao cicloTemp(Contexto& ctx, float temp, unsigned long t) {
  Leitura l = base();
  l.temperatura = temp;
  return avaliar(l, ctx, t);
}

static void teste_perfis() {  // R11
  std::printf("perfis (R11)\n");
  CHECK(quase(PERFIS[PERFIL_VACINA].tMin, 2.0f) && quase(PERFIS[PERFIL_VACINA].tMax, 8.0f));
  CHECK(quase(PERFIS[PERFIL_VACINA].tAlvo, 5.0f));
  CHECK(quase(PERFIS[PERFIL_RESFRIADO].tMin, 0.0f) && quase(PERFIS[PERFIL_RESFRIADO].tMax, 4.0f));
  CHECK(quase(PERFIS[PERFIL_CONGELADO].tMax, -18.0f) && quase(PERFIS[PERFIL_CONGELADO].tAlvo, -20.0f));
  CHECK(quase(PERFIS[PERFIL_PLASMA].tMax, -25.0f) && quase(PERFIS[PERFIL_PLASMA].tAlvo, -30.0f));
  CHECK(quase(PERFIS[PERFIL_VACINA].tolerancia, 1.0f));
  CHECK(PERFIL_ATIVO == PERFIL_VACINA);
  CHECK(quase(HISTERESE_C, 1.0f));
}

static void teste_validacao_temperatura() {  // R6
  std::printf("validacao da temperatura (R6)\n");
  CHECK(temperaturaValida(4.8f));
  CHECK(!temperaturaValida(NAN));
  CHECK(!temperaturaValida(85.0f));
  CHECK(!temperaturaValida(-50.0f));
  CHECK(temperaturaValida(-40.0f) && temperaturaValida(80.0f));
}

static void teste_normal_e_atencao() {  // R12, R13
  std::printf("NORMAL e ATENCAO por porta ou deriva (R12, R13)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d = avaliar(base(), ctx, T0);
  CHECK(d.estado == ST_NORMAL && d.motivo == MT_NENHUM);
  CHECK(!d.cortarAquecedor && !d.ligarVentoinha);

  Contexto c2; iniciarContexto(c2);
  Leitura porta = base(); porta.luzPct = 80.0f;
  d = avaliar(porta, c2, T0);
  CHECK(d.estado == ST_ATENCAO && d.motivo == MT_PORTA_ABERTA);
  CHECK(!d.cortarAquecedor && !d.ligarVentoinha);

  Contexto c3; iniciarContexto(c3);
  d = cicloTemp(c3, 8.5f, T0);   // acima de 8, dentro da tolerância (até 9)
  d = cicloTemp(c3, 8.5f, T0 + PASSO);
  CHECK(d.estado == ST_ATENCAO && d.motivo == MT_DERIVA);
  CHECK(!d.cortarAquecedor && !d.ligarVentoinha);
}

static void teste_janela_degelo() {  // R18
  std::printf("janela de degelo permitido (R18)\n");
  Leitura l = base();
  l.minutoDoDia = 360; CHECK(degeloNoHorario(l));
  l.minutoDoDia = 389; CHECK(degeloNoHorario(l));
  l.minutoDoDia = 1080; CHECK(degeloNoHorario(l));
  l.minutoDoDia = 1109; CHECK(degeloNoHorario(l));
  l.horaValida = false; l.minutoDoDia = 720; CHECK(degeloNoHorario(l));   // sem hora: só vale a duração
#if MODO_DEMO
  l.horaValida = true; l.minutoDoDia = 120; CHECK(degeloNoHorario(l));    // demo desliga a checagem
#else
  l.horaValida = true;
  l.minutoDoDia = 390; CHECK(!degeloNoHorario(l));
  l.minutoDoDia = 1110; CHECK(!degeloNoHorario(l));
  l.minutoDoDia = 120; CHECK(!degeloNoHorario(l));
#endif
}

static void teste_degelo_legitimo() {  // R14
  std::printf("degelo legitimo (R14)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d = avaliar(degelo(), ctx, T0);
  CHECK(d.estado == ST_ATENCAO && d.motivo == MT_DEGELO);
  CHECK(d.degeloRestanteMs == DEGELO_MAX_MS);
  CHECK(!d.cortarAquecedor && !d.ligarVentoinha);
  d = avaliar(degelo(), ctx, T0 + PASSO);
  CHECK(d.estado == ST_ATENCAO && d.degeloRestanteMs == DEGELO_MAX_MS - PASSO);
}

static void teste_degelo_forcado() {  // R15
  std::printf("D1 degelo forcado (R15)\n");
  Contexto ctx; iniciarContexto(ctx);
  avaliar(degelo(), ctx, T0);
  Decisao d = avaliar(degelo(), ctx, T0 + DEGELO_MAX_MS + PASSO);
  CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_DEGELO_FORCADO);
  CHECK(d.novoBloqueio && ctx.bloqueios == 1);
  CHECK(d.cortarAquecedor && d.ligarVentoinha);   // corta e começa com o termostato ligado (R22)
  d = avaliar(degelo(), ctx, T0 + DEGELO_MAX_MS + 2 * PASSO);
  CHECK(!d.novoBloqueio && ctx.bloqueios == 1);   // o evento sai uma vez só
}

static void teste_degelo_fora_do_horario() {  // R15, R18
  std::printf("D1 degelo fora do horario e sem hora valida (R15, R18)\n");
#if !MODO_DEMO
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = degelo(); l.minutoDoDia = 720;   // 12:00, fora das janelas, com hora válida
  Decisao d = avaliar(l, ctx, T0);
  CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_DEGELO_FORCADO);
#endif
  Contexto c2; iniciarContexto(c2);
  Leitura s = degelo(); s.horaValida = false; s.minutoDoDia = 720;
  Decisao d2 = avaliar(s, c2, T0);   // sem hora válida só vale a duração
  CHECK(d2.estado == ST_ATENCAO && d2.motivo == MT_DEGELO);
  d2 = avaliar(s, c2, T0 + DEGELO_MAX_MS + PASSO);
  CHECK(d2.estado == ST_BLOQUEIO && d2.motivo == MT_DEGELO_FORCADO);
}

static void teste_aquecedor_longo() {  // R16
  std::printf("D2 aquecedor longo (R16)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura l = base(); l.aquecedorCmd = true; l.ventoinhaCmd = true;   // as duas ligadas
  Decisao d = avaliar(l, ctx, T0);
  CHECK(d.estado == ST_NORMAL);                      // por pouco tempo, nada acontece
  d = avaliar(l, ctx, T0 + DEGELO_MAX_MS + PASSO);
  CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_AQUECEDOR_LONGO);
}

// Sobe 5 °C -> 7 °C em 10 s: taxa de 12 °C/min, muito acima do limite. Devolve o último instante.
static unsigned long subirRapido(Contexto& ctx, Leitura molde, Decisao& saida) {
  unsigned long t = T0;
  float temp = 5.0f;
  for (int i = 0; i <= 5; i++) {
    molde.temperatura = temp;
    saida = avaliar(molde, ctx, t);
    temp += 0.4f;
    t += PASSO;
  }
  return t - PASSO;
}

static void teste_subida_rapida() {  // R17
  std::printf("D3 subida rapida (R17)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d;
  subirRapido(ctx, base(), d);
  CHECK(d.estado == ST_BLOQUEIO && d.motivo == MT_SUBIDA_RAPIDA);
  CHECK(ctx.bloqueios == 1 && d.cortarAquecedor);

  Contexto c2; iniciarContexto(c2);   // a mesma subida com a porta aberta é só ATENCAO
  Leitura aberta = base(); aberta.luzPct = 80.0f;
  Decisao d2;
  subirRapido(c2, aberta, d2);
  CHECK(d2.estado == ST_ATENCAO && d2.motivo == MT_PORTA_ABERTA);
  CHECK(!c2.bloqueado && !d2.cortarAquecedor);
}

static void teste_falha_sensor() {  // R8, R20
  std::printf("FALHA_SENSOR e confirmacao (R8, R20)\n");
  Contexto ctx; iniciarContexto(ctx);
  Leitura ruim = base(); ruim.valida = false;
  Decisao d = avaliar(ruim, ctx, T0);
  CHECK(d.estado != ST_FALHA_SENSOR);                // 1 ciclo não basta
  d = avaliar(ruim, ctx, T0 + PASSO);
  CHECK(d.estado == ST_FALHA_SENSOR && d.motivo == MT_SENSOR_SEM_LEITURA);

  Contexto c2; iniciarContexto(c2);                  // FALHA_SENSOR sobe sobre ATENCAO (porta aberta)
  Leitura aberta = ruim; aberta.luzPct = 80.0f;
  avaliar(aberta, c2, T0);
  d = avaliar(aberta, c2, T0 + PASSO);
  CHECK(d.estado == ST_FALHA_SENSOR);
}

static void teste_ordens_ignoradas() {  // R24
  std::printf("controlador ignorado no bloqueio (R24)\n");
  Contexto ctx; iniciarContexto(ctx);
  const unsigned long t = bloquearPorDegelo(ctx);
  CHECK(ctx.ignorados == 0);
  Leitura l = degelo(); l.aquecedorCmd = false;      // o atacante solta o aquecedor
  Decisao d = avaliar(l, ctx, t + PASSO);
  CHECK(d.mudancaIgnorada && ctx.ignorados == 1);
  CHECK(d.cortarAquecedor && d.estado == ST_BLOQUEIO);   // os relés não mudam
  l.ventoinhaCmd = true;                             // e liga a ventoinha
  d = avaliar(l, ctx, t + 2 * PASSO);
  CHECK(d.mudancaIgnorada && ctx.ignorados == 2);
  d = avaliar(l, ctx, t + 3 * PASSO);                // sem mudança nova, nada é contado
  CHECK(!d.mudancaIgnorada && ctx.ignorados == 2);
  CHECK(d.cortarAquecedor);
}

static void teste_termostato() {  // R23
  std::printf("termostato de emergencia (R23)\n");
  Contexto ctx; iniciarContexto(ctx);
  unsigned long t = bloquearPorDegelo(ctx);
  Decisao d = cicloTemp(ctx, 5.0f, t += PASSO);
  CHECK(d.ligarVentoinha);                           // começa ligado e 5 °C está entre 4 e 6
  d = cicloTemp(ctx, 3.9f, t += PASSO);
  CHECK(!d.ligarVentoinha && d.cortarAquecedor);     // desliga em T_ALVO - histerese
  d = cicloTemp(ctx, 5.0f, t += PASSO);
  CHECK(!d.ligarVentoinha);                          // histerese: continua desligado
  d = cicloTemp(ctx, 6.0f, t += PASSO);
  CHECK(d.ligarVentoinha);                           // religa em T_ALVO + histerese
  d = cicloTemp(ctx, 5.0f, t += PASSO);
  CHECK(d.ligarVentoinha);                           // continua ligado até 4 °C
}

static void teste_sem_expiracao() {  // R25
  std::printf("bloqueio sem expiracao (R25)\n");
  Contexto ctx; iniciarContexto(ctx);
  const unsigned long t = bloquearPorDegelo(ctx);
  Decisao d = cicloTemp(ctx, 5.0f, t + 2UL * 3600UL * 1000UL);   // 2 h depois, tudo normal
  CHECK(d.estado == ST_BLOQUEIO && ctx.bloqueado && d.cortarAquecedor);
}

static void teste_liberacao() {  // R26, R27
  std::printf("liberacao presencial (R26, R27)\n");
  Contexto ctx; iniciarContexto(ctx);
  unsigned long t = bloquearPorDegelo(ctx);
  Leitura l = base(); l.liberacao = true;            // chaves normais, 5 °C
  Decisao d = avaliar(l, ctx, t += PASSO);
  CHECK(d.liberou && !ctx.bloqueado);
  CHECK(d.estado == ST_NORMAL && !d.cortarAquecedor && !d.ligarVentoinha);

  Contexto c2; iniciarContexto(c2);                  // temperatura ruim: continua no controle
  t = bloquearPorDegelo(c2);
  Leitura quente = base(); quente.liberacao = true; quente.temperatura = 10.5f;
  d = avaliar(quente, c2, t += PASSO);
  CHECK(d.liberacaoRecusada && !d.liberou && c2.bloqueado);
  CHECK(d.estado == ST_BLOQUEIO && d.cortarAquecedor);

  Contexto c3; iniciarContexto(c3);                  // sensor inválido também recusa
  t = bloquearPorDegelo(c3);
  Leitura cega = base(); cega.liberacao = true; cega.valida = false;
  d = avaliar(cega, c3, t += PASSO);
  CHECK(d.liberacaoRecusada && c3.bloqueado);
}

static void teste_falha_no_bloqueio() {  // R29
  std::printf("falha de sensor durante o bloqueio (R29)\n");
  Contexto ctx; iniciarContexto(ctx);
  unsigned long t = bloquearPorDegelo(ctx);
  cicloTemp(ctx, 3.9f, t += PASSO);                  // termostato desligado
  Leitura cega = base(); cega.valida = false;
  avaliar(cega, ctx, t += PASSO);
  Decisao d = avaliar(cega, ctx, t += PASSO);
  CHECK(d.ligarVentoinha && d.cortarAquecedor);      // sem temperatura, refrigerar é o lado seguro
  CHECK(d.estado == ST_BLOQUEIO);                    // BLOQUEIO tem prioridade sobre FALHA_SENSOR
}

static void teste_critico() {  // R19, R21, R30
  std::printf("CRITICO e prioridade (R19, R21, R30)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d = cicloTemp(ctx, 10.0f, T0);
  CHECK(d.estado != ST_CRITICO);                     // 1 ciclo não basta
  d = cicloTemp(ctx, 10.0f, T0 + PASSO);
  CHECK(d.estado == ST_CRITICO && d.cortarAquecedor && d.ligarVentoinha);
  d = cicloTemp(ctx, 5.0f, T0 + 2 * PASSO);          // voltou à faixa, sem bloqueio
  CHECK(d.estado == ST_NORMAL && !d.cortarAquecedor && !d.ligarVentoinha);

  Contexto c2; iniciarContexto(c2);                  // CRITICO sobe sobre BLOQUEIO
  unsigned long t = bloquearPorDegelo(c2);
  cicloTemp(c2, 10.0f, t += PASSO);
  d = cicloTemp(c2, 10.0f, t += PASSO);
  CHECK(d.estado == ST_CRITICO && c2.bloqueado);
  d = cicloTemp(c2, 5.0f, t += PASSO);               // temperatura volta, o bloqueio persiste
  CHECK(d.estado == ST_BLOQUEIO);
}

static void teste_exposicao() {  // R31
  std::printf("exposicao termica (R31)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d;
  for (unsigned long t = T0; t <= T0 + 120000UL; t += PASSO) d = cicloTemp(ctx, 10.0f, t);
  CHECK(quase(d.exposicao, 4.0f, 0.01f));            // (10 - 8) °C x 2 min
}

static void teste_sem_rebloqueio() {  // R28
  std::printf("sem rebloqueio apos liberar (R28)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d;
  unsigned long t = subirRapido(ctx, base(), d);     // bloqueia por subida rápida, termina em ~7 °C
  CHECK(ctx.bloqueado);
  Leitura l = base(); l.temperatura = 7.0f; l.liberacao = true;
  d = avaliar(l, ctx, t += PASSO);
  CHECK(d.liberou);
  l.liberacao = false;
  for (int i = 0; i < 8; i++) d = avaliar(l, ctx, t += PASSO);
  CHECK(d.estado == ST_NORMAL && !ctx.bloqueado);    // a subida antiga não reabre o bloqueio
}

static void teste_ventoinha_efetiva() {  // R5
  std::printf("ventoinha efetiva nunca menor que a do controlador (R5)\n");
  Decisao d = Decisao();
  Leitura l = base();
  d.ligarVentoinha = false; l.ventoinhaCmd = true;  CHECK(ventoinhaEfetiva(d, l));
  d.ligarVentoinha = true;  l.ventoinhaCmd = false; CHECK(ventoinhaEfetiva(d, l));
  d.ligarVentoinha = false; l.ventoinhaCmd = false; CHECK(!ventoinhaEfetiva(d, l));
  d.ligarVentoinha = true;  l.ventoinhaCmd = true;  CHECK(ventoinhaEfetiva(d, l));
}

static void teste_rele_so_corta_e_liga() {  // R5, R22
  std::printf("rele so corta aquecedor e liga ventoinha (R5, R22)\n");
  Contexto ctx; iniciarContexto(ctx);
  Decisao d = avaliar(base(), ctx, T0);
  CHECK(!d.cortarAquecedor && !d.ligarVentoinha);    // NORMAL: repouso
  d = avaliar(degelo(), ctx, T0 + PASSO);
  CHECK(!d.cortarAquecedor && !d.ligarVentoinha);    // degelo legítimo: repouso
  Contexto c2; iniciarContexto(c2);
  unsigned long t = bloquearPorDegelo(c2);
  d = cicloTemp(c2, 5.0f, t + PASSO);
  CHECK(d.cortarAquecedor);                          // bloqueio: corta o aquecedor
}

static void teste_nomes() {  // R37, R39
  std::printf("nomes de estado e motivo (R37, R39)\n");
  CHECK(std::string(nomeEstado(ST_NORMAL)) == "NORMAL");
  CHECK(std::string(nomeEstado(ST_ATENCAO)) == "ATENCAO");
  CHECK(std::string(nomeEstado(ST_BLOQUEIO)) == "BLOQUEIO");
  CHECK(std::string(nomeEstado(ST_CRITICO)) == "CRITICO");
  CHECK(std::string(nomeEstado(ST_FALHA_SENSOR)) == "FALHA_SENSOR");
  CHECK(std::string(nomeMotivo(MT_NENHUM)) == "-");
  CHECK(std::string(nomeMotivo(MT_DEGELO)) == "DEGELO");
  CHECK(std::string(nomeMotivo(MT_DEGELO_FORCADO)) == "DEGELO_FORCADO");
  CHECK(std::string(nomeMotivo(MT_AQUECEDOR_LONGO)) == "AQUECEDOR_LONGO");
  CHECK(std::string(nomeMotivo(MT_SUBIDA_RAPIDA)) == "SUBIDA_RAPIDA");
}

int main() {
  std::printf("MODO_DEMO=%d\n", MODO_DEMO);
  teste_perfis();
  teste_validacao_temperatura();
  teste_normal_e_atencao();
  teste_janela_degelo();
  teste_degelo_legitimo();
  teste_degelo_forcado();
  teste_degelo_fora_do_horario();
  teste_aquecedor_longo();
  teste_subida_rapida();
  teste_falha_sensor();
  teste_ordens_ignoradas();
  teste_termostato();
  teste_sem_expiracao();
  teste_liberacao();
  teste_falha_no_bloqueio();
  teste_critico();
  teste_exposicao();
  teste_sem_rebloqueio();
  teste_ventoinha_efetiva();
  teste_rele_so_corta_e_liga();
  teste_nomes();
  std::printf("\n%d verificacoes, %d falhas\n", total, falhas);
  return falhas == 0 ? 0 : 1;
}
