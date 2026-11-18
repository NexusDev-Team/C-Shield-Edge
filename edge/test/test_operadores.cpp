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
 * test_operadores.cpp — testes da lógica pura dos cartões (operadores.h), sem placa.
 * Cobre a busca do operador pelo UID, a formatação do UID lido pelo leitor e a
 * tradução de uma decisão de liberação em aceita ou recusada (com o motivo).
 *
 * Compilar e rodar:
 *   g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_operadores.cpp -o /tmp/o && /tmp/o
 */
#include <cstdio>
#include <cstring>
#include "operadores.h"

static int falhas = 0;
static int total = 0;

#define CHECK(cond)                                                              \
  do {                                                                           \
    total++;                                                                     \
    if (!(cond)) {                                                               \
      falhas++;                                                                  \
      std::printf("  FALHOU (linha %d): %s\n", __LINE__, #cond);                 \
    }                                                                            \
  } while (0)

static void teste_busca_cadastrados() {
  std::printf("busca de operador cadastrado\n");
  CHECK(buscarOperador("01:02:03:04") >= 0);
  CHECK(std::strcmp(OPERADORES[buscarOperador("01:02:03:04")].codigo, "RT01") == 0);
  CHECK(std::strcmp(OPERADORES[buscarOperador("11:22:33:44")].codigo, "ENF01") == 0);
  CHECK(std::strcmp(OPERADORES[buscarOperador("55:66:77:88")].codigo, "MAN01") == 0);
  CHECK(std::strcmp(OPERADORES[buscarOperador("C0:FF:EE:99")].codigo, "GER01") == 0);
}

static void teste_busca_tag_de_7_bytes() {
  std::printf("busca de tag NFC de 7 bytes\n");
  int i = buscarOperador("04:11:22:33:44:55:66");
  CHECK(i >= 0);
  CHECK(std::strcmp(OPERADORES[i].codigo, "SUP01") == 0);
}

static void teste_busca_desconhecido() {
  std::printf("cartao nao cadastrado e entradas invalidas\n");
  CHECK(buscarOperador("AA:BB:CC:DD") == -1);   // cartão vermelho do Wokwi
  CHECK(buscarOperador("") == -1);
  CHECK(buscarOperador(nullptr) == -1);
  CHECK(buscarOperador("01:02:03") == -1);      // UID parcial não vale
  CHECK(buscarOperador("01:02:03:04:05") == -1);
}

static void teste_papel_cabe_no_lcd() {
  std::printf("papel cabe em uma linha do LCD (16 colunas)\n");
  for (int i = 0; i < N_OPERADORES; i++) CHECK(std::strlen(OPERADORES[i].papel) <= 16);
}

static void teste_uid_para_texto() {
  std::printf("formatacao do UID lido\n");
  char s[32];
  const uint8_t classico[4] = {0x01, 0x02, 0x03, 0x04};
  uidParaTexto(classico, 4, s);
  CHECK(std::strcmp(s, "01:02:03:04") == 0);
  const uint8_t letras[4] = {0xaa, 0xbb, 0xcc, 0xdd};
  uidParaTexto(letras, 4, s);
  CHECK(std::strcmp(s, "AA:BB:CC:DD") == 0);   // sempre maiúsculo
  const uint8_t tag[7] = {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  uidParaTexto(tag, 7, s);
  CHECK(std::strcmp(s, "04:11:22:33:44:55:66") == 0);
  CHECK(buscarOperador(s) >= 0);              // o que o leitor monta é o que a tabela guarda
}

static void teste_uid_sem_separador() {
  std::printf("UID sem separador para o payload\n");
  char s[32];
  uidSemSeparador("AA:BB:CC:DD", s);
  CHECK(std::strcmp(s, "AABBCCDD") == 0);
}

static Decisao decisaoVazia() { return Decisao(); }

static void teste_motivo_recusa() {
  std::printf("motivo de recusa da liberacao\n");
  Decisao d = decisaoVazia();
  CHECK(motivoRecusa(d) == nullptr);            // nada a recusar
  CHECK(liberacaoEfetiva(d) == false);

  d = decisaoVazia(); d.liberacaoRecusada = true;
  CHECK(std::strcmp(motivoRecusa(d), "TRAVA") == 0);
  CHECK(!liberacaoEfetiva(d));

  d = decisaoVazia(); d.liberou = true;         // liberou limpo
  CHECK(motivoRecusa(d) == nullptr);
  CHECK(liberacaoEfetiva(d));

  d = decisaoVazia(); d.liberou = true; d.novoBloqueio = true;   // ordem ainda inválida
  CHECK(std::strcmp(motivoRecusa(d), "ORDEM") == 0);
  CHECK(!liberacaoEfetiva(d));

  d = decisaoVazia(); d.liberou = true; d.estado = ST_CRITICO;   // temperatura ainda ruim
  CHECK(std::strcmp(motivoRecusa(d), "TEMPERATURA") == 0);
  CHECK(!liberacaoEfetiva(d));
}

int main() {
  teste_busca_cadastrados();
  teste_busca_tag_de_7_bytes();
  teste_busca_desconhecido();
  teste_papel_cabe_no_lcd();
  teste_uid_para_texto();
  teste_uid_sem_separador();
  teste_motivo_recusa();
  std::printf("\n%d verificacoes, %d falhas\n", total, falhas);
  return falhas == 0 ? 0 : 1;
}
