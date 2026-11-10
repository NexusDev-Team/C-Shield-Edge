# Plano — Edge Computing do C-Shield (GS 2026.2 · Nexus Vision)

> Plano autoritativo do projeto CShield, recebido do usuário em 10/11/2026. Método: spec-driven. Primeiro a especificação, depois a aprovação da equipe, depois o código. O `CLAUDE.md` manda no processo de trabalho; este plano manda nos detalhes de implementação.

## 0. Correção de escopo de 10/11/2026 (prevalece sobre o restante do plano)

A equipe decidiu: **o vídeo de Edge usa só o simulador Wokwi, sem maquete.** As seis correções abaixo já estão refletidas nas seções afetadas deste plano.

- **C1. Três camadas sem hardware.** A simulação do Wokwi conecta no Wi-Fi `Wokwi-GUEST`, publica no FIWARE da EC2 (tópico `/TEF/cshield001/attrs`) e o site da GS mostra os dados na rota `/ao-vivo`. O firmware não muda por causa disso; apenas garantir que a publicação funcione a partir do Wokwi.
- **C2. T18 (controlador vulnerável) fica fora da entrega de Edge.** Não fazer agora. Só será feito se a equipe montar a maquete para o pitch e pedir.
- **C3. T19 (ligações da maquete) vira baixa prioridade.** Fazer por último, só se sobrar tempo.
- **C4. Nova tarefa T18b — teste ponta a ponta sem hardware** (detalhada na Fase D).
- **C5. T21 (roteiro do vídeo) com novos itens 5 e 6** (detalhados na Fase E).
- **C6. Status de aplicação.** Na data da correção o projeto ainda não tinha começado a Fase A; portanto `requirements.md`, `design.md` e `tasks.md` já nascem com estas mudanças, antes do pedido de aprovação (T5).

O relatório final (§4.4) deve trazer uma linha dizendo quais destas seis correções foram aplicadas.

### 0.1 Correção de local (10/11/2026)

O trabalho de Edge fica num repositório separado do site de Front e Web.

- **L1.** Repositório de Edge: `c-shield-edge` (pasta `C:\Users\isacn\desktop\c-shield-edge`, git inicializado em `main`), no lugar de `guardiao-edge`. O repositório do site será `c-shield-web`.
- **L2.** Nada de Edge havia sido criado em outro lugar: não há arquivos a mover nem commit de remoção ("chore: move código de Edge para repositório separado") a fazer no repositório do site, que ainda não existe. Se algum arquivo de Edge aparecer lá no futuro, aplicar esse commit.
- **L3.** As tarefas seguem neste repositório, com a estrutura de pastas da §2.1.
- **L4.** Nova tarefa **T0 — README com Ficha CitizenTech**: criar `README.md` com a Ficha CitizenTech no topo (texto fornecido pela equipe, sem inventar) e um link para o repositório do site (placeholder até ele existir). Quando o repositório do site for criado, o README dele recebe a mesma ficha e o link para `c-shield-edge`.

### 0.2 Mudança de nome (10/11/2026)

O produto se chama **C-Shield** (de Cyber Shield). As substituições abaixo já foram aplicadas em todo este plano.

- **N1.** `NOME_PRODUTO = "C-Shield"` no `config.h`. Tudo o que aparece no LCD, no Serial e nos documentos lê essa constante.
- **N2.** Identificadores FIWARE: `ID_DEVICE = "cshield001"`, `entity_name = urn:ngsi-ld:CShield:001`, `entity_type = CShield`, tópico `/TEF/cshield001/attrs`. Valem para o `provisionar.sh`, o T18b e os documentos.
- **N3.** Repositórios: `c-shield-edge` (este) e `c-shield-web` (site).
- **N4.** Pasta do firmware `edge/cshield/` com arquivo principal `cshield.ino`. A pasta e o `.ino` precisam ter o mesmo nome, senão o Arduino não compila.
- **N5.** "Guardião" continua nos textos só como descrição do papel do dispositivo; o nome exibido é sempre C-Shield.
- **N6.** A revisão da §4 deve ser rodada de novo depois da troca. Na data da mudança nenhum código existia, então a troca foi só no plano; a revisão completa acontece normalmente ao fim da Fase E, já com os nomes novos.

