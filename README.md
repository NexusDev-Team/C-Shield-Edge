<a id="readme-top"></a>

<!-- Cores: azul-noite #0B1F3A · azul elétrico #2563EB · ciano #00B8D9 · ardósia #1E293B · verde ok #34D399 -->

<div align="center">

  <img src="docs/img/logo.svg" alt="Logo C-Shield" width="96" height="96">

  <h1>🛡️ C-Shield ⚡</h1>
  <h3>🔐 Um escudo cibernético para a câmara fria</h3>

  <p>
    Um ESP32 entre o controlador e a geladeira, com sensores próprios e uma política gravada nele.<br>
    Se alguém invade o controlador e manda esquentar (ou esfriar), o C-Shield percebe, assume o controle e avisa no local.<br>
    <strong>Vê de longe. Só obedece de perto.</strong>
  </p>

  <p>
    <a href="https://wokwi.com/projects/480328417992209409"><img src="https://img.shields.io/badge/%F0%9F%9B%A1%EF%B8%8F_SIMULA%C3%87%C3%83O_NO_WOKWI-2563EB?style=for-the-badge&logo=arduino&logoColor=white" alt="Simulação no Wokwi"></a>
    <a href="https://github.com/NexusDev-Team/C-Shield-Edge"><img src="https://img.shields.io/badge/%F0%9F%92%BB_REPOSIT%C3%93RIO_EDGE-0B1F3A?style=for-the-badge&logo=github&logoColor=white" alt="Repositório Edge"></a>
    <a href="docs/edge-testes-estados.md"><img src="https://img.shields.io/badge/%F0%9F%A7%AA_TESTES_DOS_ESTADOS-00B8D9?style=for-the-badge&logo=readthedocs&logoColor=0B1F3A" alt="Testes dos estados"></a>
    <!-- TODO: trocar o href "#" pelo link do vídeo no YouTube quando for publicado -->
    <a href="#"><img src="https://img.shields.io/badge/%E2%96%B6%EF%B8%8F_V%C3%8DDEO_NO_YOUTUBE-34D399?style=for-the-badge&logo=youtube&logoColor=0B1F3A" alt="Vídeo no YouTube"></a>
  </p>

  <p>
    <a href="https://github.com/NexusDev-Team/C-Shield-Edge/issues">Reportar problema</a>
  </p>

  <p>
    <img src="https://img.shields.io/badge/FIAP-Global_Solution_2026.2-2563EB?style=flat-square" alt="FIAP Global Solution 2026.2">
    <img src="https://img.shields.io/badge/CitizenTech-Edge_Computing-0B1F3A?style=flat-square" alt="CitizenTech Edge Computing">
  </p>

</div>

<br>

<!-- SUMÁRIO -->
<details>
  <summary><strong>Sumário</strong></summary>
  <ol>
    <li><a href="#ficha-citizentech">Ficha CitizenTech</a></li>
    <li>
      <a href="#sobre-o-projeto">Sobre o projeto</a>
      <ul>
        <li><a href="#o-problema">O problema</a></li>
        <li><a href="#a-solução">A solução</a></li>
        <li><a href="#funcionalidades">Funcionalidades</a></li>
        <li><a href="#feito-com">Feito com</a></li>
      </ul>
    </li>
    <li><a href="#arquitetura">Arquitetura</a></li>
    <li>
      <a href="#detalhes-técnicos">Detalhes técnicos</a>
      <ul>
        <li><a href="#hardware">Hardware</a></li>
        <li><a href="#estados-e-alertas">Estados e alertas</a></li>
        <li><a href="#regras-de-bloqueio">Regras de bloqueio</a></li>
        <li><a href="#nomes-no-fiware">Nomes no FIWARE</a></li>
      </ul>
    </li>
    <li><a href="#como-rodar">Como rodar</a></li>
    <li><a href="#dashboard-central">Dashboard (central)</a></li>
    <li><a href="#estrutura-do-repositório">Estrutura do repositório</a></li>
    <li><a href="#documentos">Documentos</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#equipe">Equipe</a></li>
  </ol>
</details>

---

<!-- FICHA -->
## Ficha CitizenTech

