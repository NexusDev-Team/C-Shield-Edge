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
 * Todas as telas de estado seguem o mesmo molde, para ficar fácil de ler:
 *   linha 1:  "Estado: <nome>"        (Ideal, Atencao, Bloqueio, Critico, Falha)
 *   linha 2:  alterna a cada ALTERNA_TELA_MS entre "Temp: 5.0°C" e as mensagens do estado
 *             (motivo, o que fazer, exposição...)
 *
 * Cada linha tem no máximo 16 caracteres, sem acento (o LCD não tem). Os textos montados aqui
 * são guardados e só reescritos no LCD quando mudam, para o display não piscar. O nome do
 * produto vem sempre de NOME_PRODUTO.
 *
 * Telas especiais:
 *   Boot        NOME_PRODUTO / Iniciando...
 *   Liberado    "LIBERADO POR" / papel do cartão (por LIBERADO_TELA_MS após liberar)
 *   Recusado    "Cartao recusado" / motivo (trava, ordem, temperatura ou cartão desconhecido)
 *   Sem bloqueio "Sem bloqueio" / "Nada a liberar" (cartão lido sem bloqueio ativo)
 */
#ifndef PAINEL_H
#define PAINEL_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "decisao.h"

// Símbolo de grau do LCD HD44780 (código 0xDF, em octal para não colar na letra seguinte).
#define GRAU "\337"

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

// Nome do estado na linha 1 ("Estado: ..."). Em NORMAL a temperatura está na faixa ideal.
static const char* nomeEstadoLcd(Estado e) {
  switch (e) {
    case ST_NORMAL:       return "Ideal";
    case ST_ATENCAO:      return "Atencao";
    case ST_BLOQUEIO:     return "Bloqueio";
    case ST_CRITICO:      return "Critico";
    case ST_FALHA_SENSOR: return "Falha";
  }
  return "?";
}

// Motivo do bloqueio em até 16 colunas. A alteração de setpoint mostra só o motivo: o valor
// recebido não aparece (a temperatura da câmara já tem a sua própria mensagem).
static void motivoNoLcd(char* buf, size_t n, Motivo m) {
  switch (m) {
    case MT_SETPOINT_INVALIDO:   snprintf(buf, n, "Alt. de Setpoint"); break;   // alteração de setpoint
    case MT_DEGELO_FORA_HORARIO: snprintf(buf, n, "Degelo fora hora"); break;
    case MT_DEGELO_LONGO:        snprintf(buf, n, "Degelo longo"); break;
    case MT_AQUECIMENTO_RAPIDO:  snprintf(buf, n, "Aquec. rapido"); break;
    case MT_RESFRIAMENTO_RAPIDO: snprintf(buf, n, "Resfria rapido"); break;
    default:                     snprintf(buf, n, "Bloqueio ativo"); break;
  }
}

// Desenha a tela do estado atual. `l` traz a temperatura e o setpoint; `d`, o estado, o motivo,
// a trava restante e a exposição; `agoraMs` faz a linha 2 alternar entre as mensagens.
static void mostrarEstado(const Decisao& d, const Leitura& l, unsigned long agoraMs) {
  const Perfil p = perfilAtivo();
  char l1[24];
  snprintf(l1, sizeof(l1), "Estado: %s", nomeEstadoLcd(d.estado));

  // Mensagens da linha 2: a temperatura primeiro e depois as do estado.
  char msg[4][24];
  int n = 0;
  if (l.valida) snprintf(msg[n++], sizeof(msg[0]), "Temp: %.1f" GRAU "C", l.temperatura);

  switch (d.estado) {
    case ST_ATENCAO:
      snprintf(msg[n++], sizeof(msg[0]), "%s", d.motivo == MT_PORTA_ABERTA ? "Porta aberta" : "Temp fora faixa");
      break;
    case ST_BLOQUEIO: {
      motivoNoLcd(msg[n++], sizeof(msg[0]), d.motivo);
      if (l.valida && (l.temperatura > p.tMax || l.temperatura < p.tMin))
        snprintf(msg[n++], sizeof(msg[0]), "Restaurando temp");
      unsigned long s = d.travaRestanteMs / 1000UL;
      if (s >= 100)    snprintf(msg[n++], sizeof(msg[0]), "Liberacao em %lum", s / 60UL);
      else if (s > 0)  snprintf(msg[n++], sizeof(msg[0]), "Liberacao em %lus", s);
      else             snprintf(msg[n++], sizeof(msg[0]), "Passe o cartao");
      break;
    }
    case ST_CRITICO:
      if (d.bloqueado) snprintf(msg[n++], sizeof(msg[0]), "Bloqueio ativo");
      snprintf(msg[n++], sizeof(msg[0]), "Exposicao %.1f", d.exposicao);
      break;
    case ST_FALHA_SENSOR:
      snprintf(msg[n++], sizeof(msg[0]), "Sensor sem dado");
      break;
    default:
      break;
  }
  if (n == 0) snprintf(msg[n++], sizeof(msg[0]), "Lendo sensor...");

  escreverLinha(0, l1);
  escreverLinha(1, msg[(agoraMs / ALTERNA_TELA_MS) % (unsigned long)n]);
}

// Confirmação mostrada logo depois de a liberação presencial ser aceita: diz QUEM liberou
// (o papel do cartão, de OPERADORES em config.h).
static void mostrarLiberadoPor(const char* papel) {
  escreverLinha(0, "LIBERADO POR");
  escreverLinha(1, papel);
}

// Cartão lido, mas a liberação não aconteceu. `motivo` é o texto de motivoRecusa() ou
// "NAO_CADASTRADO"; aqui vira uma mensagem curta de até 16 colunas.
static void mostrarCartaoRecusado(const char* motivo) {
  const char* texto = "Recusado";
  if (strcmp(motivo, "TRAVA") == 0)               texto = "Trava ativa";
  else if (strcmp(motivo, "ORDEM") == 0)          texto = "Ordem invalida";
  else if (strcmp(motivo, "TEMPERATURA") == 0)    texto = "Temp. fora faixa";
  else if (strcmp(motivo, "NAO_CADASTRADO") == 0) texto = "Nao cadastrado";
  escreverLinha(0, "Cartao recusado");
  escreverLinha(1, texto);
}

// Cartão lido sem nenhum bloqueio ativo: não há nada a liberar.
static void mostrarSemBloqueio() {
  escreverLinha(0, "Sem bloqueio");
  escreverLinha(1, "Nada a liberar");
}

#endif