### 0.3 Prazo, display e integrantes (10/11/2026)

- **P1. Prazo: 11/11/2026 à noite.** Velocidade é prioridade. Ordem de corte se apertar: T19 (já de baixa prioridade) → T18 (já fora) → refinamentos visuais do `diagram.json`. Nunca cortar: testes da `decisao.h` (T8), os 5 estados, falha segura, ausência de `subscribe`, Serial e o roteiro do vídeo (T21).
- **P2. Display: LCD 16x2 I2C no lugar do OLED SSD1306**, por ser mais fácil de usar no físico. Módulo com backpack PCF8574, endereço `0x27` (constante `LCD_ENDERECO` no `config.h`, porque alguns módulos físicos usam `0x3F`), nos mesmos GPIO 21 (SDA) e 22 (SCL). No Wokwi: componente `wokwi-lcd1602` com atributo `"pins": "i2c"`. Biblioteca: `LiquidCrystal I2C` (no lugar de `Adafruit GFX Library` e `Adafruit SSD1306`). No físico, alimentar o LCD em 5 V (pino VIN do ESP32).
- **P3. Layout das telas (16 colunas × 2 linhas; nenhum texto passa de 16 caracteres):**

  | Estado | Linha 1 | Linha 2 |
  | --- | --- | --- |
  | Boot | `NOME_PRODUTO` (ex.: `C-Shield`) | `Iniciando...` |
  | NORMAL | `C-Shield  NORMAL` | `T 4.8C  [2a8C]` (temperatura e faixa do perfil) |
  | ATENCAO | `PORTA ABERTA` ou `DERIVA` | `T 8.6C  [2a8C]` |
  | BLOQUEIO | `ATAQUE BLOQUEADO` | motivo curto + tempo de trava (ex.: `SETPOINT 45C 58s`) |
  | CRITICO | `PRODUTO EM RISCO` | `T12.3 EX 45.2` (temperatura e exposição °C·min) |
  | FALHA_SENSOR | `FALHA NO SENSOR` | `DHT22 sem dado` |
  | Botão segurado | `LIBERANDO...` | barra de progresso de 16 blocos (3 s) |

  Os textos ficam em constantes no `config.h` ou no `painel.h`. O LCD só é reescrito quando o conteúdo muda, para não piscar.
- **P4. Integrantes:** os nomes ficam exatamente como estão na lista da T14; não há nomes a completar.

---

## 1. Contexto e objetivo

### Quem somos e o que estamos fazendo

Somos a equipe **Nexus Vision** (Engenharia de Software, 1º ano, FIAP). Esta é a entrega de **Edge Computing & Computer Systems** da Global Solution 2026.2 "CitizenTech", que vale nota e concorre a prêmio. O professor responsável é o Prof. Dr. Fábio H. Cabrini.

### O problema real

Em outubro e novembro de 2026, invasores entraram pela internet nos controladores de refrigeração de supermercados e de um banco de sangue na Europa e mandaram os equipamentos esquentar:

- França, 11/10: centenas de supermercados; freezers a até 50 °C; o software atacado era da Danfoss.
- Europa, 01 e 02/11: segunda onda em mais de 100 lojas. Na ICA de Estocolmo, o ataque foi às 2h e só foi percebido às 5h30.
- Banco de sangue francês (EFS), 02/11: o plasma, guardado a cerca de -30 °C, teve o controle ajustado para até 110 °C. Só se salvou porque um alarme tocou e um técnico estava de plantão.
- Os atacantes usaram acesso remoto, senhas padrão e firmware desatualizado. Em testes da Claroty, o painel do controlador mostrava temperatura normal enquanto o refrigerador esquentava.

Em São Paulo, os mesmos equipamentos existem em salas de vacina de UBS, hemocentros, farmácias, mercados de bairro e açougues.

### A solução: o Guardião

Um ESP32 independente, instalado junto da câmara fria, que:

