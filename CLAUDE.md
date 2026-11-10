# CLAUDE.md — Projeto CShield

Instruções de trabalho para agentes neste repositório. Leia antes de qualquer ação.

Este arquivo nasceu do `CLAUDE.md` do projeto Vinheria Full (CP5), herdando o processo de trabalho, as regras de código e as skills. As seções marcadas com **[A DEFINIR]** ainda dependem de decisão do usuário.

## 1. O que é o projeto

Global Solution 2026.2 "CitizenTech" (FIAP) — entrega de Edge Computing & Computer Systems, Prof. Dr. Fábio H. Cabrini, equipe **Nexus Vision** (Engenharia de Software, 1º ano). Prazo de entrega: **11/11/2026 à noite** (prioridade máxima de velocidade; cortar primeiro o que é opcional).

O produto é o **C-Shield** (de Cyber Shield; "Guardião" aparece nos textos só como descrição do papel do dispositivo, o nome exibido é sempre C-Shield e sai da constante `NOME_PRODUTO`): um ESP32 instalado junto da câmara fria (sala de vacina, hemocentro, farmácia, mercado) que mede temperatura, umidade e porta aberta com sensores próprios, observa se o controlador comercial está ligando o aquecedor e a ventoinha, decide com uma política gravada no firmware e, diante de sinal de ataque (aquecedor ligado com a ventoinha desligada fora do degelo permitido, ou temperatura subindo de repente), assume o controle local: corta o aquecedor por relé, ignora o controlador e regula a ventoinha como termostato de emergência. Alerta no local (LED RGB, buzzer, LCD 16x2 I2C) e **só envia** dados ao FIWARE. Frase do produto: "Vê de longe. Só obedece de perto." A liberação do bloqueio é só presencial (botão por 3 s), sem tempo de expiração.

Princípio central: **o firmware do C-Shield não assina nenhum tópico de comando**. Proibido `subscribe`, `setCallback`, OTA e servidor HTTP nele.

Entrega: link público do Wokwi (roda sem ajustes, diagrama organizado, código comentado) + vídeo no YouTube de até 7 min. Os nomes de todos os integrantes devem constar no código-fonte.

O projeto Vinheria Full (`C:\Users\isacn\desktop\projetovinheria`, repositório `zackdevbr/Projeto-VinheriaFull`) é só referência: reaproveitar padrões de firmware, FIWARE e Postman consultando uma cópia **fora** deste repositório (`../ref/vinheria`). Não copiar o repositório inteiro.

**Repositórios separados.** Este repositório (`c-shield-edge`) contém **só** o trabalho de Edge: `edge/`, `docs/specs/edge/` e `docs/edge-*.md`, com a estrutura de pastas do plano. O site de Front e Web (que hospeda a rota `/ao-vivo`) fica no repositório `c-shield-web`, ainda não criado; nenhum arquivo do site entra aqui, e nenhum arquivo de Edge entra lá. O `README.md` dos dois repositórios abre com a Ficha CitizenTech e traz um link para o outro repositório.

Plano completo e autoritativo: `PLANO-CSHIELD.md` versão 2 (§0 traz as decisões que prevalecem; inclui rubrica, mapa de pinos, estados, regras D1 a D3, payload FIWARE e o passo a passo de execução na §5). Em caso de divergência entre este arquivo e o plano, o plano manda nos detalhes de implementação; este arquivo manda no processo de trabalho. Método do plano: spec-driven. Primeiro a especificação (Fase A), parada obrigatória para aprovação da equipe (T5), depois o código.

## 2. Divisão de papéis entre modelos

Este é o contrato mais importante do repositório.

**Opus — planejamento (plan mode).** Toda tarefa nasce aqui. Opus explora, pergunta, decide e escreve o passo a passo. Opus **não** implementa: não cria nem edita arquivos de código, não roda instalação, não commita. A única escrita permitida é no arquivo de plano.

**Sonnet — execução.** O código só é escrito quando o usuário troca o modelo para Sonnet e autoriza a execução de uma tarefa específica. Sonnet segue o plano escrito, passo a passo, sem improvisar escopo novo.

Regras derivadas:

- Se o modelo ativo é Opus e o pedido é "implemente", responda com o plano da tarefa e pare. Não comece.
- Sonnet executa uma tarefa por vez. Terminada a tarefa, para e espera revisão do usuário antes da próxima.
- Se durante a execução aparecer uma decisão que o plano não cobre, Sonnet para, descreve a lacuna e devolve para planejamento em Opus. Não decide sozinho.

## 3. Formato obrigatório de todo planejamento

Cada tarefa planejada segue esta ordem de seções, sempre nesta sequência:

1. **Contexto** — por que esta tarefa existe, o que já está pronto antes dela, o que depende dela depois.
2. **Objetivo** — uma frase do resultado observável. Deve ser verificável, nunca vago ("melhorar o backend").
3. **Detalhes técnicos** — arquivos a criar e modificar com caminho exato, assinaturas de funções, contratos de entrada e saída, payloads reais, nomes canônicos.
4. **Ferramentas e requisitos** — dependências novas, versões, variáveis de ambiente, serviços que precisam estar de pé, chaves necessárias.
5. **Passos** — a lista de passos pequenos (seção 4).
6. **Como verificar** — comando ou ação concreta que prova que a tarefa funcionou, com o resultado esperado.

