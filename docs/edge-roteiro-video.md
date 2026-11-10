# C-Shield — Roteiro do vídeo (até 7 min)

Projeto C-Shield · Nexus Vision · GS 2026.2 "CitizenTech". Vídeo no YouTube de até 7 minutos, na ordem da rubrica (problema, circuito, lógica de borda, código, simulação). Tempo total previsto: **6 min 50 s**, com 10 s de folga.

## Antes de gravar (checklist, 10 minutos antes)

- [ ] Ligar a EC2 e confirmar o IP elástico (`curl http://<IP>:1026/version`). Se o IP mudou, atualizar `BROKER_MQTT` no `config.h`.
- [ ] Rodar `FIWARE_HOST=<IP> bash edge/fiware/provisionar.sh` (pode rodar de novo sem duplicar).
- [ ] Abrir o site da GS na rota `/ao-vivo` e confirmar que ele responde.
- [ ] Abrir o Wokwi, iniciar a simulação e conferir no Serial `rede=OK`.
- [ ] **Lembrete: o Wokwi só publica enquanto a aba do navegador estiver aberta e a simulação ativa.** Não minimize nem feche a aba durante a gravação.
- [ ] Deixar o ponto de partida: DHT22 em 5 °C, LDR escuro, chave do **aquecedor à esquerda** (desligada), chave da **ventoinha à direita** (ligada), estado NORMAL.
- [ ] Ter o `MODO_DEMO 1` no `config.h`: o degelo máximo é de 15 s, e a checagem de horário fica desligada, então a hora local não atrapalha.
- [ ] Ter abertas, em abas separadas: o Wokwi, o `decisao.h` (função `avaliar`), o `rede.h`, o terminal com o `curl` do Orion e o site `/ao-vivo`.

## 1. A ideia (0:00 a 1:05 · 65 s)

**Na tela:** slide ou imagem com o problema; depois o diagrama do C-Shield junto de uma câmara fria.

**Fala:**

- "Em outubro e novembro de 2026, invasores entraram pela internet nos controladores de refrigeração de supermercados na Europa e mandaram os equipamentos esquentar. Na França, freezers chegaram a 50 °C. Na ICA de Estocolmo, os evaporadores foram colocados em degelo às 2h, e só perceberam às 5h30."
- "Em um banco de sangue francês, o plasma guardado a cerca de -30 °C teve o controle ajustado para até 110 °C. Só foi salvo porque um alarme tocou e um técnico estava de plantão. E o painel do controlador chegou a mostrar temperatura normal enquanto o refrigerador esquentava."
- "Em São Paulo, os mesmos equipamentos existem em salas de vacina de UBS, hemocentros, farmácias e mercados de bairro. Quem sofre é o paciente que precisa de transfusão, a criança que vai tomar vacina e a família que compra no mercado."
- "O C-Shield é um ESP32 independente, instalado entre o controlador e o aquecedor e a ventoinha da geladeira. Ele mede com sensores próprios e, ao reconhecer um ataque, **assume o controle local**: corta o aquecedor e regula a ventoinha sozinho, mesmo sem internet."
- "Ele conversa com os ODS 3, saúde e bem-estar (vacinas e sangue), 9, indústria, inovação e infraestrutura (infraestrutura crítica protegida), e 2, fome zero (alimento que não se perde)."
- Frase do produto: **"Vê de longe. Só obedece de perto."**

## 2. Cada componente e por que está ali (1:05 a 2:10 · 65 s)

**Na tela:** o `diagram.json` aberto no Wokwi, apontando para cada peça. Dividir a explicação em dois lados: o **lado do controlador** (as duas chaves) e o **lado da geladeira** (relés e LEDs).

