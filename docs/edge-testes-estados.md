# C-Shield — Testes dos estados no Wokwi

Projeto C-Shield · Nexus Vision · GS 2026.2 "CitizenTech". Este roteiro serve para a equipe validar cada estado no simulador antes de gravar o vídeo, e para o teste ponta a ponta com o FIWARE (T18b).

A lógica de decisão também tem testes automáticos no computador (`edge/test/test_decisao.cpp`), com um caso por requisito. Esta tabela confere o conjunto completo no Wokwi, com LED, buzzer, LCD, relés e Serial. As regras de degelo (fora de horário e longo demais) não têm chave no Wokwi: ficam cobertas só pelos testes automáticos do computador. O GPIO 18 existe para o fio do controlador na maquete.

## Antes de começar

1. No Wokwi, abra o projeto com `edge/cshield/` (o `.ino`, os `.h`, o `diagram.json` e o `libraries.txt`).
2. Perfil ativo: vacina (+2 a +8 °C, tolerância 1 °C). Modo demonstração ligado (`MODO_DEMO 1`): a trava mínima do bloqueio é de 60 s.
3. Abra o Serial Monitor. A cada 2 s aparece uma linha como
   `[cshield001] t=4.8C h=81% l=12% sp=5.0C dg=0 taxa=+0.02C/min st=NORMAL mt=- trava=0s rede=OK`.
4. **Ponto de partida (NORMAL):** DHT22 em 5 °C, LDR escuro, botão solto. A chave "Controlador" fica na posição NORMAL (esquerda), e o Serial mostra `sp=5.0C`.
5. Em NORMAL o LED fica verde, o buzzer em silêncio, os dois LEDs indicadores mostram aquecedor (laranja) aceso e ventoinha (azul) apagada.

## Tabela de estados

Legenda de relés: "Aquecedor" é o LED laranja no contato NF do relé 1 (aceso = aquecedor ligado, apagado = cortado). "Ventoinha" é o LED azul no contato NA do relé 2.

| Estado | O que mexer no Wokwi | LED RGB | Buzzer | LCD (16x2) | Aquecedor / Ventoinha | Serial | Resultado |
| --- | --- | --- | --- | --- | --- | --- | --- |
| NORMAL | Ponto de partida acima | Verde fixo | Silêncio | `C-Shield NORMAL` / `5.0C 2a8C` | aceso / apagado | `st=NORMAL mt=-` | |
| ATENCAO (porta aberta) | Subir o slider de luz do LDR até `l=` passar de 40 no Serial | Amarelo fixo | 1 bipe curto a cada 10 s | `PORTA ABERTA` / temperatura e faixa | aceso / apagado | `st=ATENCAO mt=PORTA_ABERTA` | |
| ATENCAO (deriva) | Com a porta fechada, DHT22 em 8,5 °C (entre 8 e 9) | Amarelo fixo | 1 bipe curto a cada 10 s | `DERIVA` / `8.5C 2a8C` | aceso / apagado | `st=ATENCAO mt=DERIVA` | |
| BLOQUEIO (setpoint) | Mover a chave "Controlador" para ATAQUE (`sp=45.0C`) | Vermelho piscando | Intermitente (300 ms) | `ATAQUE BLOQUEADO` / `SETPOINT 45C 58s` | **apagado** / **aceso** | `*** BLOQUEIO (SETPOINT_INVALIDO) ***`, `st=BLOQUEIO`, `trava=` decrescendo | |
| BLOQUEIO (aquecimento rápido) | Com a porta fechada, subir o DHT22 de 5 °C para cerca de 7 °C em poucos segundos (taxa acima de 0,5 °C/min) | Vermelho piscando | Intermitente | `ATAQUE BLOQUEADO` / `AQUEC RAPIDO 58s` | apagado / aceso | `mt=AQUECIMENTO_RAPIDO`, `taxa=+` alta | |
| Aquecimento rápido com porta aberta | Igual ao anterior, mas com o LDR claro | Amarelo fixo | 1 bipe a cada 10 s | `PORTA ABERTA` | aceso / apagado | `st=ATENCAO` (não bloqueia) | |
| Trava ignora ordem | Durante o BLOQUEIO, mexer na chave "Controlador" de novo (voltar para NORMAL e para ATAQUE) | Vermelho piscando | Intermitente | continua `ATAQUE BLOQUEADO` | apagado / aceso | `ordem do controlador IGNORADA (trava ativa)` | |
| CRITICO | DHT22 em 10 °C ou mais (acima de 9), ou 0 °C ou menos (abaixo de 1), por 2 ciclos | Vermelho fixo | Contínuo | `PRODUTO EM RISCO` / `T10.0 EX 0.1` | apagado / aceso | `st=CRITICO` | |
| FALHA_SENSOR | Desconectar o fio de dados do DHT22 (ou apagar a conexão `dht1:SDA` no diagrama), por 2 ciclos | Amarelo piscando | 2 bipes a cada 10 s | `FALHA NO SENSOR` / `DHT22 sem dado` | aceso / apagado | `st=FALHA_SENSOR`, `t=-999.0` | |
| Liberação com temperatura boa | Em BLOQUEIO, trazer `sp` de volta para 2 a 8 e a temperatura para a faixa; esperar o `trava=0s`; segurar o botão por 3 s | Volta ao verde | Silêncio | `LIBERANDO...` com barra de 16 blocos, depois `C-Shield NORMAL` | aceso / apagado | `st=NORMAL` | |
| Liberação antes da trava | Segurar o botão por 3 s com `trava` ainda maior que 0 | Continua piscando | Continua | `LIBERANDO...` e volta a `ATAQUE BLOQUEADO` | apagado / aceso | segue `st=BLOQUEIO` | |
| Liberação com ordem ainda inválida | Segurar o botão por 3 s depois da trava, com `sp` ainda fora da faixa | Volta a piscar | Intermitente | `ATAQUE BLOQUEADO` | apagado / aceso | novo `*** BLOQUEIO ***`, `bq` sobe | |
| Offline | Desligar o Wi-Fi (ou usar um `BROKER_MQTT` inválido) | Mesmas respostas locais | Iguais | Iguais | Iguais | `rede=OFF`, decisões iguais | |

