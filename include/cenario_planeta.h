#ifndef CENARIO_PLANETA_H
#define CENARIO_PLANETA_H

#include "raylib.h"
#include "exploracao.h"

/* Todos os cenários usam piso x=16..623, y=196..343. Obstáculos locais
   são passados ao movimento e desenhados pela tela que os definiu. */
#define PLANETA_PISO_X 16
#define PLANETA_PISO_Y 196
#define PLANETA_PISO_LARGURA 608
#define PLANETA_PISO_ALTURA 148

typedef struct {
    Texture2D fundo;
    PixelExploracao *pixels;
    MapaExploracao piso;
} CenarioPlaneta;

typedef struct {
    Texture2D sprite;
    PontoExploracao pes;
    int direcao, quadro;
    float tempo;
    bool correndo;
} ExploradorPlaneta;

void iniciarCenarioPlaneta(CenarioPlaneta *cenario, const char *arquivo);
void desenharCenarioPlaneta(const CenarioPlaneta *cenario, Color piso, Color destaque);
void encerrarCenarioPlaneta(CenarioPlaneta *cenario);
void iniciarExploradorPlaneta(ExploradorPlaneta *explorador, PontoExploracao posicao);
bool atualizarExploradorPlaneta(ExploradorPlaneta *explorador,
    const MapaExploracao *piso, const ObstaculoExploracao *obstaculos, int quantidade,
    float dt, bool permitirCorrida);
void desenharExploradorPlaneta(const ExploradorPlaneta *explorador);
void encerrarExploradorPlaneta(ExploradorPlaneta *explorador);
void desenharObstaculoPlaneta(ObstaculoExploracao obstaculo, Color cor);
void caixaPlaneta(int x, int y, int largura, int altura);
void textoPlaneta(const char *mensagem, float x, float y, int tamanho, Color cor);

#endif