1. **Mede** com sensores próprios: temperatura e umidade (DHT22) e porta aberta (LDR). Também observa o que o controlador comercial está mandando: setpoint e degelo.
2. **Decide** com uma política gravada no próprio firmware. Ela nunca é alterada pela rede.
3. **Atua** com um relé entre o controlador e o equipamento. Diante de uma ordem perigosa, corta o aquecimento ou degelo, liga a ventoinha de emergência e entra em bloqueio temporário.
4. **Alerta** no local (LED RGB, buzzer, LCD) e **só envia** dados ao FIWARE.

Frase do produto: **"Vê de longe. Só obedece de perto."** A única forma de liberar um bloqueio é presencial: segurar um botão físico por 3 segundos.

O ponto central do projeto é que **o firmware não assina nenhum tópico de comando**. Não existe código que receba ordens pela rede. No CP5, a Vinheria recebia o comando `set_limits` pela internet; o Guardião inverte essa arquitetura de propósito.

### O que precisa ser entregue (rubrica oficial)

Entrega: **link público do Wokwi** (abre e roda sem ajustes, diagrama organizado, código comentado) + **vídeo no YouTube de até 7 min**. Os nomes de todos os integrantes devem constar no código-fonte.

| Critério | Pontos | O que a rubrica pede |
| --- | --- | --- |
| Problema e conexão com a CitizenTech | 15 | Cidadão, problema e ODS claros; o dispositivo resolve uma parte real do problema |
| Circuito | 20 | Sensores, atuadores e interface adequados; ligações corretas; diagrama organizado |
| Lógica de borda | 25 | Limites justificados, no mínimo 3 estados, respostas distintas, tratamento de leitura inválida e suavização |
| Qualidade do código | 15 | Funções, constantes, `millis()`, comentários, saída no Serial Monitor |
| Vídeo | 25 | Ideia clara e simulação comentada mostrando todos os estados |

O objetivo da disciplina também pede **3 camadas** (dispositivo IoT, back-end FIWARE e dashboard) e um sistema que funcione **online ou offline**. As 3 camadas são demonstradas **sem hardware** (C1): Guardião simulado no Wokwi → FIWARE na EC2 → site da GS na rota `/ao-vivo`.

### Objetivo desta sessão

Produzir, com especificação aprovada antes:

1. O firmware do Guardião (o mesmo código roda no Wokwi e, se um dia houver, na maquete física).
2. O `diagram.json` do Wokwi, organizado.
3. Testes da lógica de decisão rodando no computador, sem placa.
4. O provisionamento do device no FIWARE, só com atributos e sem comandos.
5. O teste ponta a ponta sem hardware (Wokwi → Orion), com comando e resposta registrados (T18b).
6. A documentação: tabela de testes dos estados e roteiro do vídeo; ligações da maquete só se sobrar tempo.

Fora da entrega de Edge (C2): o firmware do "controlador vulnerável".

### Material de referência

Clonar o projeto anterior da equipe **fora deste repositório** (por exemplo, em `../ref/vinheria`) e usá-lo só como referência. Não copiar o repositório inteiro: ele foi feito por outra formação de equipe.

```bash
git clone --depth 1 https://github.com/zackdevbr/Projeto-VinheriaFull.git ../ref/vinheria
```

Arquivos úteis: `firmware/vinheria_full.ino`, `firmware/led_rgb.h`, `firmware/limites.h`, `firmware/diagram.json`, `firmware/libraries.txt`, `PLANO-CP5-VINHERIA.md` (seção de entidade FIWARE e provisionamento) e `postman/`.

---

## 2. Detalhamento técnico

### 2.1 Estrutura do repositório

```
edge/
  cshield/
    cshield.ino          # setup() e loop(): só orquestra, chama as funções
    config.h              # TODAS as constantes: pinos, perfis, limites, tempos, rede
    decisao.h             # lógica pura de decisão (sem Arduino.h): testável no PC
    sensores.h            # leitura, validação e média móvel
    atuadores.h           # relés, LED RGB e padrões do buzzer (não bloqueantes)
    painel.h              # telas do LCD
    rede.h                # Wi-Fi + MQTT SOMENTE publicação
    diagram.json          # circuito do Wokwi
    libraries.txt         # bibliotecas do Wokwi
  controlador-vulneravel/
    controlador.ino       # só para a maquete (opcional)
  test/
    test_decisao.cpp      # testes da decisao.h com g++
  fiware/
    provisionar.sh        # curl do device sem comandos + subscription do STH-Comet
docs/
  specs/edge/requirements.md
  specs/edge/design.md
  specs/edge/tasks.md
  edge-ligacoes-maquete.md
  edge-testes-estados.md
  edge-roteiro-video.md
```

