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
 * cshield.ino — ESP32 independente instalado junto da câmara fria, entre o controlador
 * comercial e o aquecedor e a ventoinha da geladeira.
 *
 * Problema: controladores de refrigeração conectados à internet podem ser invadidos e
 * mandados esquentar (supermercados na França, banco de sangue). O C-Shield mede com
 * sensores próprios, observa o que o controlador manda e compara com uma política gravada no
 * firmware. Ao reconhecer o ataque (aquecedor ligado com a ventoinha desligada fora do degelo
 * permitido, ou temperatura subindo de repente), assume o controle local: corta o aquecedor
 * por relé, ignora o controlador e regula a ventoinha como termostato de emergência. Alerta
 * no local (LED RGB, buzzer, LCD) e SÓ ENVIA dados ao FIWARE: não existe código que receba
 * ordens pela rede. A liberação do bloqueio é presencial (botão por 3 s), sem expiração.
 *
 * Este arquivo só orquestra: setup() e loop() chamam os módulos.
 *   config.h     constantes          decisao.h    lógica pura (testada no PC)
 *   sensores.h   leitura e debounce  atuadores.h  relés, LED e buzzer
 *   painel.h     LCD 16x2            rede.h       Wi-Fi, NTP e MQTT (só publicação)
 *   simulacao.h  modelo térmico só da demonstração no Wokwi
 */

#include <Arduino.h>
#include "config.h"
#include "decisao.h"
#include "sensores.h"
#include "atuadores.h"
#include "painel.h"
#include "rede.h"
#include "simulacao.h"

static Leitura  leitura;      // última leitura montada
static Contexto contexto;     // memória da decisão entre ciclos
static Decisao  decisao;      // última decisão
static char     payload[192]; // última linha UltraLight pronta para publicar
static bool     eventoPendente = false;  // bloqueio novo ainda não publicado

// Aviso temporário no LCD depois de apertar o botão: liberação aceita ou recusada.
static unsigned long avisoAteMs = 0;
static bool          avisoLiberado = false;   // true = "LIBERADO!", false = "FORA DA FAIXA"

// Detalhe legível do que causou o estado, para a linha de evento do Serial.
static void detalheDoEvento(char* buf, size_t n) {
  switch (decisao.motivo) {
    case MT_DEGELO_FORCADO:
      snprintf(buf, n, "aquecedor ligado e ventoinha desligada ha %lus", decisao.aquecedorLigadoMs / 1000UL);
      break;
    case MT_AQUECEDOR_LONGO:
      snprintf(buf, n, "aquecedor ligado ha %lus", decisao.aquecedorLigadoMs / 1000UL);
      break;
    case MT_SUBIDA_RAPIDA:
      snprintf(buf, n, "taxa %+.2fC/min", decisao.taxaCPorMin);
      break;
    default:
      snprintf(buf, n, "t=%.1fC", leitura.valida ? leitura.temperatura : VALOR_AUSENTE);
  }
}

// Imprime uma linha por ciclo de decisão, com os mesmos nomes do payload, e uma linha de
// evento (">>>") quando o estado mudou.
static void imprimirSerial(Estado estadoAnterior) {
  Serial.printf("[%s] t=%.1fC h=%.0f%% l=%.0f%% aq=%d vt=%d vf=%d taxa=%+.2fC/min st=%s mt=%s ci=%d rede=%s\n",
                ID_DEVICE, leitura.valida ? leitura.temperatura : VALOR_AUSENTE,
                leitura.umidade, leitura.luzPct, leitura.aquecedorCmd ? 1 : 0,
                leitura.ventoinhaCmd ? 1 : 0, ventoinhaEfetiva(decisao, leitura) ? 1 : 0,
                decisao.taxaCPorMin, nomeEstado(decisao.estado), nomeMotivo(decisao.motivo),
                contexto.ignorados, redeOk() ? "OK" : "OFF");
  if (decisao.estado != estadoAnterior) {
    char detalhe[64];
    detalheDoEvento(detalhe, sizeof(detalhe));
    Serial.printf(">>> %s: %s (%s)\n", nomeEstado(decisao.estado), nomeMotivo(decisao.motivo), detalhe);
  }
  if (decisao.novoBloqueio)
    Serial.printf("[%s] *** BLOQUEIO: aquecedor cortado, C-Shield no controle ***\n", ID_DEVICE);
  if (decisao.liberou)
    Serial.printf("[%s] liberacao presencial ACEITA: o controlador volta a mandar\n", ID_DEVICE);
  if (decisao.liberacaoRecusada)
    Serial.printf("[%s] liberacao RECUSADA: temperatura fora da faixa, o C-Shield segue no controle\n", ID_DEVICE);
  if (decisao.mudancaIgnorada)
    Serial.printf("[%s] ordem do controlador IGNORADA (bloqueio ativo), ci=%d\n", ID_DEVICE, contexto.ignorados);
}

// Um ciclo de decisão: lê, decide, atua, registra e prepara o payload.
static void cicloDeDecisao(unsigned long agora) {
  int minuto = 0;
  leitura.horaValida = horaAtual(minuto);
  leitura.minutoDoDia = minuto;
  leitura.liberacao = botaoConsumirLiberacao();

  // Só na demonstração: com o C-Shield no controle, a ventoinha (decisão anterior) esfria a
  // temperatura que o slider do Wokwi não consegue baixar sozinho.
  if (leitura.valida)
    leitura.temperatura = modeloTermicoDemo(leitura.temperatura, decisao.cortarAquecedor,
                                            ventoinhaEfetiva(decisao, leitura), agora);

  const Estado estadoAnterior = decisao.estado;
  decisao = avaliar(leitura, contexto, agora);
  aplicarReles(decisao.cortarAquecedor, decisao.ligarVentoinha);
  imprimirSerial(estadoAnterior);

  montarPayload(payload, sizeof(payload), leitura, decisao, contexto);
  if (decisao.novoBloqueio) eventoPendente = true;
  if (decisao.liberou)           { avisoAteMs = agora + AVISO_TELA_MS; avisoLiberado = true; }
  if (decisao.liberacaoRecusada) { avisoAteMs = agora + AVISO_TELA_MS; avisoLiberado = false; }
}

void setup() {
  iniciarReles();             // PRIMEIRO: relés em repouso = falha segura
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
  atualizarEntradas(agora);   // debounce das chaves e contagem dos 3 s do botão

  if (lerSensores(agora, leitura)) cicloDeDecisao(agora);   // a cada 2 s

  aplicarLed(decisao.estado, agora);
  aplicarBuzzer(decisao.estado, agora);

  // LCD, em ordem de prioridade: boot, aviso do botão, barra de liberação, restauração, estado.
  if (agora < BOOT_TELA_MS) {
    // mantém a tela de boot
  } else if (agora < avisoAteMs) {
    if (avisoLiberado) mostrarLiberado(); else mostrarForaDaFaixa();
  } else if (botaoProgressoPct() > 0) {
    mostrarLiberacao(botaoProgressoPct());
  } else if (contexto.bloqueado && leitura.valida && leitura.temperatura > perfilAtivo().tMax) {
    mostrarRestaurando();     // bloqueio com temperatura ainda alta: a ventoinha está esfriando
  } else {
    mostrarEstado(decisao, leitura, agora);
  }

  // Só publica depois do primeiro ciclo de decisão (antes disso o payload está vazio).
  if (payload[0] != '\0') publicarSePreciso(payload, decisao.estado, eventoPendente, agora);
}
