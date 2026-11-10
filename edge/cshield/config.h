/*
 * C-Shield — Edge Computing do Guardião (GS 2026.2 "CitizenTech")
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

// 1 = modo demonstração para o vídeo: a trava mínima cai de 30 min para 60 s.
// Mude para 0 na versão "de produção".
#define MODO_DEMO 1

// 1 = modelo térmico de demonstração (simulacao.h): no Wokwi o DHT22 é um slider manual,
// então a ventoinha de emergência "esfria" a temperatura por software. Use 0 na maquete.
#define MODELO_TERMICO_DEMO 1

// 1 no Wokwi (uma chave de 2 posições representa o setpoint que o controlador manda:
// NORMAL ou ATAQUE); 0 na maquete física, onde não há chave e o setpoint fica ausente.
#define TEM_POT_SETPOINT 1

// Muitos módulos de relé físicos acionam em LOW. 0 = aciona em HIGH (Wokwi).
#define RELE_ATIVO_EM_LOW 0

// ---------------------------------------------------------------------------
// Mapa de pinos (fixo; é o mesmo do plano da equipe)
// ---------------------------------------------------------------------------

const int PIN_DHT       = 4;   // DHT22: temperatura e umidade próprias
const int PIN_LDR       = 34;  // LDR (saída analógica): porta aberta
const int PIN_CHAVE_SP  = 32;  // chave NORMAL/ATAQUE (só Wokwi): setpoint do controlador
const int PIN_DEGELO    = 18;  // fio do controlador (maquete): degelo comandado; sem nada ligado fica em LOW
const int PIN_BOTAO     = 19;  // botão de liberação presencial (INPUT_PULLUP)
const int PIN_RELE_AQUEC = 23; // relé 1, contato NF: corta o aquecedor/degelo
const int PIN_RELE_VENT = 33;  // relé 2, contato NA: liga a ventoinha de emergência
const int PIN_LED_R     = 25;  // LED RGB, canal vermelho (220 ohms)
const int PIN_LED_G     = 26;  // LED RGB, canal verde (220 ohms)
const int PIN_LED_B     = 27;  // LED RGB, canal azul (220 ohms)
const int PIN_BUZZER    = 5;   // buzzer: alerta sonoro
const int PIN_I2C_SDA   = 21;  // I2C do LCD
const int PIN_I2C_SCL   = 22;  // I2C do LCD

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
  float tolerancia;  // margem além da faixa antes de ser CRITICO, °C
};

enum IdPerfil { PERFIL_VACINA = 0, PERFIL_CONGELADO = 1, PERFIL_RESFRIADO = 2, PERFIL_PLASMA = 3 };

// Tolerância de 1,0 °C: margem curta o bastante para avisar antes de o produto
// estragar, e longa o bastante para não disparar CRITICO por oscilação do sensor.
//
// Vacina: +2 a +8 °C (faixa de refrigeração padrão de vacinas); a caixa com gelo em gel
//   da maquete fica nessa faixa e conecta com as salas de vacina de São Paulo.
// Congelado: máximo de -18 °C (padrão da cadeia do congelado). O mínimo é só o piso
//   prático do sensor, pois frio extra não estraga o alimento.
// Resfriado: 0 a +4 °C (carnes e laticínios frescos).
// Plasma: máximo de -25 °C (plasma guardado a cerca de -30 °C). Mínimo no piso do sensor.
static const Perfil PERFIS[4] = {
  { 2.0f,   8.0f, 1.0f },  // PERFIL_VACINA
  { -30.0f, -18.0f, 1.0f },// PERFIL_CONGELADO
  { 0.0f,   4.0f, 1.0f },  // PERFIL_RESFRIADO
  { -40.0f, -25.0f, 1.0f } // PERFIL_PLASMA
};

// Perfil ativo. Padrão: vacina.
static const int PERFIL_ATIVO = PERFIL_VACINA;

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

// VALOR PROVISÓRIO, espelho do anterior: resfriamento acima de 0,5 °C/min (abaixo do centro da
// faixa) é tratado como ataque que esfria. A DPS calibra junto com a taxa de aquecimento.
const float TAXA_MAX_RESFRIAMENTO_C_POR_MIN = 0.5f;

// Degelo comandado por mais de 30 min é suspeito: um degelo normal é curto.
const unsigned long DEGELO_MAX_MS = 30UL * 60UL * 1000UL;

// Horários permitidos para degelo (hora cheia, relógio local). Fora deles, com hora
// válida por NTP, a ordem de degelo é considerada perigosa.
const int HORA_DEGELO_1 = 6;
const int HORA_DEGELO_2 = 18;
// Cada janela dura 1 hora (06:00 a 07:00 e 18:00 a 19:00).
const int JANELA_DEGELO_H = 1;

// Trava mínima de bloqueio: o botão de liberação só vale depois dela. 30 min no uso real;
// 60 s no modo demonstração, para caber no vídeo de 7 min.
#if MODO_DEMO
const unsigned long TRAVA_MS = 60UL * 1000UL;
#else
const unsigned long TRAVA_MS = 30UL * 60UL * 1000UL;
#endif

// Segurar o botão por 3 s evita liberação por toque acidental.
const unsigned long LIBERACAO_MS = 3000;

// Debounce do botão: ignora contatos que oscilam por menos de 50 ms.
const unsigned long DEBOUNCE_MS = 50;

// ---------------------------------------------------------------------------
// Modelo térmico da demonstração (simulacao.h)
// ---------------------------------------------------------------------------

// Velocidade com que a ventoinha de emergência baixa a temperatura na simulação. 6 °C/min
// leva de 12 °C ao centro da faixa em cerca de 1 min, cabendo no vídeo.
const float RESFRIAMENTO_DEMO_C_POR_MIN = 6.0f;

// Variação do sensor acima da qual o modelo entende que a pessoa mexeu no slider. Fica acima
// do ruído do DHT22 (cerca de 0,1 a 0,2 °C).
const float MODELO_MUDANCA_C = 0.3f;

// ---------------------------------------------------------------------------
// Setpoint observado (chave de 2 posições do Wokwi)
// ---------------------------------------------------------------------------

// Chave em NORMAL: o controlador manda um setpoint dentro da faixa do perfil vacina
// (2 a 8 °C). Chave em ATAQUE: manda um valor quente, fora da faixa, o que dispara o
// bloqueio. Dois valores fixos evitam o ajuste fino que o potenciômetro exigia.
const float SETPOINT_DEMO_NORMAL = 5.0f;
const float SETPOINT_DEMO_ATAQUE = 45.0f;

// Valor enviado em `sp` quando não há setpoint observado.
const float SETPOINT_AUSENTE = -999.0f;

// ---------------------------------------------------------------------------
// Alertas locais (tempos em ms)
// ---------------------------------------------------------------------------

const unsigned long LED_PISCA_MS       = 400;   // meio período do pisca do LED vermelho/amarelo
const unsigned long BUZZER_BIPE_MS     = 120;   // duração de um bipe curto
const unsigned long BUZZER_INTERVALO_MS = 10000; // ATENCAO e FALHA_SENSOR: um aviso a cada 10 s
const unsigned long BUZZER_INTERM_MS   = 300;   // BLOQUEIO: liga/desliga a cada 300 ms
const unsigned int  BUZZER_FREQ_HZ     = 2000;  // tom do buzzer

// Aviso sonoro inicial do bloqueio: o apito só toca enquanto a temperatura está fora da
// faixa, mas um ataque com a temperatura ainda boa (por exemplo, um setpoint perigoso)
// também precisa ser ouvido. 5 s bastam para chamar atenção sem virar incômodo.
const unsigned long ALARME_INICIAL_MS = 5000;

// A linha 2 do LCD alterna entre a temperatura e as mensagens do estado a cada 2 s.
const unsigned long ALTERNA_TELA_MS = 2000;

// Tempo em que o LCD confirma "Liberado" depois da liberação presencial.
// 3 s: o bastante para ler a confirmação antes de a tela voltar ao estado normal.
const unsigned long LIBERADO_TELA_MS = 3000;

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
