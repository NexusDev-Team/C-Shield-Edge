# Requisitos — Edge do C-Shield

Projeto: C-Shield (de Cyber Shield), GS 2026.2 "CitizenTech" · Nexus Vision · FIAP · Prof. Dr. Fábio H. Cabrini.
Origem: `PLANO-CSHIELD.md` (seções 2.3 a 2.7 e correções 0, 0.1, 0.2 e 0.3). Prazo de entrega: 11/11/2026 à noite.

Convenção: "o C-Shield" é o firmware em `edge/cshield/`. Cada requisito tem um identificador `R<n>`, usado nos testes (`edge/test/test_decisao.cpp`) e em `tasks.md`. Valores entre crases são constantes de `config.h`.

## 1. Princípios do produto

- **R1 — Falha segura no boot.** QUANDO o ESP32 ligar ou reiniciar, O C-SHIELD DEVE colocar os pinos dos dois relés no estado desenergizado antes de qualquer outra inicialização (Serial, Wi-Fi, display, sensores). Com os relés desenergizados, o contato NF do relé 1 permanece fechado e o equipamento funciona como antes.
- **R2 — Só envia.** O C-SHIELD NÃO DEVE conter `subscribe`, `setCallback`, OTA nem servidor HTTP. Nenhum caminho de código altera a política a partir da rede.
- **R3 — Funciona offline.** QUANDO não houver Wi-Fi ou MQTT, O C-SHIELD DEVE manter a decisão e todas as respostas locais (LED, buzzer, LCD, relés) sem alteração.
- **R4 — Reconexão sem travar.** ENQUANTO não houver conexão, O C-SHIELD DEVE tentar reconectar a cada `INTERVALO_RECONEXAO_MS` (10 s) sem bloquear o `loop()`.
- **R5 — Nunca impede a refrigeração.** O C-SHIELD NÃO DEVE desligar nem comandar o compressor. As únicas atuações permitidas são cortar o aquecedor/degelo (relé 1) e ligar a ventoinha de emergência (relé 2).

## 2. Sensoriamento

- **R6 — Período de leitura.** A CADA `PERIODO_LEITURA_MS` (2000 ms, mínimo do DHT22), O C-SHIELD DEVE ler temperatura e umidade do DHT22, a luminosidade do LDR, o setpoint observado e o degelo comandado.
- **R7 — Validação.** QUANDO o DHT22 retornar NaN ou temperatura fora de -40 a 80 °C, O C-SHIELD DEVE marcar a leitura como inválida e não usá-la na média nem na taxa.
- **R8 — Suavização.** O C-SHIELD DEVE calcular a temperatura usada nas decisões como média móvel das últimas `JANELA_MEDIA` (5) leituras válidas.
- **R9 — Confirmação.** O C-SHIELD DEVE exigir `CONFIRMACOES` (2) ciclos consecutivos com a mesma condição antes de declarar CRITICO ou FALHA_SENSOR, para ignorar picos isolados.
- **R10 — Porta aberta.** QUANDO a luminosidade do LDR (em %) for maior que `LIMIAR_PORTA_PCT` (40), O C-SHIELD DEVE considerar a porta aberta.
- **R11 — Setpoint observado.** QUANDO `TEM_POT_SETPOINT` for 1, O C-SHIELD DEVE ler a chave NORMAL/ATAQUE (GPIO 32) e tratar o setpoint como `SETPOINT_DEMO_NORMAL` (5 °C) ou `SETPOINT_DEMO_ATAQUE` (45 °C). QUANDO for 0 (maquete), DEVE considerar o setpoint ausente (`sp = -999`) e NÃO DEVE avaliar a regra de setpoint (R20).
- **R12 — Degelo comandado.** O C-SHIELD DEVE ler o nível do GPIO 18 como "degelo comandado" (1 = ligado).
- **R13 — Hora válida.** QUANDO houver hora obtida por NTP, O C-SHIELD DEVE tratá-la como válida e usá-la nos horários de degelo. QUANDO não houver (offline), DEVE valer somente a duração máxima do degelo (R21).

## 3. Estados e respostas

Prioridade: **CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL** (R19).

