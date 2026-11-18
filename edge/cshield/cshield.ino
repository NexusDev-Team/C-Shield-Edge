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
 * cshield.ino — ESP32 independente instalado junto da câmara fria.
 *
 * Problema: controladores de refrigeração conectados à internet podem ser invadidos e
 * mandados esquentar (supermercados na França, banco de sangue). O C-Shield mede com
 * sensores próprios, compara com uma política gravada no firmware e, diante de uma ordem
 * perigosa do controlador, corta o aquecimento/degelo por relé, liga a ventoinha de
 * emergência e entra em bloqueio. Alerta no local (LED RGB, buzzer, LCD) e SÓ ENVIA dados
 * ao FIWARE: não existe código que receba ordens pela rede. A liberação do bloqueio é
 * presencial: só um cartão NFC cadastrado libera, e cada
 * liberação ou recusa fica registrada (Serial e histórico no FIWARE).
 *
 * Este arquivo só orquestra: setup() e loop() chamam os módulos.
 *   config.h     constantes          decisao.h   lógica pura (testada no PC)
 *   sensores.h   leitura e validação  atuadores.h relés, LED e buzzer
 *   cartao.h     leitor NFC           operadores.h  cartões autorizados (lógica pura)
 *   painel.h     LCD 16x2            rede.h      Wi-Fi, NTP e MQTT (só publicação)
 */

#include <Arduino.h>
#include "config.h"
#include "decisao.h"
#include "sensores.h"
#include "operadores.h"
#include "cartao.h"
#include "atuadores.h"
#include "painel.h"
#include "rede.h"
#include "simulacao.h"

static Leitura  leitura;      // última leitura montada
static Contexto contexto;     // memória da decisão entre ciclos
static Decisao  decisao;      // última decisão
static char     payload[224]; // última linha UltraLight pronta para publicar
static char     sufixoEvento[64] = "";   // trecho extra do payload de um cartão ainda não publicado
static bool     eventoPendente = false;  // bloqueio novo ou cartão ainda não publicado
static int      totalLiberacoes = 0;     // liberações aceitas desde o boot (atributo nl)
static int      totalRecusas = 0;        // recusas desde o boot (atributo nr)
static unsigned long liberadoAteMs = 0;  // até quando o LCD mostra "LIBERADO POR"
static unsigned long recusadoAteMs = 0;  // até quando o LCD mostra "Cartao recusado"
static unsigned long semBloqueioAteMs = 0; // até quando o LCD mostra "Sem bloqueio" (cartão sem bloqueio ativo)
static char          papelLiberou[LCD_COLUNAS + 1] = "";  // papel mostrado em "LIBERADO POR"
static char          motivoRecusaLcd[16] = "";            // motivo mostrado em "Cartao recusado"
static unsigned long travaRecusaMs = 0;                   // quanto faltava da trava na recusa (LCD)

// Imprime uma linha por ciclo de decisão, com os mesmos nomes do payload.
static void imprimirSerial() {
  char sinal = decisao.taxaCPorMin >= 0 ? '+' : '-';
  Serial.printf("[%s] t=%.1fC h=%.0f%% l=%.0f%% sp=%.1fC dg=%d taxa=%c%.2fC/min st=%s mt=%s trava=%lus apito=%s rede=%s\n",
                ID_DEVICE, leitura.valida ? leitura.temperatura : SETPOINT_AUSENTE,
                leitura.umidade, leitura.luzPct,
                leitura.temSetpoint ? leitura.setpoint : SETPOINT_AUSENTE,
                leitura.degelo ? 1 : 0, sinal, fabsf(decisao.taxaCPorMin),
                nomeEstado(decisao.estado), nomeMotivo(decisao.motivo),
                decisao.travaRestanteMs / 1000UL, decisao.alarme ? "ON" : "OFF",
                redeOk() ? "OK" : "OFF");
  if (decisao.novoBloqueio)
    Serial.printf("[%s] *** BLOQUEIO (%s): %s ***\n", ID_DEVICE, nomeMotivo(decisao.motivo),
                  decisao.cortaAquecedor ? "aquecedor cortado, ventoinha ligada"
                                         : "ataque de FRIO: aquecedor liberado, ventoinha desligada");
  if (decisao.mudancaIgnorada)
    Serial.printf("[%s] ordem do controlador IGNORADA (trava ativa)\n", ID_DEVICE);
}

// Registra uma recusa: log no Serial com hora, contador, LCD e trecho do payload para o FIWARE.
// `quem` é o código do papel (ou o UID sem ":" se o cartão não é cadastrado).
static void registrarRecusa(const char* quem, const char* motivo, unsigned long agora) {
  char hora[24];
  horaTexto(hora, sizeof(hora));
  totalRecusas++;
  Serial.printf("[%s] %s cartao RECUSADO (%s): %s\n", ID_DEVICE, hora, motivo, quem);
  if (strcmp(motivo, "TRAVA") == 0)
    Serial.printf("[%s] faltam %lus da trava minima (conta no tempo da simulacao, que no Wokwi pode andar devagar)\n",
                  ID_DEVICE, (decisao.travaRestanteMs + 999UL) / 1000UL);
  if (strcmp(motivo, "ORDEM") == 0)
    Serial.printf("[%s] o controlador ainda manda ordem invalida: o bloqueio recomecou com trava nova. Volte a chave para NORMAL antes de liberar\n",
                  ID_DEVICE);
  snprintf(motivoRecusaLcd, sizeof(motivoRecusaLcd), "%s", motivo);
  travaRecusaMs = decisao.travaRestanteMs;
  recusadoAteMs = agora + LIBERADO_TELA_MS;
  montarSufixoRecusa(sufixoEvento, sizeof(sufixoEvento), quem, motivo, totalRecusas);
  eventoPendente = true;
}

