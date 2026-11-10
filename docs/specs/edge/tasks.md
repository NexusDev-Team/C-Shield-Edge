# Tarefas — Edge do C-Shield

Projeto: C-Shield · Nexus Vision · GS 2026.2 "CitizenTech" · FIAP. Origem: `PLANO-CSHIELD.md` §3, já com a correção de escopo (§0), de local (§0.1), de nome (§0.2) e de prazo/display (§0.3). Requisitos: `requirements.md`. Design: `design.md`.

Regras de execução: uma tarefa por vez; cada tarefa termina com um commit em português no formato Conventional Commits, sem co-author; não avançar com testes falhando; passo que falha para e reporta.

**Prazo: 11/11/2026 à noite.** Ordem de corte se apertar: T19 → refinamento do `diagram.json` (T15). T18 está fora da entrega. Nunca cortar: T8, T10 (relés seguros), T13 (sem `subscribe`), T14, T18b e T21.

## Fase A — Especificação

| Tarefa | O que é | Requisitos | Status |
| --- | --- | --- | --- |
| T1 | Ler o plano e a referência da Vinheria; listar o que reaproveita e o que muda | — | Concluída |
| T2 | `requirements.md` | todos | Concluída |
| T3 | `design.md` | todos | Concluída |
| T4 | `tasks.md` (este arquivo) | todos | Concluída |
| T5 | **Parar e pedir aprovação da equipe** | — | Dispensada pelo usuário em 10/11 (execução autônoma com autorrevisão) |

## Fase B — Lógica pura e testes

| Tarefa | Entrega | Requisitos |
| --- | --- | --- |
| T6 | `edge/cshield/config.h`: pinos, perfis, limites, tempos, rede, textos; cada valor com comentário de justificativa; cabeçalho com integrantes | R40, R41, R42, R45 |
| T7 | `edge/cshield/decisao.h`: `Leitura`, `Contexto`, `Decisao` e `avaliar()` sem `Arduino.h` | R14 a R32, R46 |
| T8 | `edge/test/test_decisao.cpp`: um caso por requisito (lista abaixo). Comando: `g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_decisao.cpp -o /tmp/t && /tmp/t` | R14 a R32 |

Casos de T8, com o requisito de cada um:

- setpoint fora da faixa → BLOQUEIO imediato (R20)
- degelo além de `DEGELO_MAX_MS` → BLOQUEIO (R21)
- degelo fora do horário com hora válida → BLOQUEIO; sem hora válida → não bloqueia por horário (R22, R13)
- aquecimento rápido com porta fechada → BLOQUEIO (R23)
- o mesmo com porta aberta → só ATENCAO (R24, R15)
- deriva dentro da tolerância → ATENCAO `DERIVA` (R16)
- sensor NaN por 2 ciclos → FALHA_SENSOR (R7, R19a, R9)
- sensor fora de -40 a 80 °C → FALHA_SENSOR (R7)
- em FALHA_SENSOR a taxa não é avaliada, mas setpoint e degelo continuam (R29)
- trava ignora nova ordem e marca `mudancaIgnorada` (R25)
- liberação antes da trava mínima não libera; depois, com temperatura boa, sai do bloqueio (R26)
- liberação com temperatura além da tolerância → CRITICO (R27)
- liberação com setpoint ainda inválido → volta a BLOQUEIO e `bloqueios` sobe (R28)
- temperatura além da tolerância por 2 ciclos → CRITICO, também para frio (R18, R9, D4)
- um pico isolado não gera CRITICO (R9)
- exposição térmica acumulada acima de `T_MAX` (R31)
- prioridade: CRITICO vence BLOQUEIO, que vence FALHA_SENSOR, ATENCAO e NORMAL (R19)
- relés: `cortaAquecedor` e `ligaVentoinha` verdadeiros em BLOQUEIO e CRITICO, falsos nos demais (R5, R17, R18)
- cada perfil carrega os limites certos (R41)

## Fase C — Firmware

| Tarefa | Entrega | Requisitos |
| --- | --- | --- |
| T9 | `sensores.h`: DHT22 a cada 2 s, validação, média móvel de 5, LDR em %, potenciômetro em °C, GPIO 18 | R6 a R13 |
| T10 | `atuadores.h`: relés com `RELE_ATIVO_EM_LOW`, estado seguro no boot, LED RGB por estado, buzzer não bloqueante | R1, R5, R17, R18, R44 |
| T11 | `painel.h`: telas do LCD 16x2 conforme o plano §0.3, sem redesenhar quando o conteúdo não muda | R42, R43 |
| T12 | Botão de liberação: debounce, 3 s e barra de progresso de 16 blocos | R26 |
| T13 | `rede.h`: Wi-Fi `Wokwi-GUEST`, NTP, MQTT só com `publish`, reconexão a cada 10 s, publicação a cada 5 s e imediata em mudança de estado | R2, R3, R4, R33 a R36 |
| T14 | `cshield.ino`: cabeçalho do projeto; `setup()` e `loop()` só orquestrando; linha do Serial | R1, R39, R44, R45 |

