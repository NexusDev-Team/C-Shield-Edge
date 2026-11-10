# C-Shield — Roteiro do vídeo (até 7 min)

Projeto C-Shield · Nexus Vision · GS 2026.2 "CitizenTech". Vídeo no YouTube de até 7 minutos, na ordem da rubrica (problema, circuito, lógica de borda, código, simulação). Tempo total previsto: **6 min 50 s**, com 10 s de folga.

## Antes de gravar (checklist, 10 minutos antes)

- [ ] Ligar a EC2 e confirmar o IP elástico (`curl http://<IP>:1026/version`). Se o IP mudou, atualizar `BROKER_MQTT` no `config.h`.
- [ ] Rodar `FIWARE_HOST=<IP> bash edge/fiware/provisionar.sh` (pode rodar de novo sem duplicar).
- [ ] Abrir o site da GS na rota `/ao-vivo` e confirmar que ele responde.
- [ ] Abrir o Wokwi, iniciar a simulação e conferir no Serial `rede=OK`.
- [ ] **Lembrete: o Wokwi só publica enquanto a aba do navegador estiver aberta e a simulação ativa.** Não minimize nem feche a aba durante a gravação.
- [ ] Deixar o ponto de partida: DHT22 em 5 °C, LDR escuro, `sp=` entre 2 e 8 no Serial, estado NORMAL.
- [ ] Conferir o relógio local: se for entre 6h e 7h ou entre 18h e 19h, o degelo não bloqueia na hora (ver `docs/edge-testes-estados.md`).
- [ ] Ter abertas, em abas separadas: o Wokwi, o `decisao.h` (função `avaliar`), o `rede.h`, o terminal com o `curl` do Orion e o site `/ao-vivo`.

## 1. A ideia (0:00 a 1:15 · 75 s)

**Na tela:** slide ou imagem com o problema; depois o diagrama do C-Shield junto de uma câmara fria.

**Fala:**

- "Em outubro e novembro de 2026, invasores entraram pela internet nos controladores de refrigeração de supermercados na Europa e mandaram os equipamentos esquentar. Na França, freezers chegaram a 50 °C. Na ICA de Estocolmo, o ataque foi às 2h e só foi percebido às 5h30."
- "Em um banco de sangue francês, o plasma guardado a cerca de -30 °C teve o controle ajustado para até 110 °C. Só foi salvo porque um alarme tocou e um técnico estava de plantão."
- "Os atacantes usaram acesso remoto, senhas padrão e firmware desatualizado. E o painel do controlador chegou a mostrar temperatura normal enquanto o refrigerador esquentava."
- "Em São Paulo, os mesmos equipamentos existem em salas de vacina de UBS, hemocentros, farmácias e mercados de bairro. Quem sofre é o paciente que precisa de transfusão, a criança que vai tomar vacina e a família que compra no mercado."
- "O C-Shield é um ESP32 independente, instalado junto da câmara fria, com sensores próprios e uma política gravada nele. Se uma ordem perigosa chega ao controlador, ele corta o aquecimento e avisa no local, mesmo sem internet. Ele resolve a parte do problema que está na ponta: a decisão não fica na nuvem."
- "Ele conversa com os ODS 3, saúde e bem-estar (vacinas e sangue), 9, indústria, inovação e infraestrutura (infraestrutura crítica protegida), e 2, fome zero (alimento que não se perde)."
- Frase do produto: **"Vê de longe. Só obedece de perto."**

## 2. Cada componente e por que está ali (1:15 a 2:30 · 75 s)

**Na tela:** o `diagram.json` aberto no Wokwi, apontando para cada peça.

| Componente | O que dizer |
| --- | --- |
| ESP32 | "O cérebro. Roda a política de decisão e funciona offline." |
| DHT22 (GPIO 4) | "Mede temperatura e umidade próprias, independentes do controlador, que é justamente o que pode mentir." |
| LDR (GPIO 34) | "Detecta porta aberta pela luz. Porta aberta justifica o calor e muda a resposta." |
| Chave "Controlador" NORMAL/ATAQUE (GPIO 32) | "No simulador, representa o que o controlador comercial está mandando. Em ATAQUE, ele manda um setpoint perigoso. É o nosso 'ataque'." |
| Botão (GPIO 19) | "A única forma de liberar um bloqueio, e só presencial: segurar por 3 segundos." |
| Relé 1, contato NF (GPIO 23) | "Fica entre o controlador e o aquecedor. Energizado, abre o circuito e corta o aquecimento. Desenergizado, fica fechado: se o C-Shield falhar, o equipamento funciona como antes." |
| Relé 2, contato NA (GPIO 33) | "Liga a ventoinha de emergência no bloqueio." |
| LEDs laranja e azul | "No simulador mostram o aquecedor cortado e a ventoinha de emergência ligada." |
| LED verde (ventoinha de refrigeração) | "É a ventoinha normal do controlador. No ataque ela apaga, como se o atacante desligasse o frio, e a de emergência assume." |
| LED RGB (GPIO 25, 26, 27) | "A cor do estado: verde, amarelo, vermelho." |
| Buzzer (GPIO 5) | "Um padrão sonoro diferente por estado." |
| LCD 16x2 I2C (GPIO 21 e 22) | "Mostra o estado e o motivo para quem está na sala." |

## 3. Os limites e o trecho de `avaliar()` (2:30 a 3:45 · 75 s)

**Na tela:** `config.h` (limites com comentário) e `decisao.h`, função `avaliar()`.

**Fala:**

