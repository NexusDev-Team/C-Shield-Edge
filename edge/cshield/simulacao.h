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
 * simulacao.h — modelo térmico SÓ PARA A DEMONSTRAÇÃO NO WOKWI.
 *
 * O DHT22 do simulador é um slider manual: quando a ventoinha liga, ele não esfria sozinho.
 * Este módulo representa o efeito físico que a maquete teria enquanto o C-Shield está no
 * controle (bloqueio ou crítico):
 *   - com a ventoinha efetiva ligada, a temperatura que a decisão enxerga cai até um piso
 *     logo abaixo do ponto em que o termostato desliga;
 *   - com a ventoinha desligada, ela volta a subir em direção ao valor do slider (o calor do
 *     ambiente), sem passar dele. Assim o termostato liga e desliga sozinho no vídeo.
 * Se a pessoa mexer no slider, o modelo volta a seguir o valor dele. Fora do controle do
 * C-Shield, o valor é mantido.
 *
 * Fica separado da decisão de propósito: decisao.h não sabe que a simulação existe e é
 * testada só com leituras. Na maquete física, MODELO_TERMICO_DEMO vale 0 e a leitura real
 * do sensor passa direto, sem alteração.
 */
#ifndef SIMULACAO_H
#define SIMULACAO_H

#include <math.h>
#include "config.h"
#include "decisao.h"

static float         simTemperatura = 0.0f;    // temperatura que o modelo entrega
static float         simSliderAnterior = 0.0f; // última leitura do sensor, para detectar mudança
static bool          simIniciado = false;
static unsigned long simUltimoMs = 0;

// Recebe a temperatura medida (já suavizada), se o C-Shield está no controle (relé 1 acionado)
// e se a ventoinha está efetivamente ligada; devolve a temperatura que a decisão deve usar.
static float modeloTermicoDemo(float temperaturaMedida, bool noControle, bool ventoinhaLigada,
                               unsigned long agoraMs) {
#if MODELO_TERMICO_DEMO
  if (!simIniciado) {
    simIniciado = true;
    simTemperatura = temperaturaMedida;
    simSliderAnterior = temperaturaMedida;
    simUltimoMs = agoraMs;
    return simTemperatura;
  }
  const float dtMin = (float)(agoraMs - simUltimoMs) / 60000.0f;
  simUltimoMs = agoraMs;
  const Perfil p = perfilAtivo();
  // Piso: meio grau abaixo do ponto de desligar, para o termostato desligar de fato.
  const float piso = p.tAlvo - HISTERESE_C - 0.5f;

  if (fabsf(temperaturaMedida - simSliderAnterior) > MODELO_MUDANCA_C) {
    simTemperatura = temperaturaMedida;                          // a pessoa mexeu no sensor
  } else if (noControle && ventoinhaLigada) {
    simTemperatura -= RESFRIAMENTO_DEMO_C_POR_MIN * dtMin;       // a ventoinha esfria a câmara
    if (simTemperatura < piso) simTemperatura = piso;
  } else if (noControle && simTemperatura < temperaturaMedida) {
    simTemperatura += AQUECIMENTO_DEMO_C_POR_MIN * dtMin;        // o calor do ambiente volta
    if (simTemperatura > temperaturaMedida) simTemperatura = temperaturaMedida;
  }
  simSliderAnterior = temperaturaMedida;
  return simTemperatura;
#else
  (void)noControle; (void)ventoinhaLigada; (void)agoraMs;
  return temperaturaMedida;
#endif
}

#endif
