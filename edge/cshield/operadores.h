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
 * operadores.h — lógica PURA dos cartões de liberação (sem Arduino.h, testada no computador
 * em edge/test/test_operadores.cpp).
 *
 * O leitor MFRC522 (cartao.h) entrega os bytes do UID; aqui eles viram texto, são procurados
 * na tabela OPERADORES de config.h e, depois da decisão, o resultado da liberação é traduzido
 * em "aceita" ou "recusada" com o motivo. A regra de liberar continua em decisao.h.
 */
#ifndef OPERADORES_H
#define OPERADORES_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "config.h"
#include "decisao.h"

// Índice do operador na tabela OPERADORES, ou -1 se o UID não for cadastrado.
// `uid` é o texto no formato de uidParaTexto ("01:02:03:04"); nullptr e vazio dão -1.
static inline int buscarOperador(const char* uid) {
  if (uid == nullptr || uid[0] == '\0') return -1;
  for (int i = 0; i < N_OPERADORES; i++)
    if (strcmp(uid, OPERADORES[i].uid) == 0) return i;
  return -1;
}

// Converte os bytes do UID em texto "AA:BB:CC:DD" (hexadecimal maiúsculo, separado por ":").
// `saida` precisa de 3 * n bytes (n = 10 no máximo no padrão ISO 14443, então 30).
static inline void uidParaTexto(const uint8_t* bytes, uint8_t n, char* saida) {
  static const char HEX_MAIUSCULO[] = "0123456789ABCDEF";
  size_t k = 0;
  for (uint8_t i = 0; i < n; i++) {
    if (i > 0) saida[k++] = ':';
    saida[k++] = HEX_MAIUSCULO[bytes[i] >> 4];
    saida[k++] = HEX_MAIUSCULO[bytes[i] & 0x0F];
  }
  saida[k] = '\0';
}

// Tira os ":" do UID ("AA:BB:CC:DD" vira "AABBCCDD"). Usado na recusa de cartão desconhecido,
// porque ":" tem significado no payload UltraLight. `saida` precisa do mesmo tamanho de `uidTexto`.
static inline void uidSemSeparador(const char* uidTexto, char* saida) {
  size_t k = 0;
  for (size_t i = 0; uidTexto[i] != '\0'; i++)
    if (uidTexto[i] != ':') saida[k++] = uidTexto[i];
  saida[k] = '\0';
}

// Motivo da recusa de uma liberação, a partir da decisão do ciclo; nullptr se não houve recusa.
//   TRAVA        a trava mínima ainda não acabou (decisao.liberacaoRecusada)
//   ORDEM        a trava acabou, mas o controlador ainda manda ordem inválida: o bloqueio recomeça
//   TEMPERATURA  a trava acabou, mas a temperatura segue além da tolerância (CRITICO)
// (NAO_CADASTRADO é decidido antes, no cshield.ino, porque o cartão nem chega à decisão.)
static inline const char* motivoRecusa(const Decisao& d) {
  if (d.liberacaoRecusada) return "TRAVA";
  if (d.liberou && d.novoBloqueio) return "ORDEM";
  if (d.liberou && d.estado == ST_CRITICO) return "TEMPERATURA";
  return nullptr;
}

// true se o bloqueio foi encerrado de fato por este cartão (liberou e nenhum motivo de recusa).
static inline bool liberacaoEfetiva(const Decisao& d) {
  return d.liberou && motivoRecusa(d) == nullptr;
}

#endif  // OPERADORES_H
