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
 * painel.h — telas do LCD 16x2 I2C.
 *
 * Cada linha tem no máximo 16 caracteres. Os textos montados aqui são guardados e
 * só reescritos no LCD quando mudam, para o display não piscar. O nome do produto
 * vem sempre de NOME_PRODUTO.
 *
 * Telas (layout do plano, seção 0.3):
 *   Boot          NOME_PRODUTO / Iniciando...
 *   NORMAL        "C-Shield NORMAL" / "4.8C 2a8C"
 *   ATENCAO       "PORTA ABERTA" ou "DERIVA" / temperatura e faixa
 *   BLOQUEIO      "ATAQUE BLOQUEADO" / motivo e tempo de trava
 *   CRITICO       "PRODUTO EM RISCO" / temperatura e exposição
 *   FALHA_SENSOR  "FALHA NO SENSOR" / "DHT22 sem dado"
 *   Liberando     "LIBERANDO..." / barra de progresso de 16 blocos
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

// Texto curto do motivo de bloqueio para a linha 2 (cabe em 16 colunas com o tempo).
static const char* motivoNoLcd(Motivo m) {
  switch (m) {
    case MT_SETPOINT_INVALIDO:   return "SETPOINT";
    case MT_DEGELO_FORA_HORARIO: return "DEGELO HORA";
    case MT_DEGELO_LONGO:        return "DEGELO LONGO";
    case MT_AQUECIMENTO_RAPIDO:  return "AQUEC RAPIDO";
    default:                     return "BLOQUEADO";
  }
}

// Desenha a tela do estado atual. `l` traz a temperatura e o setpoint; `d`, o estado,
// o motivo, a trava restante e a exposição.
static void mostrarEstado(const Decisao& d, const Leitura& l) {
  const Perfil p = perfilAtivo();
  char l1[24], l2[24];

  // Faixa do perfil na linha 2 dos estados com temperatura: "4.8C 2a8C".
  char faixa[24];
  snprintf(faixa, sizeof(faixa), "%.1fC %da%dC", l.temperatura, (int)p.tMin, (int)p.tMax);

  switch (d.estado) {
    case ST_NORMAL:
      snprintf(l1, sizeof(l1), "%s NORMAL", NOME_PRODUTO);
      snprintf(l2, sizeof(l2), "%s", faixa);
      break;
    case ST_ATENCAO:
      snprintf(l1, sizeof(l1), "%s", d.motivo == MT_PORTA_ABERTA ? "PORTA ABERTA" : "DERIVA");
      snprintf(l2, sizeof(l2), "%s", faixa);
      break;
    case ST_BLOQUEIO: {
      snprintf(l1, sizeof(l1), "ATAQUE BLOQUEADO");
      unsigned long s = d.travaRestanteMs / 1000UL;
      char tempo[8];
      if (s >= 100) snprintf(tempo, sizeof(tempo), "%lum", s / 60UL);  // minutos quando longo
      else          snprintf(tempo, sizeof(tempo), "%lus", s);
      if (d.motivo == MT_SETPOINT_INVALIDO)   // mostra o valor recebido
        snprintf(l2, sizeof(l2), "SETPOINT %.0fC %s", l.setpoint, tempo);
      else
        snprintf(l2, sizeof(l2), "%s %s", motivoNoLcd(d.motivo), tempo);
      break;
    }
    case ST_CRITICO:
      snprintf(l1, sizeof(l1), "PRODUTO EM RISCO");
      snprintf(l2, sizeof(l2), "T%.1f EX %.1f", l.temperatura, d.exposicao);
      break;
    case ST_FALHA_SENSOR:
      snprintf(l1, sizeof(l1), "FALHA NO SENSOR");
      snprintf(l2, sizeof(l2), "DHT22 sem dado");
      break;
    default:
      l1[0] = l2[0] = '\0';
  }
  escreverLinha(0, l1);
  escreverLinha(1, l2);
}

// Botão segurado durante a trava mínima: avisa por que ainda não vai liberar.
static void mostrarTravaAtiva(unsigned long travaRestanteMs) {
  char l2[24];
  unsigned long s = travaRestanteMs / 1000UL;
  if (s >= 100) snprintf(l2, sizeof(l2), "AGUARDE %lum", s / 60UL);
  else          snprintf(l2, sizeof(l2), "AGUARDE %lus", s);
  escreverLinha(0, "TRAVA ATIVA");
  escreverLinha(1, l2);
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
