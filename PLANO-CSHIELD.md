# Plano — Edge Computing do C-Shield (GS 2026.2 · Nexus Vision) · versão 2

> Plano autoritativo do projeto C-Shield. A versão 2 foi recebida do usuário em 10/11/2026 e substitui a versão 1 (que continua no histórico do git, até o commit `511469e`). Método: spec-driven. O `CLAUDE.md` manda no processo de trabalho; este plano manda nos detalhes de implementação.
>
> Ordem de leitura: **§0** (decisões sobre a v2, prevalece sobre tudo), **§1 a §4** (texto da v2, como recebido), **§5** (plano de execução passo a passo para o Sonnet).

## 0. Decisões sobre a v2 (10/11/2026, prevalecem sobre §1 a §4)

Decisões do usuário tomadas na sessão de replanejamento:

- **V1. Display: LCD 16x2 I2C, não OLED.** Onde §1 a §4 dizem "OLED" ou "SSD1306", leia "LCD 16x2 I2C (0x27)". A biblioteca continua `LiquidCrystal I2C`. Os textos de tela são encurtados para 16 colunas conforme a tabela da §5.3.
- **V2. Modelo térmico da demo mantido e adaptado** (`edge/cshield/simulacao.h`, chave `MODELO_TERMICO_DEMO`). No Wokwi o DHT22 é um slider manual; o modelo representa o efeito físico da ventoinha e do aquecimento passivo **só enquanto o C-Shield está no controle (bloqueio)**, para o termostato de emergência ligar e desligar sozinho no vídeo. Detalhes na §5.4. A tela "RESTAURANDO TEMP / BLOQUEIO ATIVO" (pedido do usuário) continua.
- **V3. Execução direta e auto-revisada.** Este replanejamento vale como a aprovação da T5: o Sonnet **não para** no fim da Fase A, nem entre tarefas. Executa as Fases A a E em sequência, um commit por passo verificado, testes verdes antes de avançar. Só para se aparecer uma lacuna que este plano não cobre (regra do `CLAUDE.md` §2).
- **V4. Prazo: 11/11/2026 à noite.** Ordem de corte se apertar: T18 (ponta a ponta, deixar o passo a passo) → refinamentos visuais do `diagram.json` → modelo térmico (desligar com `MODELO_TERMICO_DEMO 0`). Nunca cortar: testes da `decisao.h`, os 5 estados, falha segura, ausência de `subscribe`, Serial e roteiro do vídeo.

Decisões herdadas da v1 que continuam valendo:

- **H1.** Repositório `c-shield-edge` separado do site `c-shield-web`. README com Ficha CitizenTech já existe (manter, só atualizar o conteúdo técnico).
- **H2.** Nomes dos integrantes ficam exatamente como estão (sem "completar o nome").
- **H3.** Vídeo só no Wokwi, sem maquete. Ligações da maquete física não fazem parte da entrega.
- **H4.** Regras de commit do `CLAUDE.md` §8: Conventional Commits em português, **nunca** `Co-Authored-By`, autor só o usuário logado.
- **H5.** `g++` é o WinLibs instalado pelo winget e não está no PATH. Compilar com o caminho completo e `-static` (sem `-static` o executável não abre por falta de DLL):
  `"/c/Users/isacn/AppData/Local/Microsoft/WinGet/Packages/BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe/mingw64/bin/g++.exe" -std=c++17 -Wall -Wextra -static -I edge/cshield edge/test/test_decisao.cpp -o "$TEMP/t.exe" && "$TEMP/t.exe"`

Interpretações que a v2 deixava em aberto, já decididas (o Sonnet não reabre):

- **I1. Ventoinha durante o bloqueio com o termostato em "desligado".** O relé 2 é um comutador: em repouso a ventoinha segue o controlador; acionado, o C-Shield a liga direto. Com o termostato em "desligado", o relé volta ao repouso e a ventoinha segue o controlador. Isso está de acordo com o princípio "o C-Shield nunca impede a refrigeração": ele só pode **acrescentar** frio, nunca tirar. Logo `vf = rele2Acionado ? 1 : vt`.
- **I2. Termostato ao entrar em bloqueio** começa **ligado** (refrigerar é o lado seguro) e a partir daí segue a histerese.
- **I3. CRITICO sem bloqueio** aplica as mesmas ações: corta o aquecedor e roda o termostato (que estará ligado, porque a temperatura está acima de `T_ALVO + HISTERESE_C`). Quando o CRITICO acaba sem bloqueio, os relés voltam ao repouso.
- **I4. Janela de degelo permitido** de 30 min: 06:00 a 06:30 e 18:00 a 18:30. Para isso a `Leitura` passa a ter `minutoDoDia` (0 a 1439) no lugar de `hora`.
- **I5. Degelo legítimo** = aquecedor ligado **e** ventoinha desligada, dentro do horário (ou sem hora válida, ou em `MODO_DEMO`) e há menos de `DEGELO_MAX_MS`. Mostra ATENCAO com motivo `DEGELO` e a contagem regressiva. Aquecedor ligado **com** ventoinha ligada por pouco tempo é NORMAL.
- **I6. Atributos longos no FIWARE**, mantendo os já existentes: `t temperature`, `h humidity`, `l luminosity`, `aq heater`, `vt fanCommand`, `vf fan`, `st state`, `mt reason`, `bq blocks`, `ci ignoredCommands`, `ex exposure`. Subscriptions do STH-Comet: `temperature humidity luminosity fan state blocks ignoredCommands exposure`. O site `c-shield-web` deve ler estes nomes.
- **I7. "Guardião" sai dos textos de código e docs**, porque a verificação §4.1 item 6 exige busca vazia. Cabeçalho novo: `C-Shield — Edge Computing (GS 2026.2 "CitizenTech")`.
- **I8. Débito de compilação.** O `arduino-cli` não está instalado. A lógica é testada com `g++`; o `.ino` completo é verificado no Wokwi pelo usuário. O relatório final diz isso.

