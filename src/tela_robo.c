#include "tela_robo.h"
#include "cenario_planeta.h"
#include "partida.h"
#include "robo_logica.h"
#include "constantes.h"

#include <math.h>
#include <stddef.h>

typedef enum { ROBO_SALA, ROBO_CONVERSA, ROBO_DESAFIO } ModoCientista;
static CenarioPlaneta salaRobo;
static ExploradorPlaneta exploradorRobo;
static Laboratorio laboratorio;
static ModoCientista modoRobo;
static bool conversouCientista, mostrarDica;
static int paginaCientista;
static float tempoRobo, avisoRobo;
static ResultadoPasso resultadoRobo;
static const char *mensagemCientista;
static const ObstaculoExploracao objetosRobo[] = {
    {264,232,112,32}, {486,244,28,8}, {152,212,40,34}
};

void iniciarTelaRobo(void)
{
    iniciarCenarioPlaneta(&salaRobo,NULL);
    iniciarExploradorPlaneta(&exploradorRobo,(PontoExploracao){80,316});
    iniciarLaboratorio(&laboratorio,MAPA_LAB_PADRAO);
}

/* Nova partida: o desafio e a conversa com o cientista começam do zero. */
void reiniciarTelaRobo(void)
{
    iniciarLaboratorio(&laboratorio,MAPA_LAB_PADRAO);
    conversouCientista=false;
    mostrarDica=false;
    paginaCientista=0;
}

void entrarTelaRobo(void)
{
    exploradorRobo.pes=(PontoExploracao){80,316};
    modoRobo=ROBO_SALA;
    avisoRobo=0;
}

static bool pertoCientista(void)
{
    return hypotf(exploradorRobo.pes.x-500,exploradorRobo.pes.y-252)<58;
}

static bool pertoConsole(void)
{
    float dx=fmaxf(264-exploradorRobo.pes.x,exploradorRobo.pes.x-376);
    float dy=fmaxf(232-exploradorRobo.pes.y,exploradorRobo.pes.y-264);
    return hypotf(fmaxf(0,dx),fmaxf(0,dy))<42;
}

static void falarCientista(void)
{
    ProgressoPartida *p=progressoPartida();
    paginaCientista=0;
    conversouCientista=true;
    if (p->roboConcluido && !p->temTradutor) {
        p->temTradutor=true;
        mensagemCientista="Percurso concluído! Este tradutor é seu.\nEle traduz a fala alienígena e estima o tom.\nA leitura do tom pode falhar.";
    } else if (p->temTradutor) {
        mensagemCientista="O tradutor já está com você.\nVolte ao extraterrestre e converse novamente.\nO tom de voz é uma pista, nunca uma certeza.";
    } else {
        mensagemCientista="Sou o cientista deste laboratório.\nResolva o trajeto no console para receber\num tradutor para o encontro alienígena.";
    }
    modoRobo=ROBO_CONVERSA;
}

static void atualizarDesafioRobo(float dt)
{
    if (laboratorio.fase==ROBO_EXECUTANDO) {
        tempoRobo+=dt;
        while (tempoRobo>=ROBO_TEMPO_PASSO && laboratorio.fase==ROBO_EXECUTANDO) {
            tempoRobo-=ROBO_TEMPO_PASSO;
            resultadoRobo=passoDoRobo(&laboratorio);
        }
        if (laboratorio.fase==ROBO_VENCEU) progressoPartida()->roboConcluido=true;
        return;
    }
    if (IsKeyPressed(KEY_R)) {
        int tentativas=laboratorio.tentativas;
        iniciarLaboratorio(&laboratorio,MAPA_LAB_PADRAO);
        laboratorio.tentativas=tentativas;
        resultadoRobo=PASSO_NADA;
    }
    if (IsKeyPressed(KEY_H)) mostrarDica=!mostrarDica;
    if (IsKeyPressed(KEY_UP)||IsKeyPressed(KEY_W)) adicionarComando(&laboratorio,CMD_CIMA);
    if (IsKeyPressed(KEY_DOWN)||IsKeyPressed(KEY_S)) adicionarComando(&laboratorio,CMD_BAIXO);
    if (IsKeyPressed(KEY_LEFT)||IsKeyPressed(KEY_A)) adicionarComando(&laboratorio,CMD_ESQUERDA);
    if (IsKeyPressed(KEY_RIGHT)||IsKeyPressed(KEY_D)) adicionarComando(&laboratorio,CMD_DIREITA);
    if (IsKeyPressed(KEY_BACKSPACE)) removerUltimoComando(&laboratorio);
    if (IsKeyPressed(KEY_ENTER) && executarPrograma(&laboratorio)) {
        tempoRobo=0;
        resultadoRobo=PASSO_NADA;
    }
}

