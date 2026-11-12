#include "cenario_planeta.h"
#include "constantes.h"
#include "recursos.h"

#include <math.h>
#include <stdlib.h>

void textoPlaneta(const char *s, float x, float y, int tamanho, Color cor)
{
    DrawTextEx(fontePadrao(), s, (Vector2){x,y}, (float)tamanho, 0, cor);
}

void caixaPlaneta(int x, int y, int largura, int altura)
{
    DrawRectangle(x+3,y+3,largura,altura,Fade(BLACK,0.5f));
    DrawRectangle(x,y,largura,altura,GetColor(0x211C2EFF));
    DrawRectangleLines(x,y,largura,altura,GetColor(0x8C77ADFF));
    DrawRectangle(x+2,y+2,largura-4,1,GetColor(0x514464FF));
}

void iniciarCenarioPlaneta(CenarioPlaneta *c, const char *arquivo)
{
    *c=(CenarioPlaneta){0};
    if (arquivo && FileExists(arquivo)) {
        c->fundo=LoadTexture(arquivo);
        if (c->fundo.id) SetTextureFilter(c->fundo,TEXTURE_FILTER_POINT);
    }
    c->pixels=calloc((size_t)LARGURA_VIRTUAL*ALTURA_VIRTUAL,sizeof(*c->pixels));
    if (!c->pixels) return;
    for (int y=PLANETA_PISO_Y;y<PLANETA_PISO_Y+PLANETA_PISO_ALTURA;y++) {
        for (int x=PLANETA_PISO_X;x<PLANETA_PISO_X+PLANETA_PISO_LARGURA;x++) {
            c->pixels[y*LARGURA_VIRTUAL+x]=(PixelExploracao){255,255,255,255};
        }
    }
    c->piso=(MapaExploracao){c->pixels,LARGURA_VIRTUAL,ALTURA_VIRTUAL};
}

void desenharCenarioPlaneta(const CenarioPlaneta *c, Color piso, Color destaque)
{
    ClearBackground(GetColor(0x191522FF));
    if (c->fundo.id) {
        DrawTexturePro(c->fundo,(Rectangle){0,0,(float)c->fundo.width,(float)c->fundo.height},
                       (Rectangle){0,0,640,360},(Vector2){0,0},0,WHITE);
    } else {
        /* A exploração continua utilizável se o fundo decorativo faltar. */
        for (int x=16;x<640;x+=72) {
            DrawRectangle(x,48,60,144,GetColor(0x332A44FF));
            DrawRectangle(x+8,65,44,3,destaque);
        }
    }
    DrawRectangle(0,PLANETA_PISO_Y-4,640,4,GetColor(0x17131FFF));
    if (!c->fundo.id) {
        DrawRectangle(0,PLANETA_PISO_Y,640,PLANETA_PISO_ALTURA,piso);
        Color linhas=ColorBrightness(piso,0.10f);
        for (int x=16;x<640;x+=32) DrawLine(x,196,x,344,linhas);
        for (int y=196;y<344;y+=24) DrawLine(16,y,624,y,linhas);
    }
    DrawRectangle(0,196,16,148,GetColor(0x18131FFF));
    DrawRectangle(624,196,16,148,GetColor(0x18131FFF));
    DrawRectangle(0,344,640,16,GetColor(0x16121EFF));
    DrawLine(16,196,624,196,destaque);
    DrawLine(16,343,624,343,GetColor(0x514464FF));
}

void encerrarCenarioPlaneta(CenarioPlaneta *c)
{
    if (c->fundo.id) UnloadTexture(c->fundo);
    free(c->pixels);
    *c=(CenarioPlaneta){0};
}

void iniciarExploradorPlaneta(ExploradorPlaneta *e, PontoExploracao posicao)
{
    *e=(ExploradorPlaneta){ .pes=posicao, .direcao=0 };
    e->sprite=LoadTexture("assets/imagens/astronauta.png");
    if (e->sprite.id) SetTextureFilter(e->sprite,TEXTURE_FILTER_POINT);
}

bool atualizarExploradorPlaneta(ExploradorPlaneta *e, const MapaExploracao *piso,
    const ObstaculoExploracao *obstaculos, int quantidade, float dt, bool correr)
{
    float x=(float)((IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT))-
                    (IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)));
    float y=(float)((IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN))-
                    (IsKeyDown(KEY_W)||IsKeyDown(KEY_UP)));
    if (x!=0) e->direcao=x<0?2:3;
    else if (y!=0) e->direcao=y<0?1:0;
    float passo=isfinite(dt)&&dt>0?fminf(dt,0.1f):0;
    bool corrida=correr&&(IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT));
    float velocidade=corrida?VELOCIDADE_CORRIDA:VELOCIDADE_JOGADOR;
    PontoExploracao anterior=e->pes;
    e->pes=moverExploracao(piso,e->pes,x,y,velocidade,passo,obstaculos,quantidade);
    bool moveu=anterior.x!=e->pes.x||anterior.y!=e->pes.y;
    e->correndo=moveu&&corrida;
    if (moveu) {
        e->tempo+=passo*(velocidade/VELOCIDADE_JOGADOR);
        while (e->tempo>=TEMPO_PASSO_PERSONAGEM) {
            e->tempo-=TEMPO_PASSO_PERSONAGEM;
            e->quadro=e->quadro%4+1;
        }
    } else { e->tempo=0; e->quadro=0; }
    return moveu;
}

void desenharExploradorPlaneta(const ExploradorPlaneta *e)
{
    int x=(int)floorf(e->pes.x), y=(int)floorf(e->pes.y);
    DrawEllipse(x,y-3,13,4,Fade(BLACK,0.4f));
    if (e->sprite.id) {
        DrawTextureRec(e->sprite,(Rectangle){(float)(e->quadro*44),(float)(e->direcao*58),44,58},
                       (Vector2){(float)x-22,(float)y-58},WHITE);
    } else {
        DrawRectangle(x-9,y-30,18,28,GetColor(0xEEE6D8FF));
        DrawRectangle(x-7,y-26,14,8,GetColor(0x30283FFF));
    }
    DrawRectangle(x-4,y-68,8,2,COR_MARCADOR);
    DrawRectangle(x-2,y-66,4,2,COR_MARCADOR);
}

void encerrarExploradorPlaneta(ExploradorPlaneta *e)
{
    if (e->sprite.id) UnloadTexture(e->sprite);
    *e=(ExploradorPlaneta){0};
}

void desenharObstaculoPlaneta(ObstaculoExploracao o, Color cor)
{
    int x=(int)o.x,y=(int)o.y,w=(int)o.largura,h=(int)o.altura;
    DrawRectangle(x+3,y+3,w,h,Fade(BLACK,0.35f));
    DrawRectangle(x,y,w,h,cor);
    DrawRectangleLines(x,y,w,h,ColorBrightness(cor,0.25f));
    DrawRectangle(x+2,y+2,w-4,3,ColorBrightness(cor,0.18f));
}
