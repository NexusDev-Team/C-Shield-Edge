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
 * simulacao.h — modelo térmico SÓ PARA A DEMONSTRAÇÃO NO WOKWI.
 *
 * O DHT22 do simulador é um slider manual: quando a ventoinha de emergência liga, ele não
 * esfria sozinho. Este módulo representa o efeito físico que a maquete teria: enquanto a

 * ventoinha está ligada, a temperatura que o C-Shield enxerga cai até o centro da faixa do
 * produto; e, num ataque de frio com o aquecedor liberado, ela sobe até o centro. Se a pessoa mexe no slider, o modelo volta a seguir o valor do slider.
 *
 * Fica separado da decisão de propósito: decisao.h não sabe que existe simulação e continua
 * testada só com leituras. Na maquete física, MODELO_TERMICO_DEMO vale 0 e a leitura real
 * do sensor passa direto, sem alteração.
 */
#ifndef SIMULACAO_H
#define SIMULACAO_H

#include <math.h>
#include "config.h"
#include "decisao.h"

static float         simTemperatura = 0.0f;   // temperatura que o modelo entrega
static float         simSliderAnterior = 0.0f; // última leitura do sensor, para detectar mudança
static bool          simIniciado = false;
static unsigned long simUltimoMs = 0;

// Recebe a temperatura medida (já suavizada), se a ventoinha de emergência está ligada e se o
// C-Shield está no controle de um ataque de FRIO com o aquecedor liberado; devolve a
// temperatura que a decisão deve usar.
static float modeloTermicoDemo(float temperaturaMedida, bool ventoinhaLigada,
                               bool aquecedorAquecendo, unsigned long agoraMs) {
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
  const float centro = (p.tMin + p.tMax) / 2.0f;

  if (fabsf(temperaturaMedida - simSliderAnterior) > MODELO_MUDANCA_C) {
    simTemperatura = temperaturaMedida;       // a pessoa mexeu no sensor: segue o novo valor
  } else if (ventoinhaLigada && simTemperatura > centro) {
    simTemperatura -= RESFRIAMENTO_DEMO_C_POR_MIN * dtMin;   // a ventoinha esfria a câmara
    if (simTemperatura < centro) simTemperatura = centro;
  } else if (aquecedorAquecendo && simTemperatura < centro) {
    simTemperatura += AQUECIMENTO_DEMO_C_POR_MIN * dtMin;    // o aquecedor liberado esquenta a câmara
    if (simTemperatura > centro) simTemperatura = centro;
  }
  simSliderAnterior = temperaturaMedida;
  return simTemperatura;
#else
  (void)ventoinhaLigada; (void)aquecedorAquecendo; (void)agoraMs;
  return temperaturaMedida;
#endif
}

#endif
