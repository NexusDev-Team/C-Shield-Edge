# C-Shield — Edge

Repositório da camada de Edge do C-Shield (Global Solution 2026.2 "CitizenTech", FIAP · Edge Computing & Computer Systems · Prof. Dr. Fábio H. Cabrini · equipe Nexus Vision).

> Site (Front e Web): repositório [`c-shield-web`]([link]) — em criação.

## Ficha CitizenTech

| Campo | Conteúdo |
| --- | --- |
| **Nome da CitizenTech** | C-Shield (de Cyber Shield) |
| **Equipe** | Nexus Vision — Isac Nilton Fernandes de Oliveira (RM 573282), João Benedito de Oliveira Simplício (RM 570206), Leonardo Formigari (RM 573291), Luis Fernando (RM 570959), Nicolas Cinel (RM 572940) |
| **Cidade e bairro** | São Paulo — [bairro] |
| **Cidadão beneficiado** | Quem depende do que está guardado no frio sem ver o equipamento: o paciente que precisa de transfusão, a criança que vai tomar vacina na UBS e a família que compra no mercado do bairro. Quem usa o C-Shield é o responsável pelo frio sem equipe de segurança: o dono do açougue, a farmacêutica, a enfermeira da sala de vacina. |
| **Problema, em uma frase** | Controladores de câmaras frias conectados à internet podem ser invadidos e mandados esquentar, e quem guarda vacinas, sangue e alimentos só descobre horas depois, quando o produto já está perdido. |
| **Como funciona** | Um ESP32 independente, instalado junto da câmara, mede a temperatura com sensores próprios e compara com uma política gravada nele. Diante de uma ordem perigosa, corta o aquecimento por um relé, trava novas ordens e avisa no local, mesmo sem internet. Ele não recebe comandos pela rede: só envia dados. A liberação é presencial, por um botão no aparelho. |
| **O que cada parte faz** | **Edge:** o C-Shield no Wokwi, publicando no FIWARE · **Python:** central que cadastra câmaras, avalia ordens recebidas e calcula a exposição térmica · **DPS:** modelo do aquecimento (lei de Newton) que mostra quanto tempo o produto aguenta e quanto o C-Shield evita perder · **Front e Web:** site com o emulador do ataque, o painel do dispositivo e o registro de incidentes · **Experiência:** requisitos, caso de uso e protótipo · **Pitch:** a história e o negócio |
| **Links** | Repositório do site (`c-shield-web`): [link] · Repositório do Edge (`c-shield-edge`): [link] · Wokwi: [link] · Site publicado: [link] |
| **Indicador de impacto** | Tempo entre o ataque e a resposta (na Europa, 3h30; com o C-Shield, segundos) · exposição térmica evitada (°C·min) · ordens perigosas bloqueadas |
| **ODS** | 3 Saúde e bem-estar · 9 Indústria, inovação e infraestrutura · 2 Fome zero |

## Como o C-Shield funciona (versão 2)

O C-Shield fica entre o controlador comercial e os componentes da geladeira (o aquecedor de degelo e a ventoinha). Ele mede com o DHT22 e o LDR e observa se o controlador está ligando o aquecedor e a ventoinha. Reconhece o ataque por três sinais:

- **D1.** Aquecedor ligado com a ventoinha desligada fora do degelo permitido (6h e 18h, 30 minutos cada, ou por mais que a duração máxima).
- **D2.** Aquecedor ligado com a ventoinha ligada por tempo demais.
- **D3.** Temperatura subindo rápido (acima de 0,5 °C/min, valor provisório) com a porta fechada.

Ao reconhecer um deles, **assume o controle local**: o relé 1 corta o aquecedor, o controlador passa a ser ignorado (cada ordem ignorada é contada) e o relé 2 faz a ventoinha trabalhar como termostato de emergência, ligando em alvo + 1 °C e desligando em alvo − 1 °C. O bloqueio **não expira**: só termina quando alguém segura o botão do aparelho por 3 segundos, e só se a temperatura estiver dentro da faixa do produto. O C-Shield nunca desliga a refrigeração: só corta o aquecedor e acrescenta ventoinha. Se o ESP32 desligar, os relés ficam em repouso e a geladeira volta a obedecer ao controlador.

O firmware **só envia dados**. Não há `subscribe`, `setCallback`, atualização remota nem servidor.

## Abrir no Wokwi

1. Crie um projeto ESP32 no Wokwi e copie para ele todos os arquivos de `edge/cshield/`: `cshield.ino`, `config.h`, `decisao.h`, `sensores.h`, `atuadores.h`, `painel.h`, `rede.h`, `simulacao.h`, `diagram.json` e `libraries.txt`.
2. Inicie a simulação. No ponto de partida, a chave "Controlador: ventoinha" fica à direita (ligada) e a "Controlador: aquecedor" à esquerda (desligada).
3. Para simular um ataque, ligue o aquecedor e desligue a ventoinha por mais de 15 s (modo demonstração). Para liberar, segure o botão verde por 3 s. A tabela completa de testes está em `docs/edge-testes-estados.md` e o roteiro do vídeo em `docs/edge-roteiro-video.md`.

O Wokwi só publica no FIWARE com a aba aberta e a simulação ativa. O broker MQTT está em `BROKER_MQTT` no `config.h` (placeholder: confirmar o IP elástico da EC2).

## Testes da lógica

A decisão (`decisao.h`) não usa Arduino e roda no computador. Com `g++` (no Windows, o do WinLibs):

```bash
g++ -std=c++17 -Wall -Wextra -I edge/cshield edge/test/test_decisao.cpp -o t && ./t
g++ -std=c++17 -Wall -Wextra -DMODO_DEMO=0 -I edge/cshield edge/test/test_decisao.cpp -o t0 && ./t0
```

A segunda compilação cobre o modo de produção (degelo de 30 min e janelas de horário). Para checar a sintaxe do firmware inteiro sem as bibliotecas do ESP32:

```bash
g++ -std=c++17 -fsyntax-only -Wall -Wextra -x c++ -I edge/test/stubs -I edge/cshield edge/cshield/cshield.ino
```

## FIWARE

Device `cshield001`, entidade `urn:ngsi-ld:CShield:001`, tipo `CShield`, headers `fiware-service: smart` e `fiware-servicepath: /`. Tópico de publicação `/TEF/cshield001/attrs`, payload UltraLight:

```
t|4.8|h|81|l|12|aq|0|vt|1|vf|1|st|NORMAL|mt|-|bq|0|ci|0|ex|0
```

`aq` e `vt` são o aquecedor e a ventoinha que o controlador manda; `vf`, a ventoinha efetiva depois do C-Shield; `bq`, os bloqueios; `ci`, os comandos ignorados; `ex`, a exposição térmica em °C·min. Para cadastrar o device (só atributos, sem comandos) e as subscriptions do STH-Comet: `FIWARE_HOST=<ip> bash edge/fiware/provisionar.sh`.