### 2.2 Mapa de pinos (fixo; é o mesmo do plano da equipe)

| Pino | Componente | Função |
| --- | --- | --- |
| GPIO 4 | DHT22 (dados) | Temperatura e umidade próprias |
| GPIO 34 | LDR (saída analógica) | Porta aberta |
| GPIO 32 | Chave NORMAL/ATAQUE (só Wokwi) | Setpoint que o controlador está mandando (5 ou 45 °C) |
| GPIO 18 | Chave deslizante (Wokwi) / fio do controlador (maquete) | Degelo comandado pelo controlador |
| GPIO 19 | Botão, `INPUT_PULLUP` | Liberação presencial (segurar 3 s) |
| GPIO 23 | Relé 1, contato **NF** | Corta o aquecedor/degelo no bloqueio |
| GPIO 33 | Relé 2, contato **NA** | Liga a ventoinha de emergência |
| GPIO 25, 26, 27 | LED RGB R, G, B (220 Ω cada) | Cor do estado |
| GPIO 5 | Buzzer | Alerta sonoro |
| GPIO 21, 22 | LCD 16x2 I2C (PCF8574) SDA, SCL (0x27) | Mensagens (16 colunas × 2 linhas) |

No Wokwi, o relé não liga nada. Colocar um **LED laranja "aquecedor"** no contato NF do relé 1 e um **LED azul "ventoinha"** no contato NA do relé 2, cada um com resistor de 220 Ω, para o vídeo mostrar o corte acontecendo.

### 2.3 Constantes principais (`config.h`)

- **Perfil ativo de produto**, escolhido por constante: `PERFIL_VACINA` (padrão, +2 a +8 °C, porque a caixa com gelo em gel da maquete fica nessa faixa e conecta com as salas de vacina de São Paulo), `PERFIL_CONGELADO` (≤ -18 °C), `PERFIL_RESFRIADO` (0 a +4 °C) e `PERFIL_PLASMA` (≤ -25 °C). Cada perfil define `T_MIN`, `T_MAX` e `TOLERANCIA_C` (1,0 °C).
- `TAXA_MAX_C_POR_MIN = 0.5`: valor provisório; a DPS da equipe vai calibrar com a maquete. Deixar um comentário dizendo isso.
- `PERIODO_LEITURA_MS = 2000` (mínimo do DHT22) · `JANELA_MEDIA = 5` · `CONFIRMACOES = 2` · `JANELA_TAXA_MS = 60000`.
- `LIMIAR_PORTA_PCT = 40`: luminosidade acima disso significa porta aberta.
- Degelo: `DEGELO_MAX_MS` (30 min) e horários permitidos (`HORA_DEGELO_1 = 6`, `HORA_DEGELO_2 = 18`). Se não houver hora válida por NTP (offline), vale só a duração máxima.
- `TRAVA_MS`: 30 min. Com `MODO_DEMO 1`, cai para 60 s, para o vídeo.
- `SETPOINT_DEMO_NORMAL = 5`, `SETPOINT_DEMO_ATAQUE = 45`: setpoints que a chave do Wokwi representa.
- `TEM_POT_SETPOINT`: 1 no Wokwi, 0 na maquete.
- `RELE_ATIVO_EM_LOW`: muitos módulos físicos acionam em LOW; deixar configurável.
- Rede: `SSID = "Wokwi-GUEST"`, `BROKER_MQTT` (placeholder; o IP elástico da EC2 da Vinheria era `3.215.3.216`, confirmar se ainda existe), porta 1883, `ID_DEVICE = "cshield001"`.
- `NOME_PRODUTO = "C-Shield"` (de Cyber Shield, definido em 10/11): tudo que aparece no display, nos logs e nos documentos deve ler esta constante.

### 2.4 Estados e prioridade

Prioridade, da mais alta para a mais baixa: **CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL**.

