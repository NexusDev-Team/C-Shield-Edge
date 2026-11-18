# Atualização do site: liberação por cartão NFC

Este documento lista o que o site do projeto (repositório `WebFront-GS`, publicado em https://c-shield-eight.vercel.app) precisa mudar depois que o Edge trocou o **botão de liberação por um cartão NFC**. Foi levantado lendo o código-fonte do site; os arquivos citados são caminhos dentro desse repositório.

## O que mudou no produto

| Antes | Agora |
| --- | --- |
| Segurar um botão físico por 3 s libera o bloqueio | Encostar no aparelho um **cartão NFC cadastrado** libera o bloqueio, depois da trava mínima |
| Ninguém sabia quem liberou | Cada cartão é um **papel** (responsável técnico, enfermagem, manutenção, supervisão, gerência). A liberação fica no Serial e no histórico do FIWARE |
| Tentativa errada era só um aviso no LCD | Toda tentativa recusada também é registrada, com o motivo: trava ainda ativa, ordem do controlador ainda inválida, temperatura ainda alta ou cartão não cadastrado |
| LCD: `Liberando...` com barra, `Segure botao 3s` | LCD: `LIBERADO POR` + papel, `Cartao recusado` + motivo, `Passe o cartao` |

A promessa do produto não muda, e fica até mais forte: "Vê de longe. Só obedece de perto." Ninguém libera pela internet, nem pelo dashboard.

## 1. Textos a trocar

### Página Como funciona (`/como-funciona`)

**`src/content/comoFunciona.js`, passo 04 "Alertar e bloquear"**

- Antes: `...O bloqueio só termina presencialmente: no botão físico do aparelho.`
- Depois: `...O bloqueio só termina presencialmente: com um cartão NFC cadastrado, encostado no aparelho. Cada liberação fica registrada com o papel de quem liberou.`

**`src/content/comoFunciona.js`, lista `etapas` do bloco "Uma direção só" (última linha)**

- Antes: `Só o botão físico do aparelho libera o bloqueio.`
- Depois: `Só um cartão NFC cadastrado, encostado no aparelho, libera o bloqueio.`

**`src/components/comoFunciona/DiagramaArquitetura.jsx`, linhas 153 e 154 (caixa do diagrama)**

- Antes: título `Botão no aparelho`, detalhe `físico, só ele libera`
- Depois: título `Leitor de cartão NFC`, detalhe `presencial, só ele libera`
- Se o texto novo não couber na caixa de 180 de largura, aumente a largura ou use o título `Cartão NFC`.

**`src/components/comoFunciona/DiagramaArquitetura.jsx`, linha 224 (legenda abaixo do diagrama)**

- Antes: `Não escuta a rede. Só o botão físico do aparelho libera o bloqueio.`
- Depois: `Não escuta a rede. Só um cartão NFC cadastrado, encostado no aparelho, libera o bloqueio.`
- Atualize também o comentário da linha 153 (`só o botão físico libera`).

### Home (`/`)

**`src/content/inicio.js`, passo 04 "Alertar e bloquear"**

- Antes: `...Para liberar, é preciso estar presente e usar o botão físico do aparelho.`
- Depois: `...Para liberar, é preciso estar presente e encostar no aparelho um cartão NFC cadastrado.`

### Outras páginas

- `src/content/linhaDoTempo.js` (linha 56) e `src/pages/Problema.jsx` (linha 62) dizem "só quem está presente consegue liberar". Continuam corretos, **não mudam**.
- `README.md` do site, tabela "Como funciona": trocar `A liberação é presencial, por um botão no aparelho.` por `A liberação é presencial: só um cartão NFC cadastrado, encostado no aparelho, encerra o bloqueio, e cada liberação fica registrada com o papel de quem liberou.`
- `docs/PLANEJAMENTO.md` do site (linhas 26, 84, 118, 136, 189): trocar "botão segurado por 3 s" por "cartão NFC cadastrado". É documento interno, mas fica desatualizado.

### Imagem e texto alternativo

- `src/components/emulador/ImagemDispositivo.jsx` (linha 21), texto alternativo: trocar `...LED verde de estado e botão de liberação` por `...LED verde de estado e leitor de cartão NFC para a liberação`.
- `src/assets/dispositivo.png` mostra um botão físico de liberação. Precisa de uma ilustração nova com o leitor de cartão (uma área marcada "Aproxime o cartão" na caixa). É a única mudança que exige arte.

## 2. Emulador (`/emulador`): o botão de segurar vira cartão

O emulador reproduz o aparelho, então o gesto de segurar 3 s precisa virar o gesto do cartão. Os textos que o visitante vê mudam junto.

**Interação** (`src/components/ui/BotaoSegurar.jsx`, `src/hooks/useSegurar.js`, `src/components/camara/PainelCamara.jsx`)

- Trocar o botão `Segure 3 s` / `Continue segurando...` por dois botões de clique: **`Encostar cartão cadastrado`** e **`Encostar cartão desconhecido`**. O segundo mostra a recusa por cartão não cadastrado.
- O aria-label `Segure por 3 segundos para liberar o C-Shield` vira `Encostar um cartão cadastrado para liberar o C-Shield`. O `progressbar` da liberação deixa de existir.
- O gesto de segurar existia para evitar toque acidental; com o cartão isso não é mais necessário. `useSegurar.js` e a constante `LIBERACAO_MS` (`src/domain/config.js`, linha 75) podem ser removidos, junto com os testes deles.
- A regra de decisão (`src/domain/cshield.js`) **não muda**: a trava mínima, a recusa e a liberação continuam iguais. Só o gatilho passa a ser o clique no cartão.

**Telas do LCD** (`src/domain/lcd.js`), para ficarem iguais às do firmware

| Hoje | Depois |
| --- | --- |
| `Segure botao 3s` (linha 79, quando a trava acabou) | `Passe o cartao` |
| `telaLiberando`: `Liberando...` + barra de 16 blocos | remover |
| `telaTrava`: `Trava ativa` / `Aguarde 45s` | `Cartao recusado` / `Faltam 45s` |
| `telaLiberado`: `Estado: Liberado` / `Controle normal` | `LIBERADO POR` / nome do papel (por exemplo `Resp. tecnico`) |
| (não existe) | nova tela para cartão desconhecido: `Cartao recusado` / `Nao cadastrado` |
| `telaSemBloqueio`: `Sem bloqueio` / `Nada a liberar` | igual |

Os testes de `lcd` e de `camara-vista` (`src/domain/camara-vista.js`) que citam botão ou barra de progresso precisam ser ajustados.

**Comentários de código** que ainda falam em botão segurado: `src/domain/cidade.js` (linhas 113 e 220), `src/domain/cidade-protecao.js` (linha 57), `src/domain/config.js` (linhas 70 e 74), `src/components/emulador/AcoesDefesa.jsx` (linha 15) e `src/components/camara/PainelCamara.jsx` (linha 14).

## 3. Texto novo: o dashboard

O site antigo previa uma rota `/simulador-real` lendo o FIWARE; hoje ela só redireciona para a seção do Wokwi. O dashboard real agora existe como site próprio (neste repositório, em `dashboard/`). Sugestões:

- No Footer ou na seção "Dispositivo real" do emulador, um link `Ver a central ao vivo` para o endereço do dashboard, assim que ele for publicado no Vercel. Ele deve ficar em `src/content/links.js`, como os outros links.
- Frase de apoio: `A central lê os dados que o C-Shield envia: mapa da frota, temperatura em tempo real, histórico de ataques e de liberações. Nada ali manda ordens ao aparelho.`

## 4. Roteiro para quem for editar

1. Trocar os textos da seção 1 (cinco arquivos de conteúdo e um componente, mais README e planejamento).
2. Trocar a ilustração `dispositivo.png` e o texto alternativo.
3. Reescrever o gesto do emulador e as telas do LCD (seção 2) e ajustar os testes: `npm run test` deve voltar a passar.
4. Conferir à mão em `/como-funciona`, `/` e `/emulador`: procurar a palavra "botão" no código-fonte (`grep -rni "bot[aã]o" src`) e deixar só as ocorrências de botões de interface (como `BotaoLed`), nunca o botão de liberação.