// Trata o cartão lido desde o ciclo anterior, depois da decisão: libera, recusa (e diz por quê)
// ou avisa que não há bloqueio. Cartão não cadastrado nunca chegou a `avaliar()`.
static void registrarCartao(bool cartao, int operador, const char* uid, unsigned long agora) {
  if (!cartao) return;
  if (operador < 0) {
    char semSeparador[32];
    uidSemSeparador(uid, semSeparador);
    registrarRecusa(semSeparador, "NAO_CADASTRADO", agora);
    return;
  }
  const Operador& op = OPERADORES[operador];
  const char* motivo = motivoRecusa(decisao);
  if (liberacaoEfetiva(decisao)) {
    char hora[24];
    horaTexto(hora, sizeof(hora));
    totalLiberacoes++;
    Serial.printf("[%s] %s bloqueio LIBERADO por %s (%s)\n", ID_DEVICE, hora, op.codigo, op.papel);
    snprintf(papelLiberou, sizeof(papelLiberou), "%s", op.papel);
    liberadoAteMs = agora + LIBERADO_TELA_MS;
    montarSufixoLiberacao(sufixoEvento, sizeof(sufixoEvento), op.codigo, totalLiberacoes);
    eventoPendente = true;
  } else if (motivo != nullptr) {
    registrarRecusa(op.codigo, motivo, agora);
  } else {
    // Cartão válido, mas não havia bloqueio a liberar: só avisa (não vira evento no FIWARE).
    semBloqueioAteMs = agora + LIBERADO_TELA_MS;
    Serial.printf("[%s] cartao %s lido: nao ha bloqueio ativo\n", ID_DEVICE, op.codigo);
  }
}

// Um ciclo de decisão: lê, decide, atua, registra e prepara o payload.
static void cicloDeDecisao(unsigned long agora) {
  int hora = 0;
  leitura.horaValida = horaAtual(hora);
  leitura.hora = hora;

  // Cartão apresentado desde o ciclo anterior: só um operador cadastrado pede a liberação.
  char uidCartao[32] = "";
  int  operador = -1;
  const bool cartao = cartaoConsumir(uidCartao, operador);
  leitura.liberacao = cartao && operador >= 0;

  // Só na demonstração: a ventoinha de emergência (da decisão anterior) esfria a temperatura.
  if (leitura.valida)
    leitura.temperatura = modeloTermicoDemo(leitura.temperatura, decisao.ligaVentoinha,
                                            decisao.bloqueado && !decisao.cortaAquecedor, agora);

  decisao = avaliar(leitura, contexto, agora);
  aplicarReles(decisao.cortaAquecedor, decisao.ligaVentoinha);
  imprimirSerial();

  registrarCartao(cartao, operador, uidCartao, agora);   // depois da decisão: já sabe se liberou
  montarPayload(payload, sizeof(payload), leitura, decisao, contexto.bloqueios, sufixoEvento);
  if (decisao.novoBloqueio) eventoPendente = true;
}

void setup() {
  iniciarReles();             // PRIMEIRO: relés desenergizados = falha segura
  Serial.begin(115200);
  Serial.printf("%s — Edge Computing (%s)\n", NOME_PRODUTO, ID_DEVICE);

  iniciarLedRgb();
  iniciarBuzzer();
  iniciarVentoinhaRefrigeracao();
  iniciarSensores();
  iniciarPainel();
  iniciarLeitorCartao();
  mostrarBoot();
  iniciarContexto(contexto);
  iniciarRede();              // não espera conexão: sem rede o C-Shield funciona igual
}

void loop() {
  unsigned long agora = millis();

  atualizarRede(agora);       // reconexão a cada 10 s e keepalive do MQTT
  atualizarCartao(agora);     // consulta o leitor NFC a cada CARTAO_LEITURA_MS

  if (lerSensores(agora, leitura)) cicloDeDecisao(agora);   // a cada 2 s

  aplicarLed(decisao, agora);
  aplicarBuzzer(decisao, agora);
  aplicarVentoinhaRefrigeracao(decisao.bloqueado, chaveControladorEmNormal());   // a chave só liga, no bloqueio

  // A decisão roda a cada 2 s, mas a contagem da trava precisa fluir segundo a segundo na
  // tela: recalcula o tempo restante a cada volta, só para o LCD (a decisão não muda).
  if (contexto.bloqueado) {
    unsigned long decorrido = agora - contexto.inicioBloqueioMs;
    decisao.travaRestanteMs = decorrido >= TRAVA_MS ? 0 : TRAVA_MS - decorrido;
  }

  // LCD: boot por um instante, resultado do cartão por alguns segundos, senão o estado.
  if (agora < BOOT_TELA_MS) {
    // mantém a tela de boot
  } else if (agora < liberadoAteMs) {
    mostrarLiberadoPor(papelLiberou);       // diz QUEM liberou
  } else if (agora < recusadoAteMs) {
    mostrarCartaoRecusado(motivoRecusaLcd, travaRecusaMs); // diz por que não liberou (e quanto falta da trava)
  } else if (agora < semBloqueioAteMs) {
    mostrarSemBloqueio();                   // o cartão foi lido, mas não há bloqueio para liberar
  } else {
    mostrarEstado(decisao, leitura, agora); // "Estado: ..." e a linha 2 alternando
  }

  // Só publica depois do primeiro ciclo de decisão (antes disso o payload está vazio).
  if (payload[0] != '\0') publicarSePreciso(payload, decisao.estado, eventoPendente, agora);
  // Publicado: o trecho do cartão não precisa mais ir junto (o histórico ganha uma linha por evento).
  if (!eventoPendente) sufixoEvento[0] = '\0';
}
