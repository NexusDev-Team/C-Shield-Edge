# Requisitos — Edge do C-Shield (versão 2)

Projeto: C-Shield (de Cyber Shield), GS 2026.2 "CitizenTech" · Nexus Vision · FIAP · Prof. Dr. Fábio H. Cabrini.
Origem: `PLANO-CSHIELD.md` versão 2 (seções 0 a 2). Prazo de entrega: 11/11/2026 à noite.

Formato: QUANDO <condição>, O C-SHIELD DEVE <resposta>. Cada requisito tem um número fixo (R1, R2...) que os testes e as tarefas citam.

## 1. Princípios do produto

- **R1 — Falha segura.** QUANDO o ESP32 ligar ou reiniciar, O C-SHIELD DEVE colocar os pinos dos dois relés no estado de repouso antes de qualquer outra inicialização, para que a geladeira continue comandada pelo controlador enquanto o firmware sobe.
- **R2 — Só envia.** O firmware NÃO DEVE conter `subscribe`, `setCallback`, OTA nem servidor HTTP. Nenhuma informação vinda da rede altera a política, os relés ou o bloqueio.
- **R3 — Offline funciona.** QUANDO não houver Wi-Fi ou MQTT, O C-SHIELD DEVE manter sensoriamento, detecção, bloqueio, termostato de emergência, alertas locais e liberação presencial sem alteração.
- **R4 — Reconexão sem travar.** QUANDO a rede estiver fora, O C-SHIELD DEVE tentar reconectar a cada 10 s sem parar o `loop()` por mais de 1 s (sonda TCP curta antes do MQTT).
- **R5 — Nunca impedir a refrigeração.** O C-SHIELD NÃO DEVE desligar a ventoinha que o controlador mandou ligar. Ele só PODE cortar o aquecedor e acrescentar ventoinha. A ventoinha efetiva é `vf = ventoinha do C-Shield ligada ? 1 : ventoinha comandada pelo controlador`.

## 2. Sensoriamento

- **R6 — Leitura do DHT22.** A cada 2 s, O C-SHIELD DEVE ler temperatura e umidade. Valor NaN ou temperatura fora de -40 a 80 °C é leitura inválida.
- **R7 — Suavização.** O C-SHIELD DEVE usar a média móvel das últimas 5 leituras válidas de temperatura.
- **R8 — Confirmação.** Os estados CRITICO e FALHA_SENSOR só VALEM após 2 ciclos de decisão seguidos com a condição, para um pico isolado não disparar o alarme.
- **R9 — Porta aberta.** O C-SHIELD DEVE converter o LDR em % (0 = escuro, 100 = claro) e considerar a porta aberta acima de 40%.
- **R10 — Ordens do controlador.** O C-SHIELD DEVE ler o aquecedor comandado (GPIO 18) e a ventoinha comandada (GPIO 17), cada uma com debounce de 50 ms. Pino solto vale desligado.
- **R11 — Perfil de produto.** O C-SHIELD DEVE usar o perfil escolhido em `config.h` (`T_MIN`, `T_MAX`, `T_ALVO` e `TOLERANCIA_C` de 1,0 °C). Padrão: vacina, +2 a +8 °C, alvo 5 °C.

## 3. Estados e detecção

- **R12 — NORMAL.** QUANDO a temperatura está em [T_MIN, T_MAX], a porta está fechada e não há degelo nem ataque, O C-SHIELD DEVE ficar em NORMAL com os relés em repouso.
- **R13 — ATENCAO por porta ou deriva.** QUANDO a porta estiver aberta, OU a temperatura estiver fora de [T_MIN, T_MAX] mas dentro da tolerância, O C-SHIELD DEVE ficar em ATENCAO, com motivo `PORTA_ABERTA` ou `DERIVA`, sem mexer nos relés.
- **R14 — Degelo legítimo.** QUANDO o aquecedor estiver ligado e a ventoinha desligada, dentro do horário permitido (ou sem hora válida, ou em `MODO_DEMO`) e há menos de `DEGELO_MAX_MS`, O C-SHIELD DEVE ficar em ATENCAO com motivo `DEGELO` e informar o tempo restante. Os relés ficam em repouso.
- **R15 — D1, degelo forçado.** QUANDO o aquecedor estiver ligado com a ventoinha desligada e isso acontecer fora dos horários permitidos (hora válida e fora do `MODO_DEMO`) OU durar mais que `DEGELO_MAX_MS`, O C-SHIELD DEVE entrar em BLOQUEIO com motivo `DEGELO_FORCADO`.
- **R16 — D2, aquecedor longo.** QUANDO o aquecedor estiver ligado com a ventoinha ligada por mais que `DEGELO_MAX_MS`, O C-SHIELD DEVE entrar em BLOQUEIO com motivo `AQUECEDOR_LONGO`.
- **R17 — D3, subida rápida.** QUANDO a taxa de aquecimento (janela de 60 s) passar de `TAXA_MAX_C_POR_MIN` com a porta fechada, O C-SHIELD DEVE entrar em BLOQUEIO com motivo `SUBIDA_RAPIDA`. Com a porta aberta é só ATENCAO. Em FALHA_SENSOR a regra não é avaliada.
- **R18 — Janela de degelo permitido.** O horário permitido é 06:00 a 06:30 e 18:00 a 18:30 (relógio local, UTC-3, por NTP). Sem hora válida vale só a duração. Em `MODO_DEMO` a checagem de horário é desligada e `DEGELO_MAX_MS` cai para 15 s.
- **R19 — CRITICO.** QUANDO a temperatura ficar acima de `T_MAX + TOLERANCIA_C` ou abaixo de `T_MIN - TOLERANCIA_C` por 2 ciclos, O C-SHIELD DEVE entrar em CRITICO, com ou sem bloqueio.
- **R20 — FALHA_SENSOR.** QUANDO o sensor ficar inválido por 2 ciclos, O C-SHIELD DEVE entrar em FALHA_SENSOR e não publicar valor velho como se fosse leitura.
- **R21 — Prioridade.** O estado mostrado DEVE seguir CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL. O bloqueio é uma condição persistente separada do estado mostrado.