| Estado | Entra quando | Resposta local | Publica |
| --- | --- | --- | --- |
| NORMAL | Temperatura dentro de [T_MIN, T_MAX] e nenhuma ordem inválida | LED verde; LCD com temperatura e faixa | A cada 5 s |
| ATENCAO | Porta aberta, ou temperatura fora da faixa mas dentro da tolerância | LED amarelo; 1 bipe curto a cada 10 s; LCD "PORTA ABERTA" ou "DERIVA" | Leitura + motivo |
| BLOQUEIO | Ordem perigosa detectada (regras abaixo) | Relé 1 abre o NF (corta aquecedor), relé 2 liga a ventoinha, LED vermelho piscando, buzzer intermitente, LCD "ATAQUE BLOQUEADO" + motivo; trava por `TRAVA_MS` | Evento imediato com motivo e valor recebido |
| CRITICO | Temperatura além da tolerância (confirmada), com ou sem bloqueio | LED vermelho fixo; buzzer contínuo; LCD "PRODUTO EM RISCO"; relés como no bloqueio | Evento + exposição térmica acumulada |
| FALHA_SENSOR | DHT22 retorna NaN ou valor fora de -40 a 80 °C | LED amarelo piscando; LCD "FALHA NO SENSOR" | Evento de falha |

**Regras do BLOQUEIO (julgar a ordem antes do efeito):**

1. Setpoint comandado fora de [T_MIN, T_MAX] → bloqueio imediato, sem esperar a temperatura subir.
2. Degelo comandado fora dos horários permitidos (quando a hora é válida) ou por mais de `DEGELO_MAX_MS` → bloqueio.
3. Taxa de aquecimento acima de `TAXA_MAX_C_POR_MIN` **com a porta fechada** → bloqueio. Com a porta aberta, é só ATENCAO.
4. Durante a trava, qualquer nova mudança vinda do controlador é ignorada e registrada.
5. Liberação: botão segurado por 3 s (com debounce). Só sai do bloqueio se a temperatura estiver dentro da tolerância; caso contrário, vai para CRITICO.
6. Em FALHA_SENSOR, a regra 3 não é avaliada, mas as regras 1 e 2 continuam valendo.

**Princípios que não podem ser quebrados:**

- O Guardião **nunca** impede a refrigeração. Ele só veta o que aumenta o risco.
- **Falha segura:** com o ESP32 desligado ou reiniciando, os relés ficam desenergizados. O NF continua fechado e o equipamento funciona como antes. No `setup()`, os pinos dos relés vão para o estado desenergizado **antes** de qualquer outra coisa.
- **Só envia:** proibido `MQTT.subscribe`, `setCallback`, OTA ou servidor HTTP no firmware do Guardião.
- **Offline funciona:** sem Wi-Fi ou MQTT, a decisão e as respostas locais continuam. A reconexão é tentada a cada 10 s sem travar o `loop()`.

### 2.5 Exposição térmica

Enquanto a temperatura estiver acima de `T_MAX`, acumular `(T − T_MAX) × Δt` em °C·min. É a integral que a DPS usa. Publicar em `ex` e mostrar no LCD no estado CRITICO.

### 2.6 FIWARE (UltraLight 2.0 sobre MQTT)

- Reaproveitar o service group da Vinheria: headers `fiware-service: smart` e `fiware-servicepath: /`, apikey `TEF`.
- Tópico de publicação: `/TEF/cshield001/attrs`. **Nenhum tópico de comando.**
- Payload, uma linha por publicação: `t|4.8|h|81|l|12|sp|5.0|dg|0|st|NORMAL|mt|-|bq|0|ex|0`
  - `t` temperatura (°C), `h` umidade (%), `l` luminosidade (%), `sp` setpoint observado (°C, -999 quando não houver), `dg` degelo comandado (0/1), `st` estado, `mt` motivo curto sem espaços (ex.: `SETPOINT_INVALIDO`), `bq` contador de bloqueios, `ex` exposição térmica (°C·min).