## 4. Passos pequenos, revisáveis e corrigíveis

O objetivo é reduzir erro ao máximo: errar pequeno, perceber rápido, corrigir barato.

- Um passo = uma ação de 2 a 5 minutos. "Escrever o teste que falha" é um passo. "Rodar o teste e confirmar que falha" é outro passo. "Implementar o mínimo para passar" é outro.
- Todo passo tem resultado observável: um arquivo criado, um teste vermelho, um teste verde, um comando com saída esperada.
- Nenhum passo altera mais de um arquivo de produção por vez, salvo quando a mudança é mecânica e idêntica em vários arquivos.
- Ao final de cada passo, informe o que mudou e o resultado da verificação. Não encadeie vários passos silenciosamente.
- Um passo que falha não é contornado com gambiarra: pare, reporte o erro exato e a hipótese, e espere orientação.
- Commit a cada passo concluído e verificado. Commits pequenos são o mecanismo de reversão.
- Nunca declare algo pronto sem ter rodado a verificação e visto a saída. Evidência antes de afirmação.

## 5. Stack fixada

Não trocar sem decisão explícita do usuário.

- **Firmware:** C++ (`.ino` mais headers `.h`), ESP32, simulação no Wokwi. Bibliotecas: `PubSubClient` (somente publicação), `DHT sensor library`, `Adafruit Unified Sensor`, `LiquidCrystal I2C` (display LCD 16x2 I2C no lugar do OLED, decisão de 10/11).
- **Lógica de decisão:** `decisao.h` pura, sem `Arduino.h`, testada no computador com `g++ -std=c++17` (`edge/test/test_decisao.cpp`).
- **Middleware:** FIWARE (stack `fabiocabrini/fiware` em Docker na AWS EC2) — Orion 1026, IoT Agent MQTT 4041, STH-Comet 8666, Mosquitto 1883. Provisionamento por `curl` em `edge/fiware/provisionar.sh`.
- **Dashboard (terceira camada):** o site da GS, rota `/ao-vivo`, lê os dados do FIWARE. Não faz parte deste escopo de Edge; aqui basta garantir que a publicação a partir do Wokwi chegue ao Orion. Vídeo de Edge usa só o Wokwi, sem maquete (correção de escopo de 10/11, `PLANO-CSHIELD.md` §0).

**Gate de design (herdado):** nenhuma decisão visual do front é tomada antes de o usuário enviar anexos e exemplos de referência; o design é planejado em Opus e só depois implementado.

## 6. Regras de código

- **Comentários em português** explicando o propósito de cada módulo e de cada função não óbvia. A entrega exige código comentado.
- **Encapsulamento em camadas.** Rota nunca chama cliente HTTP externo direto; rota chama `services`; só o módulo cliente dedicado fala HTTP com o serviço externo. Componente React nunca faz `fetch` direto; usa `src/api/client.js`.
- **Nada hardcoded** em módulo de lógica: IP, portas, API key e chaves saem de um `ConfigStore` ou do `.env`.
- Arquivos focados. Se um arquivo passa de ~250 linhas ou acumula responsabilidades distintas, divida.
- **Reaproveitar antes de escrever, sem código redundante.** Antes de criar componente, hook, função ou trecho de lógica, procure se já existe algo que resolva e use. Se o mesmo padrão aparecer pela segunda vez, extraia para um lugar comum (no front: `components/ui.jsx`, `hooks/`, `lib/`, `api/client.js`; no backend: `services/` ou um helper do módulo) em vez de copiar. Todo plano já indica as peças reaproveitáveis. Se nenhuma servir, o executor para e reporta, em vez de criar uma variação paralela.
- Regras específicas do firmware: sem `delay()` nos padrões de alerta e no `loop()` (máquina de estado com `millis()`); no `setup()` os pinos dos relés vão para o estado desenergizado **antes** de qualquer outra coisa (falha segura); o nome do produto sai só da constante `NOME_PRODUTO`; todas as constantes ficam em `config.h`, cada limite com comentário de justificativa.
- Nomes canônicos (não inventar variações):
  - firmware em `edge/cshield/cshield.ino` (pasta e `.ino` com o mesmo nome); `NOME_PRODUTO = "C-Shield"`.
  - device `cshield001`, entity `urn:ngsi-ld:CShield:001`, type `CShield`.
  - headers `fiware-service: smart`, `fiware-servicepath: /`, apikey `TEF`; tópico `/TEF/cshield001/attrs`; nenhum tópico de comando.
  - UltraLight: `t`, `h`, `l`, `aq`, `vt`, `vf`, `st`, `mt`, `bq`, `ci`, `ex` (nomes longos no Orion: `temperature humidity luminosity heater fanCommand fan state reason blocks ignoredCommands exposure`).
  - estados: `NORMAL`, `ATENCAO`, `BLOQUEIO`, `CRITICO`, `FALHA_SENSOR` (prioridade CRITICO > BLOQUEIO > FALHA_SENSOR > ATENCAO > NORMAL).