## 4. Bloqueio e termostato de emergência

- **R22 — Entrada no bloqueio.** QUANDO uma regra D1 a D3 for violada, O C-SHIELD DEVE: acionar o relé 1 (corta o aquecedor), ligar o termostato de emergência, incrementar `bq`, registrar o evento imediato e passar a ignorar o controlador.
- **R23 — Termostato.** DURANTE o bloqueio (e no CRITICO), O C-SHIELD DEVE regular o relé 2: ligar a ventoinha quando a temperatura chegar a `T_ALVO + HISTERESE_C`, soltar quando cair a `T_ALVO - HISTERESE_C` e manter o estado entre os dois. Ao entrar, o termostato começa ligado. Com o relé 2 solto, a ventoinha volta a seguir o controlador (R5).
- **R24 — Controlador ignorado.** DURANTE o bloqueio, toda mudança no aquecedor ou na ventoinha comandados DEVE ser ignorada, sem alterar relés, e DEVE incrementar `ci`.
- **R25 — Sem expiração.** O bloqueio NÃO DEVE terminar por tempo, por rede nem por o controlador voltar ao normal. Só a liberação presencial o encerra.
- **R26 — Liberação presencial.** QUANDO o botão (GPIO 19, `INPUT_PULLUP`, debounce) for mantido por 3 s e a temperatura for válida e estiver em [T_MIN - TOLERANCIA_C, T_MAX + TOLERANCIA_C], O C-SHIELD DEVE encerrar o bloqueio, soltar os relés e devolver o comando ao controlador.
- **R27 — Liberação recusada.** QUANDO o botão for mantido por 3 s com a temperatura fora dessa faixa (ou inválida), O C-SHIELD DEVE continuar no controle e avisar `FORA DA FAIXA` / `AGUARDE`.
- **R28 — Sem rebloqueio.** AO liberar, O C-SHIELD DEVE descartar o histórico de temperatura da janela da taxa, para a subida do episódio antigo não reabrir o bloqueio.
- **R29 — Falha no bloqueio.** QUANDO o sensor falhar DURANTE o bloqueio, O C-SHIELD DEVE ligar a ventoinha direto (sem temperatura, refrigerar é o lado seguro). A regra D3 não é avaliada.
- **R30 — CRITICO sem bloqueio.** QUANDO houver CRITICO sem bloqueio, O C-SHIELD DEVE aplicar as mesmas ações do bloqueio e, ao sair do CRITICO, soltar os relés.
- **R31 — Exposição térmica.** ENQUANTO a temperatura estiver acima de `T_MAX`, O C-SHIELD DEVE acumular `(T - T_MAX) × Δt` em °C·min (`ex`), desde que ligou.

## 5. Alertas locais

- **R32 — LED RGB.** NORMAL verde fixo; ATENCAO amarelo fixo; FALHA_SENSOR amarelo piscando; BLOQUEIO vermelho piscando; CRITICO vermelho fixo.
- **R33 — Buzzer.** NORMAL silêncio; ATENCAO e FALHA_SENSOR 1 bipe a cada 10 s; BLOQUEIO intermitente de 300 ms; CRITICO contínuo. Sempre por `millis()`, sem `delay()`.
- **R34 — LCD 16x2 I2C.** O C-SHIELD DEVE mostrar as telas da seção 5.3 do plano (nome vindo de `NOME_PRODUTO`, no máximo 16 colunas por linha) e só reescrever uma linha quando o texto mudar.
- **R35 — Feedback do botão.** ENQUANTO o botão é segurado, o LCD DEVE mostrar `LIBERANDO...` com barra de 16 blocos; ao aceitar, `BLOQUEIO` / `LIBERADO!` por 3 s; ao recusar, `FORA DA FAIXA` / `AGUARDE` por 3 s.
- **R36 — Restaurando.** QUANDO houver bloqueio e a temperatura estiver acima de `T_MAX`, o LCD DEVE mostrar `RESTAURANDO TEMP` / `BLOQUEIO ATIVO`.