- Device: `device_id = cshield001`, `entity_name = urn:ngsi-ld:CShield:001`, `entity_type = CShield`, protocolo `PDI-IoTA-UltraLight`, transporte MQTT, **só `attributes`, sem `commands`**.
- O `provisionar.sh` usa `curl` com o IP da EC2 em uma variável no topo. Ele cria o device (tratando 409 como OK) e cria a subscription do Orion para o STH-Comet (porta 8666) dos atributos `t`, `h`, `l`, `st`, `bq` e `ex`. Seguir o modelo do `PLANO-CP5-VINHERIA.md` e do Postman da Vinheria.

### 2.7 Serial Monitor

Uma linha por ciclo de decisão, legível e com os mesmos nomes do payload. Exemplo:

`[cshield001] t=4.8C h=81% l=12% sp=5.0C dg=0 taxa=+0.02C/min st=NORMAL mt=- trava=0s rede=OK`

### 2.8 Controlador vulnerável (fora da entrega de Edge — C2)

> Não implementar agora. Só será feito se a equipe montar a maquete para o pitch e pedir. A especificação abaixo fica registrada para esse caso.

Roda no segundo ESP32. Ele representa o controlador comercial que aceita comandos pela internet:

- Recebe comandos pelo FIWARE (device separado `controlador001`, com comandos `atacar` e `normal`), como a Vinheria fazia.
- `atacar` liga o relé dele (contato NA) no GPIO 23, que fecha o circuito do aquecedor, e coloca o GPIO 18 em HIGH (o fio "degelo ligado" que vai ao GPIO 18 do Guardião, com GND comum).
- `normal` desliga os dois.
- O cabeçalho deve dizer claramente que este firmware existe só para demonstração em laboratório, com tensão baixa (12 V).

---

## 3. Pequenas tarefas

Trabalhar em ordem. Cada tarefa termina com um commit em português (por exemplo, `feat(edge): média móvel e validação do DHT22`). Não avançar com testes falhando.

**Fase A — Especificação (parar no final e pedir aprovação)**

- [ ] **T1.** Ler este plano e os arquivos de referência da Vinheria. Listar em 5 linhas o que vai reaproveitar e o que muda.
- [ ] **T2.** Escrever `docs/specs/edge/requirements.md` com requisitos no formato "QUANDO <condição>, O GUARDIÃO DEVE <resposta>", numerados (R1, R2...), cobrindo os 5 estados, as 6 regras de bloqueio, os princípios da seção 2.4, a publicação FIWARE (incluindo a publicação a partir do Wokwi via `Wokwi-GUEST`, C1) e o Serial.
- [ ] **T3.** Escrever `docs/specs/edge/design.md`: módulos e responsabilidades, a máquina de estados (diagrama em Mermaid), o fluxo de um ciclo do `loop()` e a assinatura de cada função pública.
- [ ] **T4.** Escrever `docs/specs/edge/tasks.md`: as tarefas da Fase B em diante, cada uma apontando para os requisitos que cobre, já com a correção de escopo (§0): T18 fora, T18b incluída, T19 por último.
- [ ] **T5.** **PARAR.** Mostrar um resumo da especificação e listar as dúvidas. Só continuar depois que a equipe aprovar.

**Fase B — Lógica pura e testes**

- [ ] **T6.** `config.h` com todas as constantes da seção 2.3, comentadas com o motivo de cada valor.
- [ ] **T7.** `decisao.h` sem dependência do Arduino: struct `Leitura` (temperatura, umidade, luz, setpoint, degelo, validade, tempo), struct `Contexto` (trava, contadores, exposição, tempo do degelo) e a função `Decisao avaliar(const Leitura&, Contexto&, unsigned long agoraMs)` que devolve estado, motivo e ações (corta aquecedor, liga ventoinha).
- [ ] **T8.** `test/test_decisao.cpp` com um caso por requisito: setpoint inválido, degelo longo, degelo fora do horário, aquecimento rápido com porta fechada, o mesmo com porta aberta, sensor NaN, sensor fora da faixa, trava ignorando nova ordem, liberação com temperatura boa, liberação com temperatura ruim, crítico, exposição acumulada e prioridade entre estados. Rodar com `g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_decisao.cpp -o /tmp/t && /tmp/t`.

**Fase C — Firmware**

