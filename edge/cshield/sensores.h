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
 * sensores.h — leitura dos sensores e do que o controlador comercial está mandando.
 *
 *   DHT22         temperatura e umidade próprias do C-Shield
 *   LDR           luminosidade (porta aberta)
 *   chave GPIO 32 setpoint observado: NORMAL ou ATAQUE (só no Wokwi)
 *   GPIO 18       degelo comandado pelo controlador
 *   botão         liberação presencial (segurar por LIBERACAO_MS)
 *
 * Aqui só se lê e se valida. Quem decide é decisao.h.
 */
#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"
#include "decisao.h"

static DHT dht(PIN_DHT, DHT22);

// --- Média móvel da temperatura (só entram leituras válidas) ---
static float bufferTemp[JANELA_MEDIA];
static int   qtdTemp = 0;      // quantas leituras já há no buffer (até JANELA_MEDIA)
static int   posTemp = 0;      // próxima posição a escrever

// Acrescenta uma leitura válida e devolve a média das últimas JANELA_MEDIA.
static float mediaMovel(float novo) {
  bufferTemp[posTemp] = novo;
  posTemp = (posTemp + 1) % JANELA_MEDIA;
  if (qtdTemp < JANELA_MEDIA) qtdTemp++;
  float soma = 0;
  for (int i = 0; i < qtdTemp; i++) soma += bufferTemp[i];
  return soma / (float)qtdTemp;
}

// Lê o LDR e converte para % de luminosidade (0 a 100).
// Invertido de propósito: no módulo de LDR usado (igual ao da Vinheria), o pino AO sobe
// quando escurece. Sem a inversão, 100% significaria escuro total.
static float luminosidadePct() {
  int adc = analogRead(PIN_LDR);
  return (float)map(adc, 0, 4095, 100, 0);
}

// Setpoint que o controlador está mandando, lido da chave NORMAL/ATAQUE (nível alto =
// ATAQUE). Na maquete (TEM_POT_SETPOINT = 0) devolve "ausente".
static float setpointObservado() {
#if TEM_POT_SETPOINT
  return digitalRead(PIN_CHAVE_SP) == HIGH ? SETPOINT_DEMO_ATAQUE : SETPOINT_DEMO_NORMAL;
#else
  return SETPOINT_AUSENTE;
#endif
}

// true quando o controlador está mandando degelo (GPIO 18 em nível alto).
static bool degeloComandado() { return digitalRead(PIN_DEGELO) == HIGH; }

// --- Botão de liberação presencial: debounce + contagem de LIBERACAO_MS ---
static bool          botaoEstavel = false;   // estado já filtrado (true = apertado)
static bool          botaoBruto = false;     // última leitura crua
static unsigned long botaoMudouEm = 0;       // quando a leitura crua mudou
static unsigned long botaoApertadoEm = 0;    // quando o estado estável virou "apertado"
static bool          liberacaoPendente = false; // 3 s completos, esperando o próximo ciclo
static int           botaoProgresso = 0;     // 0 a 100 enquanto segura

// Chamar a CADA volta do loop(): a contagem de 3 s precisa de resolução fina, ao
// contrário da decisão, que roda a cada 2 s.
static void atualizarBotao(unsigned long agoraMs) {
  bool bruto = (digitalRead(PIN_BOTAO) == LOW);   // INPUT_PULLUP: apertado = LOW
  if (bruto != botaoBruto) { botaoBruto = bruto; botaoMudouEm = agoraMs; }

  // Debounce: só aceita a mudança depois de DEBOUNCE_MS estável.
  if (bruto != botaoEstavel && (agoraMs - botaoMudouEm) >= DEBOUNCE_MS) {
    botaoEstavel = bruto;
    if (botaoEstavel) botaoApertadoEm = agoraMs;
  }

  if (botaoEstavel) {
    unsigned long segurado = agoraMs - botaoApertadoEm;
    botaoProgresso = (int)min(100UL, segurado * 100UL / LIBERACAO_MS);
    if (segurado >= LIBERACAO_MS) liberacaoPendente = true;
  } else {
    botaoProgresso = 0;
  }
}

// Progresso da liberação em % (0 quando o botão não está apertado).
static int botaoProgressoPct() { return botaoProgresso; }

// Entrega (e limpa) a liberação concluída. Chamado uma vez por ciclo de decisão.
static bool botaoConsumirLiberacao() {
  bool r = liberacaoPendente;
  liberacaoPendente = false;
  return r;
}

// Liga os sensores e as entradas. O botão usa o pull-up interno.
// true quando a chave do controlador está em NORMAL (ventoinha de refrigeração permitida).
// Só existe no Wokwi; na maquete (TEM_POT_SETPOINT = 0) a ventoinha do controlador é a real.
static bool chaveControladorEmNormal() {
#if TEM_POT_SETPOINT
  return digitalRead(PIN_CHAVE_SP) == LOW;
#else
  return true;
#endif
}

static void iniciarSensores() {
  dht.begin();
  // Sem chave no Wokwi o GPIO 18 fica solto; o pull-down o mantém em LOW (sem degelo)
  // em vez de ler ruído e gerar um bloqueio fantasma.
  pinMode(PIN_DEGELO, INPUT_PULLDOWN);
  pinMode(PIN_CHAVE_SP, INPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);
}

static unsigned long ultimaLeituraMs = 0;
static bool          jaLeu = false;

// A cada PERIODO_LEITURA_MS monta uma Leitura nova e devolve true. Entre um ciclo e
// outro devolve false sem tocar na saída. A hora e a liberação são preenchidas depois,
// pelo cshield.ino (vêm da rede e do botão).
static bool lerSensores(unsigned long agoraMs, Leitura& saida) {
  if (jaLeu && (agoraMs - ultimaLeituraMs) < PERIODO_LEITURA_MS) return false;
  jaLeu = true;
  ultimaLeituraMs = agoraMs;

  float t = dht.readTemperature();
  float u = dht.readHumidity();

  saida.valida = temperaturaValida(t);               // NaN ou fora de -40 a 80 °C = inválida
  if (saida.valida) saida.temperatura = mediaMovel(t);
  saida.umidade = (u != u) ? 0.0f : u;               // umidade NaN vira 0 (não decide nada)
  saida.luzPct = luminosidadePct();
#if TEM_POT_SETPOINT
  saida.temSetpoint = true;
#else
  saida.temSetpoint = false;
#endif
  saida.setpoint = setpointObservado();
  saida.degelo = degeloComandado();
  return true;
}

#endif