## 6. Serial e FIWARE

- **R37 — Serial por ciclo.** A cada ciclo de decisão: `[cshield001] t=4.8C h=81% l=12% aq=0 vt=1 vf=1 taxa=+0.02C/min st=NORMAL mt=- ci=0 rede=OK`.
- **R38 — Serial de evento.** EM toda mudança de estado, O C-SHIELD DEVE imprimir `>>> <ESTADO>: <MOTIVO> (detalhe)`. Também imprime a liberação aceita, a recusada e cada ordem ignorada.
- **R39 — Payload.** O C-SHIELD DEVE publicar em `/TEF/cshield001/attrs`: `t|..|h|..|l|..|aq|..|vt|..|vf|..|st|..|mt|..|bq|..|ci|..|ex|..`. Com sensor inválido, `t` e `h` vão como -999.
- **R40 — Periodicidade.** A publicação ocorre a cada 5 s e imediatamente em mudança de estado ou bloqueio novo. Sem conexão, a pendência sai na primeira oportunidade.
- **R41 — Provisionamento.** O device `cshield001` (entidade `urn:ngsi-ld:CShield:001`, tipo `CShield`) DEVE ser criado só com atributos, sem `commands`, tratando HTTP 409 como sucesso. Atributos longos: `temperature humidity luminosity heater fanCommand fan state reason blocks ignoredCommands exposure`.
- **R42 — Histórico.** O script DEVE criar subscriptions do Orion para o STH-Comet (porta 8666) de `temperature humidity luminosity fan state blocks ignoredCommands exposure`, sem duplicar ao rodar de novo.
- **R43 — Ponta a ponta.** QUANDO a simulação do Wokwi estiver rodando e o degelo for forçado, `GET /v2/entities/urn:ngsi-ld:CShield:001` DEVE mostrar `state=BLOQUEIO`, `blocks` incrementado e `ignoredCommands` subindo ao mexer nas chaves (tarefa T18).

## 7. Qualidade, Wokwi e demonstração

- **R44 — Constantes.** Todo número com significado vive em `config.h`, com o motivo escrito ao lado. `TAXA_MAX_C_POR_MIN` é provisória (a DPS calibra).
- **R45 — Funções e comentários.** Funções pequenas, comentários em português por módulo e por função não óbvia; nenhum `delay()` no `loop()`.
- **R46 — Integrantes.** Todo arquivo de código começa com o cabeçalho do projeto, da disciplina, do professor e dos cinco integrantes com RM.
- **R47 — Circuito organizado.** O `diagram.json` DEVE separar o lado do controlador (chaves) e o lado da geladeira (relés e LEDs indicadores), com cores por função e rótulos. LED laranja "Aquecedor" pelo contato NF do relé 1; LED azul "Ventoinha" no COM do relé 2.
- **R48 — Modelo térmico da demo.** QUANDO `MODELO_TERMICO_DEMO` for 1, O C-SHIELD DEVE entregar à decisão uma temperatura modelada durante o bloqueio: esfria com a ventoinha efetiva ligada até abaixo do ponto de desligar, aquece em direção à medida com ela desligada, e volta a seguir o slider se a pessoa o mexer. Na maquete física vale 0 e a leitura real passa direto.
- **R49 — Testes.** A lógica de `decisao.h` DEVE rodar no computador com `g++ -std=c++17`, com um caso por requisito de lógica, nas configurações `MODO_DEMO` 1 e 0.

## 8. Dúvidas e valores provisórios

- **D1.** `TAXA_MAX_C_POR_MIN = 0,5` é provisório; a DPS calibra com a maquete (modelo de Newton).
- **D2.** `DEGELO_MAX_MS` de 30 min e as janelas de 30 min às 6h e 18h seguem a v2; a equipe pode ajustar conforme o equipamento real.
- **D3.** O IP da EC2 (`3.215.3.216`) é placeholder; confirmar antes de gravar o vídeo.
- **D4.** `RESFRIAMENTO_DEMO_C_POR_MIN` (6) e `AQUECIMENTO_DEMO_C_POR_MIN` (3) só existem para caber o ciclo do termostato no vídeo.