- [ ] **T9.** `sensores.h`: leitura do DHT22 a cada 2 s, validação (NaN e faixa física), média móvel de 5, confirmação em 2 leituras, LDR em %, potenciômetro em °C e leitura do GPIO 18.
- [ ] **T10.** `atuadores.h`: relés com `RELE_ATIVO_EM_LOW`, estado seguro no boot, cores do LED RGB por estado e padrões do buzzer com `millis()` (sem `delay()` no `loop()`).
- [ ] **T11.** `painel.h`: telas do LCD 16x2 por estado, conforme o layout da §0.3, com o nome vindo de `NOME_PRODUTO` e sem redesenhar o LCD a cada ciclo (só quando o conteúdo muda, para não piscar).
- [ ] **T12.** Botão de liberação: debounce e contagem de 3 s, com barra de progresso no LCD enquanto o botão é segurado.
- [ ] **T13.** `rede.h`: Wi-Fi e MQTT só com `publish`, reconexão a cada 10 s sem bloquear, publicação a cada 5 s e imediata em mudança de estado.
- [ ] **T14.** `cshield.ino`: cabeçalho com o nome do projeto, a disciplina, o professor e os integrantes (ver abaixo); `setup()` e `loop()` só chamando funções; a linha do Serial da seção 2.7.

Integrantes para o cabeçalho (nomes definitivos, como estão):

```
Isac Nilton Fernandes de Oliveira — RM573282
João Benedito de Oliveira Simplício — RM570206
Leonardo Formigari — RM573291
Luis Fernando — RM570959
Nicolas Cinel — RM572940
```

**Fase D — Wokwi, FIWARE e maquete**

- [ ] **T15.** `diagram.json` com os componentes da seção 2.2 e os dois LEDs indicadores. Usar os nomes de pinos dos componentes do Wokwi (conferir na documentação oficial e no `diagram.json` da Vinheria). Fios sem cruzamentos desnecessários, cores por função (vermelho 3V3/5V, preto GND, verde sinais, azul I2C) e rótulos (`"label"`) nos componentes que representam o controlador: "Setpoint do controlador" e "Degelo do controlador".
- [ ] **T16.** `libraries.txt` com as bibliotecas necessárias.
- [ ] **T17.** `fiware/provisionar.sh` conforme a seção 2.6.
- [ ] ~~**T18.**~~ (Fora da entrega de Edge — C2) `controlador-vulneravel/controlador.ino` conforme a seção 2.8. Só se a equipe montar a maquete para o pitch e pedir.
- [ ] **T18b.** Teste ponta a ponta sem hardware (C4). Com a simulação do Wokwi rodando, confirmar no Orion que os atributos `st` e `bq` mudam quando o potenciômetro do setpoint sai da faixa:

  ```bash
  curl -s "http://$FIWARE_HOST:1026/v2/entities/urn:ngsi-ld:CShield:001" \
    -H "fiware-service: smart" -H "fiware-servicepath: /"
  ```

  Rodar uma vez com o setpoint dentro da faixa (esperado `st` = `NORMAL`) e outra depois de girar o potenciômetro para fora (esperado `st` = `BLOQUEIO` e `bq` incrementado). Registrar o comando e as duas respostas em `docs/edge-testes-estados.md`. Se não for possível rodar o Wokwi a partir do agente, deixar o passo a passo para a equipe executar e dizer isso no relatório final.

**Fase E — Documentação**

