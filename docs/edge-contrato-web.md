# Contrato entre o Edge e o site (c-shield-web)

Este documento é para quem faz o site. Ele diz o que o Edge publica, como ler e o que o emulador precisa reproduzir. A fonte da verdade da lógica é `edge/cshield/decisao.h`, com exemplos em `edge/test/test_decisao.cpp`.

## Páginas do site

| Rota | O que faz |
| --- | --- |
| `/simulador-real` | Mostra os dados **de verdade**, lidos do FIWARE, enquanto o Wokwi (ou o ESP32) está publicando. Antes se chamava `/ao-vivo`. |
| `/emulador` | Roda a lógica do C-Shield **no navegador**, sem ESP32 e sem Wokwi, para qualquer pessoa testar. Tem um botão **"Abrir simulador real"** que leva a `/simulador-real`. |

## 1. O que o Edge publica no Orion

- Orion: `http://<IP da EC2>:1026` (IP elástico, hoje `3.215.3.216`; confirmar antes de usar).
- Entidade: `urn:ngsi-ld:CShield:001`, tipo `CShield`.
- Cabeçalhos obrigatórios: `fiware-service: smart` e `fiware-servicepath: /`.
- Consulta: `GET /v2/entities/urn:ngsi-ld:CShield:001?options=keyValues`

Exemplo de resposta (Wokwi rodando, chave em ATAQUE):

```json
{"id":"urn:ngsi-ld:CShield:001","type":"CShield","TimeInstant":"2026-11-11T03:28:14.697Z",
 "blocks":1,"defrost":0,"exposure":0,"humidity":80,"luminosity":28,
 "reason":"SETPOINT_INVALIDO","setpoint":45,"state":"BLOQUEIO","temperature":4.9}
```

| Atributo | Tipo | Significado |
| --- | --- | --- |
| `state` | texto | `NORMAL`, `ATENCAO`, `BLOQUEIO`, `CRITICO` ou `FALHA_SENSOR` |
| `reason` | texto | Motivo: `-`, `PORTA_ABERTA`, `DERIVA`, `SETPOINT_INVALIDO`, `DEGELO_FORA_HORARIO`, `DEGELO_LONGO`, `AQUECIMENTO_RAPIDO`, `RESFRIAMENTO_RAPIDO`, `TEMP_FORA_TOLERANCIA`, `SENSOR_SEM_LEITURA` |
| `temperature` | número (°C) | Temperatura da câmara; `-999` quando o sensor está sem leitura |
| `humidity` | número (%) | Umidade; `-999` quando o sensor está sem leitura |
| `luminosity` | inteiro (%) | Luz no LDR; acima de 40 significa porta aberta |
| `setpoint` | número (°C) | Setpoint que o controlador está mandando; `-999` se ausente |
| `defrost` | 0 ou 1 | Degelo comandado pelo controlador |
| `blocks` | inteiro | Quantos bloqueios aconteceram desde que o dispositivo ligou (não zera ao liberar) |
| `exposure` | número (°C·min) | Exposição térmica acumulada acima do máximo da faixa |
| `TimeInstant` | data ISO | Quando o dado chegou. Se tiver mais de uns 15 s, o dispositivo está desligado: mostre "offline" |

Atualização: o dispositivo publica a cada 5 s e **na hora** em toda mudança de estado. Consulte o Orion a cada 2 ou 3 s.

## 2. Como o site lê o Orion

- O Orion já aceita chamadas de qualquer site (**CORS liberado**, com os cabeçalhos `fiware-service` e `fiware-servicepath`).
- **Problema do HTTPS:** se o site estiver na Vercel (https), o navegador **bloqueia** chamar `http://...:1026` direto (conteúdo misto). Faça uma rota de servidor no próprio site que consulta o Orion e devolve o JSON. Exemplo para a Vercel (`api/cshield.js`):

  ```js
  export default async function handler(req, res) {
    const r = await fetch(`${process.env.FIWARE_URL}/v2/entities/urn:ngsi-ld:CShield:001?options=keyValues`, {
      headers: { 'fiware-service': 'smart', 'fiware-servicepath': '/' },
    });
    res.status(r.status).json(await r.json());
  }
  ```

  O endereço do Orion fica na variável de ambiente `FIWARE_URL`, nunca fixo no código.
