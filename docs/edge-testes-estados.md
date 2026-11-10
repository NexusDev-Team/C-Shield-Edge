# C-Shield — Testes dos estados no Wokwi

Projeto C-Shield · Nexus Vision · GS 2026.2 "CitizenTech". Este roteiro serve para a equipe validar cada estado e cada regra de ataque (D1 a D3) no simulador antes de gravar o vídeo, e para o teste ponta a ponta com o FIWARE (T18).

A lógica de decisão também tem testes automáticos no computador (`edge/test/test_decisao.cpp`), com um caso por requisito. Esta tabela confere o conjunto completo no Wokwi, com LED, buzzer, LCD, relés e Serial.

## Antes de começar

1. No Wokwi, abra o projeto com todos os arquivos de `edge/cshield/` (`cshield.ino`, os `.h`, `diagram.json` e `libraries.txt`).
2. Perfil ativo: vacina (+2 a +8 °C, alvo 5 °C, tolerância 1 °C). Modo demonstração ligado (`MODO_DEMO 1`): o degelo máximo é de **15 s** e a checagem de horário fica desligada.
3. Uma única chave, "Controlador: NORMAL / DEGELO", representa o **controlador comercial**. À **esquerda (NORMAL)**: ventoinha ligada e aquecedor desligado (GPIO 17 em alto, GPIO 18 em baixo). À **direita (DEGELO)**: aquecedor ligado e ventoinha desligada, o inverso. É exatamente o padrão de um degelo, legítimo ou forçado.
4. Abra o Serial Monitor. A cada 2 s aparece uma linha como
   `[cshield001] t=5.0C h=80% l=0% aq=0 vt=1 vf=1 taxa=+0.00C/min st=NORMAL mt=- ci=0 rede=OK`.
5. **Ponto de partida (NORMAL):** DHT22 em 5 °C, LDR escuro, botão solto, chave do controlador em NORMAL (esquerda): só a ventoinha trabalha.
6. Nos LEDs indicadores: laranja "Aquecedor" mostra se o aquecedor está de fato alimentado; azul "Ventoinha" mostra se a ventoinha está de fato ligada (a do controlador ou a do C-Shield).

Legenda de relés: o relé 1 (NF) corta o aquecedor quando acionado, e o LED laranja apaga. O relé 2 é comutador: em repouso o LED azul segue a chave da ventoinha do controlador; acionado, o C-Shield liga a ventoinha direto e o LED azul acende.

## Tabela de estados e regras

| Estado ou regra | O que mexer no Wokwi | LED RGB | Buzzer | LCD (16x2) | Laranja / Azul | Serial | Resultado |
| --- | --- | --- | --- | --- | --- | --- | --- |
| NORMAL | Ponto de partida acima | Verde fixo | Silêncio | `C-Shield NORMAL` / `5.0C 2a8C` | apagado / aceso | `st=NORMAL mt=- aq=0 vt=1 vf=1` | |
| ATENCAO (porta aberta) | Subir o slider de luz do LDR até `l=` passar de 40 | Amarelo fixo | 1 bipe curto a cada 10 s | `PORTA ABERTA` / temperatura e faixa | apagado / aceso | `st=ATENCAO mt=PORTA_ABERTA` | |
| ATENCAO (deriva) | Com a porta fechada, DHT22 em 8,5 °C (entre 8 e 9) | Amarelo fixo | 1 bipe a cada 10 s | `DERIVA` / `8.5C 2a8C` | apagado / aceso | `st=ATENCAO mt=DERIVA` | |
| ATENCAO (degelo legítimo) | Chave do controlador para DEGELO (direita) e voltar a NORMAL antes de 15 s | Amarelo fixo | 1 bipe a cada 10 s | `DEGELO 0:12` (contagem regressiva) / `5.0C 2a8C` | aceso / apagado | `st=ATENCAO mt=DEGELO aq=1 vt=0` | |
| D1, degelo forçado (BLOQUEIO) | Mesma posição do degelo legítimo, mas **deixar passar de 15 s** | Vermelho piscando | Intermitente (300 ms) | `ATAQUE BLOQUEADO` / alterna `DEGELO FORCADO` e `SEGURE BOTAO 3s` | **apagado** / **aceso** | `>>> BLOQUEIO: DEGELO_FORCADO (aquecedor ligado e ventoinha desligada ha 16s)`, `*** BLOQUEIO ***`, `bq` sobe | |
| D1 fora do horário | Só com `MODO_DEMO 0`: degelo com a hora local fora de 06:00 e 18:00 (30 min cada) | Vermelho piscando | Intermitente | `ATAQUE BLOQUEADO` / `DEGELO FORCADO` | apagado / aceso | `mt=DEGELO_FORCADO` já no primeiro ciclo | |
| D3, subida rápida (BLOQUEIO) | Com a porta fechada, subir o DHT22 de 5 °C para cerca de 7 °C em poucos segundos (taxa acima de 0,5 °C/min) | Vermelho piscando | Intermitente | `ATAQUE BLOQUEADO` / `SUBIDA RAPIDA` | apagado / aceso | `mt=SUBIDA_RAPIDA`, `taxa=+` alta | |
| D3 com porta aberta | Igual ao anterior, mas com o LDR claro | Amarelo fixo | 1 bipe a cada 10 s | `PORTA ABERTA` | apagado / aceso | `st=ATENCAO` (não bloqueia) | |
| Controlador ignorado | Durante o BLOQUEIO, virar a chave do controlador (cada virada conta 2 ordens, porque aquecedor e ventoinha mudam juntos) | Continua piscando | Continua | continua `ATAQUE BLOQUEADO` | laranja **continua apagado** | `ordem do controlador IGNORADA (bloqueio ativo), ci=1` (e `ci` sobe a cada mudança) | |
| Termostato de emergência | Em BLOQUEIO com o slider do DHT22 em 7 °C, só esperar | Vermelho piscando | Intermitente | `RESTAURANDO TEMP` / `BLOQUEIO ATIVO` enquanto passa de 8 °C; depois `ATAQUE BLOQUEADO` | azul liga e desliga sozinho | `t=` cai até 3.8 (azul apaga, `vf=0`), volta a 6.0 (azul acende, `vf=1`), ciclo de cerca de 1 min | |
| Bloqueio sem expiração | Em BLOQUEIO, devolver as chaves ao normal e esperar 5 min sem apertar o botão | Continua piscando | Continua | continua `ATAQUE BLOQUEADO` | apagado / conforme termostato | `st=BLOQUEIO` o tempo todo | |
| CRITICO | DHT22 em 12 °C (acima de 9) por 2 ciclos | Vermelho fixo | Contínuo | `PRODUTO EM RISCO` / `T10.0 EX 0.3` | apagado / aceso | `st=CRITICO`, `ex` subindo | |
| FALHA_SENSOR | Desconectar o fio de dados do DHT22 (apagar a conexão `dht1:SDA` no diagrama) por 2 ciclos | Amarelo piscando | 1 bipe a cada 10 s | `SENSOR SEM LEIT.` / `DHT22 sem dado` | apagado / aceso | `st=FALHA_SENSOR`, `t=-999.0` | |
| FALHA no bloqueio | Com um BLOQUEIO ativo, desconectar o DHT22 | Vermelho piscando | Intermitente | continua `ATAQUE BLOQUEADO` | apagado / **aceso** (ventoinha ligada direto) | `st=BLOQUEIO`, `vf=1` | |
| Liberação recusada | Com um BLOQUEIO ativo e a chave em NORMAL, subir o DHT22 para 12 °C e segurar o botão 3 s logo em seguida (o modelo térmico esfria a temperatura a 6 °C/min, então a recusa só vale enquanto ela passa de 9 °C) | Continua | Continua | `LIBERANDO...` com barra, depois `FORA DA FAIXA` / `AGUARDE` por 3 s | sem mudança | `liberacao RECUSADA: temperatura fora da faixa` | |
| Liberação com temperatura boa | Chave do controlador em NORMAL (esquerda), DHT22 em 5 °C, segurar o botão 3 s | Volta ao verde | Silêncio | `LIBERANDO...`, depois `BLOQUEIO LIBERADO!` por 3 s, depois `C-Shield NORMAL` | apagado / conforme a chave da ventoinha | `liberacao presencial ACEITA`, `st=NORMAL` | |
| Sem rebloqueio | Logo depois de liberar, esperar 1 min | Verde | Silêncio | `C-Shield NORMAL` | apagado / aceso | `st=NORMAL` (a subida antiga não reabre o bloqueio) | |

