/*
 * C-Shield — Edge Computing (GS 2026.2 "CitizenTech")
 * Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
 * Equipe Nexus Vision. Integrantes:
 *   Isac Nilton Fernandes de Oliveira — RM573282
 *   João Benedito de Oliveira Simplício — RM570206
 *   Leonardo Formigari — RM573291
 *   Luis Fernando — RM570959
 *   Nicolas Cinel — RM572940
 *
 * Stub mínimo só para checar a sintaxe do firmware no computador (g++ -fsyntax-only),
 * sem as bibliotecas do ESP32. Não é usado na placa nem no Wokwi.
 */
#pragma once
#define WL_CONNECTED 3
#define WIFI_STA 1
struct WiFiClass{ void mode(int){} void begin(const char*,const char*){} int status(){return 0;} };
static WiFiClass WiFi;
struct Client{};
struct WiFiClient: Client{ bool connect(const char*,int,int){return false;} void stop(){} };
inline void configTime(long,int,const char*){}