- **R14 — NORMAL.** QUANDO a temperatura estiver dentro de [`T_MIN`, `T_MAX`] do perfil ativo, a porta fechada e nenhuma ordem inválida, O C-SHIELD DEVE entrar em NORMAL: LED verde fixo, buzzer em silêncio, LCD com temperatura e faixa, relés desenergizados.
- **R15 — ATENCAO por porta.** QUANDO a porta estiver aberta e não houver estado de prioridade maior, O C-SHIELD DEVE entrar em ATENCAO com motivo `PORTA_ABERTA`: LED amarelo fixo, 1 bipe curto a cada 10 s, LCD `PORTA ABERTA`.
- **R16 — ATENCAO por deriva.** QUANDO a temperatura estiver fora de [`T_MIN`, `T_MAX`] mas dentro de `TOLERANCIA_C` e não houver estado de prioridade maior, O C-SHIELD DEVE entrar em ATENCAO com motivo `DERIVA`, com as mesmas respostas de R15 e LCD `DERIVA`.
- **R17 — BLOQUEIO.** QUANDO uma regra de bloqueio (R20 a R23) for violada, O C-SHIELD DEVE entrar em BLOQUEIO: relé 1 abre o NF (corta aquecedor/degelo), relé 2 liga a ventoinha, LED vermelho piscando, buzzer intermitente, LCD `ATAQUE BLOQUEADO` com o motivo, e publicar evento imediato com motivo e valor recebido.
- **R18 — CRITICO.** QUANDO a temperatura suavizada ficar além da tolerância (abaixo de `T_MIN - TOLERANCIA_C` ou acima de `T_MAX + TOLERANCIA_C`) por `CONFIRMACOES` ciclos, com ou sem bloqueio, O C-SHIELD DEVE entrar em CRITICO: LED vermelho fixo, buzzer contínuo, LCD `PRODUTO EM RISCO` com temperatura e exposição, relés como no BLOQUEIO, e publicar evento com a exposição térmica acumulada.
- **R19 — Prioridade.** QUANDO mais de um estado se aplicar, O C-SHIELD DEVE adotar o de maior prioridade na ordem acima.
- **R19a — FALHA_SENSOR.** QUANDO a leitura do DHT22 for inválida por `CONFIRMACOES` ciclos consecutivos, O C-SHIELD DEVE entrar em FALHA_SENSOR: LED amarelo piscando, buzzer com bipe duplo a cada 10 s, LCD `FALHA NO SENSOR`, e publicar evento de falha.

## 4. Regras de bloqueio

- **R20 — Setpoint perigoso.** QUANDO o setpoint comandado estiver fora de [`T_MIN`, `T_MAX`], O C-SHIELD DEVE entrar em BLOQUEIO imediato com motivo `SETPOINT_INVALIDO`, sem esperar a temperatura subir.
- **R21 — Degelo longo.** QUANDO o degelo ficar comandado por mais de `DEGELO_MAX_MS` (30 min), O C-SHIELD DEVE entrar em BLOQUEIO com motivo `DEGELO_LONGO`.
- **R22 — Degelo fora do horário.** QUANDO o degelo for comandado e a hora for válida e estiver fora das janelas permitidas (`HORA_DEGELO_1` = 6 e `HORA_DEGELO_2` = 18), O C-SHIELD DEVE entrar em BLOQUEIO com motivo `DEGELO_FORA_HORARIO`. Cada janela vai de `HORA_DEGELO_n`:00 até `HORA_DEGELO_n + JANELA_DEGELO_H`:00 (ver dúvida D2).
- **R23 — Aquecimento rápido.** QUANDO a taxa de aquecimento (°C/min, calculada sobre `JANELA_TAXA_MS` = 60 s da temperatura suavizada) for maior que `TAXA_MAX_C_POR_MIN` (0,5) e a porta estiver fechada, O C-SHIELD DEVE entrar em BLOQUEIO com motivo `AQUECIMENTO_RAPIDO`.
- **R24 — Porta aberta atenua a taxa.** QUANDO a mesma taxa ocorrer com a porta aberta, O C-SHIELD DEVE apenas entrar em ATENCAO (R15) e NÃO DEVE bloquear.
- **R25 — Trava.** ENQUANTO estiver em BLOQUEIO, O C-SHIELD DEVE ignorar toda nova mudança vinda do controlador (setpoint e degelo), registrá-la no Serial e manter os relés na posição de bloqueio.
- **R26 — Liberação presencial.** QUANDO o botão (GPIO 19, `INPUT_PULLUP`, com debounce) for mantido pressionado por `LIBERACAO_MS` (3 s) e a trava mínima `TRAVA_MS` já tiver passado (ver dúvida D1), O C-SHIELD DEVE sair do BLOQUEIO e reavaliar. Enquanto o botão é segurado, o LCD mostra `LIBERANDO...` com barra de progresso.
- **R27 — Liberação com temperatura ruim.** QUANDO a liberação for pedida e a temperatura estiver além da tolerância, O C-SHIELD DEVE ir para CRITICO em vez de NORMAL.
- **R28 — Liberação com ordem ainda inválida.** QUANDO a liberação for pedida e a ordem do controlador ainda for perigosa (por exemplo, o setpoint continua fora da faixa), O C-SHIELD DEVE voltar a BLOQUEIO na reavaliação seguinte e contar novo bloqueio.
- **R29 — Falha de sensor e regras.** ENQUANTO estiver em FALHA_SENSOR, O C-SHIELD NÃO DEVE avaliar a regra de taxa (R23), mas DEVE continuar avaliando R20, R21 e R22.
- **R30 — Trava mínima.** `TRAVA_MS` é 30 min; com `MODO_DEMO` igual a 1 cai para 60 s, para o vídeo.