**D2 (aquecedor e ventoinha ligados juntos por mais de 15 s)** não pode ser provocada com a chave única, porque ela liga um e desliga o outro. A regra continua no firmware e tem teste automático no computador (`teste_aquecedor_longo`).

## Modelo térmico da demonstração

O DHT22 do Wokwi é um slider manual. Para o termostato aparecer sem você mexer no slider, o firmware tem um modelo térmico só para a demo (`simulacao.h`, `MODELO_TERMICO_DEMO 1`): com o C-Shield no controle, a ventoinha ligada esfria a temperatura que ele enxerga até 3,5 °C, e com a ventoinha desligada ela sobe de volta em direção ao valor do slider. Se você mexer no slider, o modelo volta a seguir o valor dele. O slider continua mostrando o mesmo número no Wokwi; o Serial e o LCD mostram a temperatura modelada. Na maquete física, o valor é 0 e a leitura real do sensor passa direto.

## Teste ponta a ponta com o FIWARE (T18)

Objetivo: confirmar que a simulação do Wokwi publica no FIWARE e que `state`, `blocks` e `ignoredCommands` mudam no Orion quando um degelo forçado acontece.

**Estado em 10/11:** a EC2 em `3.215.3.216` não respondeu (`curl` com 8 s de limite devolveu `http=000`). Este teste **não foi executado**. Passo a passo para a equipe:

1. Ligue a EC2 e confirme o IP elástico. Se mudou, troque `BROKER_MQTT` no `config.h` e use `FIWARE_HOST=<ip>` no script.
2. Provisione o device (só uma vez): `bash edge/fiware/provisionar.sh`. Esperado: `ok` para o service group, o device (sem comandos) e as 8 subscriptions.
3. Rode a simulação no Wokwi e espere o Serial mostrar `rede=OK` (o Wokwi só publica com a aba aberta e a simulação ativa).
4. Consulte a entidade:
   ```bash
   curl -s http://<ip>:1026/v2/entities/urn:ngsi-ld:CShield:001 \
     -H 'fiware-service: smart' -H 'fiware-servicepath: /'
   ```
   Esperado: `state` = `NORMAL`, `blocks` = 0, `ignoredCommands` = 0.
5. No Wokwi, chave do controlador para DEGELO (direita); espere mais de 15 s até o LCD mostrar `ATAQUE BLOQUEADO`.
6. Repita o `curl` do passo 4. Esperado: `state` = `BLOQUEIO`, `reason` = `DEGELO_FORCADO`, `blocks` = 1, `heater` = 1, `fan` = 1.
7. Vire a chave duas vezes e repita o `curl`. Esperado: `ignoredCommands` maior que 0.
8. Registre abaixo o comando e as duas respostas.

Resultado (a equipe preenche): ____________________________