---

## 1. Contexto e objetivo (texto da v2)

### Quem somos e o que estamos fazendo

Somos a equipe **Nexus Vision** (Engenharia de Software, 1º ano, FIAP). Esta é a entrega de **Edge Computing & Computer Systems** da Global Solution 2026.2 "CitizenTech", que vale nota e concorre a prêmio. O professor é o Prof. Dr. Fábio H. Cabrini.

### O problema real

Em outubro e novembro de 2026, invasores entraram pela internet nos controladores de refrigeração de supermercados e de um banco de sangue na Europa e mandaram os equipamentos esquentar:

- França, 11/10: centenas de supermercados; freezers a até 50 °C; o software atacado era da Danfoss.
- Europa, 01 e 02/11: segunda onda em mais de 100 lojas. Os evaporadores foram colocados em modo de degelo. Na ICA de Estocolmo, o ataque foi às 2h e só foi percebido às 5h30.
- Banco de sangue francês (EFS), 02/11: o plasma, guardado a cerca de -30 °C, teve o controle ajustado para até 110 °C. Só se salvou porque um alarme tocou e um técnico estava de plantão.
- Em testes da Claroty, o painel do controlador mostrava temperatura normal enquanto o refrigerador esquentava. Um sistema invadido não avisa que foi invadido.

Em São Paulo, os mesmos equipamentos existem em salas de vacina de UBS, hemocentros, farmácias, mercados de bairro e açougues.

### A solução: o C-Shield

**C-Shield** (de Cyber Shield) é um ESP32 independente, instalado junto da câmara fria, entre o controlador comercial e os componentes da geladeira (aquecedor de degelo e ventoinha/compressor). Ele:

1. **Mede** com sensores próprios: temperatura e umidade (DHT22) e porta aberta (LDR). Também observa o que o controlador comercial está mandando: se o **aquecedor** está ligado e se a **ventoinha/compressor** está ligada.
2. **Decide** com uma política gravada no próprio firmware, que nunca é alterada pela rede. Ele reconhece o ataque por dois sinais: **a temperatura subindo de repente** com a porta fechada, ou **o aquecedor ligado com a ventoinha desligada** fora do degelo permitido.
3. **Assume o controle localmente**, pelos relés: corta o aquecedor, ignora tudo o que o controlador comercial mandar dali em diante e passa a regular a ventoinha sozinho, como um termostato de emergência, até a temperatura voltar ao padrão do produto.
4. **Alerta** no local (LED RGB, buzzer, display) e **só envia** dados ao FIWARE.

**O bloqueio só termina presencialmente:** alguém segura o botão físico do aparelho por 3 segundos. Não há tempo de expiração nem comando remoto de liberação.

Frase do produto: **"Vê de longe. Só obedece de perto."** O ponto central é que **o firmware não assina nenhum tópico de comando**. No CP5, a Vinheria recebia o comando `set_limits` pela internet; o C-Shield inverte essa arquitetura de propósito.

### O que precisa ser entregue (rubrica oficial)

Entrega: **link público do Wokwi** (abre e roda sem ajustes, diagrama organizado, código comentado) + **vídeo no YouTube de até 7 min**. Os nomes de todos os integrantes devem constar no código-fonte.

| Critério | Pontos | O que a rubrica pede |
| --- | --- | --- |
| Problema e conexão com a CitizenTech | 15 | Cidadão, problema e ODS claros; o dispositivo resolve uma parte real do problema |
| Circuito | 20 | Sensores, atuadores e interface adequados; ligações corretas; diagrama organizado |
| Lógica de borda | 25 | Limites justificados, no mínimo 3 estados, respostas distintas, tratamento de leitura inválida e suavização |
| Qualidade do código | 15 | Funções, constantes, `millis()`, comentários, saída no Serial Monitor |
| Vídeo | 25 | Ideia clara e simulação comentada mostrando todos os estados |

O objetivo da disciplina também pede **3 camadas** (dispositivo IoT, back-end FIWARE e dashboard) e um sistema que funcione **online ou offline**.

**Decisões da equipe:**

- **O vídeo de Edge usa só o Wokwi, sem maquete.** A simulação conecta no Wi-Fi `Wokwi-GUEST`, publica no FIWARE de verdade e o site da GS (outro repositório, rota `/ao-vivo`) mostra os dados. Assim as 3 camadas aparecem sem hardware.
- **Este repositório não contém o site.** O código do site fica em `c-shield-web`, entregue a outras disciplinas.

### Objetivo desta sessão

1. O firmware do C-Shield (o mesmo código roda no Wokwi e numa placa física).
2. O `diagram.json` do Wokwi, organizado.
3. Testes da lógica de decisão rodando no computador, sem placa.
4. O provisionamento do device no FIWARE, só com atributos e sem comandos.
5. A documentação: tabela de testes dos estados e roteiro do vídeo.

### Material de referência

Projeto anterior da equipe, só como referência, fora deste repositório (`../ref/vinheria`, clonado de `https://github.com/zackdevbr/Projeto-VinheriaFull`). Na v1 ele já foi consultado; os padrões aproveitados já estão no código atual.

---

## 2. Detalhamento técnico (texto da v2, com as decisões da §0 aplicadas)

### 2.1 Estrutura do repositório