| Componente | O que dizer |
| --- | --- |
| ESP32 | "O cérebro. Roda a política de decisão e funciona offline." |
| DHT22 (GPIO 4) | "Mede temperatura e umidade próprias, independentes do controlador, que é justamente o que pode mentir." |
| LDR (GPIO 34) | "Detecta porta aberta pela luz. Porta aberta justifica o calor e muda a resposta." |
| Chave "Controlador: aquecedor" (GPIO 18) | "No simulador, representa o controlador comercial mandando ligar o aquecedor de degelo." |
| Chave "Controlador: ventoinha" (GPIO 17) | "Representa o controlador mandando ligar a ventoinha. Um degelo normal liga o aquecedor e desliga a ventoinha. É por isso que o C-Shield olha o tempo e o horário." |
| Botão (GPIO 19) | "A única forma de liberar um bloqueio, e só presencial: segurar por 3 segundos." |
| Relé 1, contato NF (GPIO 23) | "Fica entre o controlador e o aquecedor. Acionado, abre o circuito e corta o aquecimento. Em repouso fica fechado: se o C-Shield falhar, o equipamento funciona como antes." |
| Relé 2, comutador (GPIO 33) | "Em repouso, a ventoinha segue o controlador. Acionado, o C-Shield liga a ventoinha direto. Ele nunca desliga a refrigeração, só acrescenta." |
| LEDs laranja e azul | "No simulador mostram o efeito real: laranja, se o aquecedor está alimentado; azul, se a ventoinha está ligada." |
| LED RGB (GPIO 25, 26, 27) | "A cor do estado: verde, amarelo, vermelho." |
| Buzzer (GPIO 5) | "Um padrão sonoro diferente por estado." |
| LCD 16x2 I2C (GPIO 21 e 22) | "Mostra o estado e o motivo para quem está na sala." |

## 3. Os limites e o trecho de `avaliar()` (2:10 a 3:15 · 65 s)

**Na tela:** `config.h` (limites com comentário) e `decisao.h`, função `avaliar()`.

**Fala:**

- "A faixa vem do produto. Vacina fica entre +2 e +8 °C, com tolerância de 1 °C antes de virar crítico. Há perfis para congelado, resfriado e plasma."
- "A taxa máxima de aquecimento, 0,5 °C por minuto com a porta fechada, é um valor provisório, que a DPS vai calibrar. O degelo legítimo vale no máximo 30 minutos, nos horários de 6h e 18h. Na demonstração, usamos 15 segundos."
- "Leitura inválida é tratada: NaN ou fora de -40 a 80 °C marca falha de sensor. A temperatura usada é a média móvel de 5 leituras, e CRITICO e falha de sensor só valem depois de 2 ciclos seguidos."
- "São 5 estados, com prioridade: CRITICO, BLOQUEIO, FALHA_SENSOR, ATENCAO, NORMAL."
- Mostrar o trecho de `avaliar()`: "Três sinais de ataque. D1: aquecedor ligado com a ventoinha desligada fora do degelo permitido. D2: aquecedor ligado por tempo demais. D3: temperatura subindo rápido com a porta fechada."
- "Ao bloquear, o relé 1 corta o aquecedor, o controlador passa a ser ignorado, e a ventoinha vira um termostato de emergência com histerese. O bloqueio **não expira**: só o botão libera."
- "Esta função não usa nada do Arduino. Por isso tem testes que rodam no computador, um por requisito."
- Opcional: rodar o comando `g++ ... test_decisao.cpp` e mostrar `0 falhas`.

## 4. Aqui não entra nada (3:15 a 3:40 · 25 s)

**Na tela:** `rede.h`, e o terminal com o `grep`.

**Fala:**

- "No projeto anterior da equipe, a Vinheria recebia o comando `set_limits` pela internet. Aqui invertemos de propósito."
- Rodar `grep -rnE "subscribe|setCallback|ArduinoOTA|WebServer" edge/cshield/` e mostrar a saída **vazia**.
- "O firmware não assina nenhum tópico, não tem callback, não tem atualização remota nem servidor. Só publica. Aqui não entra nada."

## 5. A simulação passando por todos os estados (3:40 a 6:15 · 155 s)

**Na tela:** o Wokwi com o Serial Monitor visível, o LCD, o LED RGB e os dois LEDs indicadores em quadro. As duas chaves fazem o papel do controlador (direita = ligado).

**Uma frase sobre o simulador, dita na primeira vez que a temperatura cair sozinha:** "O sensor do simulador é um slider manual, então um modelo térmico da demonstração faz a ventoinha esfriar a temperatura que o C-Shield enxerga. Na maquete física, ele é desligado."

