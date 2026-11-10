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
 * rede.h — Wi-Fi, hora por NTP e MQTT SOMENTE PUBLICAÇÃO.
 *
 * Este módulo nunca recebe ordens: o C-Shield só envia dados ao FIWARE, no tópico
 * /TEF/cshield001/attrs (UltraLight 2.0). Não há assinatura de tópico, callback,
 * OTA nem servidor HTTP. "Vê de longe. Só obedece de perto."
 *
 * Sem rede o resto do firmware continua igual: a decisão e as respostas locais não
 * dependem daqui. A reconexão é tentada a cada INTERVALO_RECONEXAO_MS.
 * Antes de cada tentativa de MQTT há uma sonda TCP de no máximo TIMEOUT_SONDA_MS, para o
 * loop() não ficar parado por segundos quando o broker está fora do ar.
 */
#ifndef REDE_H
#define REDE_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include "config.h"
#include "decisao.h"

static WiFiClient   clienteWifi;
static PubSubClient mqtt(clienteWifi);

static unsigned long ultimaTentativaMs = 0;
static bool          primeiraTentativa = true;

// Começa a conexão Wi-Fi e o relógio NTP sem esperar: o setup() não trava.
static void iniciarRede() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  mqtt.setServer(BROKER_MQTT, BROKER_PORTA);
  mqtt.setSocketTimeout(2);            // segundos: limita quanto uma tentativa pode demorar
  configTime(NTP_FUSO_SEGUNDOS, 0, NTP_SERVIDOR);
}

// true quando Wi-Fi e MQTT estão conectados.
static bool redeOk() { return WiFi.status() == WL_CONNECTED && mqtt.connected(); }

// Abre e fecha uma conexão TCP curta só para saber se o broker responde.
static bool brokerAlcancavel() {
  WiFiClient sonda;
  bool ok = sonda.connect(BROKER_MQTT, BROKER_PORTA, TIMEOUT_SONDA_MS);
  sonda.stop();
  return ok;
}

// Tenta reconectar a cada INTERVALO_RECONEXAO_MS, sem insistir no resto do tempo.
// Chamar a cada volta do loop().
static void atualizarRede(unsigned long agoraMs) {
  const bool wifiOk = (WiFi.status() == WL_CONNECTED);
  if (wifiOk && mqtt.connected()) { mqtt.loop(); return; }
  const bool vencido = (agoraMs - ultimaTentativaMs) >= INTERVALO_RECONEXAO_MS;

  if (!wifiOk) {
    // A primeira conexão já foi pedida em iniciarRede(); pedir de novo antes do intervalo
    // interrompe a tentativa em andamento (erro "sta is connecting").
    if (vencido) { ultimaTentativaMs = agoraMs; WiFi.begin(WIFI_SSID, WIFI_SENHA); }
    return;
  }

  // Wi-Fi no ar: tenta o broker já na primeira vez e depois a cada intervalo.
  if (!primeiraTentativa && !vencido) return;
  primeiraTentativa = false;
  ultimaTentativaMs = agoraMs;
  // Sonda rápida: com o broker fora do ar, o mqtt.connect() seguraria o loop() por segundos
  // (e o botão de liberação ficaria sem resposta). A sonda limita o bloqueio a TIMEOUT_SONDA_MS.
  if (!brokerAlcancavel()) {
    Serial.println("Broker MQTT inacessivel; tentando de novo em 10 s");
    return;
  }
  Serial.print("Conectando ao broker MQTT... ");
  Serial.println(mqtt.connect(ID_DEVICE) ? "ok" : "falhou");
}

// Minuto do dia (0 a 1439) pelo NTP. false enquanto o relógio ainda não sincronizou
// (por exemplo, offline): nesse caso vale só a duração máxima do degelo.
static bool horaAtual(int& minutoDoDia) {
  time_t agora = time(nullptr);
  if (agora < 1700000000L) return false;     // antes de nov/2023: relógio ainda não ajustado
  struct tm info;
  localtime_r(&agora, &info);
  minutoDoDia = info.tm_hour * 60 + info.tm_min;
  return true;
}

// Monta o payload UltraLight:
//   t|4.8|h|81|l|12|aq|0|vt|1|vf|1|st|NORMAL|mt|-|bq|0|ci|0|ex|0
// aq/vt = o que o controlador manda; vf = ventoinha efetiva depois do C-Shield; bq = bloqueios;
// ci = comandos ignorados. Com o sensor inválido, t e h vão como -999 (não publica valor velho
// como se fosse leitura).
static void montarPayload(char* buf, size_t n, const Leitura& l, const Decisao& d, const Contexto& ctx) {
  float t = l.valida ? l.temperatura : VALOR_AUSENTE;
  float h = l.valida ? l.umidade : VALOR_AUSENTE;
  snprintf(buf, n, "t|%.1f|h|%.0f|l|%.0f|aq|%d|vt|%d|vf|%d|st|%s|mt|%s|bq|%d|ci|%d|ex|%.1f",
           t, h, l.luzPct, l.aquecedorCmd ? 1 : 0, l.ventoinhaCmd ? 1 : 0,
           ventoinhaEfetiva(d, l) ? 1 : 0, nomeEstado(d.estado), nomeMotivo(d.motivo),
           ctx.bloqueios, ctx.ignorados, d.exposicao);
}

static Estado ultimoEstadoPublicado = ST_NORMAL;
static bool   jaPublicou = false;
static unsigned long ultimaPublicacaoMs = 0;

// Publica o payload a cada PERIODO_PUBLICACAO_MS, e na hora quando o estado mudou ou
// quando `evento` é true (um bloqueio novo). Sem conexão não faz nada: a pendência
// continua e sai na primeira oportunidade. Devolve true se publicou.
static bool publicarSePreciso(const char* payload, Estado estado, bool& evento, unsigned long agoraMs) {
  if (!mqtt.connected()) return false;
  bool mudou = !jaPublicou || estado != ultimoEstadoPublicado;
  bool vencido = (agoraMs - ultimaPublicacaoMs) >= PERIODO_PUBLICACAO_MS;
  if (!(mudou || evento || vencido)) return false;
  bool ok = mqtt.publish(TOPICO_ATTRS, payload);
  if (ok) {
    jaPublicou = true;
    ultimoEstadoPublicado = estado;
    ultimaPublicacaoMs = agoraMs;
    evento = false;
  }
  return ok;
}

#endif
