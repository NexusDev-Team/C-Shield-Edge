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
 * atuadores.h — relés, LED RGB e buzzer.
 *
 * Relé 1 (contato NF) corta o aquecedor/degelo; relé 2 (contato NA) liga a ventoinha
 * de emergência. FALHA SEGURA: relé desenergizado = NF fechado = o equipamento funciona
 * como se o C-Shield não existisse. Por isso iniciarReles() é a PRIMEIRA coisa do setup().
 *
 * O LED e o buzzer não usam delay(): o padrão (fixo, pisca, bipe) é calculado a partir
 * de millis(), então o loop() nunca trava.
 */
#ifndef ATUADORES_H
#define ATUADORES_H

#include <Arduino.h>
#include "config.h"
#include "decisao.h"

// --- Relés ---

// Nível que ENERGIZA o relé: HIGH no Wokwi; LOW em muitos módulos físicos.
static inline int nivelReleAtivo() { return RELE_ATIVO_EM_LOW ? LOW : HIGH; }
static inline int nivelReleInativo() { return RELE_ATIVO_EM_LOW ? HIGH : LOW; }

// Coloca os dois relés desenergizados ANTES de qualquer outra coisa. Escreve o nível
// antes de configurar o pino como saída, para não haver pulso no boot.
static void iniciarReles() {
  digitalWrite(PIN_RELE_AQUEC, nivelReleInativo());
  digitalWrite(PIN_RELE_VENT, nivelReleInativo());
  pinMode(PIN_RELE_AQUEC, OUTPUT);
  pinMode(PIN_RELE_VENT, OUTPUT);
}

// Energiza o relé 1 para cortar o aquecedor e o relé 2 para ligar a ventoinha.
// Só escreve no pino quando o valor muda.
static void aplicarReles(bool cortaAquecedor, bool ligaVentoinha) {
  static int ultimoAquec = -1;
  static int ultimoVent = -1;
  int nAquec = cortaAquecedor ? nivelReleAtivo() : nivelReleInativo();
  int nVent = ligaVentoinha ? nivelReleAtivo() : nivelReleInativo();
  if (nAquec != ultimoAquec) { digitalWrite(PIN_RELE_AQUEC, nAquec); ultimoAquec = nAquec; }
  if (nVent != ultimoVent) { digitalWrite(PIN_RELE_VENT, nVent); ultimoVent = nVent; }
}

// --- LED RGB (catodo comum, um resistor de 220 ohms por canal) ---

enum CorLed { COR_APAGADO, COR_VERDE, COR_AMARELO, COR_VERMELHO };

static int ultimaCorAplicada = -1;   // evita repetir digitalWrite a cada loop()

// Liga os três pinos como saída e apaga o LED.
static void iniciarLedRgb() {
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_B, LOW);
  ultimaCorAplicada = COR_APAGADO;
}

// Acende a cor pedida. Amarelo = vermelho + verde. O canal azul fica sempre apagado.
static void aplicarCor(CorLed cor) {
  if ((int)cor == ultimaCorAplicada) return;
  ultimaCorAplicada = cor;
  digitalWrite(PIN_LED_R, (cor == COR_VERMELHO || cor == COR_AMARELO) ? HIGH : LOW);
  digitalWrite(PIN_LED_G, (cor == COR_VERDE || cor == COR_AMARELO) ? HIGH : LOW);
  digitalWrite(PIN_LED_B, LOW);
}

// Cor do LED por estado. Função pura (só depende dos argumentos).
//   NORMAL verde · ATENCAO amarelo fixo · BLOQUEIO vermelho piscando
//   CRITICO vermelho fixo · FALHA_SENSOR amarelo piscando
static CorLed corDoEstado(Estado e, unsigned long agoraMs) {
  bool faseAcesa = ((agoraMs / LED_PISCA_MS) % 2) == 0;
  switch (e) {
    case ST_NORMAL:       return COR_VERDE;
    case ST_ATENCAO:      return COR_AMARELO;
    case ST_BLOQUEIO:     return faseAcesa ? COR_VERMELHO : COR_APAGADO;
    case ST_CRITICO:      return COR_VERMELHO;
    case ST_FALHA_SENSOR: return faseAcesa ? COR_AMARELO : COR_APAGADO;
  }
  return COR_APAGADO;
}

static void aplicarLed(Estado e, unsigned long agoraMs) { aplicarCor(corDoEstado(e, agoraMs)); }

// --- Buzzer ---

// true se o buzzer deve soar neste instante. Função pura, sem estado.
//   NORMAL        silêncio
//   ATENCAO       1 bipe curto a cada BUZZER_INTERVALO_MS (10 s)
//   FALHA_SENSOR  2 bipes curtos a cada BUZZER_INTERVALO_MS
//   BLOQUEIO      intermitente (liga/desliga a cada BUZZER_INTERM_MS)
//   CRITICO       contínuo
static bool buzzerDeveSoar(Estado e, unsigned long agoraMs) {
  unsigned long fase = agoraMs % BUZZER_INTERVALO_MS;
  switch (e) {
    case ST_NORMAL:       return false;
    case ST_ATENCAO:      return fase < BUZZER_BIPE_MS;
    case ST_FALHA_SENSOR: return fase < BUZZER_BIPE_MS ||
                                 (fase >= 2 * BUZZER_BIPE_MS && fase < 3 * BUZZER_BIPE_MS);
    case ST_BLOQUEIO:     return ((agoraMs / BUZZER_INTERM_MS) % 2) == 0;
    case ST_CRITICO:      return true;
  }
  return false;
}

static void iniciarBuzzer() { pinMode(PIN_BUZZER, OUTPUT); }

// Liga ou desliga o tom só quando o estado do buzzer muda.
static void aplicarBuzzer(Estado e, unsigned long agoraMs) {
  static int ligado = -1;
  int quer = buzzerDeveSoar(e, agoraMs) ? 1 : 0;
  if (quer == ligado) return;
  ligado = quer;
  if (quer) tone(PIN_BUZZER, BUZZER_FREQ_HZ); else noTone(PIN_BUZZER);
}

#endif