## 5. Exposição térmica

- **R31 — Acúmulo.** ENQUANTO a temperatura suavizada estiver acima de `T_MAX`, O C-SHIELD DEVE acumular `(T - T_MAX) × Δt` em °C·min.
- **R32 — Publicação e exibição.** O C-SHIELD DEVE publicar o acumulado no atributo `ex` e mostrá-lo no LCD no estado CRITICO. O acumulado vale desde o boot (ver dúvida D3).

## 6. Publicação FIWARE

- **R33 — Tópico.** O C-SHIELD DEVE publicar somente em `/TEF/cshield001/attrs` (UltraLight 2.0 sobre MQTT, porta 1883, apikey `TEF`). NÃO DEVE existir tópico de comando.
- **R34 — Payload.** Cada publicação DEVE ser uma linha no formato `t|4.8|h|81|l|12|sp|5.0|dg|0|st|NORMAL|mt|-|bq|0|ex|0`, onde `t` é a temperatura (°C), `h` a umidade (%), `l` a luminosidade (%), `sp` o setpoint observado (°C, `-999` quando ausente), `dg` o degelo comandado (0/1), `st` o estado, `mt` o motivo curto sem espaços (`-` quando não houver), `bq` o contador de bloqueios e `ex` a exposição térmica (°C·min).
- **R35 — Cadência.** O C-SHIELD DEVE publicar a cada `PERIODO_PUBLICACAO_MS` (5 s) e imediatamente a cada mudança de estado.
- **R36 — Publicação a partir do Wokwi.** QUANDO rodando no Wokwi, O C-SHIELD DEVE conectar no Wi-Fi `Wokwi-GUEST` (sem senha) e publicar no broker configurado em `BROKER_MQTT`, de modo que o Orion receba os atributos.
- **R37 — Provisionamento.** O `provisionar.sh` DEVE criar o device `cshield001` (`entity_name` `urn:ngsi-ld:CShield:001`, `entity_type` `CShield`, protocolo `PDI-IoTA-UltraLight`, transporte MQTT, **só `attributes`, sem `commands`**), tratar HTTP 409 como sucesso e criar a subscription do Orion para o STH-Comet (porta 8666) dos atributos `t`, `h`, `l`, `st`, `bq` e `ex`. Headers: `fiware-service: smart`, `fiware-servicepath: /`. O IP da EC2 vem de uma variável no topo do script.
- **R38 — Ponta a ponta.** QUANDO a chave "Controlador" for para ATAQUE com o Wokwi rodando, `GET /v2/entities/urn:ngsi-ld:CShield:001` DEVE mostrar `st` = `BLOQUEIO` e `bq` incrementado (tarefa T18b).

## 7. Serial, configuração e código

- **R39 — Serial.** A CADA ciclo de decisão, O C-SHIELD DEVE imprimir uma linha legível com os mesmos nomes do payload, por exemplo `[cshield001] t=4.8C h=81% l=12% sp=5.0C dg=0 taxa=+0.02C/min st=NORMAL mt=- trava=0s rede=OK`.
- **R40 — Constantes.** TODAS as constantes (pinos, perfis, limites, tempos, rede) DEVEM ficar em `config.h`, cada limite com comentário de justificativa. `TAXA_MAX_C_POR_MIN` DEVE ter comentário dizendo que o valor é provisório e será calibrado pela DPS.
- **R41 — Perfis.** O C-SHIELD DEVE oferecer os perfis `PERFIL_VACINA` (padrão, +2 a +8 °C), `PERFIL_CONGELADO` (≤ -18 °C), `PERFIL_RESFRIADO` (0 a +4 °C) e `PERFIL_PLASMA` (≤ -25 °C), cada um com `T_MIN`, `T_MAX` e `TOLERANCIA_C` (1,0 °C).
- **R42 — Nome do produto.** TODO texto exibido no LCD, no Serial e nos documentos gerados pelo código DEVE ler `NOME_PRODUTO` ("C-Shield").
- **R43 — LCD.** O LCD 16x2 I2C (endereço `LCD_ENDERECO`, `0x27`) DEVE mostrar as telas do layout da seção 0.3 do plano, sem texto maior que 16 colunas, e só ser reescrito quando o conteúdo mudar.
- **R44 — Sem `delay()`.** O `loop()` e os padrões de LED e buzzer NÃO DEVEM usar `delay()`; tudo com `millis()`.
- **R45 — Cabeçalho.** TODO arquivo de código DEVE começar com o cabeçalho do projeto (nome, disciplina, professor e os cinco integrantes com RM), com comentários em português explicando o propósito de cada módulo e função não óbvia.
- **R46 — Lógica testável.** A lógica de decisão DEVE ficar em `decisao.h`, sem `Arduino.h`, de modo que `g++ -std=c++17` a compile e teste no computador.

