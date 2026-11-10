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
 * sensores.h — leitura dos sensores e do que o controlador comercial está mandando.
 *
 *   DHT22      temperatura e umidade próprias do C-Shield
 *   LDR        luminosidade (porta aberta)
 *   GPIO 18    aquecedor/degelo comandado pelo controlador (chave no Wokwi)
 *   GPIO 17    ventoinha/compressor comandado pelo controlador (chave no Wokwi)
 *   botão      liberação presencial (segurar por LIBERACAO_MS)
 *
 * Aqui só se lê e se valida. Quem decide é decisao.h. As duas chaves e o botão usam o mesmo
 * filtro de debounce (struct Debounce), para não repetir a lógica.
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

// --- Debounce: aceita a mudança de uma entrada só depois de DEBOUNCE_MS estável ---
struct Debounce {
  bool          estavel;      // valor já filtrado
  bool          bruto;        // última leitura crua
  unsigned long mudouEm;      // quando a leitura crua mudou
  unsigned long estavelDesde; // quando o valor filtrado mudou
};

// Chamar a cada volta do loop(). Devolve o valor filtrado.
static bool atualizarDebounce(Debounce& d, bool leituraBruta, unsigned long agoraMs) {
  if (leituraBruta != d.bruto) { d.bruto = leituraBruta; d.mudouEm = agoraMs; }
  if (d.bruto != d.estavel && (agoraMs - d.mudouEm) >= DEBOUNCE_MS) {
    d.estavel = d.bruto;
    d.estavelDesde = agoraMs;
  }
  return d.estavel;
}

static Debounce dbAquecedor;   // chave do controlador: aquecedor
static Debounce dbVentoinha;   // chave do controlador: ventoinha
static Debounce dbBotao;       // botão de liberação (estável = apertado)

// --- Botão de liberação presencial: contagem de LIBERACAO_MS ---
static bool liberacaoPendente = false;   // 3 s completos, esperando o próximo ciclo
static int  botaoProgresso = 0;          // 0 a 100 enquanto segura

// Chamar a CADA volta do loop(): a contagem de 3 s precisa de resolução fina, ao contrário
// da decisão, que roda a cada 2 s.
static void atualizarEntradas(unsigned long agoraMs) {
  atualizarDebounce(dbAquecedor, digitalRead(PIN_CHAVE_AQ) == HIGH, agoraMs);
  atualizarDebounce(dbVentoinha, digitalRead(PIN_CHAVE_VT) == HIGH, agoraMs);
  atualizarDebounce(dbBotao, digitalRead(PIN_BOTAO) == LOW, agoraMs);   // INPUT_PULLUP: apertado = LOW

  if (dbBotao.estavel) {
    unsigned long segurado = agoraMs - dbBotao.estavelDesde;
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

// Liga os sensores e as entradas. As chaves usam pull-down: pino solto vale desligado, nunca
// ruído. O botão usa o pull-up interno.
static void iniciarSensores() {
  dht.begin();
  pinMode(PIN_CHAVE_AQ, INPUT_PULLDOWN);
  pinMode(PIN_CHAVE_VT, INPUT_PULLDOWN);
  pinMode(PIN_BOTAO, INPUT_PULLUP);
}

static unsigned long ultimaLeituraMs = 0;
static bool          jaLeu = false;

// A cada PERIODO_LEITURA_MS monta uma Leitura nova e devolve true. Entre um ciclo e outro
// devolve false sem tocar na saída. A hora e a liberação são preenchidas depois, pelo
// cshield.ino (vêm da rede e do botão).
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
  saida.aquecedorCmd = dbAquecedor.estavel;
  saida.ventoinhaCmd = dbVentoinha.estavel;
  return true;
}

#endif