- [ ] **T19.** (Baixa prioridade — C3: fazer por último, depois de T20 e T21, só se sobrar tempo.) `docs/edge-ligacoes-maquete.md`: tabela de ligações dos dois ESP32, o circuito do aquecedor (fonte 12 V → relé do controlador NA → relé do Guardião NF → resistor 22 Ω 10 W → fonte) e os cuidados de segurança (só baixa tensão, resistor sobre chapinha de alumínio, longe do plástico, nunca ligado sem supervisão).
- [ ] **T20.** `docs/edge-testes-estados.md`: para cada estado, o que mexer no Wokwi (slider do DHT22, LDR, potenciômetro, chave, botão), o que deve aparecer no LED, buzzer, LCD, relés e Serial, e uma coluna "resultado" para a equipe preencher.
- [ ] **T21.** `docs/edge-roteiro-video.md`: roteiro de até 7 min na ordem da rubrica. (1) A ideia: cidadão, problema em uma frase, onde o Guardião fica e ODS 3, 9 e 2. (2) Cada componente e por que está ali. (3) Os limites e o trecho de `avaliar()`. (4) A remoção do `subscribe`: "aqui não entra nada". (5) A simulação no Wokwi passando por todos os estados, inclusive a falha de sensor; o "ataque" é girar o potenciômetro do setpoint ou ligar a chave do degelo, e os LEDs laranja e azul mostram o aquecedor cortado e a ventoinha ligada. (6) As 3 camadas: o Serial mostrando a publicação, a entidade atualizando no FIWARE e o site `/ao-vivo` registrando o bloqueio. Indicar o tempo de cada parte. Incluir o lembrete: o Wokwi só publica enquanto a aba do navegador estiver aberta e a simulação ativa; ligar a EC2 e testar o site 10 minutos antes de gravar.

---

## 4. Revisão e autocorreção

Ao terminar a Fase E, fazer esta revisão **antes** de declarar o trabalho pronto. Para cada item, rodar o comando ou abrir o arquivo e corrigir o que falhar. Depois rodar a lista inteira de novo.

### 4.1 Verificações automáticas

```bash
# 1. Testes da lógica: todos devem passar
g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_decisao.cpp -o /tmp/t && /tmp/t

# 2. O Guardião não pode receber comandos: o resultado deve ser VAZIO
grep -rnE "subscribe|setCallback|ArduinoOTA|WebServer" edge/cshield/

# 3. Nada de delay() no loop (só é aceito no setup, com justificativa)
grep -n "delay(" edge/cshield/*.ino edge/cshield/*.h

# 4. diagram.json válido
python3 -m json.tool edge/cshield/diagram.json > /dev/null && echo "diagram.json OK"

# 5. Todo pino do config.h aparece no diagram.json
grep -nE "#define PIN|const int PIN" edge/cshield/config.h

# 6. Compilar, se o arduino-cli com o core esp32 estiver disponível
arduino-cli compile --fqbn esp32:esp32:esp32 edge/cshield || echo "arduino-cli indisponível: registrar no relatório"
```

### 4.2 Conferência contra a rubrica

Montar uma tabela com cada linha da rubrica e a evidência (arquivo e linha) que a atende:

- [ ] No mínimo 3 estados, com respostas **distintas** de LED, buzzer e LCD (temos 5).
- [ ] Cada limite tem justificativa escrita em comentário no `config.h`.
- [ ] Leitura inválida tratada (NaN e faixa física) e suavização (média móvel + confirmação).
- [ ] Funções pequenas, constantes no lugar de números soltos, `millis()`, comentários e Serial legível.
- [ ] Nomes de todos os integrantes no código-fonte.
- [ ] Diagrama organizado, sem fios cruzando sem necessidade.
- [ ] Funciona offline: desligar o Wi-Fi no código não muda as decisões nem as respostas locais.

### 4.3 Conferência dos princípios do produto

- [ ] Nenhum caminho de código altera a política a partir da rede.
- [ ] Boot e reset deixam os relés desenergizados (falha segura).
- [ ] O Guardião nunca desliga a refrigeração; ele só corta aquecimento ou degelo e liga a ventoinha.
- [ ] O nome do produto vem só de `NOME_PRODUTO`.
- [ ] Nenhuma senha ou IP real foi commitado além dos placeholders combinados.

### 4.4 Relatório final

Entregar um resumo curto com: o que foi feito; os testes que passaram; o que não foi possível verificar (por exemplo, a compilação sem `arduino-cli` ou o teste real no Wokwi); os requisitos que ficaram de fora e por quê; os 3 próximos passos para a equipe (ex.: subir o projeto no Wokwi e gerar o link público, calibrar `TAXA_MAX_C_POR_MIN` com a DPS, rodar o T18b se o agente não conseguiu); e uma linha dizendo quais das seis correções de escopo (§0, C1 a C6) foram aplicadas.

Se, durante a revisão, encontrar um requisito sem teste ou um teste sem requisito, corrigir a especificação ou o código e registrar a mudança no `tasks.md`.
