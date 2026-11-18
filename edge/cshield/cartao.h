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
 * cartao.h — leitor de cartão NFC/RFID MFRC522 (liberação presencial do bloqueio).
 *
 * O leitor só LÊ o UID de um cartão encostado nele: é entrada local, não recebe nada da rede.
 * atualizarCartao() roda a cada volta do loop() (mas só conversa com o leitor a cada
 * CARTAO_LEITURA_MS) e guarda o último cartão lido. O cshield.ino pega esse cartão uma vez por
 * ciclo de decisão com cartaoConsumir(), como era feito com o antigo botão.
 *
 * A tradução do UID em operador (e a decisão de liberar) fica em operadores.h e decisao.h.
 */
#ifndef CARTAO_H
#define CARTAO_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"
#include "operadores.h"

static MFRC522 leitorRfid(PIN_RFID_SS, PIN_RFID_RST);

static bool          cartaoPendente = false;        // há um cartão lido esperando o próximo ciclo
static char          cartaoUidPendente[32] = "";    // UID do cartão pendente ("01:02:03:04")
static char          cartaoUidAnterior[32] = "";    // último UID lido, para o antirrepique
static unsigned long cartaoAnteriorEm = 0;          // quando o último UID foi aceito
static unsigned long cartaoConsultouEm = 0;         // última conversa com o leitor

// Liga o SPI nos pinos escolhidos (HSPI) e inicia o leitor.
static void iniciarLeitorCartao() {
  SPI.begin(PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SS);
  leitorRfid.PCD_Init();
}

// Chamar a CADA volta do loop(). Se um cartão novo estiver no leitor, guarda o UID como pendente.
static void atualizarCartao(unsigned long agoraMs) {
  if ((agoraMs - cartaoConsultouEm) < CARTAO_LEITURA_MS) return;
  cartaoConsultouEm = agoraMs;

  if (!leitorRfid.PICC_IsNewCardPresent()) return;
  if (!leitorRfid.PICC_ReadCardSerial()) return;

  char uid[32];
  uidParaTexto(leitorRfid.uid.uidByte, leitorRfid.uid.size, uid);
  leitorRfid.PICC_HaltA();   // manda o cartão "dormir": ele só é lido de novo depois de afastado

  // Antirrepique: o mesmo cartão logo em seguida é o mesmo toque, não uma segunda liberação.
  if (strcmp(uid, cartaoUidAnterior) == 0 && (agoraMs - cartaoAnteriorEm) < CARTAO_ANTIREPIQUE_MS) return;
  strcpy(cartaoUidAnterior, uid);
  cartaoAnteriorEm = agoraMs;

  strcpy(cartaoUidPendente, uid);
  cartaoPendente = true;
}

// Entrega (e limpa) o cartão lido. Chamado uma vez por ciclo de decisão.
// Devolve false se nenhum cartão foi lido; senão copia o UID para `uidSaida` (32 bytes) e
// põe em `operador` o índice na tabela OPERADORES, ou -1 se o cartão não for cadastrado.
static bool cartaoConsumir(char* uidSaida, int& operador) {
  if (!cartaoPendente) return false;
  cartaoPendente = false;
  strcpy(uidSaida, cartaoUidPendente);
  operador = buscarOperador(uidSaida);
  return true;
}

#endif
