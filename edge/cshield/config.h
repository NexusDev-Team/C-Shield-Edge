/*
 * C-Shield — Edge Computing (GS 2026.2 "CitizenTech")
 * Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
 * Equipe Nexus Vision. Integrantes:
 *   Isac Nilton Fernandes de Oliveira — RM573282
 *   João Benedito de Oliveira Simplício — RM570206
 *   Leonardo Formigari — RM573291
 *   Luis Fernando — RM570959
 *   Nicolas Cinel — RM572940
 */

/*
 * config.h — TODAS as constantes do C-Shield: pinos, perfis de produto, limites,
 * tempos e rede. Nenhum outro módulo tem número solto: tudo o que precisa de
 * justificativa fica aqui, com o motivo escrito ao lado.
 *
 * Este arquivo não inclui Arduino.h de propósito: a lógica pura (decisao.h) o usa
 * e precisa compilar no computador com g++ para os testes.
 */
#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------------------
// Identidade do produto
// ---------------------------------------------------------------------------

// Nome exibido no LCD, no Serial e nos documentos. Nenhum outro arquivo escreve
// o nome do produto: todos leem esta constante.
static const char* const NOME_PRODUTO = "C-Shield";

// ---------------------------------------------------------------------------
// Chaves de compilação
// ---------------------------------------------------------------------------

// 1 = modo demonstração para o vídeo: o degelo máximo cai de 30 min para 15 s e a
// checagem de horário do degelo é desligada. Os testes compilam também com 0
// (-DMODO_DEMO=0) para cobrir o comportamento de produção.
#ifndef MODO_DEMO
#define MODO_DEMO 1
#endif

// 1 = modelo térmico de demonstração (simulacao.h): no Wokwi o DHT22 é um slider manual,
// então a ventoinha "esfria" a temperatura por software durante o bloqueio. Use 0 na maquete.
#ifndef MODELO_TERMICO_DEMO
#define MODELO_TERMICO_DEMO 1
#endif

// Muitos módulos de relé físicos acionam em LOW. 0 = aciona em HIGH (Wokwi).
#define RELE_ATIVO_EM_LOW 0

// ---------------------------------------------------------------------------
// Mapa de pinos (fixo; é o mesmo do plano da equipe)
// ---------------------------------------------------------------------------

const int PIN_DHT        = 4;   // DHT22: temperatura e umidade próprias
const int PIN_LDR        = 34;  // LDR (saída analógica): porta aberta
const int PIN_CHAVE_AQ   = 18;  // chave (Wokwi) ou fio do controlador: aquecedor/degelo comandado
const int PIN_CHAVE_VT   = 17;  // chave (Wokwi) ou fio do controlador: ventoinha/compressor comandado
const int PIN_BOTAO      = 19;  // botão de liberação presencial (INPUT_PULLUP)
const int PIN_RELE_AQUEC = 23;  // relé 1, contato NF em série com o aquecedor: acionado, corta
const int PIN_RELE_VENT  = 33;  // relé 2, comutador da ventoinha: acionado, o C-Shield liga direto
const int PIN_LED_R      = 25;  // LED RGB, canal vermelho (220 ohms)
const int PIN_LED_G      = 26;  // LED RGB, canal verde (220 ohms)
const int PIN_LED_B      = 27;  // LED RGB, canal azul (220 ohms)
const int PIN_BUZZER     = 5;   // buzzer: alerta sonoro
const int PIN_I2C_SDA    = 21;  // I2C do LCD
const int PIN_I2C_SCL    = 22;  // I2C do LCD

// ---------------------------------------------------------------------------
// Display LCD 16x2 I2C (substitui o OLED; mais fácil de usar no físico)
// ---------------------------------------------------------------------------

const int LCD_ENDERECO = 0x27; // backpack PCF8574 comum; alguns módulos vêm em 0x3F
const int LCD_COLUNAS  = 16;
const int LCD_LINHAS   = 2;

// ---------------------------------------------------------------------------
// Perfis de produto (faixa ideal de temperatura)
// ---------------------------------------------------------------------------

struct Perfil {
  float tMin;        // limite inferior da faixa ideal, °C
  float tMax;        // limite superior da faixa ideal, °C
  float tAlvo;       // temperatura que o termostato de emergência persegue, °C
  float tolerancia;  // margem além da faixa antes de ser CRITICO, °C
};