```
edge/
  cshield/
    cshield.ino           # setup() e loop(): só orquestra
    config.h              # TODAS as constantes: pinos, perfis, limites, tempos, rede
    decisao.h             # lógica pura de decisão (sem Arduino.h): testável no PC
    sensores.h            # leitura, validação, média móvel e debounce das chaves
    atuadores.h           # relés, LED RGB e padrões do buzzer (não bloqueantes)
    painel.h              # telas do LCD 16x2 (V1)
    rede.h                # Wi-Fi + MQTT SOMENTE publicação + NTP
    simulacao.h           # modelo térmico só da demo no Wokwi (V2)
    diagram.json          # circuito do Wokwi
    libraries.txt         # bibliotecas do Wokwi
  test/
    test_decisao.cpp      # testes da decisao.h com g++
  fiware/
    provisionar.sh        # curl do device sem comandos + subscription do STH-Comet
docs/
  specs/edge/requirements.md
  specs/edge/design.md
  specs/edge/tasks.md
  edge-testes-estados.md
  edge-roteiro-video.md
README.md                 # Ficha CitizenTech no topo + link para c-shield-web
```

### 2.2 Mapa de pinos

| Pino | Componente | Função |
| --- | --- | --- |
| GPIO 4 | DHT22 (dados) | Temperatura e umidade próprias |
| GPIO 34 | LDR (saída analógica) | Porta aberta |
| GPIO 18 | Chave deslizante 1 (Wokwi) / fio do controlador | **Aquecedor (degelo) comandado** pelo controlador comercial |
| GPIO 17 | Chave deslizante 2 (Wokwi) / fio do controlador | **Ventoinha/compressor comandada** pelo controlador comercial |
| GPIO 19 | Botão, `INPUT_PULLUP` | Liberação presencial (segurar 3 s) |
| GPIO 23 | Relé 1, contato **NF** em série com o aquecedor | Em repouso, o aquecedor obedece ao controlador; acionado, o C-Shield corta o aquecedor |
| GPIO 33 | Relé 2, **comutador** da ventoinha | Em repouso (NF), a ventoinha segue o controlador; acionado (NA), o C-Shield liga a ventoinha direto |
| GPIO 25, 26, 27 | LED RGB R, G, B (220 Ω cada) | Cor do estado |
| GPIO 5 | Buzzer | Alerta sonoro |
| GPIO 21, 22 | LCD 16x2 I2C SDA, SCL (0x27) | Mensagens (V1) |

**Indicadores do Wokwi** (o relé não liga nada no simulador; estes LEDs mostram o efeito real):

- **LED laranja "Aquecedor"**: alimentado pelo sinal da chave 1 passando pelo contato NF do relé 1. Acende quando o controlador liga o aquecedor; apaga quando o C-Shield corta.
- **LED azul "Ventoinha"**: ligado ao COM do relé 2. No NF chega o sinal da chave 2 (o controlador); no NA chega 3V3 (o C-Shield). Mostra o estado efetivo da ventoinha.

Cada LED com resistor de 220 Ω. Pinos do relé no Wokwi: `VCC`, `GND`, `IN`, `COM`, `NO`, `NC`.

### 2.3 Constantes principais (`config.h`)

- `NOME_PRODUTO = "C-Shield"`.
- Perfis com `T_MIN`, `T_MAX`, `T_ALVO` e `TOLERANCIA_C` (1,0 °C): `PERFIL_VACINA` (padrão: +2 a +8 °C, alvo 5 °C), `PERFIL_CONGELADO` (≤ -18 °C, alvo -20 °C), `PERFIL_RESFRIADO` (0 a +4 °C, alvo 2 °C), `PERFIL_PLASMA` (≤ -25 °C, alvo -30 °C).
- `HISTERESE_C = 1.0`: no termostato de emergência a ventoinha liga em `T_ALVO + HISTERESE_C` e desliga em `T_ALVO - HISTERESE_C`.
- `TAXA_MAX_C_POR_MIN = 0.5`: valor provisório, a DPS vai calibrar (comentário obrigatório).
- `PERIODO_LEITURA_MS = 2000` · `JANELA_MEDIA = 5` · `CONFIRMACOES = 2` · `JANELA_TAXA_MS = 60000`.
- `LIMIAR_PORTA_PCT = 40`.
- Degelo legítimo: `DEGELO_MAX_MS` (30 min), `HORA_DEGELO_1 = 6`, `HORA_DEGELO_2 = 18`, janela de 30 min cada (I4). Hora por NTP (`pool.ntp.org`, fuso -3). Sem hora válida, vale só a duração.
- `MODO_DEMO 1`: desativa a checagem de horário e reduz `DEGELO_MAX_MS` para 15 s.
- `RELE_ATIVO_EM_LOW`: configurável.
- Rede: `Wokwi-GUEST`, `BROKER_MQTT` (placeholder `3.215.3.216`), porta 1883, `ID_DEVICE = "cshield001"`.

### 2.4 Estados, detecção e resposta

**Prioridade**: **CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL**.

| Estado | Entra quando | Resposta local | Publica |
| --- | --- | --- | --- |
| NORMAL | Temperatura dentro de [T_MIN, T_MAX] e nenhum sinal de ataque | Relés em repouso (o controlador manda); LED verde; display com temperatura e faixa | A cada 5 s |
| ATENCAO | Porta aberta; ou temperatura fora da faixa mas dentro da tolerância; ou degelo legítimo em andamento | Relés em repouso; LED amarelo; 1 bipe a cada 10 s; "PORTA ABERTA", "DERIVA" ou "DEGELO 0:12" (contagem) | Leitura + motivo |
| BLOQUEIO | Um dos sinais de ataque (D1 a D3) | Relé 1 corta o aquecedor; relé 2 pelo termostato de emergência; sinais do controlador ignorados e contados; LED vermelho piscando; buzzer intermitente; "ATAQUE BLOQUEADO" + motivo + "SEGURE BOTAO 3s" | Evento imediato com motivo |
| CRITICO | Temperatura acima de `T_MAX + TOLERANCIA_C` (confirmada), com ou sem bloqueio | Mesmas ações do bloqueio; LED vermelho fixo; buzzer contínuo; "PRODUTO EM RISCO" + exposição térmica | Evento + exposição |
| FALHA_SENSOR | DHT22 retorna NaN ou valor fora de -40 a 80 °C | LED amarelo piscando; "SENSOR SEM LEIT."; se já estiver em bloqueio, a ventoinha fica ligada direto | Evento de falha |