Tela atualizarTelaRobo(void)
{
    float dt=fminf(GetFrameTime(),0.1f);
    if (avisoRobo>0) avisoRobo-=dt;
    if (modoRobo==ROBO_CONVERSA) {
        if (IsKeyPressed(KEY_ESCAPE)) modoRobo=ROBO_SALA;
        else if (IsKeyPressed(KEY_E)||IsKeyPressed(KEY_ENTER)) {
            if (progressoPartida()->temTradutor || ++paginaCientista>=2) modoRobo=ROBO_SALA;
        }
        return TELA_ROBO;
    }
    if (modoRobo==ROBO_DESAFIO) {
        if (IsKeyPressed(KEY_ESCAPE)) modoRobo=ROBO_SALA;
        else atualizarDesafioRobo(dt);
        return TELA_ROBO;
    }
    if (IsKeyPressed(KEY_ESCAPE)) return TELA_NAVE;
    atualizarExploradorPlaneta(&exploradorRobo,&salaRobo.piso,objetosRobo,3,dt,true);
    if (IsKeyPressed(KEY_E)) {
        if (pertoCientista()) falarCientista();
        else if (pertoConsole()) {
            if (conversouCientista) modoRobo=ROBO_DESAFIO;
            else avisoRobo=3;
        } else if (exploradorRobo.pes.x<90) return TELA_NAVE;
    }
    return TELA_ROBO;
}

static void cientistaRobo(void)
{
    Color metal=GetColor(0xA58DAFFF), escuro=GetColor(0x282136FF), luz=GetColor(0x68D8CDFF);
    DrawRectangle(480,198,40,52,escuro);
    DrawRectangle(484,200,32,24,metal);
    DrawRectangle(488,206,24,12,escuro);
    DrawRectangle(491,209,6,5,luz);
    DrawRectangle(503,209,6,5,luz);
    DrawRectangle(487,228,26,18,metal);
    DrawRectangle(492,232,16,3,COR_MARCADOR);
    DrawRectangle(486,247,10,5,metal);
    DrawRectangle(504,247,10,5,metal);
    DrawRectangle(498,190,4,10,metal);
    DrawRectangle(496,188,8,4,luz);
}

static void setaComando(Comando c, int x, int y, Color cor)
{
    static const char *const letras[]={"C","B","E","D"};
    textoPlaneta(letras[c],(float)x,(float)y,8,cor);
}

static const char *estadoDesafio(void)
{
    if (laboratorio.fase==ROBO_VENCEU) return "Percurso concluído!\nVolte ao cientista para\nreceber o tradutor.";
    if (laboratorio.fase==ROBO_EXECUTANDO) return "Executando a fila...";
    if (resultadoRobo==PASSO_LAVA) return "O robô entrou na lava.\nTente outro caminho.";
    if (resultadoRobo==PASSO_PAREDE) return "O robô encontrou uma parede.\nTente outro caminho.";
    if (resultadoRobo==PASSO_SEM_COMANDOS) return "A fila terminou antes da saída.\nPlaneje o trajeto completo.";
    return "Monte a fila para alcançar\na saída. Evite paredes e lava.";
}

