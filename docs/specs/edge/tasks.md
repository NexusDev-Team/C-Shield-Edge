# Tarefas — Edge do C-Shield (versão 2)

Projeto: C-Shield · Nexus Vision · GS 2026.2 "CitizenTech" · FIAP. Origem: `PLANO-CSHIELD.md` versão 2, seção 5. Requisitos: `requirements.md`. Design: `design.md`.

Regras de execução: execução direta e auto-revisada (decisão V3 do plano); um commit por passo verificado, em português, Conventional Commits, sem co-author; não avançar com testes falhando; passo que falha para e reporta.

**Prazo: 11/11/2026 à noite.** Ordem de corte se apertar: T18 (deixar o passo a passo) → refinamento do `diagram.json` → modelo térmico (`MODELO_TERMICO_DEMO 0`). Nunca cortar: testes da `decisao.h`, os 5 estados, falha segura, ausência de `subscribe`, Serial e roteiro do vídeo.

## Fase A — Especificação

| Tarefa | O que é | Status |
| --- | --- | --- |
| T1 | Reaproveitamento e mudanças (plano, seção 5.1) | Concluída |
| T2 | `requirements.md` (R1 a R49) | Concluída |
| T3 | `design.md` | Concluída |
| T4 | `tasks.md` (este arquivo) | Concluída |
| T5 | Aprovação | Dada pelo usuário no replanejamento da v2 (10/11) |

## Fase B — Lógica pura e testes

| Tarefa | Entrega | Requisitos | Status |
| --- | --- | --- | --- |
| T6 | `config.h`: pinos, perfis, limites, tempos, rede; cada valor justificado | R11, R18, R44 | Concluída |
| T7 | `decisao.h`: `Leitura`, `Contexto`, `Decisao`, `avaliar()`, `liberar()` sem `Arduino.h` | R12 a R31 | Concluída |
| T8 | `test_decisao.cpp`: um caso por requisito de lógica, nas duas configurações (`MODO_DEMO` 1 e 0) | R49 | Concluída |

## Fase C — Firmware

| Tarefa | Entrega | Requisitos | Status |
| --- | --- | --- | --- |
| T9 | `sensores.h`: DHT22, validação, média, LDR, chaves e botão com debounce compartilhado | R6 a R10 | Concluída |
| T10 | `atuadores.h`: relés em repouso no boot, LED RGB e buzzer por estado, sem `delay()` | R1, R32, R33 | Concluída |
| T11 | `painel.h`: telas do LCD | R34 a R36 | Concluída |
| T12 | `simulacao.h`: modelo térmico da demo | R48 | Concluída |
| T13 | `rede.h`: payload novo e `minutoDoDia`; sem `subscribe` | R2 a R4, R39, R40 | Concluída |
| T14 | `cshield.ino`: orquestra, Serial por ciclo e de evento | R37, R38, R45 | Concluída |

## Fase D — Wokwi e FIWARE

| Tarefa | Entrega | Requisitos | Status |
| --- | --- | --- | --- |
| T15 | `diagram.json`: chaves do controlador, relés, LEDs indicadores, LCD | R47 | Concluída |
| T16 | `libraries.txt` conferido | — | Concluída |
| T17 | `provisionar.sh` com os atributos novos | R41, R42 | Concluída |
| T18 | Teste ponta a ponta (ou o passo a passo, se a EC2 estiver fora) | R43 | Passo a passo escrito; EC2 fora do ar em 10/11, não executado |

## Fase E — Documentação

| Tarefa | Entrega | Status |
| --- | --- | --- |
| T19 | `docs/edge-testes-estados.md` | Concluída |
| T20 | `docs/edge-roteiro-video.md` | Concluída |
| T21 | `README.md` | Pendente |

## Casos de teste da T8 e requisitos cobertos

| Caso | Requisito |
| --- | --- |
| 1. Degelo legítimo: ATENCAO `DEGELO`, tempo restante, relés em repouso | R14 |
| 2. Degelo além de `DEGELO_MAX_MS`: BLOQUEIO `DEGELO_FORCADO` | R15 |
| 3. Degelo fora do horário com hora válida (só com `MODO_DEMO` 0) | R15, R18 |
| 4. Degelo sem hora válida: só vale a duração | R18 |
| 5. Aquecedor com ventoinha ligada além do tempo: `AQUECEDOR_LONGO` | R16 |
| 6. Subida rápida com porta fechada: `SUBIDA_RAPIDA` | R17 |
| 7. A mesma subida com porta aberta: só ATENCAO | R13, R17 |
| 8. Sensor NaN: 2 ciclos entram em FALHA_SENSOR, 1 não | R8, R20 |
| 9. Sensor fora de -40 a 80 °C é inválido | R6 |
| 10. Chaves mudam no bloqueio: `ci` sobe e relés não mudam | R24 |
| 11. Termostato: começa ligado, desliga em 4 °C, mantém em 5 °C, liga em 6 °C | R23 |
| 12. Bloqueio sem expiração: 2 h depois continua | R25 |
| 13. Liberação com temperatura boa: NORMAL, relés em repouso | R26 |
| 14. Liberação com temperatura ruim: continua bloqueado, recusada | R27 |
| 15. Falha de sensor no bloqueio: ventoinha ligada | R29 |
| 16. CRITICO confirmado: corta aquecedor e liga ventoinha | R19, R30 |
| 17. Exposição: 10 °C por 2 min = 4 °C·min | R31 |
| 18. Prioridade: CRITICO sobre BLOQUEIO, BLOQUEIO sobre FALHA | R21 |
| 19. Após liberar, a subida antiga não rebloqueia | R28 |
| 20. CRITICO sem bloqueio que volta à faixa: relés voltam ao repouso | R30 |
| 21. Ventoinha efetiva: nunca menor que a comandada pelo controlador | R5 |
| 22. NORMAL e ATENCAO por porta ou deriva com relés em repouso | R12, R13 |