**Detecção do ataque (entra em BLOQUEIO):**

- **D1. Aquecedor ligado com a ventoinha desligada, fora do degelo permitido** (motivo `DEGELO_FORCADO`): fora dos horários (com hora válida, fora do `MODO_DEMO`) ou por mais que `DEGELO_MAX_MS`. Dentro do permitido é só ATENCAO com a contagem (I5).
- **D2. Aquecedor ligado com a ventoinha ligada por mais que `DEGELO_MAX_MS`** (motivo `AQUECEDOR_LONGO`).
- **D3. Subida rápida:** taxa acima de `TAXA_MAX_C_POR_MIN` com a porta fechada (motivo `SUBIDA_RAPIDA`). Porta aberta: só ATENCAO. Em FALHA_SENSOR, D3 não é avaliada.

**Durante o BLOQUEIO:**

1. Relé 1 acionado o tempo todo: o aquecedor não liga.
2. Relé 2 é o termostato de emergência (liga em `T_ALVO + HISTERESE_C`, solta em `T_ALVO - HISTERESE_C`; começa ligado, I2; solto = segue o controlador, I1).
3. Toda mudança nas chaves do controlador é ignorada e incrementa `ci`.
4. **Não há expiração.** Só o botão libera.

**Liberação presencial:** botão por 3 s. Se a temperatura estiver dentro de [T_MIN − TOLERANCIA_C, T_MAX + TOLERANCIA_C] e válida, os relés voltam ao repouso. Se não estiver, continua no controle e mostra "FORA DA FAIXA" / "AGUARDE".

**Princípios que não podem ser quebrados:** nunca impedir a refrigeração; falha segura (relés em repouso antes de tudo no `setup()`); só envia (proibido `subscribe`, `setCallback`, OTA, servidor HTTP); offline funciona (reconexão a cada 10 s sem travar o `loop()`).

### 2.5 Exposição térmica

Acima de `T_MAX`, acumular `(T − T_MAX) × Δt` em °C·min. Publicar em `ex` e mostrar no CRITICO.

### 2.6 FIWARE (UltraLight 2.0 sobre MQTT)

- Headers `fiware-service: smart`, `fiware-servicepath: /`, apikey `TEF`. Tópico `/TEF/cshield001/attrs`. Nenhum tópico de comando.
- Payload: `t|4.8|h|81|l|12|aq|0|vt|1|vf|1|st|NORMAL|mt|-|bq|0|ci|0|ex|0`
- Device `cshield001`, entity `urn:ngsi-ld:CShield:001`, type `CShield`, `PDI-IoTA-UltraLight`, MQTT, só `attributes` (nomes longos na I6).
- `provisionar.sh` com IP numa variável no topo, 409 como OK, subscriptions do STH-Comet (I6).

### 2.7 Serial Monitor

`[cshield001] t=4.8C h=81% l=12% aq=0 vt=1 vf=1 taxa=+0.02C/min st=NORMAL mt=- ci=0 rede=OK`

Em toda mudança de estado: `>>> BLOQUEIO: DEGELO_FORCADO (aquecedor ligado e ventoinha desligada ha 15s)`.

---

## 3. Tarefas da v2 (resumo; o passo a passo está na §5)

- **Fase A — Especificação:** T1 (o que reaproveita e o que muda, já respondido na §5.1), T2 `requirements.md`, T3 `design.md`, T4 `tasks.md`, T5 aprovação (dada por este replanejamento, V3).
- **Fase B — Lógica pura e testes:** T6 `config.h`, T7 `decisao.h`, T8 `test_decisao.cpp`.
- **Fase C — Firmware:** T9 `sensores.h`, T10 `atuadores.h`, T11 `painel.h`, T12 botão, T13 `rede.h`, T14 `cshield.ino` (+ `simulacao.h`, V2).
- **Fase D — Wokwi e FIWARE:** T15 `diagram.json`, T16 `libraries.txt`, T17 `provisionar.sh`, T18 teste ponta a ponta.
- **Fase E — Documentação:** T19 `edge-testes-estados.md`, T20 `edge-roteiro-video.md`, T21 `README.md`.

## 4. Revisão e autocorreção (texto da v2)

### 4.1 Verificações automáticas

```bash
# 1. Testes da lógica (usar o comando da H5)
# 2. Não recebe comandos: resultado VAZIO
grep -rnE "subscribe|setCallback|ArduinoOTA|WebServer" edge/cshield/
# 3. Nada de delay() no loop
grep -n "delay(" edge/cshield/*.ino edge/cshield/*.h
# 4. diagram.json válido
python -m json.tool edge/cshield/diagram.json > /dev/null && echo "diagram.json OK"
# 5. Todo pino do config.h aparece no diagram.json
grep -nE "#define PIN|const int PIN" edge/cshield/config.h
# 6. Não sobrou nada da versão anterior: resultado VAZIO
grep -rniE "guardiao|guardião|setpoint|potenci|TRAVA_MS" edge/ docs/
# 7. Compilar, se houver arduino-cli (não há: registrar no relatório, I8)
arduino-cli compile --fqbn esp32:esp32:esp32 edge/cshield || echo "arduino-cli indisponível"
```

### 4.2 Conferência contra a rubrica