enum IdPerfil { PERFIL_VACINA = 0, PERFIL_CONGELADO = 1, PERFIL_RESFRIADO = 2, PERFIL_PLASMA = 3 };

// Tolerância de 1,0 °C: margem curta o bastante para avisar antes de o produto
// estragar, e longa o bastante para não disparar CRITICO por oscilação do sensor.
//
// Vacina: +2 a +8 °C (faixa de refrigeração padrão de vacinas); alvo 5 °C, o meio da faixa.
// Congelado: máximo de -18 °C (padrão da cadeia do congelado); o mínimo é só o piso prático
//   do sensor, pois frio extra não estraga o alimento; alvo -20 °C, com folga abaixo do máximo.
// Resfriado: 0 a +4 °C (carnes e laticínios frescos); alvo 2 °C.
// Plasma: máximo de -25 °C (plasma guardado a cerca de -30 °C); mínimo no piso do sensor;
//   alvo -30 °C.
static const Perfil PERFIS[4] = {
  { 2.0f,   8.0f,   5.0f,  1.0f },  // PERFIL_VACINA
  { -30.0f, -18.0f, -20.0f, 1.0f }, // PERFIL_CONGELADO
  { 0.0f,   4.0f,   2.0f,  1.0f },  // PERFIL_RESFRIADO
  { -40.0f, -25.0f, -30.0f, 1.0f }  // PERFIL_PLASMA
};

// Perfil ativo. Padrão: vacina.
static const int PERFIL_ATIVO = PERFIL_VACINA;

// Histerese do termostato de emergência: a ventoinha liga em T_ALVO + 1 °C e desliga em
// T_ALVO - 1 °C. A folga de 2 °C entre os dois pontos evita liga-desliga a cada ciclo.
const float HISTERESE_C = 1.0f;

// ---------------------------------------------------------------------------
// Sensoriamento e suavização
// ---------------------------------------------------------------------------

// 2 s é o mínimo de intervalo do DHT22; ler mais rápido devolve NaN.
const unsigned long PERIODO_LEITURA_MS = 2000;

// Média móvel das últimas 5 leituras (10 s): filtra ruído sem atrasar muito a reação.
const int JANELA_MEDIA = 5;

// Um estado de alarme (CRITICO, FALHA_SENSOR) só vale após 2 ciclos seguidos,
// para um pico isolado do sensor não disparar o alarme.
const int CONFIRMACOES = 2;

// Faixa física válida do DHT22: fora dela o valor é erro de leitura, não temperatura.
const float DHT_TEMP_MIN_FISICA = -40.0f;
const float DHT_TEMP_MAX_FISICA = 80.0f;

// Luminosidade (LDR, em %) acima disso significa porta aberta: a luz da sala entra
// na câmara escura. 40% dá folga para a luz interna da câmara.
const float LIMIAR_PORTA_PCT = 40.0f;

// Valor enviado em `t` e `h` quando o sensor está inválido (não publica valor velho).
const float VALOR_AUSENTE = -999.0f;

// ---------------------------------------------------------------------------
// Regras de bloqueio
// ---------------------------------------------------------------------------

// Janela de 60 s para calcular a taxa de aquecimento: longa o bastante para suavizar
// ruído, curta o bastante para reagir a um ataque em segundos.
const unsigned long JANELA_TAXA_MS = 60000;

// Amostras na janela da taxa (uma a cada PERIODO_LEITURA_MS, mais a de ponta).
const int N_AMOSTRAS_TAXA = (int)(JANELA_TAXA_MS / PERIODO_LEITURA_MS) + 1;

// Intervalo mínimo entre a amostra mais antiga e a mais nova para calcular a taxa.
// Abaixo disso a conta amplifica ruído (poucos segundos dão taxas enormes).
const unsigned long TAXA_MIN_INTERVALO_MS = 10000;

// VALOR PROVISÓRIO: 0,5 °C/min de aquecimento com a porta fechada é o limite
// inicial. A DPS da equipe vai calibrar com a maquete (modelo de Newton).
const float TAXA_MAX_C_POR_MIN = 0.5f;

// Duração máxima de um degelo legítimo. Um degelo normal é curto: 30 min no uso real.
// No modo demonstração cai para 15 s, para mostrar o degelo virando ataque no vídeo.
#if MODO_DEMO
const unsigned long DEGELO_MAX_MS = 15UL * 1000UL;
#else
const unsigned long DEGELO_MAX_MS = 30UL * 60UL * 1000UL;
#endif

