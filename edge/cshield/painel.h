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
 * painel.h — telas do LCD 16x2 I2C.
 *
 * Cada linha tem no máximo 16 caracteres. Os textos montados aqui são guardados e
 * só reescritos no LCD quando mudam, para o display não piscar. O nome do produto
 * vem sempre de NOME_PRODUTO.
 *
 * Telas (linha 1 / linha 2):
 *   Boot          NOME_PRODUTO / Iniciando...
 *   NORMAL        "C-Shield NORMAL" / "5.0C 2a8C"
 *   ATENCAO       "PORTA ABERTA", "DERIVA" ou "DEGELO 0:12" / temperatura e faixa
 *   BLOQUEIO      "ATAQUE BLOQUEADO" / motivo, alternando com "SEGURE BOTAO 3s"
 *   Restaurando   "RESTAURANDO TEMP" / "BLOQUEIO ATIVO" (bloqueio com temperatura alta)
 *   CRITICO       "PRODUTO EM RISCO" / temperatura e exposição
 *   FALHA_SENSOR  "SENSOR SEM LEIT." / "DHT22 sem dado"
 *   Liberando     "LIBERANDO..." / barra de progresso de 16 blocos
 *   Liberado      "BLOQUEIO" / "LIBERADO!" (por AVISO_TELA_MS)
 *   Recusada      "FORA DA FAIXA" / "AGUARDE" (por AVISO_TELA_MS)
 */
#ifndef PAINEL_H
#define PAINEL_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "decisao.h"

static LiquidCrystal_I2C lcd(LCD_ENDERECO, LCD_COLUNAS, LCD_LINHAS);

// Último texto escrito em cada linha (16 caracteres + terminador).
static char ultimaLinha[LCD_LINHAS][LCD_COLUNAS + 1];

// Escreve a linha só se o texto mudou; completa com espaços até 16 colunas para apagar
// restos da tela anterior.
static void escreverLinha(int linha, const char* texto) {
  char buf[LCD_COLUNAS + 1];
  int i = 0;
  for (; i < LCD_COLUNAS && texto[i] != '\0'; i++) buf[i] = texto[i];
  for (; i < LCD_COLUNAS; i++) buf[i] = ' ';
  buf[LCD_COLUNAS] = '\0';
  if (strcmp(buf, ultimaLinha[linha]) == 0) return;
  strcpy(ultimaLinha[linha], buf);
  lcd.setCursor(0, linha);
  lcd.print(buf);
}

// Liga o LCD e a luz de fundo.
static void iniciarPainel() {
  lcd.init();
  lcd.backlight();
  ultimaLinha[0][0] = ultimaLinha[1][0] = '\0';
}

static void mostrarBoot() {
  escreverLinha(0, NOME_PRODUTO);
  escreverLinha(1, "Iniciando...");
}

// Texto curto do motivo de bloqueio para a linha 2 (cabe em 16 colunas).
static const char* motivoNoLcd(Motivo m) {
  switch (m) {
    case MT_DEGELO_FORCADO:  return "DEGELO FORCADO";
    case MT_AQUECEDOR_LONGO: return "AQUEC LONGO";
    case MT_SUBIDA_RAPIDA:   return "SUBIDA RAPIDA";
    default:                 return "BLOQUEADO";
  }
}

// Desenha a tela do estado atual. `l` traz a temperatura; `d`, o estado, o motivo, o degelo
// restante e a exposição. `agoraMs` alterna a linha 2 do bloqueio.
static void mostrarEstado(const Decisao& d, const Leitura& l, unsigned long agoraMs) {
  const Perfil p = perfilAtivo();
  char l1[24], l2[24];

  // Faixa do perfil na linha 2 dos estados com temperatura: "5.0C 2a8C".
  char faixa[24];
  snprintf(faixa, sizeof(faixa), "%.1fC %da%dC", l.temperatura, (int)p.tMin, (int)p.tMax);

  switch (d.estado) {
    case ST_NORMAL:
      snprintf(l1, sizeof(l1), "%s NORMAL", NOME_PRODUTO);
      snprintf(l2, sizeof(l2), "%s", faixa);
      break;
    case ST_ATENCAO:
      if (d.motivo == MT_DEGELO) {
        unsigned long s = d.degeloRestanteMs / 1000UL;   // contagem regressiva m:ss
        snprintf(l1, sizeof(l1), "DEGELO %lu:%02lu", s / 60UL, s % 60UL);
      } else {
        snprintf(l1, sizeof(l1), "%s", d.motivo == MT_PORTA_ABERTA ? "PORTA ABERTA" : "DERIVA");
      }
      snprintf(l2, sizeof(l2), "%s", faixa);
      break;
    case ST_BLOQUEIO:
      snprintf(l1, sizeof(l1), "ATAQUE BLOQUEADO");
      if (((agoraMs / ALTERNA_TELA_MS) % 2) == 0) snprintf(l2, sizeof(l2), "%s", motivoNoLcd(d.motivo));
      else                                        snprintf(l2, sizeof(l2), "SEGURE BOTAO 3s");
      break;
    case ST_CRITICO:
      snprintf(l1, sizeof(l1), "PRODUTO EM RISCO");
      snprintf(l2, sizeof(l2), "T%.1f EX %.1f", l.temperatura, d.exposicao);
      break;
    case ST_FALHA_SENSOR:
      snprintf(l1, sizeof(l1), "SENSOR SEM LEIT.");
      snprintf(l2, sizeof(l2), "DHT22 sem dado");
      break;
    default:
      l1[0] = l2[0] = '\0';
  }
  escreverLinha(0, l1);
  escreverLinha(1, l2);
}

// Bloqueio ativo com a temperatura ainda acima da faixa: a ventoinha está trazendo a
// temperatura de volta.
static void mostrarRestaurando() {
  escreverLinha(0, "RESTAURANDO TEMP");
  escreverLinha(1, "BLOQUEIO ATIVO");
}

// Confirmação mostrada logo depois de a liberação presencial ser aceita.
static void mostrarLiberado() {
  escreverLinha(0, "BLOQUEIO");
  escreverLinha(1, "LIBERADO!");
}

// Liberação recusada: o botão foi segurado mas a temperatura ainda está fora da faixa.
static void mostrarForaDaFaixa() {
  escreverLinha(0, "FORA DA FAIXA");
  escreverLinha(1, "AGUARDE");
}

// Tela de liberação: título e barra de 16 blocos que enche em LIBERACAO_MS.
static void mostrarLiberacao(int progressoPct) {
  char barra[LCD_COLUNAS + 1];
  int cheios = (progressoPct * LCD_COLUNAS) / 100;
  for (int i = 0; i < LCD_COLUNAS; i++) barra[i] = (i < cheios) ? (char)255 : ' ';
  barra[LCD_COLUNAS] = '\0';
  escreverLinha(0, "LIBERANDO...");
  escreverLinha(1, barra);
}

#endif