Tabela com cada linha da rubrica e a evidência (arquivo e linha): no mínimo 3 estados com respostas distintas (temos 5); cada limite justificado no `config.h`; leitura inválida e suavização; funções pequenas, constantes, `millis()`, comentários, Serial; integrantes no código; diagrama com lado do controlador e lado da geladeira separados; funciona offline.

### 4.3 Conferência dos princípios do produto

Nenhum caminho de código libera o bloqueio pela rede; no bloqueio as chaves do controlador não mudam os relés; o bloqueio não expira; degelo legítimo não bloqueia; boot e reset deixam os relés em repouso; o C-Shield nunca desliga a refrigeração; nenhuma senha ou IP real além dos placeholders.

### 4.4 Relatório final

O que foi feito; testes que passaram; o que não foi possível verificar (compilação do `.ino` sem `arduino-cli`, teste real no Wokwi, T18 se a EC2 estiver desligada); requisitos fora e por quê; 3 próximos passos para a equipe.

---

## 5. Plano de execução para o Sonnet

### 5.1 Contexto

A v1 está implementada e commitada (último commit `511469e`): 5 estados, regras de setpoint, degelo e taxa, trava mínima de 60 s, chave NORMAL/ATAQUE, LED de refrigeração, modelo térmico, LCD, rede só publish, 100 testes verdes. A v2 troca a entrada do controlador (setpoint por aquecedor e ventoinha), as regras (D1 a D3), a resposta no bloqueio (termostato de emergência) e a liberação (sem trava). O site `c-shield-web` depende dos nomes de atributos da I6.

**Reaproveitar sem mudança de comportamento:** `rede.h` (Wi-Fi, NTP, sonda TCP, reconexão, `publicarSePreciso`), LED RGB e buzzer de `atuadores.h`, debounce e barra do botão em `sensores.h`/`painel.h`, `escreverLinha` do LCD, média móvel, validação do DHT22, exposição, `registrarAmostra`/`calcularTaxa`, estrutura do `provisionar.sh`, README e Ficha.

**Remover:** chave NORMAL/ATAQUE (GPIO 32), `SETPOINT_*`, `TEM_POT_SETPOINT`, `TRAVA_MS`, `travaRestanteMs`, `liberacaoRecusada` por trava, `mostrarTravaAtiva`, motivo `SETPOINT_INVALIDO`, LED verde de refrigeração, atributos `sp` e `dg`, e todo texto "Guardião" (I7).

### 5.2 Objetivo

Com a chave 1 (aquecedor) ligada e a chave 2 (ventoinha) desligada por mais de 15 s no Wokwi, o C-Shield entra em BLOQUEIO `DEGELO_FORCADO`, o LED laranja apaga, o LED azul acende pelo termostato, mexer nas chaves incrementa `ci`, e segurar o botão 3 s com a temperatura na faixa devolve o controle ao controlador; tudo coberto por testes `g++` verdes e documentado.

### 5.3 Detalhes técnicos

**`config.h`** (T6)

```cpp
#define MODO_DEMO 1
#define MODELO_TERMICO_DEMO 1
#define RELE_ATIVO_EM_LOW 0

const int PIN_DHT = 4, PIN_LDR = 34, PIN_CHAVE_AQ = 18, PIN_CHAVE_VT = 17, PIN_BOTAO = 19;
const int PIN_RELE_AQUEC = 23, PIN_RELE_VENT = 33;
const int PIN_LED_R = 25, PIN_LED_G = 26, PIN_LED_B = 27, PIN_BUZZER = 5;
const int PIN_I2C_SDA = 21, PIN_I2C_SCL = 22;

struct Perfil { float tMin, tMax, tAlvo, tolerancia; };
// VACINA {2, 8, 5, 1} · CONGELADO {-30, -18, -20, 1} · RESFRIADO {0, 4, 2, 1} · PLASMA {-40, -25, -30, 1}
const float HISTERESE_C = 1.0f;
#if MODO_DEMO
const unsigned long DEGELO_MAX_MS = 15UL * 1000UL;
#else
const unsigned long DEGELO_MAX_MS = 30UL * 60UL * 1000UL;
#endif
const int HORA_DEGELO_1 = 6, HORA_DEGELO_2 = 18, JANELA_DEGELO_MIN = 30;
const unsigned long DEBOUNCE_CHAVE_MS = 50;
// modelo térmico (V2): RESFRIAMENTO_DEMO_C_POR_MIN = 6.0, AQUECIMENTO_DEMO_C_POR_MIN = 3.0,
// PISO_DEMO_C = T_ALVO - HISTERESE_C - 0.5 (calculado em simulacao.h), MODELO_MUDANCA_C = 0.3
```

Manter: `NOME_PRODUTO`, LCD, `PERIODO_LEITURA_MS`, `JANELA_MEDIA`, `CONFIRMACOES`, faixa física, `LIMIAR_PORTA_PCT`, `JANELA_TAXA_MS`, `N_AMOSTRAS_TAXA`, `TAXA_MIN_INTERVALO_MS`, `TAXA_MAX_C_POR_MIN`, `LIBERACAO_MS`, `DEBOUNCE_MS`, `LIBERADO_TELA_MS`, alertas, rede. Cada constante nova com comentário de justificativa.

**`decisao.h`** (T7) — pura, sem `Arduino.h`