// Horários permitidos para degelo (hora cheia, relógio local). Fora deles, com hora
// válida por NTP, o aquecedor ligado com a ventoinha desligada é considerado ataque.
const int HORA_DEGELO_1 = 6;
const int HORA_DEGELO_2 = 18;
// Cada janela dura 30 min (06:00 a 06:30 e 18:00 a 18:30).
const int JANELA_DEGELO_MIN = 30;

// Segurar o botão por 3 s evita liberação por toque acidental.
const unsigned long LIBERACAO_MS = 3000;

// Debounce do botão e das chaves do controlador: ignora contatos que oscilam por menos de 50 ms.
const unsigned long DEBOUNCE_MS = 50;

// ---------------------------------------------------------------------------
// Modelo térmico da demonstração (simulacao.h)
// ---------------------------------------------------------------------------

// Velocidade com que a ventoinha baixa a temperatura na simulação. 6 °C/min leva de 12 °C
// ao piso do termostato em cerca de 1 min, cabendo no vídeo.
const float RESFRIAMENTO_DEMO_C_POR_MIN = 6.0f;

// Velocidade com que a câmara volta a esquentar, em direção ao valor do slider, quando a
// ventoinha desliga. 3 °C/min mostra o termostato religando sem esperar muito.
const float AQUECIMENTO_DEMO_C_POR_MIN = 3.0f;

// Variação do sensor acima da qual o modelo entende que a pessoa mexeu no slider. Fica acima
// do ruído do DHT22 (cerca de 0,1 a 0,2 °C).
const float MODELO_MUDANCA_C = 0.3f;

// ---------------------------------------------------------------------------
// Alertas locais (tempos em ms)
// ---------------------------------------------------------------------------

const unsigned long LED_PISCA_MS       = 400;   // meio período do pisca do LED vermelho/amarelo
const unsigned long BUZZER_BIPE_MS     = 120;   // duração de um bipe curto
const unsigned long BUZZER_INTERVALO_MS = 10000; // ATENCAO e FALHA_SENSOR: um aviso a cada 10 s
const unsigned long BUZZER_INTERM_MS   = 300;   // BLOQUEIO: liga/desliga a cada 300 ms
const unsigned int  BUZZER_FREQ_HZ     = 2000;  // tom do buzzer

// Tempo em que o LCD confirma a liberação ("LIBERADO!") ou a recusa ("FORA DA FAIXA").
// 3 s: o bastante para ler antes de a tela voltar ao estado.
const unsigned long AVISO_TELA_MS = 3000;

// A linha 2 do bloqueio alterna entre o motivo e a instrução do botão a cada 2 s.
const unsigned long ALTERNA_TELA_MS = 2000;

// Tempo em que a tela de boot fica no LCD antes de mostrar o estado.
const unsigned long BOOT_TELA_MS = 1500;

// ---------------------------------------------------------------------------
// Rede e FIWARE
// ---------------------------------------------------------------------------

// Wi-Fi aberto do simulador Wokwi.
static const char* const WIFI_SSID  = "Wokwi-GUEST";
static const char* const WIFI_SENHA = "";

// Placeholder: a EC2 usa IP elástico. O IP da Vinheria era 3.215.3.216; confirmar se
// ainda existe antes de gravar o vídeo.
static const char* const BROKER_MQTT = "3.215.3.216";
const int BROKER_PORTA = 1883;

static const char* const ID_DEVICE    = "cshield001";
static const char* const TOPICO_ATTRS = "/TEF/cshield001/attrs";  // único tópico; nenhum de comando

// Reconexão de Wi-Fi/MQTT a cada 10 s, sem travar o loop().
const unsigned long INTERVALO_RECONEXAO_MS = 10000;

// Tempo máximo da sonda TCP antes de tentar o MQTT. Curto o bastante para o loop() não
// travar com o broker fora do ar; longo o bastante para o ida e volta até a EC2.
const int TIMEOUT_SONDA_MS = 800;

// Telemetria a cada 5 s (e imediata a cada mudança de estado).
const unsigned long PERIODO_PUBLICACAO_MS = 5000;

// Hora por NTP (fuso de Brasília, UTC-3, sem horário de verão).
static const char* const NTP_SERVIDOR = "pool.ntp.org";
const long NTP_FUSO_SEGUNDOS = -3L * 3600L;

#endif