- "A faixa vem do produto. Vacina fica entre +2 e +8 °C, com tolerância de 1 °C antes de virar crítico. Há perfis para congelado, resfriado e plasma."
- "A taxa máxima de aquecimento, 0,5 °C por minuto com a porta fechada, é um valor provisório, que a DPS vai calibrar. Degelo só nos horários de 6h e 18h e por no máximo 30 minutos."
- "Leitura inválida é tratada: NaN ou fora de -40 a 80 °C marca falha de sensor. A temperatura usada é a média móvel de 5 leituras, e CRITICO e falha de sensor só valem depois de 2 ciclos seguidos."
- "São 5 estados, com prioridade: CRITICO, BLOQUEIO, FALHA_SENSOR, ATENCAO, NORMAL."
- Mostrar o trecho de `avaliar()`: "A regra do bloqueio julga a ordem antes do efeito. Se o setpoint comandado sai da faixa do produto, bloqueia na hora, sem esperar a temperatura subir. O mesmo vale para degelo longo ou fora do horário, e para aquecimento rápido com a porta fechada."
- "Esta função não usa nada do Arduino. Por isso tem testes que rodam no computador, um por requisito."
- Opcional: rodar o comando `g++ ... test_decisao.cpp` e mostrar `0 falhas`.

## 4. Aqui não entra nada (3:45 a 4:15 · 30 s)

**Na tela:** `rede.h`, e o terminal com o `grep`.

**Fala:**

- "No projeto anterior da equipe, a Vinheria recebia o comando `set_limits` pela internet. Aqui invertemos de propósito."
- Rodar `grep -rnE "subscribe|setCallback|ArduinoOTA|WebServer" edge/cshield/` e mostrar a saída **vazia**.
- "O firmware não assina nenhum tópico, não tem callback, não tem atualização remota nem servidor. Só publica. Aqui não entra nada."

## 5. A simulação passando por todos os estados (4:15 a 6:15 · 120 s)

**Na tela:** o Wokwi com o Serial Monitor visível, o LCD, o LED RGB e os dois LEDs indicadores em quadro. O "ataque" é **mover a chave "Controlador" para ATAQUE**; o LED verde (refrigeração) apaga e os LEDs laranja e azul mostram o aquecedor cortado e a ventoinha de emergência ligada.

| Tempo | Ação | O que mostrar e dizer |
| --- | --- | --- |
| 4:15 a 4:30 | Ponto de partida | NORMAL: LED verde, LCD `C-Shield NORMAL`, LED laranja (aquecedor) aceso, verde (refrigeração) aceso, azul (emergência) apagado. |
| 4:30 a 4:45 | Subir o LDR | ATENCAO por porta aberta: LED amarelo, LCD `PORTA ABERTA`, bipe curto. Fechar a porta. |
| 4:45 a 5:15 | **Mover a chave "Controlador" para ATAQUE** | O ataque. BLOQUEIO imediato: LCD `ATAQUE BLOQUEADO`, LED vermelho piscando, buzzer intermitente, **LED verde apaga** (o atacante desliga a refrigeração), **LED laranja apaga** (aquecedor cortado), **LED azul acende** (ventoinha de emergência). Serial mostra `BLOQUEIO (SETPOINT_INVALIDO)` e o valor recebido. "Sem esperar a temperatura subir." |
| 5:15 a 5:30 | Mexer na chave "Controlador" de novo durante a trava | Serial: `ordem do controlador IGNORADA (trava ativa)`. |
| 5:30 a 5:45 | Subir o DHT22 além de 9 °C | CRITICO: LED vermelho fixo, buzzer contínuo, LCD `PRODUTO EM RISCO` com a exposição térmica. Voltar o DHT22 a 5 °C. |
| 5:45 a 6:00 | Desconectar o DHT22 | FALHA_SENSOR: LED amarelo piscando, LCD `FALHA NO SENSOR`, bipe duplo. Reconectar. |
| 6:00 a 6:15 | Trazer o setpoint de volta, esperar `trava=0s`, **segurar o botão por 3 s** | LCD `LIBERANDO...` com a barra enchendo; volta a NORMAL. "Só presencial." |

Dica de gravação: se a trava de 60 s atrapalhar o ritmo, faça a parte do CRITICO e da falha de sensor enquanto ela corre e libere no final.

## 6. As 3 camadas funcionando (6:15 a 6:50 · 35 s)

**Na tela:** divisão com o Serial do Wokwi, o terminal com o `curl` no Orion e o site `/ao-vivo`.

**Fala e ações:**

- "Camada 1, o dispositivo: o Serial mostra a publicação em `/TEF/cshield001/attrs`."
- "Camada 2, o FIWARE: a entidade `urn:ngsi-ld:CShield:001` no Orion." Rodar:
  ```bash
  curl -s "http://<IP>:1026/v2/entities/urn:ngsi-ld:CShield:001?options=keyValues" \
    -H "fiware-service: smart" -H "fiware-servicepath: /"
  ```
  Mostrar `state`, `reason` e `blocks` refletindo o bloqueio.
- "Camada 3, o dashboard: o site da GS na rota `/ao-vivo` registrando o bloqueio."
- Fechar: "O C-Shield vê de longe, mas só obedece de perto."

## 7. Encerramento (6:50 a 7:00 · 10 s)

- Mostrar a tela final com o nome do projeto, da equipe Nexus Vision e dos cinco integrantes, e os links do Wokwi e do repositório.

## Conferência final antes de enviar

- [ ] Duração do vídeo: no máximo 7:00.
- [ ] Todos os 5 estados apareceram, inclusive a falha de sensor.
- [ ] O corte do aquecimento (LED laranja apagando) e a troca de ventoinhas (verde apagando, azul acendendo) ficaram visíveis.
- [ ] O `grep` sem `subscribe` apareceu com saída vazia.
- [ ] O site `/ao-vivo` mostrou o bloqueio.
- [ ] O vídeo está público ou não listado, com o link do Wokwi na descrição.
