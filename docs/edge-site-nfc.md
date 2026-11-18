# Ajuste de textos do site: liberação por cartão NFC

Oi! No Edge, o **botão de liberação foi trocado por um cartão NFC** (sugestão do professor). Só os **textos** do site que falam do botão precisam mudar. Não mexa em interface, design, imagens nem na simulação do emulador: tudo continua como está.

## O que mudou no produto (para você escrever com segurança)

- Antes: segurar um botão físico por 3 segundos liberava o bloqueio.
- Agora: encostar no aparelho um **cartão NFC cadastrado** libera o bloqueio, depois da trava mínima.
- Cada cartão é um papel (responsável técnico, enfermagem, manutenção, supervisão, gerência), e **cada liberação fica registrada** com o papel de quem liberou.
- A promessa do produto continua a mesma: ninguém libera pela internet, só quem está presente.

## Textos para trocar

São cinco trocas no código e uma no README. Os arquivos são do repositório do site (`WebFront-GS`).

### 1. `src/content/comoFunciona.js`, passo 04 "Alertar e bloquear"

Trocar o final do texto:

- **Antes:** `...O bloqueio só termina presencialmente: no botão físico do aparelho.`
- **Depois:** `...O bloqueio só termina presencialmente: com um cartão NFC cadastrado, encostado no aparelho. Cada liberação fica registrada com o papel de quem liberou.`

### 2. `src/content/comoFunciona.js`, bloco "Uma direção só" (última frase da lista `etapas`)

- **Antes:** `Só o botão físico do aparelho libera o bloqueio.`
- **Depois:** `Só um cartão NFC cadastrado, encostado no aparelho, libera o bloqueio.`

### 3. `src/content/inicio.js`, passo 04 "Alertar e bloquear" (Home)

- **Antes:** `...Para liberar, é preciso estar presente e usar o botão físico do aparelho.`
- **Depois:** `...Para liberar, é preciso estar presente e encostar no aparelho um cartão NFC cadastrado.`

### 4. `src/components/comoFunciona/DiagramaArquitetura.jsx`, só os textos

Na caixa do diagrama (linha 154), trocar apenas as duas strings:

- **Antes:** `titulo="Botão no aparelho"` e `detalhe="físico, só ele libera"`
- **Depois:** `titulo="Cartão NFC"` e `detalhe="presencial, só ele libera"`

Na legenda abaixo do diagrama (linha 224):

- **Antes:** `Só o botão físico do aparelho libera o bloqueio.`
- **Depois:** `Só um cartão NFC cadastrado, encostado no aparelho, libera o bloqueio.`

Troque também o comentário da linha 153 (`só o botão físico libera`) por `só o cartão NFC libera`; ele não aparece na tela, mas senão a conferência abaixo acusa. Se a caixa ficar apertada com o texto novo, deixe o detalhe só como `só ele libera`.

### 5. `src/components/emulador/ImagemDispositivo.jsx`, texto alternativo (linha 21)

- **Antes:** `...LED verde de estado e botão de liberação`
- **Depois:** `...LED verde de estado e leitor de cartão NFC para a liberação`

A imagem em si não precisa mudar agora.

### 6. `README.md` do site, tabela "Como funciona"

- **Antes:** `A liberação é presencial, por um botão no aparelho.`
- **Depois:** `A liberação é presencial: só um cartão NFC cadastrado, encostado no aparelho, encerra o bloqueio, e cada liberação fica registrada com o papel de quem liberou.`

## O que NÃO precisa mudar

- `src/content/linhaDoTempo.js` e `src/pages/Problema.jsx`: dizem "só quem está presente consegue liberar", que continua certo.
- O emulador (`/emulador`): botão de segurar, telas do LCD e a lógica ficam como estão.
- Imagens, cores, layout e componentes.

## Conferência rápida

Depois de trocar, rode `grep -rni "botão físico\|botão no aparelho" src README.md`: não deve sobrar nenhuma ocorrência. O texto do botão do emulador (`Segure 3 s`) é interface e fica de fora desta mudança.
