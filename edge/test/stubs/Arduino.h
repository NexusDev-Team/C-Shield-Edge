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
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdint>
#include <cmath>
#include <algorithm>
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
typedef unsigned char byte;
inline void pinMode(int,int){} inline void digitalWrite(int,int){} inline int digitalRead(int){return 0;}
inline int analogRead(int){return 0;} inline long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
inline unsigned long millis(){return 0;} inline void tone(int,unsigned){} inline void noTone(int){}
template<class A,class B> auto min(A a,B b)->decltype(a<b?a:b){return a<b?a:b;}
struct SerialT{ void begin(long){} int printf(const char* f,...) __attribute__((format(printf,2,3))); void println(const char*){} void print(const char*){} } ;
inline int SerialT::printf(const char*,...){return 0;}
static SerialT Serial;
void setup(); void loop();
#include <ctime>
inline struct tm* localtime_r(const time_t* t, struct tm* r){ localtime_s(r,t); return r; }