```cpp
enum Estado { ST_NORMAL, ST_ATENCAO, ST_BLOQUEIO, ST_CRITICO, ST_FALHA_SENSOR };
enum Motivo { MT_NENHUM, MT_PORTA_ABERTA, MT_DERIVA, MT_DEGELO, MT_DEGELO_FORCADO,
              MT_AQUECEDOR_LONGO, MT_SUBIDA_RAPIDA, MT_TEMP_FORA_TOLERANCIA, MT_SENSOR_SEM_LEITURA };
// nomeMotivo: "-", "PORTA_ABERTA", "DERIVA", "DEGELO", "DEGELO_FORCADO", "AQUECEDOR_LONGO",
//             "SUBIDA_RAPIDA", "TEMP_FORA_TOLERANCIA", "SENSOR_SEM_LEITURA"

struct Leitura {
  bool valida; float temperatura; float umidade; float luzPct;
  bool aquecedorCmd; bool ventoinhaCmd;          // o que o controlador está mandando
  bool horaValida; int minutoDoDia;              // 0..1439 (I4)
  bool liberacao;                                // botão segurado 3 s
};
struct Contexto {
  bool iniciado; unsigned long ultimoMs;
  bool bloqueado; Motivo motivoBloqueio;
  bool aquecedorAtivo; unsigned long inicioAquecedorMs;   // desde quando o aquecedor está ligado
  bool termostatoLigado;                                  // ventoinha de emergência (relé 2)
  int bloqueios; int ignorados;                           // bq e ci
  float exposicao; int contCritico; int contInvalido;
  float histTemp[N_AMOSTRAS_TAXA]; unsigned long histMs[N_AMOSTRAS_TAXA]; int histN;
  bool ultimoAq; bool ultimoVt;
};
struct Decisao {
  Estado estado; Motivo motivo;
  bool cortarAquecedor; bool ligarVentoinha;     // relé 1 e relé 2
  bool novoBloqueio; bool mudancaIgnorada;
  bool liberou; bool liberacaoRecusada;          // recusada = temperatura fora da faixa
  float taxaCPorMin; float exposicao;
  unsigned long degeloRestanteMs;                // contagem do degelo legítimo (ATENCAO)
  unsigned long aquecedorLigadoMs;               // para a linha de evento do Serial
};

void   iniciarContexto(Contexto&);
bool   degeloNoHorario(const Leitura&);          // MODO_DEMO ou sem hora válida: true
bool   liberar(const Leitura&, Contexto&);       // true se liberou; chamada por avaliar()
Decisao avaliar(const Leitura&, Contexto&, unsigned long agoraMs);
const char* nomeEstado(Estado); const char* nomeMotivo(Motivo);
```

Ordem dentro de `avaliar()`: (1) sensor, histórico, exposição, contadores de CRITICO e FALHA; (2) tempo do aquecedor (`aquecedorAtivo`/`inicioAquecedorMs`, conta mesmo durante o bloqueio); (3) liberação via `liberar()`: se recusada, `d.liberacaoRecusada = true`; se aceita, `d.liberou = true`, `bloqueado = false`, `termostatoLigado = false`, `histN = 0` e registra a amostra atual; (4) se bloqueado, qualquer mudança em `aquecedorCmd` ou `ventoinhaCmd` em relação ao ciclo anterior faz `ignorados++` e `mudancaIgnorada = true`; se não bloqueado, avaliar D1, D2, D3 nessa ordem e, se houver violação, `bloqueado = true`, `bloqueios++`, `novoBloqueio = true`, `termostatoLigado = true` (I2); (5) termostato: se bloqueado ou crítico e a leitura é válida, `t >= tAlvo + H` liga, `t <= tAlvo - H` desliga, entre os dois mantém; se o sensor está em falha durante o bloqueio, liga; (6) estado pela prioridade; ATENCAO por porta, deriva ou degelo legítimo (I5, com `degeloRestanteMs`); (7) `cortarAquecedor = bloqueado || critico`; `ligarVentoinha = (bloqueado || critico) && termostatoLigado`; fora disso os dois `false` (repouso); (8) memória `ultimoAq`, `ultimoVt`. Se passar de ~250 linhas, mover as funções auxiliares para `decisao_regras.h` (também puro).

**`test_decisao.cpp`** (T8) — reaproveitar a estrutura `CHECK` atual; um caso por item, cada um citando o requisito:

1. degelo legítimo dentro do tempo: ATENCAO `DEGELO`, `degeloRestanteMs` decrescendo, relés em repouso;
2. degelo que passa de `DEGELO_MAX_MS`: BLOQUEIO `DEGELO_FORCADO`;
3. degelo fora do horário com hora válida e `MODO_DEMO` desligado no teste: BLOQUEIO imediato (testar `degeloNoHorario` com `minutoDoDia` 360, 389, 390, 1080, 120);
4. degelo sem hora válida: só a duração vale;
5. aquecedor com ventoinha ligada além do tempo: `AQUECEDOR_LONGO`;
6. subida rápida com porta fechada: `SUBIDA_RAPIDA`;
7. a mesma com porta aberta: ATENCAO, sem bloqueio;
8. sensor NaN por 2 ciclos: FALHA_SENSOR; 1 ciclo não basta;
9. sensor fora de -40 a 80: inválido;
10. chaves mudando durante o bloqueio: `ci` sobe e os relés não mudam;
11. termostato: começa ligado, desliga em 4 °C, continua desligado em 5 °C, liga em 6 °C;
12. bloqueio sem expiração: 2 h depois, sem botão, continua BLOQUEIO;
13. liberação com temperatura boa: NORMAL, relés em repouso, `liberou`;
14. liberação com temperatura ruim (10,5 °C): continua bloqueado, `liberacaoRecusada`;
15. falha de sensor durante o bloqueio: ventoinha ligada;
16. crítico confirmado: CRITICO, corta aquecedor, ventoinha ligada;
17. exposição acumulada: 10 °C por 2 min = 4 °C·min;
18. prioridade: crítico com bloqueio mostra CRITICO; bloqueio com falha mostra BLOQUEIO;
19. após liberar, a subida antiga não rebloqueia (regressão da v1);
20. crítico sem bloqueio que volta à faixa: relés voltam ao repouso (I3).