| Tempo | Ação | O que mostrar e dizer |
| --- | --- | --- |
| 3:40 a 3:50 | Ponto de partida | NORMAL: LED verde, LCD `C-Shield NORMAL`, azul aceso (ventoinha do controlador), laranja apagado. |
| 3:50 a 4:00 | Subir o LDR | ATENCAO por porta aberta: LED amarelo, LCD `PORTA ABERTA`, bipe curto. Fechar a porta. |
| 4:00 a 4:15 | Desconectar o fio de dados do DHT22, esperar, reconectar | FALHA_SENSOR: LED amarelo piscando, LCD `SENSOR SEM LEIT.`, Serial `t=-999.0`. |
| 4:15 a 4:30 | **Degelo legítimo:** chave do aquecedor à direita e da ventoinha à esquerda; soltar antes de 15 s | ATENCAO com contagem: LCD `DEGELO 0:12`, laranja aceso. "Um degelo normal não é ataque." Voltar as chaves ao ponto de partida. |
| 4:30 a 4:55 | **Degelo forçado:** repetir e **deixar passar de 15 s** | O ataque. BLOQUEIO: LCD `ATAQUE BLOQUEADO`, LED vermelho piscando, buzzer intermitente, **laranja apaga** (aquecedor cortado). Serial: `>>> BLOQUEIO: DEGELO_FORCADO (aquecedor ligado e ventoinha desligada ha 16s)`. |
| 4:55 a 5:10 | Mexer nas duas chaves durante o bloqueio | Serial: `ordem do controlador IGNORADA (bloqueio ativo), ci=...`; o laranja continua apagado. "O controlador não manda mais." |
| 5:10 a 5:30 | Deixar o DHT22 em 7 °C e esperar | Termostato de emergência: o azul apaga quando a temperatura chega a 4 °C e acende de novo em 6 °C, sozinho. Dizer a frase sobre o simulador. |
| 5:30 a 5:45 | Subir o DHT22 para 12 °C | CRITICO: LED vermelho fixo, buzzer contínuo, LCD `PRODUTO EM RISCO` com a exposição térmica. Com o bloqueio ativo, a ventoinha esfria a temperatura de volta. |
| 5:45 a 6:15 | Chaves de volta ao normal (aquecedor à esquerda, ventoinha à direita), DHT22 em 5 °C, **segurar o botão por 3 s** | LCD `LIBERANDO...` com a barra enchendo, depois `BLOQUEIO LIBERADO!` por 3 s; volta a NORMAL e o controlador volta a mandar. "Só presencial. Sem tempo de expiração, sem comando remoto." |

Dica de gravação: a **liberação recusada** (botão com a temperatura fora da faixa, LCD `FORA DA FAIXA` / `AGUARDE`) não cabe no tempo; se sobrar folga, mostre-a antes de voltar o DHT22 a 5 °C (use 12 °C e aperte logo depois de subir o slider).

## 6. As 3 camadas funcionando (6:15 a 6:50 · 35 s)

**Na tela:** divisão com o Serial do Wokwi, o terminal com o `curl` no Orion e o site `/ao-vivo`.

**Fala e ações:**

- "Camada 1, o dispositivo: o Serial mostra a publicação em `/TEF/cshield001/attrs`."
- "Camada 2, o FIWARE: a entidade `urn:ngsi-ld:CShield:001` no Orion." Rodar:
  ```bash
  curl -s "http://<IP>:1026/v2/entities/urn:ngsi-ld:CShield:001?options=keyValues" \
    -H "fiware-service: smart" -H "fiware-servicepath: /"
  ```
  Mostrar `state`, `reason`, `blocks` e `ignoredCommands` refletindo o bloqueio.
- "Camada 3, o dashboard: o site da GS na rota `/ao-vivo` registrando o bloqueio."
- Fechar: "O C-Shield vê de longe, mas só obedece de perto."

## 7. Encerramento (6:50 a 7:00 · 10 s)

- Mostrar a tela final com o nome do projeto, da equipe Nexus Vision e dos cinco integrantes, e os links do Wokwi e do repositório.

## Conferência final antes de enviar

- [ ] Duração do vídeo: no máximo 7:00.
- [ ] Todos os 5 estados apareceram, inclusive a falha de sensor.
- [ ] O degelo legítimo (ATENCAO com contagem) apareceu antes do degelo forçado.
- [ ] O corte do aquecimento (laranja apagando) e o termostato (azul acendendo e apagando) ficaram visíveis.
- [ ] O Serial mostrou `ci` subindo durante o bloqueio.
- [ ] A frase sobre o modelo térmico da demonstração foi dita.
- [ ] O `grep` sem `subscribe` apareceu com saída vazia.
- [ ] O site `/ao-vivo` mostrou o bloqueio.
- [ ] O vídeo está público ou não listado, com o link do Wokwi na descrição.