static void desenharDesafioRobo(void)
{
    DrawRectangle(0,0,640,360,GetColor(0x17131FFF));
    textoPlaneta("LABORATÓRIO DE COMANDOS",24,16,16,COR_TITULO);
    textoPlaneta("C=CIMA  B=BAIXO  E=ESQUERDA  D=DIREITA",24,43,8,COR_SUBTITULO);
    caixaPlaneta(20,62,264,188);
    for (int y=0;y<LAB_LINHAS;y++) {
        for (int x=0;x<LAB_COLUNAS;x++) {
            int px=26+x*36,py=66+y*36;
            Casa casa=laboratorio.mapa[y][x];
            Color cor=casa==CASA_PAREDE?GetColor(0x68557EFF):
                casa==CASA_LAVA?GetColor(0xAE543FFF):GetColor(0x30283FFF);
            DrawRectangle(px,py,34,34,cor);
            if (casa==CASA_SAIDA) {
                DrawRectangleLines(px+3,py+3,28,28,GetColor(0x68D8CDFF));
                textoPlaneta("FIM",(float)px+5,(float)py+13,8,GetColor(0x68D8CDFF));
            }
            if (x==laboratorio.x && y==laboratorio.y) {
                DrawRectangle(px+9,py+8,16,19,COR_TITULO);
                DrawRectangle(px+11,py+10,12,7,GetColor(0x30283FFF));
                DrawRectangle(px+12,py+12,3,3,GetColor(0x68D8CDFF));
                DrawRectangle(px+19,py+12,3,3,GetColor(0x68D8CDFF));
            }
        }
    }
    caixaPlaneta(300,62,316,188);
    textoPlaneta(estadoDesafio(),316,80,8,COR_TITULO);
    textoPlaneta(TextFormat("Tentativas: %d",laboratorio.tentativas),316,142,8,COR_SUBTITULO);
    textoPlaneta("ENTER  Executar\nBACKSPACE  Apagar último\nR  Recomeçar   H  Dica\nESC  Voltar ao laboratório",316,166,8,COR_MENU);
    textoPlaneta(TextFormat("FILA  %02d / %02d",laboratorio.fila.quantidade,ROBO_MAX_COMANDOS),24,266,8,COR_MARCADOR);
    for (int i=0;i<ROBO_MAX_COMANDOS;i++) {
        int x=24+i*24;
        DrawRectangle(x,286,21,22,GetColor(0x332A44FF));
        if (i<laboratorio.fila.quantidade) setaComando(comandoNaPosicao(&laboratorio.fila,i),x+7,293,COR_TITULO);
    }
    if (mostrarDica) {
        Comando dica[ROBO_PASSOS_DICA];
        int n=caminhoMaisCurto(&laboratorio,dica,ROBO_PASSOS_DICA);
        textoPlaneta("PRIMEIROS PASSOS:",432,270,8,COR_SUBTITULO);
        for (int i=0;i<n && i<ROBO_PASSOS_DICA;i++) setaComando(dica[i],440+i*24,294,COR_MARCADOR);
    }
    textoPlaneta("SETAS / WASD  Adicionar comandos à fila",24,330,8,COR_MENU);
}

void desenharTelaRobo(void)
{
    if (modoRobo==ROBO_DESAFIO) { desenharDesafioRobo(); return; }
    desenharCenarioPlaneta(&salaRobo,GetColor(0x282331FF),GetColor(0x68D8CDFF));
    for (int i=0;i<3;i++) if (i!=1) desenharObstaculoPlaneta(objetosRobo[i],GetColor(0x524465FF));
    textoPlaneta("CONSOLE",288,224,8,COR_SUBTITULO);
    cientistaRobo();
    desenharExploradorPlaneta(&exploradorRobo);
    caixaPlaneta(16,12,330,38);
    textoPlaneta("PLANETA DO ROBÔ",26,20,16,COR_TITULO);
    textoPlaneta(progressoPartida()->temTradutor?"Tradutor recebido":"Fale com o cientista",26,41,8,COR_MARCADOR);
    textoPlaneta("WASD/SETAS Mover   E Interagir   ESC Nave",16,348,8,COR_MENU);
    if (modoRobo==ROBO_CONVERSA) {
        DrawRectangle(0,0,640,360,Fade(BLACK,0.35f));
        caixaPlaneta(20,236,600,108);
        textoPlaneta("ROBÔ CIENTISTA",36,248,8,COR_MARCADOR);
        textoPlaneta(paginaCientista==0?mensagemCientista:
            "No console, programe o trajeto com as setas.\nEnter executa; Backspace apaga; R recomeça.\nH revela os primeiros passos. Depois, volte aqui.",36,268,8,COR_TITULO);
        textoPlaneta("E/ENTER Continuar   ESC Fechar",36,326,8,COR_SUBTITULO);
    } else if (avisoRobo>0) {
        caixaPlaneta(134,58,372,24);
        textoPlaneta("Fale primeiro com o robô cientista.",146,66,8,COR_MARCADOR);
    } else {
        const char *dica=pertoCientista()?"E  Conversar com o cientista":
            pertoConsole()?"E  Abrir laboratório":exploradorRobo.pes.x<90?"E  Voltar à nave":NULL;
        if (dica) { caixaPlaneta(166,58,308,24); textoPlaneta(dica,178,66,8,COR_MARCADOR); }
    }
}

void encerrarTelaRobo(void)
{
    encerrarExploradorPlaneta(&exploradorRobo);
    encerrarCenarioPlaneta(&salaRobo);
}