Para os testes com horário, `MODO_DEMO` precisa poder ser 0 no teste: em `config.h` usar `#ifndef MODO_DEMO` / `#define MODO_DEMO 1` / `#endif` e compilar uma segunda vez com `-DMODO_DEMO=0`. Os casos que dependem do modo ficam sob `#if MODO_DEMO`. Rodar as duas compilações.

**`sensores.h`** (T9): remover o setpoint; ler `PIN_CHAVE_AQ` e `PIN_CHAVE_VT` com `INPUT_PULLDOWN` e debounce de `DEBOUNCE_CHAVE_MS` (reaproveitar o padrão de debounce do botão numa função `struct Debounce` + `atualizarDebounce()`, usada pelas duas chaves **e** pelo botão, sem copiar código). `lerSensores` preenche `aquecedorCmd` e `ventoinhaCmd`.

**`atuadores.h`** (T10): `aplicarReles(bool cortarAquecedor, bool ligarVentoinha)` já existe; manter e conferir `RELE_ATIVO_EM_LOW`. Buzzer: CRITICO contínuo (tom ligado direto), BLOQUEIO intermitente 300 ms, ATENCAO e FALHA_SENSOR 1 bipe a cada 10 s. LED: FALHA_SENSOR amarelo **piscando**, ATENCAO amarelo fixo, CRITICO vermelho fixo, BLOQUEIO vermelho piscando, NORMAL verde.

**`painel.h`** (T11 e T12) — telas de 16 colunas:

| Situação | Linha 1 | Linha 2 |
| --- | --- | --- |
| Boot | `C-Shield` | `Iniciando...` |
| NORMAL | `C-Shield NORMAL` | `5.0C 2a8C` |
| ATENCAO porta | `PORTA ABERTA` | `5.0C 2a8C` |
| ATENCAO deriva | `DERIVA` | `8.5C 2a8C` |
| ATENCAO degelo | `DEGELO 0:12` | `5.0C 2a8C` |
| BLOQUEIO | `ATAQUE BLOQUEADO` | alterna a cada 2 s: motivo curto (`DEGELO FORCADO`, `AQUEC LONGO`, `SUBIDA RAPIDA`) / `SEGURE BOTAO 3s` |
| BLOQUEIO com t > T_MAX | `RESTAURANDO TEMP` | `BLOQUEIO ATIVO` |
| CRITICO | `PRODUTO EM RISCO` | `T9.5 EX 3.2` |
| FALHA_SENSOR | `SENSOR SEM LEIT.` | `DHT22 sem dado` |
| Segurando botão | `LIBERANDO...` | barra de 16 blocos |
| Liberação recusada (3 s) | `FORA DA FAIXA` | `AGUARDE` |
| Liberação aceita (3 s) | `BLOQUEIO` | `LIBERADO!` |

O indicador "C-SHIELD NO CONTROLE" da v2 é a linha 1 `ATAQUE BLOQUEADO` junto do LED vermelho (não cabe uma terceira linha). Remover `mostrarTravaAtiva`.

**`rede.h`** (T13): só `montarPayload` muda:
`snprintf(buf, n, "t|%.1f|h|%.0f|l|%.0f|aq|%d|vt|%d|vf|%d|st|%s|mt|%s|bq|%d|ci|%d|ex|%.1f", ...)` com `vf = d.ligarVentoinha ? 1 : (l.ventoinhaCmd ? 1 : 0)` (I1). Buffer de `payload` em `cshield.ino` passa a 192. `horaAtual` passa a devolver `minutoDoDia`.

**`cshield.ino`** (T14): Serial da §2.7 (uma linha por ciclo) e linha de evento `>>>` em toda mudança de `d.estado`; mensagens de `liberou`, `liberacaoRecusada` e `mudancaIgnorada` (esta última com o `ci` atual); prazo das telas de liberação aceita e recusada; restaurando como hoje; sem a trava.

**`simulacao.h`** (V2): `modeloTermicoDemo(float medida, bool cShieldNoControle, bool ventoinhaEfetiva, unsigned long agoraMs)`:
- se a medida mudou mais que `MODELO_MUDANCA_C`: segue a medida (a pessoa mexeu no slider);
- senão, se `cShieldNoControle` e `ventoinhaEfetiva`: esfria a `RESFRIAMENTO_DEMO_C_POR_MIN` até `PISO = tAlvo - HISTERESE_C - 0.5` (abaixo do ponto de desligar, para o termostato desligar sozinho);
- senão, se `cShieldNoControle` e a ventoinha está desligada: aquece a `AQUECIMENTO_DEMO_C_POR_MIN` em direção à medida (o calor do ambiente), sem passar dela;
- fora do bloqueio: mantém o valor (depois de liberar, a temperatura restaurada continua valendo até a pessoa mexer no slider).
Chamada em `cicloDeDecisao` antes de `avaliar`, com `contexto.bloqueado` e a `ligarVentoinha` da decisão anterior.

**`diagram.json`** (T15): duas chaves à esquerda ("Controlador: aquecedor" em GPIO 18, "Controlador: ventoinha" em GPIO 17), padrão de ligação das chaves da v1 (pino 2 no GPIO, 1 no GND, 3 no 3V3). Lado da geladeira à direita: relé 1 com `COM` no sinal da chave 1 e `NC` → resistor → LED laranja "Aquecedor" → GND; relé 2 com `NC` no sinal da chave 2, `NO` no 3V3 e `COM` → resistor → LED azul "Ventoinha" → GND. Remover `sw2` (NORMAL/ATAQUE), `led_vr`, `r_vr`. DHT22 em 5 °C. Cores: vermelho 3V3, preto GND, verde sinais, azul I2C.

**`provisionar.sh`** (T17): atributos e subscriptions da I6; trocar o cabeçalho (I7).

### 5.4 Ferramentas e requisitos