Observações para a gravação:

- A janela de degelo permitido é 06:00 a 07:00 e 18:00 a 19:00 pelo relógio de Brasília. Se for gravar nesses horários, o degelo só bloqueia depois de 30 minutos. Fora deles, bloqueia na hora.
- Para ver o alvo do aquecimento rápido sem passar do limite crítico, suba o DHT22 em passos curtos. Passar direto para um valor alto também funciona: o bloqueio por setpoint ou o CRITICO aparecem antes.
- O LDR do Wokwi foi lido com o mesmo critério da Vinheria (percentual invertido do ADC). Confira no Serial que `l=` passa de 40 quando a "porta abre".

## Teste ponta a ponta sem hardware (T18b)

Objetivo: confirmar que a simulação do Wokwi publica no FIWARE e que `st` e `bq` mudam no Orion quando a chave "Controlador" vai para ATAQUE.

**Situação em 10/11/2026:** este teste **não foi executado** pelo agente. A EC2 estava sem resposta (`curl http://3.215.3.216:1026/version` sem retorno) e o Wokwi não roda de dentro do terminal do agente. Fica o passo a passo para a equipe; depois de rodar, preencha os campos de resposta abaixo.

### Passo a passo

1. Ligue a EC2 e confirme o IP elástico. Se mudou, atualize `BROKER_MQTT` no `config.h`.
2. Confira o Orion: `curl -s http://<IP>:1026/version`. Deve devolver um JSON com a versão.
3. Provisione o device:
   ```bash
   FIWARE_HOST=<IP> bash edge/fiware/provisionar.sh
   ```
   Esperado: linhas `ok` para o service group, o device `cshield001 (sem comandos)` e as 6 subscriptions.
4. No Wokwi, abra o projeto, clique em iniciar e deixe a aba aberta (o Wokwi só publica com a simulação ativa). No Serial deve aparecer `Conectando ao broker MQTT... ok` e `rede=OK`.
5. Com o ponto de partida (NORMAL), consulte a entidade:
   ```bash
   curl -s "http://<IP>:1026/v2/entities/urn:ngsi-ld:CShield:001?options=keyValues" \
     -H "fiware-service: smart" -H "fiware-servicepath: /"
   ```
   Esperado: `"state":"NORMAL"` e `"blocks":0` (nomes longos do Orion: `state` = `st`, `blocks` = `bq`).
6. Mova a chave "Controlador" para ATAQUE (`sp=45.0C` no Serial). Espere uns 6 s e repita o comando do passo 5.
   Esperado: `"state":"BLOQUEIO"`, `"reason":"SETPOINT_INVALIDO"` e `"blocks":1`.
7. Registre abaixo o comando e as duas respostas.

### Registro do teste

| Campo | Valor |
| --- | --- |
| Data e quem executou | |
| IP da EC2 usado | |
| Resposta com NORMAL (passo 5) | |
| Resposta com BLOQUEIO (passo 6) | |
| Resultado | |

## Limitações conhecidas

- Este documento não foi executado no Wokwi pelo agente: a coluna "Resultado" fica para a equipe preencher.
- Os nomes de pino dos componentes `wokwi-relay-module` e `wokwi-lcd1602` do `diagram.json` foram escritos a partir da documentação do Wokwi conhecida, sem abrir o projeto no simulador. Se algum fio aparecer solto ao abrir, conferir os nomes dos pinos desses dois componentes.