- Histórico (gráficos): o STH-Comet, porta 8666, guarda `temperature`, `humidity`, `luminosity`, `state`, `blocks` e `exposure`.
- O Wokwi só publica enquanto a aba do navegador estiver aberta e a simulação ativa.

## 3. O que o emulador precisa reproduzir

Entradas: temperatura, umidade, luz (porta), setpoint, degelo, hora e o botão de liberação (segurar 3 s). Saídas: estado, motivo, relés, LED, buzzer e as telas do LCD.

**Estados, do mais alto para o mais baixo:** `CRITICO` > `BLOQUEIO` > `FALHA_SENSOR` > `ATENCAO` > `NORMAL`. O bloqueio é uma condição persistente, separada do estado mostrado.

**Faixa (perfil vacina):** mínimo 2 °C, máximo 8 °C, tolerância 1 °C. `CRITICO` quando a temperatura fica abaixo de 1 °C ou acima de 9 °C por 2 ciclos seguidos. Ciclo de decisão: 2 s. A temperatura usada é a média das últimas 5 leituras.

**Regras que abrem o bloqueio** (a primeira violada vale):
1. Setpoint fora de 2 a 8 °C (`SETPOINT_INVALIDO`).
2. Degelo por mais de 30 min (`DEGELO_LONGO`) ou fora dos horários 06:00 a 07:00 e 18:00 a 19:00 (`DEGELO_FORA_HORARIO`).
3. Aquecimento acima de 0,5 °C/min, medido em 60 s, com a porta fechada (`AQUECIMENTO_RAPIDO`). Com a porta aberta é só `ATENCAO`.
4. Resfriamento acima de 0,5 °C/min com a temperatura abaixo de 5 °C (`RESFRIAMENTO_RAPIDO`).

**Durante o bloqueio:** novas ordens do controlador são ignoradas; o bloqueio **não acaba sozinho**. Só a liberação presencial o encerra, e só depois da trava mínima (60 s na demonstração, 30 min no uso real). Liberar com a ordem do controlador ainda inválida reabre o bloqueio.

**Resposta nos relés:**
- Ataque de **calor** (ou temperatura acima de 8 °C): corta o aquecedor e liga a ventoinha de emergência.
- Ataque de **frio** (ou temperatura abaixo de 2 °C): aquecedor liberado e ventoinha de emergência desligada.
- Dentro da faixa vale a direção do ataque. Setpoint abaixo de 2 °C e `RESFRIAMENTO_RAPIDO` são ataques de frio.

**Alertas:**
- LED: verde (NORMAL), amarelo (ATENCAO), amarelo piscando (FALHA_SENSOR), vermelho (CRITICO) e **azul fixo** enquanto houver bloqueio.
- Buzzer: 1 bipe a cada 10 s em ATENCAO e FALHA_SENSOR. No bloqueio e no crítico toca enquanto a temperatura estiver fora da faixa (e nos 5 s iniciais de um bloqueio) e **para quando ela volta à faixa**, com o bloqueio continuando.
- LCD (16 colunas): linha 1 `Estado: Ideal | Atencao | Bloqueio | Critico | Falha`; linha 2 alterna a cada 2 s entre `Temp: 5.0°C` e as mensagens do estado. Os textos completos estão em `edge/cshield/painel.h`.

**Como validar o emulador:** os casos de `edge/test/test_decisao.cpp` descrevem entradas e saídas esperadas. Vale reproduzir os principais (setpoint inválido, aquecimento rápido, resfriamento rápido, crítico, liberação e alarme) e comparar com o resultado do código C++.

## 4. Fluxo entre as páginas

```
/emulador  ──[botão "Abrir simulador real"]──▶  /simulador-real  ◀── Orion ◀── MQTT ◀── Wokwi / ESP32
```

O emulador não depende de nada externo. O simulador real depende da EC2 ligada e do Wokwi (ou ESP32) publicando.
