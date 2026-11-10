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
 * decisao_tipos.h — estados, motivos e as estruturas de entrada, memória e saída da decisão
 * (Leitura, Contexto, Decisao). Puro: sem Arduino.h. Usado por decisao_regras.h e decisao.h.
 */
#ifndef DECISAO_TIPOS_H
#define DECISAO_TIPOS_H

#include "config.h"

// Prioridade, da mais alta para a mais baixa: CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL.
enum Estado { ST_NORMAL, ST_ATENCAO, ST_BLOQUEIO, ST_CRITICO, ST_FALHA_SENSOR };

enum Motivo {
  MT_NENHUM, MT_PORTA_ABERTA, MT_DERIVA, MT_DEGELO, MT_DEGELO_FORCADO, MT_AQUECEDOR_LONGO,
  MT_SUBIDA_RAPIDA, MT_TEMP_FORA_TOLERANCIA, MT_SENSOR_SEM_LEITURA
};

// Entrada de um ciclo de decisão (montada por sensores.h; nos testes, preenchida à mão).
struct Leitura {
  bool  valida;        // o DHT22 devolveu valor utilizável neste ciclo
  float temperatura;   // °C, já suavizada pela média móvel (só vale se valida)
  float umidade;       // %
  float luzPct;        // % do LDR (independe do DHT22)
  bool  aquecedorCmd;  // o controlador está mandando ligar o aquecedor/degelo
  bool  ventoinhaCmd;  // o controlador está mandando ligar a ventoinha/compressor
  bool  horaValida;    // hora do NTP disponível
  int   minutoDoDia;   // 0..1439, só vale se horaValida
  bool  liberacao;     // botão segurado por LIBERACAO_MS (calculado em sensores.h)
};

// Memória entre ciclos. Zere com iniciarContexto().
struct Contexto {
  bool          iniciado;          // já houve um ciclo (para calcular o intervalo)
  unsigned long ultimoMs;          // instante do ciclo anterior
  bool          bloqueado;         // condição persistente de bloqueio (não expira)
  Motivo        motivoBloqueio;
  bool          aquecedorAtivo;    // o aquecedor está comandado desde inicioAquecedorMs
  unsigned long inicioAquecedorMs;
  bool          termostatoLigado;  // ventoinha do C-Shield (relé 2) durante bloqueio/crítico
  int           bloqueios;         // contador publicado em `bq`
  int           ignorados;         // comandos do controlador ignorados, publicado em `ci`
  float         exposicao;         // °C·min acumulados acima de T_MAX (`ex`)
  int           contCritico;       // ciclos seguidos além da tolerância
  int           contInvalido;      // ciclos seguidos com sensor inválido
  float         histTemp[N_AMOSTRAS_TAXA];       // histórico da janela da taxa
  unsigned long histMs[N_AMOSTRAS_TAXA];
  int           histN;
  bool          ultimoAq;          // comandos do ciclo anterior, para detectar mudança
  bool          ultimoVt;
};

// Saída de um ciclo de decisão.
struct Decisao {
  Estado estado;
  Motivo motivo;
  bool   cortarAquecedor;     // relé 1 aciona: abre o NF e corta o aquecedor
  bool   ligarVentoinha;      // relé 2 aciona: o C-Shield liga a ventoinha direto
  bool   novoBloqueio;        // true só no ciclo em que um bloqueio começou (gera evento)
  bool   mudancaIgnorada;     // true quando o bloqueio descartou uma mudança do controlador
  bool   liberou;            // true no ciclo em que a liberação presencial foi aceita
  bool   liberacaoRecusada;  // true quando o botão foi segurado mas a temperatura está fora da faixa
  float  taxaCPorMin;         // para a linha do Serial
  float  exposicao;           // °C·min acumulados, para `ex`
  unsigned long degeloRestanteMs;   // contagem do degelo legítimo (0 fora dele)
  unsigned long aquecedorLigadoMs;  // há quanto tempo o aquecedor está comandado (linha de evento)
};

#endif
