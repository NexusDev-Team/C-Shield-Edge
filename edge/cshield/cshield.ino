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
 * presencial (botão por 3 s).
 *
 * Este arquivo só orquestra: setup() e loop() chamam os módulos.
 *   config.h     constantes          decisao.h   lógica pura (testada no PC)
 *   sensores.h   leitura e validação  atuadores.h relés, LED e buzzer
 *   painel.h     LCD 16x2            rede.h      Wi-Fi, NTP e MQTT (só publicação)
 */

#include <Arduino.h>
#include "config.h"
#include "decisao.h"
#include "sensores.h"
#include "atuadores.h"
#include "painel.h"
#include "rede.h"

static Leitura  leitura;      // última leitura montada
static Contexto contexto;     // memória da decisão entre ciclos
static Decisao  decisao;      // última decisão
static char     payload[160]; // última linha UltraLight pronta para publicar
static bool     eventoPendente = false;  // bloqueio novo ainda não publicado
static unsigned long liberadoAteMs = 0;  // até quando o LCD mostra "BLOQUEIO LIBERADO"

// Imprime uma linha por ciclo de decisão, com os mesmos nomes do payload.
static void imprimirSerial() {
  char sinal = decisao.taxaCPorMin >= 0 ? '+' : '-';
  Serial.printf("[%s] t=%.1fC h=%.0f%% l=%.0f%% sp=%.1fC dg=%d taxa=%c%.2fC/min st=%s mt=%s trava=%lus rede=%s\n",
                ID_DEVICE, leitura.valida ? leitura.temperatura : SETPOINT_AUSENTE,
                leitura.umidade, leitura.luzPct,
                leitura.temSetpoint ? leitura.setpoint : SETPOINT_AUSENTE,
                leitura.degelo ? 1 : 0, sinal, fabsf(decisao.taxaCPorMin),
                nomeEstado(decisao.estado), nomeMotivo(decisao.motivo),
                decisao.travaRestanteMs / 1000UL, redeOk() ? "OK" : "OFF");
  if (decisao.novoBloqueio)
    Serial.printf("[%s] *** BLOQUEIO (%s): aquecedor cortado, ventoinha ligada ***\n",
                  ID_DEVICE, nomeMotivo(decisao.motivo));
  if (decisao.liberou)
    Serial.printf("[%s] liberacao presencial ACEITA: bloqueio encerrado\n", ID_DEVICE);
  if (decisao.liberacaoRecusada)
    Serial.printf("[%s] liberacao RECUSADA: a trava minima ainda tem %lus\n",
                  ID_DEVICE, decisao.travaRestanteMs / 1000UL);
  if (decisao.mudancaIgnorada)
    Serial.printf("[%s] ordem do controlador IGNORADA (trava ativa)\n", ID_DEVICE);
}

// Um ciclo de decisão: lê, decide, atua, registra e prepara o payload.
static void cicloDeDecisao(unsigned long agora) {
  int hora = 0;
  leitura.horaValida = horaAtual(hora);
  leitura.hora = hora;
  leitura.liberacao = botaoConsumirLiberacao();

  decisao = avaliar(leitura, contexto, agora);
  aplicarReles(decisao.cortaAquecedor, decisao.ligaVentoinha);
  imprimirSerial();

  montarPayload(payload, sizeof(payload), leitura, decisao, contexto.bloqueios);
  if (decisao.novoBloqueio) eventoPendente = true;
  // A decisão só marca `liberou` num ciclo; guarda o prazo para a tela confirmar por mais tempo.
  if (decisao.liberou) liberadoAteMs = agora + LIBERADO_TELA_MS;
}

void setup() {
  iniciarReles();             // PRIMEIRO: relés desenergizados = falha segura
  Serial.begin(115200);
  Serial.printf("%s — Edge Computing (%s)\n", NOME_PRODUTO, ID_DEVICE);

  iniciarLedRgb();
  iniciarBuzzer();
  iniciarSensores();
  iniciarPainel();
  mostrarBoot();
  iniciarContexto(contexto);
  iniciarRede();              // não espera conexão: sem rede o C-Shield funciona igual
}

void loop() {
  unsigned long agora = millis();

  atualizarRede(agora);       // reconexão a cada 10 s e keepalive do MQTT
  atualizarBotao(agora);      // debounce e contagem dos 3 s

  if (lerSensores(agora, leitura)) cicloDeDecisao(agora);   // a cada 2 s

  aplicarLed(decisao.estado, agora);
  aplicarBuzzer(decisao.estado, agora);

  // A decisão roda a cada 2 s, mas a contagem da trava precisa fluir segundo a segundo na
  // tela: recalcula o tempo restante a cada volta, só para o LCD (a decisão não muda).
  if (contexto.bloqueado) {
    unsigned long decorrido = agora - contexto.inicioBloqueioMs;
    decisao.travaRestanteMs = decorrido >= TRAVA_MS ? 0 : TRAVA_MS - decorrido;
  }

  // LCD: boot por um instante, barra de liberação enquanto segura o botão, senão o estado.
  if (agora < BOOT_TELA_MS) {
    // mantém a tela de boot
  } else if (agora < liberadoAteMs) {
    mostrarLiberado();        // confirma a liberação por LIBERADO_TELA_MS
  } else if (botaoProgressoPct() > 0) {
    if (decisao.travaRestanteMs > 0) mostrarTravaAtiva(decisao.travaRestanteMs);
    else                             mostrarLiberacao(botaoProgressoPct());
  } else {
    mostrarEstado(decisao, leitura);
  }

  // Só publica depois do primeiro ciclo de decisão (antes disso o payload está vazio).
  if (payload[0] != '\0') publicarSePreciso(payload, decisao.estado, eventoPendente, agora);
}
