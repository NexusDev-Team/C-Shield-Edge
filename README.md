# C-Shield — Edge

Repositório da camada de Edge do C-Shield (Global Solution 2026.2 "CitizenTech", FIAP · Edge Computing & Computer Systems · Prof. Dr. Fábio H. Cabrini · equipe Nexus Vision).

> Site (Front e Web): repositório [`c-shield-web`]([link]) — em criação.

## Ficha CitizenTech

| Campo | Conteúdo |
| --- | --- |
| **Nome da CitizenTech** | C-Shield (de Cyber Shield) |
| **Equipe** | Nexus Vision — Isac Nilton Fernandes de Oliveira (RM 573282), João Benedito de Oliveira Simplício (RM 570206), Leonardo Formigari (RM 573291), Luis Fernando (RM 570959), Nicolas Cinel (RM 572940) |
| **Cidade e bairro** | São Paulo |
| **Cidadão beneficiado** | Quem depende do que está guardado no frio sem ver o equipamento: o paciente que precisa de transfusão, a criança que vai tomar vacina na UBS e a família que compra no mercado do bairro. Quem usa o C-Shield é o responsável pelo frio sem equipe de segurança: o dono do açougue, a farmacêutica, a enfermeira da sala de vacina. |
| **Problema, em uma frase** | Controladores de câmaras frias conectados à internet podem ser invadidos e mandados esquentar, e quem guarda vacinas, sangue e alimentos só descobre horas depois, quando o produto já está perdido. |
| **Como funciona** | Um ESP32 independente, instalado junto da câmara, mede a temperatura com sensores próprios e compara com uma política gravada nele. Diante de uma ordem perigosa, corta o aquecimento por um relé, trava novas ordens e avisa no local, mesmo sem internet. Ele não recebe comandos pela rede: só envia dados. A liberação é presencial, por um botão no aparelho. |
| **O que cada parte faz** | **Edge:** o C-Shield no Wokwi, publicando no FIWARE · **Python:** central que cadastra câmaras, avalia ordens recebidas e calcula a exposição térmica · **DPS:** modelo do aquecimento (lei de Newton) que mostra quanto tempo o produto aguenta e quanto o C-Shield evita perder · **Front e Web:** site com o emulador do ataque, o painel do dispositivo e o registro de incidentes · **Experiência:** requisitos, caso de uso e protótipo · **Pitch:** a história e o negócio |
| **Links** | Repositório do site (`c-shield-web`): [link] · Repositório do Edge (`c-shield-edge`): [link] · Wokwi: [projeto público no Wokwi](https://wokwi.com/projects/480328417992209409) · Site publicado: [link] |
| **Indicador de impacto** | Tempo entre o ataque e a resposta (na Europa, 3h30; com o C-Shield, segundos) · exposição térmica evitada (°C·min) · ordens perigosas bloqueadas |
| **ODS** | 3 Saúde e bem-estar · 9 Indústria, inovação e infraestrutura · 2 Fome zero |

## Como o C-Shield funciona

O C-Shield fica entre o controlador comercial e o aquecedor e a ventoinha da câmara fria. Ele mede com sensores próprios (DHT22 e LDR) e observa o que o controlador manda: o setpoint e o degelo. Uma ordem perigosa, como um setpoint fora da faixa do produto, um degelo longo ou fora do horário, ou a temperatura subindo ou caindo rápido demais, leva ao **bloqueio**:

- **Ataque de calor:** o relé 1 corta o aquecedor e o relé 2 liga a ventoinha de emergência.
- **Ataque de frio:** o aquecedor fica liberado e a ventoinha de emergência fica desligada, porque esfriar mais estragaria o produto.
- **Alertas locais:** o LED RGB fica azul enquanto o bloqueio estiver ativo; o buzzer apita até a temperatura voltar à faixa; o LCD mostra `Estado: ...` e a temperatura.
- **Liberação:** o bloqueio **não acaba sozinho**. Só termina quando alguém segura o botão do aparelho por 3 segundos, depois da trava mínima (60 s na demonstração, 30 min no uso real).

O firmware **só envia dados**: não há `subscribe`, `setCallback`, atualização remota nem servidor. Se o ESP32 desligar, os relés ficam em repouso e a câmara volta a obedecer ao controlador.

## Abrir no Wokwi

**Projeto público:** https://wokwi.com/projects/480328417992209409

1. Abra o link e clique em iniciar a simulação. Deixe a aba aberta: o Wokwi só publica no FIWARE com a simulação ativa.
2. A chave **"Controlador"** simula o que o controlador comercial manda: esquerda = NORMAL (setpoint 5 °C), direita = ATAQUE (setpoint 45 °C).
3. O slider do DHT22 muda a temperatura, o do LDR abre a porta, e o botão verde libera o bloqueio.
4. O código está em `edge/cshield/`. A tabela completa de testes está em `docs/edge-testes-estados.md` e o roteiro do vídeo em `docs/edge-roteiro-video.md`.

## Testes da lógica

A decisão (`decisao.h`) não usa Arduino e roda no computador com `g++`:

```bash
g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_decisao.cpp -o t && ./t
```

## FIWARE

Device `cshield001`, entidade `urn:ngsi-ld:CShield:001`, tipo `CShield`, headers `fiware-service: smart` e `fiware-servicepath: /`. O firmware publica em `/TEF/cshield001/attrs` (UltraLight):

```
t|4.8|h|81|l|12|sp|5.0|dg|0|st|NORMAL|mt|-|bq|0|ex|0
```

Para cadastrar o device (só atributos, sem comandos) e as subscriptions do STH-Comet: `FIWARE_HOST=<ip> bash edge/fiware/provisionar.sh`.