Observação de ordem: T12 mexe em `sensores.h` (detecção do botão) e `painel.h` (barra). Fazer como duas edições separadas, um arquivo por commit.

## Fase D — Wokwi, FIWARE e teste ponta a ponta

| Tarefa | Entrega | Requisitos |
| --- | --- | --- |
| T15 | `edge/cshield/diagram.json` com os componentes do design §7; conferir antes os nomes de pino de `wokwi-relay-module` e `wokwi-lcd1602` | R43 (e circuito da rubrica) |
| T16 | `edge/cshield/libraries.txt`: `PubSubClient`, `DHT sensor library`, `Adafruit Unified Sensor`, `LiquidCrystal I2C` | — |
| T17 | `edge/fiware/provisionar.sh` conforme R37 | R37 |
| ~~T18~~ | **Fora da entrega** (controlador vulnerável); só se a equipe montar a maquete e pedir | — |
| T18b | Teste ponta a ponta sem hardware: Wokwi rodando, `GET /v2/entities/urn:ngsi-ld:CShield:001`, `st` e `bq` mudam com o potenciômetro; comando e resposta em `docs/edge-testes-estados.md`. Se o agente não conseguir rodar o Wokwi, deixar o passo a passo para a equipe e dizer isso no relatório | R36, R38 |

## Fase E — Documentação

| Tarefa | Entrega | Requisitos |
| --- | --- | --- |
| T20 | `docs/edge-testes-estados.md`: por estado, o que mexer no Wokwi, o que aparece em LED, buzzer, LCD, relés e Serial, coluna "resultado" para a equipe; mais a seção da T18b | R14 a R19a |
| T21 | `docs/edge-roteiro-video.md`: roteiro de até 7 min na ordem da rubrica, com os itens 5 e 6 do plano §3 e o lembrete de manter a aba do Wokwi aberta e ligar a EC2 10 minutos antes de gravar | rubrica |
| T19 | (baixa prioridade, por último) `docs/edge-ligacoes-maquete.md` | — |

## Revisão final (plano §4)

Rodar todas as verificações automáticas (testes, grep sem `subscribe`, grep sem `delay()`, `diagram.json` válido, pinos do `config.h` no diagrama, compilação se o `arduino-cli` existir), a conferência contra a rubrica e contra os princípios do produto, e entregar o relatório final com a linha das correções de escopo aplicadas.

## Matriz de cobertura (requisito → tarefa)

| Requisitos | Tarefas |
| --- | --- |
| R1, R5 | T10, T14 |
| R2, R3, R4 | T13 |
| R6 a R13 | T9 |
| R14 a R32 | T7, T8 |
| R33 a R36 | T13 |
| R37 | T17 |
| R38 | T18b |
| R39 | T14 |
| R40, R41, R42 | T6 |
| R43 | T11, T15 |
| R44 | T10, T14 |
| R45 | T6 a T14 |
| R46 | T7, T8 |

## Registro de mudanças

- 10/11/2026: tarefas criadas já com T18 fora, T18b incluída e T19 por último (correção de escopo C1 a C6), nomes C-Shield (N1 a N6) e LCD 16x2 (P2, P3).
- 10/11/2026 (execução): T6 a T17, T18b (roteiro, não executado), T20 e T21 concluídas. T18 fora da entrega. T19 não feita (baixa prioridade, prazo). Compilação C++ e execução no Wokwi não verificadas por falta de `g++`/`arduino-cli` e de acesso ao simulador; ver o relatório final. Dúvidas D1 a D5 do `requirements.md` adotadas como estão.

## Status de execução (10/11/2026)

| Tarefa | Status | Evidência |
| --- | --- | --- |
| T6 `config.h` | Concluída | commit `feat(edge): adiciona config.h ...` |
| T7 `decisao.h` | Concluída | commit da lógica pura; lógica exercitada por porta em JS (34 verificações, 0 falhas) |
| T8 `test_decisao.cpp` | Concluída | g++ (WinLibs 16.2) com `-Wall -Wextra`, sem avisos: 95 verificações, 0 falhas (10/11) |
| T9 a T14 firmware | Concluídas, **não compiladas** | revisão de código; compilar no Wokwi |
| T15, T16 | Concluídas | `diagram.json` válido; 13 pinos conferidos; pinos de relé e LCD a conferir no Wokwi |
| T17 `provisionar.sh` | Concluída | `bash -n` e `DRY_RUN=1` (8 corpos JSON válidos, nenhum com `commands`) |
| T18 | Fora da entrega | correção C2 |
| T18b | Roteiro pronto, **não executado** | EC2 sem resposta em 10/11 |
| T19 | Não feita | baixa prioridade (C3) |
| T20, T21 | Concluídas | `docs/edge-testes-estados.md`, `docs/edge-roteiro-video.md` |