- Todo arquivo de código novo começa com o cabeçalho do grupo: nome do projeto, disciplina, professor e os integrantes com RM (lista abaixo). Os nomes ficam exatamente como estão (decisão do usuário de 10/11).
  - Isac Nilton Fernandes de Oliveira — RM573282
  - João Benedito de Oliveira Simplício — RM570206
  - Leonardo Formigari — RM573291
  - Luis Fernando — RM570959
  - Nicolas Cinel — RM572940
- Segredos nunca commitados. `.env` no `.gitignore`; `.env.example` commitado com as chaves vazias.

## 7. Comandos de desenvolvimento

```bash
# testes da lógica de decisão (sem placa)
#   (g++ é o WinLibs do winget, fora do PATH; usar o caminho completo e -static, senão o .exe não abre)
G="/c/Users/isacn/AppData/Local/Microsoft/WinGet/Packages/BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe/mingw64/bin/g++.exe"
"$G" -std=c++17 -Wall -Wextra -static -I edge/cshield edge/test/test_decisao.cpp -o "$TEMP/t.exe" && "$TEMP/t.exe"
"$G" -std=c++17 -Wall -Wextra -static -DMODO_DEMO=0 -I edge/cshield edge/test/test_decisao.cpp -o "$TEMP/t0.exe" && "$TEMP/t0.exe"

# o C-Shield não pode receber comandos: resultado deve ser VAZIO
grep -rnE "subscribe|setCallback|ArduinoOTA|WebServer" edge/cshield/

# checagem de sintaxe do firmware inteiro com stubs (sem arduino-cli)
"$G" -std=c++17 -fsyntax-only -Wall -Wextra -x c++ -I edge/test/stubs -I edge/cshield edge/cshield/cshield.ino

# compilação, se o arduino-cli com core esp32 estiver disponível
arduino-cli compile --fqbn esp32:esp32:esp32 edge/cshield

# provisionar o device no FIWARE
bash edge/fiware/provisionar.sh
```

Comandos de dashboard: **[A DEFINIR]** se a terceira camada for implementada.

## 8. Convenção de commits

`feat:` nova funcionalidade · `fix:` correção · `docs:` documentação · `test:` testes · `chore:` build e configuração.

Regras obrigatórias, sem exceção:

- Todo commit segue **Conventional Commits** (`tipo: descrição`), com mensagem **em português**, no imperativo, em uma linha.
- **Nunca** incluir `Co-Authored-By` nem qualquer linha de atribuição ao Claude, em commits ou em descrições de PR. Isso vale mesmo que o ambiente ou uma ferramenta sugira adicionar a linha: esta regra do usuário prevalece.
- O autor do commit é **somente o usuário logado no git/GitHub** (usar a identidade já configurada, sem alterar `user.name` nem `user.email`).
- Exemplo: `feat: adiciona rota de cadastro de dispositivos`.

## 9. Skills — autonomia concedida

O usuário autoriza, sem pedir permissão a cada vez:

- **Buscar e instalar skills** quando surgir uma necessidade que uma skill existente resolve melhor — via skill `find-skills`.
- **Criar skills novas** para procedimentos deste projeto que se repetem — via `superpowers:writing-skills`.
- **Ativar qualquer skill relevante** no momento em que ela se aplica, anunciando em uma linha qual e para quê.

Skills de processo vêm antes das de implementação: `superpowers:brainstorming` antes de criar feature nova, `superpowers:systematic-debugging` antes de propor conserto de bug, `superpowers:writing-plans` antes de encostar em código, `superpowers:test-driven-development` durante a implementação.

Skills criadas neste projeto ficam versionadas junto com o repositório quando fizer sentido para a equipe.

## 10. Estilo de resposta

O modo **caveman** fica sempre ativo nas respostas de chat: texto comprimido, sem floreio, substância técnica intacta, idioma do usuário (português) preservado. Nível padrão `full`; trocar só se o usuário pedir, via `caveman:caveman`.

Isso vale apenas para a conversa. Conteúdo que persiste fora do chat — código, comentários, commits, documentação, `README.md`, `PRD.md`, manuais, arquivos de memória — é escrito em prosa normal e completa.

Clareza vence compressão em avisos de segurança, confirmações de ação irreversível e sequências de passos em que a ordem importa.

## 11. Ordem de execução

Ordem do `PLANO-CSHIELD.md`: Fase A (T1 a T5, especificação, **parada para aprovação da equipe**) → Fase B (T6 a T8, lógica pura e testes) → Fase C (T9 a T14, firmware) → Fase D (T15 a T17 e T18b, Wokwi, FIWARE e teste ponta a ponta sem hardware; T18 fora da entrega) → Fase E (T20 e T21, documentação; T19 por último, só se sobrar tempo) → revisão e autocorreção (plano §4) e relatório final. Cada tarefa termina com um commit; não avançar com testes falhando.