## 8. Dúvidas para aprovação (T5)

- **D1 — O que `TRAVA_MS` faz.** O plano diz "trava por `TRAVA_MS`" e também "liberação por botão 3 s". Adotado: `TRAVA_MS` é o tempo mínimo de bloqueio; o botão só vale depois dele (30 min reais, 60 s no demo). Alternativa: o botão vale a qualquer momento e `TRAVA_MS` só define por quanto tempo ordens novas são ignoradas.
- **D2 — Janela do degelo.** O plano dá só os horários 6 e 18. Adotado: janela de 1 hora (`JANELA_DEGELO_H = 1`), isto é, 06:00 a 07:00 e 18:00 a 19:00.
- **D3 — Exposição térmica.** Adotado: acumulada desde o boot, sem zerar quando a temperatura volta à faixa.
- **D4 — CRITICO por frio.** Adotado: temperatura abaixo de `T_MIN - TOLERANCIA_C` também é CRITICO (vacina congelada perde eficácia). A exposição só acumula acima de `T_MAX`.
- **D5 — Resposta sonora da FALHA_SENSOR.** O plano não define. Adotado: bipe duplo a cada 10 s, distinto da ATENCAO.
- **D6 — Prazo e commits.** Prazo de 11/11/2026 à noite; commits com escopo (`feat(edge): ...`) a confirmar.

## Alertas e ataque que esfria (revisão de 10/11)

- **R50 — Apito até regularizar.** ENQUANTO houver bloqueio (ou CRITICO) com a temperatura fora de [T_MIN, T_MAX] ou sem leitura válida, e nos primeiros `ALARME_INICIAL_MS` (5 s) de um bloqueio, O C-SHIELD DEVE manter o buzzer tocando. QUANDO a temperatura voltar à faixa, o buzzer DEVE parar.
- **R51 — Bloqueio sem depender do apito.** O fim do apito NÃO DEVE encerrar o bloqueio: só a liberação presencial (R26).
- **R52 — Resposta nos relés por direção.** Num ataque de calor, O C-SHIELD DEVE cortar o aquecedor e ligar a ventoinha de emergência. Num ataque de frio, DEVE deixar o aquecedor liberado e NÃO ligar a ventoinha de emergência, pois esfriar mais estragaria o produto (sangue, vacina). Vale também para CRITICO por frio. A temperatura medida manda: acima de T_MAX vale a resposta de calor, abaixo de T_MIN a de frio; dentro da faixa vale a direção do ataque.
- **R53 — Resfriamento rápido.** QUANDO a taxa de variação passar de -`TAXA_MAX_RESFRIAMENTO_C_POR_MIN` (valor provisório) com a temperatura abaixo do centro da faixa, O C-SHIELD DEVE entrar em BLOQUEIO com motivo `RESFRIAMENTO_RAPIDO`, com a porta aberta ou fechada. Voltar do calor até o centro da faixa não é ataque.
- **R54 — Setpoint baixo.** QUANDO o setpoint comandado ficar abaixo de T_MIN, o bloqueio DEVE ser de frio (R52).
- **R55 — LED azul.** ENQUANTO o bloqueio estiver ativo, o LED RGB DEVE ficar azul fixo, inclusive se o estado mostrado for CRITICO.
- **R56 — Molde do LCD.** O LCD DEVE mostrar `Estado: <nome>` na linha 1 e, na linha 2, alternar a cada 2 s entre `Temp: <t>°C` e as mensagens do estado (motivo, `Restaurando temp`, `Liberacao em Ns`, `Segure botao 3s`, exposição).
- **R57 — Chave do ataque no bloqueio.** ENQUANTO o bloqueio estiver ativo, a chave do controlador que simula o ataque DEVE perder a função: a ventoinha de refrigeração simulada (LED verde, GPIO 16) só pode ser LIGADA de novo (chave em NORMAL) e nunca desligada. Sem bloqueio, ela segue a chave.