- `g++` pelo caminho da H5 (duas compilações: padrão e `-DMODO_DEMO=0`).
- `python` para validar o JSON; `bash -n` e `DRY_RUN=1` para o `provisionar.sh`.
- EC2 em `3.215.3.216` só para a T18 (não respondia em 10/11; se continuar fora, deixar o passo a passo).
- Wokwi: verificação final é do usuário (copiar todos os arquivos de `edge/cshield/`).

### 5.5 Passos

Cada passo termina com verificação e commit em português (H4). Não avançar com teste vermelho.

**Fase A**
1. Reescrever `docs/specs/edge/requirements.md` para a v2 (R1 em diante, formato "QUANDO…, O C-SHIELD DEVE…"), cobrindo §2.4, I1 a I5, §2.6, §2.7 e V2. Commit `docs: reescreve requisitos para a versao 2`.
2. Reescrever `docs/specs/edge/design.md`: tabela de módulos, Mermaid da máquina de estados, fluxo do `loop()` e as assinaturas da §5.3. Commit.
3. Reescrever `docs/specs/edge/tasks.md`: tarefas T6 a T21 com os requisitos cobertos e a matriz de cobertura. Commit.
4. Atualizar `CLAUDE.md` §1 e §6 (descrição do produto, nomes UltraLight `t h l aq vt vf st mt bq ci ex`, remover o `controlador001` opcional e "Guardião"). Commit.

**Fase B**
5. `config.h` conforme §5.3. Ainda não compila com a `decisao.h` antiga: esperado. Commit junto com o passo 7.
6. Reescrever `test_decisao.cpp` com os 20 casos. Compilar: falha (as estruturas ainda não existem). Esse é o vermelho.
7. Reescrever `decisao.h`. Compilar e rodar nas duas configurações até 0 falhas. Commit `feat: logica de decisao da versao 2 com termostato de emergencia` (config + decisao + teste).
8. Revisar a contagem: cada requisito da lógica tem ao menos um teste; registrar na matriz do `tasks.md`. Commit.

**Fase C**
9. `sensores.h` (chaves com debounce compartilhado, sem setpoint). Commit.
10. `atuadores.h` (padrões da §5.3). Commit.
11. `painel.h` (tabela da §5.3). Commit.
12. `simulacao.h` adaptado. Testar o modelo com um programa descartável no `$TEMP` (como o `sim.cpp` da v1): ataque D1 → termostato desliga em 4 °C e religa em 6 °C pelo menos uma vez. Commit.
13. `rede.h` (payload e `minutoDoDia`). Commit.
14. `cshield.ino` (Serial, eventos, telas, sem trava). Commit.
15. `grep` dos símbolos removidos (`setpoint`, `TRAVA_MS`, `travaRestante`, `mostrarTravaAtiva`, `PIN_CHAVE_SP`, `SETPOINT_`) em `edge/`: vazio. Rodar os testes de novo.

**Fase D**
16. `diagram.json` (§5.3). Validar com `python -m json.tool`. Conferir que todo `PIN_` do `config.h` aparece no diagrama. Commit.
17. `libraries.txt`: conferir que não mudou (PubSubClient, DHT sensor library, Adafruit Unified Sensor, LiquidCrystal I2C).
18. `provisionar.sh` (I6, I7). `bash -n` e `DRY_RUN=1` sem `sp`/`dg` e sem `commands`. Commit.
19. T18: tentar `curl` no Orion. Se a EC2 não responder, escrever o passo a passo em `docs/edge-testes-estados.md` (chave aquecedor ligada, ventoinha desligada, esperar 15 s, `GET` mostra `st=BLOQUEIO`, `bq` +1; mexer na chave, `ci` +1). Commit.

**Fase E**
20. `docs/edge-testes-estados.md`: uma linha por estado e por regra D1 a D3, incluindo degelo legítimo, `ci`, termostato, liberação boa e ruim, falha no bloqueio, modelo térmico; colunas LED RGB, LEDs laranja e azul, buzzer, LCD, Serial, resultado. Commit.
21. `docs/edge-roteiro-video.md`: sequência da v2 (normal → porta aberta → degelo legítimo com contagem → o mesmo degelo passando de 15 s → mexer nas chaves, `ci` sobe → termostato liga e desliga sozinho (modelo térmico, dizer isso em uma frase) → botão com temperatura ruim recusado → botão com temperatura boa → controlador volta a mandar → falha de sensor), tempos que somem até 7 min, lembretes da EC2 e da aba aberta. Commit.
22. `README.md`: atualizar a parte técnica (como usar as duas chaves, como rodar os testes com a H5, payload novo), manter a Ficha. Commit.
23. Revisão §4 completa, corrigir o que falhar, rodar de novo, entregar o relatório §4.4.

### 5.6 Como verificar

- `g++` nas duas configurações: `N verificacoes, 0 falhas` em ambas.
- `grep -rnE "subscribe|setCallback|ArduinoOTA|WebServer" edge/cshield/` vazio.
- `grep -rniE "guardiao|guardião|setpoint|potenci|TRAVA_MS" edge/ docs/` vazio.
- `python -m json.tool edge/cshield/diagram.json` sem erro.
- `DRY_RUN=1 bash edge/fiware/provisionar.sh` com 11 atributos, 8 subscriptions, nenhum `commands`.
- No Wokwi (usuário): NORMAL com LED verde, laranja apagado (chave 1 desligada) e azul conforme a chave 2; chave 1 ligada e chave 2 desligada mostram `DEGELO 0:15` contando; aos 15 s `ATAQUE BLOQUEADO`, laranja apaga, azul acende; mexer nas chaves aumenta `ci` no Serial; a temperatura cai até 3,5 °C, o azul apaga, sobe até 6 °C, o azul acende; botão de 3 s com temperatura boa mostra `BLOQUEIO LIBERADO!` e o controlador volta a mandar.