| Campo | Conteúdo |
| --- | --- |
| **Nome da CitizenTech** | C-Shield (de Cyber Shield) |
| **Equipe** | Nexus Vision: Isac Nilton Fernandes de Oliveira (RM 573282), João Benedito de Oliveira Simplício (RM 570206), Leonardo Formigari (RM 573291), Luis Fernando (RM 570959), Nicolas Cinel (RM 572940) |
| **Cidade** | São Paulo |
| **Cidadão beneficiado** | Quem depende do que está guardado no frio sem ver o equipamento: o paciente que precisa de transfusão, a criança que vai tomar vacina na UBS e a família que compra no mercado do bairro. Quem usa o C-Shield é o responsável pelo frio sem equipe de segurança: o dono do açougue, a farmacêutica, a enfermeira da sala de vacina. |
| **Problema, em uma frase** | Controladores de câmaras frias conectados à internet podem ser invadidos e mandados esquentar, e quem guarda vacinas, sangue e alimentos só descobre horas depois, quando o produto já está perdido. |
| **Como funciona** | Um ESP32 independente, instalado junto da câmara, mede a temperatura com sensores próprios e compara com uma política gravada nele. Diante de uma ordem perigosa, corta o aquecimento por um relé, trava novas ordens e avisa no local, mesmo sem internet. Ele não recebe comandos pela rede: só envia dados. A liberação é presencial: só um cartão NFC cadastrado, encostado no aparelho, encerra o bloqueio, e cada liberação fica registrada com o papel de quem liberou. |
| **O que cada parte faz** | **Edge:** o C-Shield no Wokwi, publicando no FIWARE · **Python:** central que cadastra câmaras, avalia ordens recebidas e calcula a exposição térmica · **DPS:** modelo do aquecimento (lei de Newton) que mostra quanto tempo o produto aguenta e quanto o C-Shield evita perder · **Front e Web:** site com o emulador do ataque, o painel do dispositivo e o registro de incidentes · **Experiência:** requisitos, caso de uso e protótipo · **Pitch:** a história e o negócio |
| **Links** | Repositório do site (`c-shield-web`): em criação · Repositório do Edge: [NexusDev-Team/C-Shield-Edge](https://github.com/NexusDev-Team/C-Shield-Edge) · Wokwi: [projeto público](https://wokwi.com/projects/480328417992209409) · Site publicado: em criação |
| **Indicador de impacto** | Tempo entre o ataque e a resposta (na Europa, 3h30; com o C-Shield, segundos) · exposição térmica evitada (°C·min) · ordens perigosas bloqueadas |
| **ODS** | 3 Saúde e bem-estar · 9 Indústria, inovação e infraestrutura · 2 Fome zero |

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- SOBRE -->
## Sobre o projeto

Em outubro e novembro de 2026, invasores entraram pela internet nos controladores de refrigeração de supermercados e de um banco de sangue na Europa e mandaram os equipamentos esquentar. O **C-Shield** (de *Cyber Shield*) é um dispositivo de borda que protege a câmara fria desse tipo de ataque, mesmo sem internet.

Projeto da **Global Solution 2026.2 "CitizenTech"** de Edge Computing & Computer Systems, FIAP, Prof. Dr. Fábio H. Cabrini. Entrega em 11/11/2026.

| Recurso | Link |
| --- | --- |
| 🛡️ Simulação no Wokwi | [wokwi.com/projects/480328417992209409](https://wokwi.com/projects/480328417992209409) |
| 💻 Repositório do Edge | [github.com/NexusDev-Team/C-Shield-Edge](https://github.com/NexusDev-Team/C-Shield-Edge) |
| 🧪 Tabela de testes dos estados | [`docs/edge-testes-estados.md`](docs/edge-testes-estados.md) |
| 🎬 Roteiro do vídeo | [`docs/edge-roteiro-video.md`](docs/edge-roteiro-video.md) |
| 🔌 Contrato com o site | [`docs/edge-contrato-web.md`](docs/edge-contrato-web.md) |

### O problema

- **França, 11/10:** centenas de supermercados tiveram freezers chegando a 50 °C.
- **Europa, 01 e 02/11:** em mais de 100 lojas os evaporadores foram colocados em degelo. Na ICA de Estocolmo o ataque foi às 2h e só foi percebido às 5h30.
- **Banco de sangue francês, 02/11:** o plasma, guardado a cerca de -30 °C, teve o controle ajustado para até 110 °C. Só se salvou porque um alarme tocou e um técnico estava de plantão.
- **O painel mente:** em testes, o painel do controlador mostrava temperatura normal enquanto o refrigerador esquentava. Um sistema invadido não avisa que foi invadido.

Em São Paulo, os mesmos equipamentos existem em salas de vacina de UBS, hemocentros, farmácias, mercados de bairro e açougues.

### A solução

- 🌡️ **Mede por conta própria:** DHT22 (temperatura e umidade) e LDR (porta aberta), independentes do controlador.
- 🧠 **Decide no próprio dispositivo:** a política está gravada no firmware e nunca é alterada pela rede. A lógica é pura e testada no computador.
- 🛡️ **Assume o controle:** diante de uma ordem perigosa, o relé 1 corta o aquecedor e o relé 2 liga a ventoinha de emergência. Num ataque de frio, o aquecedor fica liberado e a ventoinha de emergência fica desligada.
- 🚨 **Avisa no local:** LED RGB azul no bloqueio, buzzer que apita até a temperatura voltar à faixa e LCD 16x2 com o estado e a temperatura.
- 📡 **Só envia dados:** publica no FIWARE por MQTT. Não há `subscribe`, callback, atualização remota nem servidor: "aqui não entra nada".
- 🔑 **Libera só de perto:** o bloqueio não acaba sozinho. Alguém precisa encostar no aparelho um cartão NFC cadastrado, depois da trava mínima. O papel do cartão (por exemplo, `RT01`, responsável técnico) vai para o log do aparelho e para o histórico no FIWARE; cartão desconhecido é recusado e a tentativa também fica registrada.

### Funcionalidades

- 🔌 Cinco estados com prioridade: `CRITICO` > `BLOQUEIO` > `FALHA_SENSOR` > `ATENCAO` > `NORMAL`.
- 🛡️ Quatro gatilhos de bloqueio: setpoint fora da faixa, degelo longo ou fora do horário, aquecimento rápido e resfriamento rápido.
- ⚡ Resposta no relé por direção: ataque de calor ou de frio.
- 🔐 Ordens do controlador ignoradas durante o bloqueio, com trava mínima antes da liberação.
- 🤖 Tratamento de leitura inválida (NaN e fora de -40 a 80 °C) e suavização por média móvel de 5 leituras.
- 📊 Exposição térmica acumulada em °C·min.
- 🧱 Falha segura: com o ESP32 desligado, os relés ficam em repouso e a geladeira volta a obedecer ao controlador.
- 🌐 Funciona offline: sem Wi-Fi, a detecção, o bloqueio e os alertas continuam iguais.
- 🧪 Lógica de decisão testada no computador, com 135 verificações.

### Feito com

**Firmware e hardware**

[![ESP32](https://img.shields.io/badge/ESP32-2563EB?style=for-the-badge&logo=espressif&logoColor=white)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/C%2B%2B_Arduino-0B1F3A?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![MQTT](https://img.shields.io/badge/MQTT-00B8D9?style=for-the-badge&logo=mqtt&logoColor=0B1F3A)](https://mqtt.org/)
[![Wokwi](https://img.shields.io/badge/Wokwi-34D399?style=for-the-badge&logoColor=0B1F3A)](https://wokwi.com/)

**Middleware**

[![FIWARE](https://img.shields.io/badge/FIWARE-2563EB?style=for-the-badge&logoColor=white)](https://www.fiware.org/)
[![Docker](https://img.shields.io/badge/Docker-0B1F3A?style=for-the-badge&logo=docker&logoColor=white)](https://www.docker.com/)
[![AWS EC2](https://img.shields.io/badge/AWS_EC2-00B8D9?style=for-the-badge&logo=amazonec2&logoColor=0B1F3A)](https://aws.amazon.com/ec2/)

Bibliotecas do firmware: `PubSubClient`, `DHT sensor library`, `Adafruit Unified Sensor` e `LiquidCrystal I2C`. A lógica de decisão (`decisao.h`) não usa Arduino e é testada com `g++`.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- ARQUITETURA -->
## Arquitetura

O Edge cobre as três primeiras camadas; o site (outro repositório) fecha a quarta:

```
[1] Dispositivo (Edge)   ESP32 + DHT22 + LDR + 2 relés + LED RGB + buzzer + LCD 16x2 I2C
[2] Comunicação          Wi-Fi -> Mosquitto MQTT (1883), payload UltraLight 2.0, SÓ publicação
[3] Middleware IoT       IoT Agent MQTT (4041) -> Orion (1026) + STH-Comet (8666)
[4] Apresentação         Site c-shield-web: /simulador-real (dados reais) e /emulador (sem ESP32)
```

**Fluxo de telemetria:** sensor → ESP32 → MQTT → IoT Agent → Orion → site. **Não existe fluxo de comando:** nada chega ao ESP32 pela rede.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- DETALHES TÉCNICOS -->
## Detalhes técnicos

### Hardware

| Componente | Pino do ESP32 | Função |
| --- | --- | --- |
| DHT22 | GPIO 4 | Temperatura e umidade próprias |
| LDR | GPIO 34 (ADC) | Luminosidade; acima de 40% significa porta aberta |
| Chave "Controlador" (só Wokwi) | GPIO 32 | Simula o setpoint que o controlador manda: NORMAL (5 °C) ou ATAQUE (45 °C) |
| Leitor NFC MFRC522 | SCK 14 · MISO 19 · MOSI 13 · SDA (SS) 15 · RST 17 | Liberação presencial por cartão cadastrado. Usa um SPI próprio, porque o SPI padrão colidiria com o relé 1 e o buzzer |
| Relé 1, contato NF | GPIO 23 | Em série com o aquecedor; acionado, corta |
| Relé 2, contato NA | GPIO 33 | Liga a ventoinha de emergência |
| LED RGB | R 25 · G 26 · B 27 | Cor do estado |
| Buzzer | GPIO 5 | Alerta sonoro |
| LCD 16x2 I2C | SDA 21 · SCL 22, endereço `0x27` | Estado, temperatura e motivo |
| LED verde da ventoinha de refrigeração (só Wokwi) | GPIO 16 (`RX2`) | Simula a ventoinha do controlador |

### Estados e alertas

| Estado | LED RGB | Buzzer | LCD (linha 1) |
| --- | --- | --- | --- |
| `NORMAL` | Verde | Silêncio | `Estado: Ideal` |
| `ATENCAO` (porta aberta ou deriva) | Amarelo | 1 bipe a cada 10 s | `Estado: Atencao` |
| `BLOQUEIO` | **Azul fixo** | Apita enquanto a temperatura estiver fora da faixa e para ao regularizar; o bloqueio continua | `Estado: Bloqueio` |
| `CRITICO` | Vermelho (azul se há bloqueio) | Contínuo até voltar à faixa | `Estado: Critico` |
| `FALHA_SENSOR` | Amarelo piscando | 2 bipes a cada 10 s | `Estado: Falha` |

A linha 2 do LCD alterna a cada 2 s entre `Temp: 5.0°C` e as mensagens do estado (motivo, `Liberacao em 58s`, `Passe o cartao`...). Ao ler um cartão, o LCD mostra por 3 s `LIBERADO POR` com o papel, ou `Cartao recusado` com o motivo. O LED e o buzzer rodam por `millis()`, sem `delay()`.

### Regras de bloqueio

O C-Shield julga a **ordem** antes do **efeito**:

1. **Setpoint** fora da faixa do produto (2 a 8 °C na vacina).
2. **Degelo** comandado por mais de 30 min, ou fora dos horários de 6h e 18h.
3. **Aquecimento rápido** (acima de 0,5 °C/min, valor provisório) com a porta fechada.
4. **Resfriamento rápido** (acima de 0,5 °C/min), com a porta aberta ou fechada.

Resposta nos relés: no **calor**, corta o aquecedor e liga a ventoinha de emergência; no **frio**, deixa o aquecedor liberado e mantém a ventoinha desligada. Durante o bloqueio as ordens do controlador são ignoradas. A liberação exige um cartão NFC cadastrado depois da trava mínima (60 s na demonstração, 30 min no uso real). Se a ordem do controlador ainda for inválida ou a temperatura ainda estiver além da tolerância, o bloqueio recomeça e a tentativa conta como recusada.

### Cartões de liberação

Cada cartão identifica um **papel**, não uma pessoa. A tabela fica em `OPERADORES` (`edge/cshield/config.h`) e usa os UIDs dos cartões prontos da peça MFRC522 do Wokwi. No aparelho físico, troque pelos UIDs dos cartões reais.

| Cartão no Wokwi | UID | Código | Papel no LCD |
| --- | --- | --- | --- |
| Azul | `01:02:03:04` | `RT01` | Resp. tecnico |
| Verde | `11:22:33:44` | `ENF01` | Enfermagem |
| Amarelo | `55:66:77:88` | `MAN01` | Manutencao |
| NFC Tag | `04:11:22:33:44:55:66` | `SUP01` | Supervisao |
| Key Fob (chaveiro) | `C0:FF:EE:99` | `GER01` | Gerencia |
| Vermelho | `AA:BB:CC:DD` | não cadastrado | Recusado (`NAO_CADASTRADO`) |

Motivos de recusa registrados: `TRAVA` (a trava mínima ainda não acabou), `ORDEM` (o controlador ainda manda uma ordem inválida), `TEMPERATURA` (a temperatura segue além da tolerância) e `NAO_CADASTRADO` (cartão fora da tabela). Cartão cadastrado sem bloqueio ativo só mostra `Sem bloqueio` e não gera evento.

Cada liberação ou recusa aparece no Serial com data e hora (`bloqueio LIBERADO por RT01 (Resp. tecnico)`) e é publicada **uma vez**, no momento do evento, para formar o histórico no FIWARE. Ler o cartão não é receber um comando: o leitor é entrada local, e o firmware continua sem `subscribe`, callback, atualização remota ou servidor.

### Nomes no FIWARE

| Item | Valor |
| --- | --- |
| Device | `cshield001` |
| Entidade | `urn:ngsi-ld:CShield:001`, type `CShield` |
| Headers | `fiware-service: smart` · `fiware-servicepath: /` |
| API key | `TEF` |
| Tópico | `/TEF/cshield001/attrs` (único; nenhum de comando) |
| Payload UltraLight | `t\|4.8\|h\|81\|l\|12\|sp\|5.0\|dg\|0\|st\|NORMAL\|mt\|-\|bq\|0\|ex\|0` |
| Atributos no Orion | `temperature` · `humidity` · `luminosity` · `setpoint` · `defrost` · `state` · `reason` · `blocks` · `exposure` |
| Eventos de cartão (só no momento do evento) | Liberação: `lb\|RT01\|nl\|3` → `releasedBy` · `releaseCount`. Recusa: `rj\|RT01\|rm\|TRAVA\|nr\|2` → `refusedBy` · `refusedReason` · `refusalCount` |

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- COMO RODAR -->
## Como rodar

**1. Simulação no Wokwi**

Abra o [projeto público](https://wokwi.com/projects/480328417992209409) e inicie a simulação. Deixe a aba aberta: o Wokwi só publica no FIWARE com a simulação ativa.

- Chave **"Controlador"** para a direita simula o ataque (setpoint de 45 °C).
- O slider do **DHT22** muda a temperatura; o do **LDR** abre a porta.
- Clique no leitor **MFRC522** e use **Tap** (ou as teclas `b`, `g`, `y`, `n`, `k`, `r`) para encostar um cartão. Depois da trava mínima, um cartão cadastrado **libera** o bloqueio; o cartão vermelho (`r`) é recusado.

**2. Testes da lógica (sem placa)**

```bash
g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_decisao.cpp -o t && ./t
g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_operadores.cpp -o o && ./o
```

**3. FIWARE**

Com a EC2 ligada, cadastre o device (só atributos, sem comandos) e as subscriptions do STH-Comet:

```bash
FIWARE_HOST=<ip> bash edge/fiware/provisionar.sh
```

Para conferir a entidade:

```bash
curl -s "http://<ip>:1026/v2/entities/urn:ngsi-ld:CShield:001?options=keyValues" \
  -H "fiware-service: smart" -H "fiware-servicepath: /"
```

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- DASHBOARD -->
## Dashboard (central)

A terceira camada: uma página que **lê** o FIWARE e mostra a frota. Fica em `dashboard/` e tem a estética do site do projeto. Nada ali manda ordens ao C-Shield; o próprio cadastro só provisiona o device.

| Tela | O que mostra |
| --- | --- |
| **Mapa da frota** (`/`) | Os cinco pontos da cidade, com a temperatura e o estado reais de cada C-Shield cadastrado, a Central (FIWARE) e uma linha de dados só no sentido do aparelho para a central |
| **Monitoramento** (`/dispositivo/<id>`) | Temperatura atual, histórico térmico (STH-Comet), umidade, porta, bloqueios, exposição e o **histórico de liberações**: quem encostou o cartão, quando e o que aconteceu |
| **C-Shields** (`/dispositivos`) | Cadastro e remoção. Cadastrar provisiona o device, a entidade e as subscriptions no FIWARE |
| **Avançado** (`/avancado`) | IP da EC2 e saúde do Orion, do IoT Agent e do STH-Comet |

Como a Vinheria, o navegador nunca fala com o FIWARE: ele chama `/api/...` num backend FastAPI (`dashboard/backend`, copiado da Vinheria e adaptado), e só o backend fala com a EC2.

**Rodar no notebook**

```bash
# backend (porta 8000)
cd dashboard/backend
python -m venv .venv && .venv/Scripts/python -m pip install -r requirements.txt
cp .env.example .env          # preencha FIWARE_HOST com o IP da EC2
.venv/Scripts/python -m uvicorn app.main:app --port 8000

# frontend (porta 5173; o Vite encaminha /api para o backend)
cd dashboard/frontend
npm install
npm run dev
```

Na tela **C-Shields**, cadastre `cshield001` num ponto do mapa. Com o Wokwi rodando, o mapa e o monitoramento passam a mostrar os dados ao vivo, e cada cartão encostado aparece no histórico de liberações.

**Testes**

```bash
cd dashboard/backend && .venv/Scripts/python -m pytest
cd dashboard/frontend && npx vitest run
```

**Publicar (Vercel + túnel)**

O backend roda no notebook e a Cloudflare o expõe, como na Vinheria. O Vercel serve só o site estático e reescreve `/api` para o túnel.

1. Com o backend no ar: `cloudflared tunnel --url http://localhost:8000` e copie o endereço `https://....trycloudflare.com`.
2. Em `dashboard/frontend/vercel.json`, troque `SUBSTITUA-PELO-TUNEL.trycloudflare.com` por esse endereço.
3. No Vercel, crie um projeto novo com **Root Directory** `dashboard/frontend` e faça o deploy.

O endereço do túnel muda a cada reinício do `cloudflared`; nesse caso repita os passos 2 e 3. Pelo túnel o backend **só lê**: cadastro, remoção e troca do IP da EC2 só funcionam no notebook (`localhost`), então o site público não deixa ninguém de fora alterar a frota. A API não tem autenticação.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- ESTRUTURA -->
## Estrutura do repositório

```
C-Shield-Edge/
├── README.md
├── docs/
│   ├── edge-testes-estados.md    tabela de testes dos estados e teste ponta a ponta
│   ├── edge-roteiro-video.md     roteiro do vídeo (até 7 min)
│   ├── edge-contrato-web.md      contrato com o site c-shield-web
│   └── img/                      logo
└── edge/
    ├── cshield/                  firmware (.ino, .h), diagram.json e libraries.txt do Wokwi
    ├── test/                     testes da lógica de decisão e dos cartões
    └── fiware/                   provisionar.sh (device sem comandos + subscriptions) e collection do Postman

dashboard/
├── backend/                      API FastAPI de leitura (cliente FIWARE copiado da Vinheria)
└── frontend/                     React + Vite, com a estética do site
```

---

<!-- DOCUMENTOS -->
## Documentos

| Documento | Conteúdo |
| --- | --- |
| [`docs/edge-testes-estados.md`](docs/edge-testes-estados.md) | O que mexer no Wokwi e o que deve aparecer em cada estado; teste ponta a ponta com o FIWARE (executado em 11/11) |
| [`docs/edge-roteiro-video.md`](docs/edge-roteiro-video.md) | Roteiro do vídeo, na ordem da rubrica, com os tempos |
| [`docs/edge-contrato-web.md`](docs/edge-contrato-web.md) | O que o Edge publica, como ler o Orion e as regras que o emulador do site deve reproduzir |

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- ROADMAP -->
## Roadmap

- [x] Lógica de decisão pura, com testes no computador
- [x] Firmware: sensores, relés, LED RGB, buzzer, LCD e publicação MQTT
- [x] Ataque de calor e de frio, com resposta nos relés por direção
- [x] Circuito no Wokwi e simulação pública
- [x] Provisionamento no FIWARE e teste ponta a ponta
- [ ] Vídeo de apresentação
- [ ] Site `c-shield-web`: `/simulador-real` e `/emulador`
- [ ] Versão em JavaScript da lógica de decisão, para o emulador do site
- [ ] Terceiro relé, para o C-Shield aquecer sozinho num ataque de frio
- [ ] Calibração das taxas de aquecimento e de resfriamento com a DPS

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

---

<!-- EQUIPE -->
## Equipe

**Nexus Vision**, FIAP, Global Solution 2026.2 "CitizenTech" de Edge Computing & Computer Systems, Prof. Dr. Fábio H. Cabrini.

| Nome | RM |
| --- | --- |
| Isac Nilton Fernandes de Oliveira | RM573282 |
| João Benedito de Oliveira Simplício | RM570206 |
| Leonardo Formigari | RM573291 |
| Luis Fernando | RM570959 |
| Nicolas Cinel | RM572940 |

Repositório: [github.com/NexusDev-Team/C-Shield-Edge](https://github.com/NexusDev-Team/C-Shield-Edge)

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>
