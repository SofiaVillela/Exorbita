/* tela_robo.c: planeta do robô.

   Um laboratório onde mora o robô cientista. Numa redoma de vidro
   está o tradutor, o aparelho que deixa entender a fala dos
   alienígenas. O cientista só o entrega a quem vencer os três
   circuitos da bancada de comandos.

   Em cada circuito, um robozinho precisa ir do início até a saída.
   O jogador monta uma fila de comandos (setas) e manda executar: o
   robozinho cumpre um por vez, na ordem em que entraram, sem bater em
   parede e sem cair na lava. As regras ficam em robo_logica.c e a fila
   em fila_comandos.c.

   O laboratório é uma imagem de 640x360 e o piso vem de uma máscara
   (branco = piso). Tudo é gerado por ferramentas/gerar_robo.py; as
   posições daqui são as de lá vezes 2. */

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "raylib.h"
#include "cenario_planeta.h"
#include "constantes.h"
#include "dialogo.h"
#include "partida.h"
#include "recursos.h"
#include "robo_logica.h"
#include "sons.h"
#include "tela_robo.h"

typedef enum {
    ROBO_SALA,       /* andando pelo laboratório */
    ROBO_CONVERSA,   /* o cientista falando */
    ROBO_PREMIO,     /* a entrega do tradutor */
    ROBO_DESAFIO     /* na bancada de comandos */
} ModoRobo;

typedef enum { ALVO_NENHUM, ALVO_CIENTISTA, ALVO_BANCADA, ALVO_REDOMA, ALVO_PORTA } AlvoRobo;

#define ARQUIVO_SALA      "assets/imagens/planetas/robo_laboratorio.png"
#define ARQUIVO_FRENTE    "assets/imagens/planetas/robo_laboratorio_frente.png"
#define ARQUIVO_PISO      "assets/mascaras/robo_laboratorio_mascara.png"
#define ARQUIVO_BANCADA   "assets/imagens/planetas/robo_bancada.png"
#define ARQUIVO_CIENTISTA "assets/imagens/robo_cientista.png"
#define ARQUIVO_PECAS     "assets/imagens/robo_circuito.png"

#define X_INICIO        96      /* logo acima da porta */
#define Y_INICIO        314
#define Y_SAIDA         334     /* passar daqui para baixo é sair pela porta */
#define X_CIENTISTA     444
#define Y_CIENTISTA     236
#define X_REDOMA        524     /* centro da base da redoma */
#define Y_REDOMA        200
#define X_BANCADA       320     /* centro da bancada de comandos */
#define Y_BANCADA       212     /* linha da frente dela */
#define MEIA_BANCADA    84

/* Desafio: a arena tem 7x5 casas de 40 px. */
#define PECA            40
#define ARENA_X         24
#define ARENA_Y         46
#define FILA_X          32      /* primeira casa da esteira da fila */
#define FILA_Y          278
#define FILA_PASSO      36
#define MONITOR_X       344
#define MONITOR_Y       56

/* Quadros da folha de peças (linha 0). */
enum { PECA_PISO_A, PECA_PISO_B, PECA_PAREDE, PECA_LAVA, PECA_LAVA_2, PECA_SAIDA, PECA_SAIDA_2, PECA_INICIO };
#define QUADRO_QUEBRADO 8       /* na linha 1, a do robozinho */

#define TOTAL_MOVEIS    4
#define TEMPO_AVISO     3.2f
#define COR_CIANO       GetColor(0x68D8CDFF)

/* Um móvel da camada da frente: o recorte dele na imagem e a linha do
   chão onde ele está (quem estiver acima dessa linha fica atrás dele). */
typedef struct {
    Rectangle recorte;
    float base;
} Movel;

static const Movel moveis[TOTAL_MOVEIS] = {
    { { 234, 162, 172, 54 }, 212 },     /* bancada de comandos */
    { { 502, 132, 44, 82 }, 210 },      /* redoma do tradutor */
    { { 554, 254, 52, 70 }, 320 },      /* posto de recarga */
    { { 142, 284, 70, 44 }, 324 }       /* caixas de peças */
};
#define MOVEL_BANCADA 0
#define MOVEL_REDOMA  1

/* ---------- Falas do cientista ---------- */
static const char *falasApresentacao[] = {
    "Visitantes! Faz tempo. Sou o cientista deste laboratório.\nFaço máquinas que entendem outras máquinas e outros povos.",
    "Aquele aparelho na redoma é um tradutor.\nEle traduz a fala alienígena e estima o tom de voz.",
    "Ele é seu se vencer os três circuitos da bancada.\nProgramar bem é uma forma de se fazer entender.",
    "Na bancada: as setas montam a fila, Enter executa,\nBackspace apaga e H mostra os primeiros passos."
};
static const char *falasEntrega[] = {
    "Os três circuitos, sem um arranhão no robozinho!\nO tradutor é seu. Leve com cuidado.",
    "Ele traduz a fala e lê o tom de voz de quem fala.\nO tom é uma pista, nunca uma certeza: às vezes ele erra."
};
static const char *falasJaTem[] = {
    "O tradutor já está com você.\nProcure o viajante na cidade extraterrestre."
};
static char falaAndamento[160];
static const char *falasAndamento[] = { falaAndamento };
#define TOTAL(vetor) ((int)(sizeof(vetor) / sizeof((vetor)[0])))

/* ---------- Estado ---------- */
static Texture2D sala, frente, bancada, folhaCientista, pecas;
static PixelExploracao *pixelsPiso;
static MapaExploracao piso;
static ExploradorPlaneta explorador;
static bool iniciou = false;

static ModoRobo modo;
static AlvoRobo alvo;
static float tempo;                  /* relógio das animações */
static bool conversou;               /* o cientista já explicou o desafio */
static float tempoAviso;
static const char *aviso;

static const char **falas;           /* conversa em andamento */
static int totalFalas, falaAtual;

static Laboratorio laboratorio;
static int circuito;                 /* circuito atual, de 0 a LAB_TOTAL_CIRCUITOS - 1 */
static int circuitosVencidos;
static bool mostrarDica;
static float tempoPasso;             /* segundos desde o último passo do robozinho */
static ResultadoPasso ultimoPasso;
static int deX, deY;                 /* casa de onde o robozinho saiu no último passo */
static Comando direcaoRobo;          /* para onde ele está virado */
static bool rastro[LAB_LINHAS][LAB_COLUNAS];   /* casas por onde já passou nesta execução */

/* ---------- Carregamento ---------- */

static Texture2D carregarImagem(const char *arquivo)
{
    Texture2D t = LoadTexture(arquivo);
    if (t.id != 0) {
        SetTextureFilter(t, TEXTURE_FILTER_POINT);
    }
    return t;
}

static void carregarPiso(void)
{
    Image imagem = LoadImage(ARQUIVO_PISO);
    if (imagem.data == NULL || imagem.width != LARGURA_VIRTUAL || imagem.height != ALTURA_VIRTUAL) {
        TraceLog(LOG_ERROR, "ROBO: a mascara deve ter 640x360");
        if (imagem.data != NULL) {
            UnloadImage(imagem);
        }
        return;
    }
    Color *cores = LoadImageColors(imagem);
    int quantidade = imagem.width * imagem.height;
    pixelsPiso = malloc((size_t)quantidade * sizeof(*pixelsPiso));
    if (cores != NULL && pixelsPiso != NULL) {
        for (int i = 0; i < quantidade; i++) {
            pixelsPiso[i] = (PixelExploracao){ cores[i].r, cores[i].g, cores[i].b, cores[i].a };
        }
        piso = (MapaExploracao){ pixelsPiso, imagem.width, imagem.height };
    }
    if (cores != NULL) {
        UnloadImageColors(cores);
    }
    UnloadImage(imagem);
}

static void limparRastro(void)
{
    for (int y = 0; y < LAB_LINHAS; y++) {
        for (int x = 0; x < LAB_COLUNAS; x++) {
            rastro[y][x] = false;
        }
    }
}

/* Monta o circuito atual do zero (a contagem de tentativas continua). */
static void montarCircuito(void)
{
    int tentativas = laboratorio.tentativas;
    iniciarLaboratorio(&laboratorio, CIRCUITOS_LAB[circuito]);
    laboratorio.tentativas = tentativas;
    ultimoPasso = PASSO_NADA;
    direcaoRobo = CMD_BAIXO;
    deX = laboratorio.x;
    deY = laboratorio.y;
    tempoPasso = ROBO_TEMPO_PASSO;
    limparRastro();
}

void iniciarTelaRobo(void)
{
    if (iniciou) {
        return;
    }
    sala = carregarImagem(ARQUIVO_SALA);
    frente = carregarImagem(ARQUIVO_FRENTE);
    bancada = carregarImagem(ARQUIVO_BANCADA);
    folhaCientista = carregarImagem(ARQUIVO_CIENTISTA);
    pecas = carregarImagem(ARQUIVO_PECAS);
    carregarPiso();
    iniciarExploradorPlaneta(&explorador, (PontoExploracao){ X_INICIO, Y_INICIO });
    circuito = 0;
    circuitosVencidos = 0;
    laboratorio.tentativas = 0;
    montarCircuito();
    iniciou = true;
}

/* Nova partida: os circuitos e a conversa com o cientista começam do zero. */
void reiniciarTelaRobo(void)
{
    circuito = 0;
    circuitosVencidos = 0;
    laboratorio.tentativas = 0;
    montarCircuito();
    conversou = false;
    mostrarDica = false;
}

void entrarTelaRobo(void)
{
    if (!iniciou) {
        iniciarTelaRobo();
    }
    explorador.pes = (PontoExploracao){ X_INICIO, Y_INICIO };
    explorador.direcao = 1;
    explorador.quadro = 0;
    explorador.tempo = 0;
    explorador.correndo = false;
    modo = ROBO_SALA;
    alvo = ALVO_NENHUM;
    tempoAviso = 0;
}

/* ---------- Funções de apoio ---------- */

static bool confirmou(void)
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E);
}

static float distancia(float ax, float ay, float bx, float by)
{
    return hypotf(ax - bx, ay - by);
}

static void avisar(const char *mensagem)
{
    aviso = mensagem;
    tempoAviso = TEMPO_AVISO;
}

static int larguraTexto(const char *s)
{
    int largura = 0;
    for (const char *c = s; *c != '\0'; c++) {
        if (((unsigned char)*c & 0xC0) != 0x80) {
            largura += 8;
        }
    }
    return largura;
}

/* O que está perto o bastante para usar com E. */
static AlvoRobo encontrarAlvo(void)
{
    float x = explorador.pes.x, y = explorador.pes.y;
    if (distancia(x, y, X_CIENTISTA, Y_CIENTISTA) < 54) {
        return ALVO_CIENTISTA;
    }
    /* Bancada: em frente a ela, em qualquer ponto do comprimento. */
    if (fabsf(x - X_BANCADA) < MEIA_BANCADA + 10 && y > Y_BANCADA && y < Y_BANCADA + 40) {
        return ALVO_BANCADA;
    }
    if (distancia(x, y, X_REDOMA, Y_REDOMA + 14) < 44) {
        return ALVO_REDOMA;
    }
    if (distancia(x, y, X_INICIO, Y_SAIDA - 6) < 28) {
        return ALVO_PORTA;
    }
    return ALVO_NENHUM;
}

/* ---------- Conversa ---------- */

static void comecarConversa(const char **paginas, int total)
{
    falas = paginas;
    totalFalas = total;
    falaAtual = 0;
    modo = ROBO_CONVERSA;
}

static void falarComCientista(void)
{
    ProgressoPartida *jogo = progressoPartida();
    if (jogo->roboConcluido && !jogo->temTradutor) {
        /* Venceu os três circuitos: primeiro a cena da entrega. */
        jogo->temTradutor = true;
        tocarSom(SOM_VITORIA);
        tempo = 0;
        modo = ROBO_PREMIO;
        return;
    }
    if (jogo->temTradutor) {
        comecarConversa(falasJaTem, TOTAL(falasJaTem));
    } else if (!conversou) {
        conversou = true;
        comecarConversa(falasApresentacao, TOTAL(falasApresentacao));
    } else {
        TextCopy(falaAndamento, TextFormat("Você está no circuito %d de %d. A bancada espera.\n"
                                           "Planeje o caminho inteiro antes de executar.",
                                           circuito + 1, LAB_TOTAL_CIRCUITOS));
        comecarConversa(falasAndamento, 1);
    }
}

/* ---------- Desafio ---------- */

static void atualizarDesafio(float dt)
{
    tempoPasso += dt;
    if (laboratorio.fase == ROBO_EXECUTANDO) {
        while (tempoPasso >= ROBO_TEMPO_PASSO && laboratorio.fase == ROBO_EXECUTANDO) {
            tempoPasso -= ROBO_TEMPO_PASSO;
            /* Guarda de onde ele sai e para onde vira, para animar o passo. */
            deX = laboratorio.x;
            deY = laboratorio.y;
            if (!filaVazia(&laboratorio.fila)) {
                direcaoRobo = comandoNaPosicao(&laboratorio.fila, 0);
            }
            rastro[deY][deX] = true;
            ultimoPasso = passoDoRobo(&laboratorio);
            if (ultimoPasso == PASSO_ANDOU) {
                tocarSom(SOM_ROBO_PASSO);
            } else if (ultimoPasso == PASSO_SAIDA) {
                tocarSom(SOM_TERMINAL);
            } else if (ultimoPasso != PASSO_NADA) {
                tocarSom(SOM_NEGADO);
            }
        }
        if (laboratorio.fase == ROBO_VENCEU) {
            if (circuito + 1 > circuitosVencidos) {
                circuitosVencidos = circuito + 1;
            }
            if (circuitosVencidos >= LAB_TOTAL_CIRCUITOS && !progressoPartida()->roboConcluido) {
                progressoPartida()->roboConcluido = true;
                tocarSom(SOM_VITORIA);
            }
        }
        return;
    }

    if (laboratorio.fase == ROBO_VENCEU) {
        /* Enter leva ao próximo circuito, se houver. */
        if (IsKeyPressed(KEY_ENTER) && circuito + 1 < LAB_TOTAL_CIRCUITOS) {
            circuito++;
            montarCircuito();
        } else if (IsKeyPressed(KEY_R)) {
            montarCircuito();
        }
        return;
    }
    if (IsKeyPressed(KEY_R)) {
        montarCircuito();
    }
    if (IsKeyPressed(KEY_H)) {
        mostrarDica = !mostrarDica;
    }
    bool entrou = false;     /* algum comando coube na fila neste quadro */
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) entrou |= adicionarComando(&laboratorio, CMD_CIMA);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) entrou |= adicionarComando(&laboratorio, CMD_BAIXO);
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) entrou |= adicionarComando(&laboratorio, CMD_ESQUERDA);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) entrou |= adicionarComando(&laboratorio, CMD_DIREITA);
    if (entrou) {
        tocarSom(SOM_ROBO_COMANDO);
        if (ultimoPasso != PASSO_NADA) {
            /* Voltou a montar depois de uma falha: o robozinho está no início de novo. */
            ultimoPasso = PASSO_NADA;
            deX = laboratorio.x;
            deY = laboratorio.y;
            direcaoRobo = CMD_BAIXO;
            limparRastro();
        }
    }
    if (IsKeyPressed(KEY_BACKSPACE) && removerUltimoComando(&laboratorio)) {
        tocarSom(SOM_ROBO_APAGA);
    }
    if (IsKeyPressed(KEY_ENTER) && executarPrograma(&laboratorio)) {
        tempoPasso = 0;
        ultimoPasso = PASSO_NADA;
        deX = laboratorio.x;
        deY = laboratorio.y;
        limparRastro();
    }
}

/* ---------- Lógica ---------- */

static Tela usarAlvo(void)
{
    ProgressoPartida *jogo = progressoPartida();
    switch (alvo) {
        case ALVO_CIENTISTA:
            falarComCientista();
            break;
        case ALVO_BANCADA:
            if (conversou || jogo->roboConcluido) {
                modo = ROBO_DESAFIO;
            } else {
                tocarSom(SOM_NEGADO);
                avisar("A bancada está travada. Fale primeiro com o cientista.");
            }
            break;
        case ALVO_REDOMA:
            if (jogo->temTradutor) {
                avisar("A redoma está vazia: o tradutor está com você.");
            } else if (jogo->roboConcluido) {
                avisar("A redoma destravou. Fale com o cientista.");
            } else {
                avisar("O tradutor está trancado. Vença os circuitos da bancada.");
            }
            break;
        case ALVO_PORTA:
            return TELA_NAVE;
        default:
            break;
    }
    return TELA_ROBO;
}

Tela atualizarTelaRobo(void)
{
    float dt = fminf(GetFrameTime(), 0.1f);
    tempo += dt;
    if (tempoAviso > 0) {
        tempoAviso -= dt;
    }

    if (modo == ROBO_CONVERSA) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            modo = ROBO_SALA;
        } else if (confirmou()) {
            falaAtual++;
            if (falaAtual >= totalFalas) {
                modo = ROBO_SALA;
            }
        }
        return TELA_ROBO;
    }
    if (modo == ROBO_PREMIO) {
        /* Uma pausa curta antes de aceitar a tecla, para a cena ser vista. */
        if (tempo > 0.8f && confirmou()) {
            comecarConversa(falasEntrega, TOTAL(falasEntrega));
        }
        return TELA_ROBO;
    }
    if (modo == ROBO_DESAFIO) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            modo = ROBO_SALA;
        } else {
            atualizarDesafio(dt);
        }
        return TELA_ROBO;
    }

    /* ROBO_SALA: o cientista também bloqueia a passagem. */
    ObstaculoExploracao cientista = { X_CIENTISTA - 10, Y_CIENTISTA - 8, 20, 8 };
    atualizarExploradorPlaneta(&explorador, &piso, &cientista, 1, dt, true);
    alvo = encontrarAlvo();
    if (IsKeyPressed(KEY_ESCAPE) || explorador.pes.y >= Y_SAIDA) {
        return TELA_NAVE;
    }
    if (IsKeyPressed(KEY_E)) {
        return usarAlvo();
    }
    return TELA_ROBO;
}

/* ---------- Desenho: laboratório ---------- */

/* O cientista flutua: sobe e desce devagar, e vira o rosto para quem chega perto. */
static void desenharCientista(void)
{
    int coluna = 0;
    float dx = explorador.pes.x - X_CIENTISTA;
    if (distancia(explorador.pes.x, explorador.pes.y, X_CIENTISTA, Y_CIENTISTA) < 160) {
        if (dx > 28) {
            coluna = 2;
        } else if (dx < -28) {
            coluna = 4;
        }
    }
    if (fmodf(tempo, 3.3f) < 0.14f) {
        coluna++;                          /* piscada */
    }
    float flutua = sinf(tempo * 2.2f) * 2;
    DrawEllipse(X_CIENTISTA, Y_CIENTISTA - 2, 11 - flutua, 3, Fade(BLACK, 0.4f));
    DrawEllipse(X_CIENTISTA, Y_CIENTISTA - 2, 7, 2, Fade(COR_CIANO, 0.35f));      /* clarão do propulsor */
    Rectangle origem = { (float)(coluna * LARGURA_PERSONAGEM), 0, LARGURA_PERSONAGEM, ALTURA_PERSONAGEM };
    DrawTextureRec(folhaCientista, origem,
                   (Vector2){ X_CIENTISTA - LARGURA_PERSONAGEM / 2, floorf(Y_CIENTISTA - ALTURA_PERSONAGEM - 4 + flutua) },
                   WHITE);
}

/* Holograma sobre a bancada: o circuito atual em miniatura. */
static void desenharHolograma(void)
{
    int celula = 6;
    float flutua = sinf(tempo * 1.7f) * 2;
    int x0 = X_BANCADA - LAB_COLUNAS * celula / 2;
    int y0 = (int)(128 + flutua);
    float tremor = 0.55f + 0.1f * sinf(tempo * 8);
    /* feixe que sai da base */
    DrawTriangle((Vector2){ X_BANCADA, 176 }, (Vector2){ (float)x0 + LAB_COLUNAS * celula, (float)y0 + LAB_LINHAS * 4 },
                 (Vector2){ (float)x0, (float)y0 + LAB_LINHAS * 4 }, Fade(COR_CIANO, 0.12f));
    for (int y = 0; y < LAB_LINHAS; y++) {
        for (int x = 0; x < LAB_COLUNAS; x++) {
            Casa casa = laboratorio.mapa[y][x];
            Color cor = Fade(COR_CIANO, tremor * 0.35f);
            if (casa == CASA_PAREDE) {
                cor = Fade(COR_CIANO, tremor);
            } else if (casa == CASA_LAVA) {
                cor = Fade(GetColor(0xF2A541FF), tremor);
            } else if (casa == CASA_SAIDA) {
                cor = Fade(WHITE, tremor + 0.2f);
            }
            DrawRectangle(x0 + x * celula, y0 + y * 4, celula - 1, 3, cor);
        }
    }
}

/* O tradutor dentro da redoma, girando devagar a luz em volta. */
static void desenharTradutorNaRedoma(void)
{
    if (progressoPartida()->temTradutor) {
        /* Redoma vazia: só uma luz verde na base. */
        DrawRectangle(X_REDOMA - 12, Y_REDOMA - 2, 24, 2, GetColor(0x6FBF5AFF));
        return;
    }
    float flutua = sinf(tempo * 2.0f) * 2;
    float pulso = 0.5f + 0.5f * sinf(tempo * 3);
    DrawEllipse(X_REDOMA, Y_REDOMA - 40, 12 + pulso * 3, 16 + pulso * 3, Fade(COR_CIANO, 0.10f + 0.10f * pulso));
    desenharTradutor(X_REDOMA - 12, floorf(Y_REDOMA - 54 + flutua), 0.75f, true);
    if (progressoPartida()->roboConcluido && fmodf(tempo, 0.8f) < 0.5f) {
        DrawRectangle(X_REDOMA - 12, Y_REDOMA - 2, 24, 2, GetColor(0x6FBF5AFF));    /* destravada */
    }
}

/* Bolhas subindo nos tanques e a varredura do telão. */
static void desenharVidaDoLaboratorio(void)
{
    static const int tanques[] = { 46, 94 };
    for (int t = 0; t < 2; t++) {
        for (int i = 0; i < 4; i++) {
            float sobe = fmodf(tempo * (0.25f + 0.07f * (float)i) + (float)i * 0.31f + (float)t * 0.5f, 1.0f);
            int x = tanques[t] + 6 + (i * 7 + t * 3) % 24;
            int y = 132 - (int)(sobe * 66);
            DrawRectangle(x, y, 2, 2, Fade(GetColor(0xB8F0A0FF), 0.8f - sobe * 0.5f));
        }
    }
    int varredura = 54 + (int)(fmodf(tempo * 18, 54));
    DrawRectangle(220, varredura, 200, 2, Fade(COR_CIANO, 0.10f));
}

static void desenharMovel(int indice)
{
    Rectangle r = moveis[indice].recorte;
    if (indice == MOVEL_REDOMA) {
        desenharTradutorNaRedoma();       /* dentro do vidro: antes da borda e dos reflexos */
    }
    DrawTextureRec(frente, r, (Vector2){ r.x, r.y }, WHITE);
    if (indice == MOVEL_BANCADA) {
        desenharHolograma();
    }
}

static void desenharSala(void)
{
    if (sala.id != 0) {
        DrawTexture(sala, 0, 0, WHITE);
    }
    desenharVidaDoLaboratorio();

    /* Móveis, cientista e tripulante: quem está mais para baixo na tela
       é desenhado por último, para ficar na frente. */
    typedef struct { float y; int tipo; } Figura;      /* tipo: 0 a 3 móvel, 4 cientista, 5 tripulante */
    Figura figuras[TOTAL_MOVEIS + 2];
    int total = 0;
    for (int i = 0; i < TOTAL_MOVEIS; i++) {
        figuras[total++] = (Figura){ moveis[i].base, i };
    }
    figuras[total++] = (Figura){ Y_CIENTISTA, 4 };
    figuras[total++] = (Figura){ explorador.pes.y, 5 };
    for (int i = 1; i < total; i++) {
        Figura atual = figuras[i];
        int j = i;
        while (j > 0 && figuras[j - 1].y > atual.y) {
            figuras[j] = figuras[j - 1];
            j--;
        }
        figuras[j] = atual;
    }
    for (int i = 0; i < total; i++) {
        if (figuras[i].tipo < TOTAL_MOVEIS) {
            desenharMovel(figuras[i].tipo);
        } else if (figuras[i].tipo == 4) {
            desenharCientista();
        } else {
            desenharExploradorPlaneta(&explorador);
        }
    }
}

static const char *objetivoAtual(void)
{
    ProgressoPartida *jogo = progressoPartida();
    if (jogo->temTradutor) {
        return "o tradutor está com você; volte ao módulo pela porta";
    }
    if (jogo->roboConcluido) {
        return "fale com o cientista para receber o tradutor";
    }
    if (conversou) {
        return "vença os três circuitos na bancada de comandos";
    }
    return "fale com o robô cientista";
}

static const char *dicaDoAlvo(void)
{
    switch (alvo) {
        case ALVO_CIENTISTA: return "E  Conversar com o cientista";
        case ALVO_BANCADA:   return "E  Usar a bancada de comandos";
        case ALVO_REDOMA:    return "E  Olhar a redoma";
        case ALVO_PORTA:     return "E  Voltar à nave";
        default:             return NULL;
    }
}

/* Quadradinhos dos circuitos: cheios para os já vencidos. */
static void desenharProgresso(int x, int y)
{
    for (int i = 0; i < LAB_TOTAL_CIRCUITOS; i++) {
        DrawRectangle(x + i * 12, y, 8, 8, GetColor(0x40334FFF));
        if (i < circuitosVencidos) {
            DrawRectangle(x + i * 12 + 1, y + 1, 6, 6, COR_CIANO);
        }
    }
}

static void desenharInterface(void)
{
    ProgressoPartida *jogo = progressoPartida();
    DrawRectangle(0, 0, LARGURA_VIRTUAL, 34, Fade(GetColor(0x17131FFF), 0.86f));
    textoPlaneta(tripulanteControlado()->nome, 16, 7, 8, COR_TITULO);
    desenharTradutor(92, 1, 0.5f, jogo->temTradutor);
    if (jogo->temTradutor) {
        textoPlaneta("TRADUTOR OBTIDO", 112, 7, 8, COR_CIANO);
    } else {
        textoPlaneta("SEM TRADUTOR", 112, 7, 8, COR_ALERTA);
    }
    textoPlaneta("CIRCUITOS", 440, 7, 8, COR_SUBTITULO);
    desenharProgresso(520, 7);
    textoPlaneta(TextFormat("%d/%d", circuitosVencidos, LAB_TOTAL_CIRCUITOS), 562, 7, 8, COR_MARCADOR);
    textoPlaneta("OBJETIVO", 16, 21, 8, COR_MARCADOR);
    textoPlaneta(objetivoAtual(), 88, 21, 8, COR_MENU);

    DrawRectangle(0, 346, LARGURA_VIRTUAL, 14, GetColor(0x17131FFF));
    textoPlaneta("WASD/SETAS Mover   SHIFT Correr   E Interagir   ESC Nave", 16, 350, 8, COR_MENU);

    if (modo != ROBO_SALA) {
        return;
    }
    const char *dica = dicaDoAlvo();
    if (dica != NULL) {
        int largura = larguraTexto(dica) + 16;
        int x = (int)explorador.pes.x - largura / 2;
        int y = (int)explorador.pes.y + 8;
        if (x < 8) x = 8;
        if (x + largura > LARGURA_VIRTUAL - 8) x = LARGURA_VIRTUAL - 8 - largura;
        if (y > ALTURA_VIRTUAL - 40) {
            y = (int)explorador.pes.y - 96;
        }
        caixaPlaneta(x, y, largura, 22);
        textoPlaneta(dica, (float)(x + 8), (float)(y + 7), 8, COR_MARCADOR);
    }
    if (tempoAviso > 0 && aviso != NULL) {
        int largura = larguraTexto(aviso) + 16;
        int x = (LARGURA_VIRTUAL - largura) / 2;
        caixaPlaneta(x, 42, largura, 22);
        textoPlaneta(aviso, (float)(x + 8), 49, 8, COR_TITULO);
    }
}

/* A entrega: o tradutor grande no meio da tela, com raios de luz girando. */
static void desenharPremio(void)
{
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.78f));
    Vector2 centro = { 320, 150 };
    for (int i = 0; i < 12; i++) {
        float a = tempo * 0.5f + (float)i * (2 * PI / 12);
        float abre = 0.09f;
        Vector2 p1 = { centro.x + cosf(a - abre) * 190, centro.y + sinf(a - abre) * 190 };
        Vector2 p2 = { centro.x + cosf(a + abre) * 190, centro.y + sinf(a + abre) * 190 };
        DrawTriangle(centro, p2, p1, Fade(COR_MARCADOR, i % 2 ? 0.10f : 0.16f));
    }
    float pulso = 0.5f + 0.5f * sinf(tempo * 3);
    DrawCircleV(centro, 58 + pulso * 6, Fade(COR_CIANO, 0.18f));
    DrawCircleV(centro, 40, Fade(COR_CIANO, 0.18f));
    float flutua = sinf(tempo * 2.0f) * 4;
    desenharTradutor(centro.x - 64, floorf(centro.y - 68 + flutua), 4, true);

    const char *titulo = "TRADUTOR OBTIDO";
    textoPlaneta(titulo, 320 - (float)larguraTexto(titulo) + 2, 34, 16, BLACK);
    textoPlaneta(titulo, 320 - (float)larguraTexto(titulo), 32, 16, COR_MARCADOR);
    caixaPlaneta(96, 244, 448, 66);
    textoPlaneta("Traduz a fala dos alienígenas para a sua língua.", 112, 256, 8, COR_TITULO);
    textoPlaneta("Mostra uma leitura do tom de voz, que pode falhar.", 112, 272, 8, COR_MENU);
    if (tempo > 0.8f) {
        textoPlaneta("E / ENTER  Continuar", 112, 292, 8, COR_MARCADOR);
    }
}

/* ---------- Desenho: desafio ---------- */

static void desenharPeca(int linha, int coluna, float x, float y, Color tinta)
{
    Rectangle origem = { (float)(coluna * PECA), (float)(linha * PECA), PECA, PECA };
    DrawTextureRec(pecas, origem, (Vector2){ floorf(x), floorf(y) }, tinta);
}

/* Seta de um comando: o miolo de 20x20 da peça, centrado em (x, y). */
static void desenharSeta(Comando c, float x, float y, Color tinta)
{
    Rectangle origem = { (float)(c * PECA + 10), (float)(2 * PECA + 10), 20, 20 };
    DrawTextureRec(pecas, origem, (Vector2){ floorf(x - 10), floorf(y - 10) }, tinta);
}

static void desenharArena(void)
{
    int animacao = (int)(tempo * 2.5f) % 2;
    for (int y = 0; y < LAB_LINHAS; y++) {
        for (int x = 0; x < LAB_COLUNAS; x++) {
            float px = ARENA_X + (float)x * PECA, py = ARENA_Y + (float)y * PECA;
            Casa casa = laboratorio.mapa[y][x];
            int peca = (x + y) % 2 ? PECA_PISO_B : PECA_PISO_A;
            if (casa == CASA_PAREDE) {
                peca = PECA_PAREDE;
            } else if (casa == CASA_LAVA) {
                peca = PECA_LAVA + animacao;
            } else if (casa == CASA_SAIDA) {
                peca = PECA_SAIDA + animacao;
            } else if (x == laboratorio.inicioX && y == laboratorio.inicioY) {
                peca = PECA_INICIO;
            }
            desenharPeca(0, peca, px, py, WHITE);
            if (casa == CASA_LAVA) {
                /* clarão da lava nas casas vizinhas seria demais: só um brilho nela mesma */
                DrawRectangle((int)px, (int)py, PECA, PECA, Fade(GetColor(0xFFD98AFF), 0.06f + 0.06f * sinf(tempo * 4 + (float)x)));
            }
            if (rastro[y][x] && casa != CASA_LAVA) {
                DrawRectangle((int)px + 17, (int)py + 17, 6, 6, Fade(COR_CIANO, 0.75f));
                DrawRectangle((int)px + 19, (int)py + 19, 2, 2, WHITE);
            }
        }
    }
}

static void desenharRobozinho(void)
{
    /* Posição: desliza da casa de onde saiu até a casa onde está. */
    float t = tempoPasso / ROBO_TEMPO_PASSO;
    if (t > 1) {
        t = 1;
    }
    float x = ARENA_X + ((float)deX + ((float)laboratorio.x - (float)deX) * t) * PECA;
    float y = ARENA_Y + ((float)deY + ((float)laboratorio.y - (float)deY) * t) * PECA;
    /* Na folha, a ordem é baixo, cima, esquerda, direita. */
    static const int linhaDaDirecao[] = { 1, 0, 2, 3 };     /* índice: Comando */
    int coluna = linhaDaDirecao[direcaoRobo] * 2;
    bool andando = laboratorio.fase == ROBO_EXECUTANDO || t < 1;
    if (andando && (int)(tempo * 10) % 2) {
        coluna++;
    }
    Color tinta = WHITE;

    if (ultimoPasso == PASSO_PAREDE) {
        /* Bateu: dá um tranco na direção da parede e fica quebrado. */
        static const int mudaX[] = { 0, 0, -1, 1 }, mudaY[] = { -1, 1, 0, 0 };
        float tranco = sinf(fminf(tempoPasso * 14, PI)) * 7;
        x += (float)mudaX[direcaoRobo] * tranco;
        y += (float)mudaY[direcaoRobo] * tranco;
        if (tempoPasso > 0.25f) {
            coluna = QUADRO_QUEBRADO;
        }
    } else if (ultimoPasso == PASSO_LAVA && t >= 1) {
        /* Afundando na lava: fica alaranjado e desce. */
        float afunda = fminf((tempoPasso - ROBO_TEMPO_PASSO) * 12, 14);
        y += afunda;
        tinta = GetColor(0xFFB070FF);
        coluna = QUADRO_QUEBRADO;
        DrawRectangle((int)x + 8, (int)y + 30, 24, 4, Fade(GetColor(0xFFD98AFF), 0.6f));
    }
    DrawEllipse((int)x + 20, (int)y + 34, 12, 3, Fade(BLACK, 0.35f));
    desenharPeca(1, coluna, x, y, tinta);

    if (ultimoPasso == PASSO_SEM_COMANDOS) {
        /* Parou no meio do caminho: um ponto de interrogação pisca em cima. */
        if (fmodf(tempo, 0.8f) < 0.5f) {
            textoPlaneta("?", x + 16, y - 10, 8, COR_MARCADOR);
        }
    } else if (laboratorio.fase == ROBO_VENCEU) {
        /* Chegou: faíscas em volta. */
        for (int i = 0; i < 6; i++) {
            float a = tempo * 2 + (float)i * (2 * PI / 6);
            DrawRectangle((int)(x + 19 + cosf(a) * 20), (int)(y + 18 + sinf(a) * 20), 2, 2, COR_MARCADOR);
        }
    }
}

static const char *estadoDoDesafio(void)
{
    if (laboratorio.fase == ROBO_VENCEU) {
        if (circuitosVencidos >= LAB_TOTAL_CIRCUITOS && circuito + 1 >= LAB_TOTAL_CIRCUITOS) {
            return "TODOS OS CIRCUITOS VENCIDOS!\nVolte ao cientista para\nreceber o tradutor.";
        }
        return "CIRCUITO VENCIDO!\nO robozinho chegou à saída.";
    }
    if (laboratorio.fase == ROBO_EXECUTANDO) {
        return "Executando a fila...";
    }
    if (ultimoPasso == PASSO_LAVA) {
        return "O robozinho caiu na lava.\nA fila foi apagada:\ntente outro caminho.";
    }
    if (ultimoPasso == PASSO_PAREDE) {
        return "O robozinho bateu na parede.\nA fila foi apagada:\ntente outro caminho.";
    }
    if (ultimoPasso == PASSO_SEM_COMANDOS) {
        return "A fila acabou antes da saída.\nPlaneje o caminho inteiro.";
    }
    return "Monte a fila de comandos\npara levar o robozinho\ndo início até a saída.";
}

static void desenharMonitor(void)
{
    float x = MONITOR_X, y = MONITOR_Y;
    Color titulo = COR_CIANO;
    if (laboratorio.fase == ROBO_VENCEU) {
        titulo = COR_MARCADOR;
    } else if (laboratorio.fase == ROBO_FALHOU) {
        titulo = COR_ALERTA;
    }
    textoPlaneta(estadoDoDesafio(), x, y, 8, titulo);

    textoPlaneta(TextFormat("TENTATIVAS  %d", laboratorio.tentativas), x, y + 46, 8, COR_TITULO);
    textoPlaneta(TextFormat("FILA        %02d/%02d", laboratorio.fila.quantidade, ROBO_MAX_COMANDOS),
                 x, y + 60, 8, COR_TITULO);

    if (laboratorio.fase == ROBO_VENCEU) {
        if (circuito + 1 < LAB_TOTAL_CIRCUITOS) {
            if (fmodf(tempo, 1.0f) < 0.65f) {
                textoPlaneta("ENTER  Próximo circuito", x, y + 86, 8, COR_MARCADOR);
            }
        } else {
            textoPlaneta("ESC  Voltar ao laboratório", x, y + 86, 8, COR_MARCADOR);
        }
        textoPlaneta("R  Jogar este de novo", x, y + 102, 8, COR_MENU);
        return;
    }
    if (mostrarDica) {
        Comando dica[ROBO_PASSOS_DICA];
        int n = caminhoMaisCurto(&laboratorio, dica, ROBO_PASSOS_DICA);
        textoPlaneta("PRIMEIROS PASSOS", x, y + 86, 8, COR_MARCADOR);
        for (int i = 0; i < n && i < ROBO_PASSOS_DICA; i++) {
            desenharSeta(dica[i], x + 12 + (float)i * 26, y + 110, COR_MARCADOR);
        }
        textoPlaneta(TextFormat("de %d", n), x + 116, y + 106, 8, COR_MENU);
    } else {
        textoPlaneta("H  Mostrar os primeiros passos", x, y + 86, 8, COR_MENU);
    }
    textoPlaneta("BACKSPACE  Apagar o último", x, y + 134, 8, COR_MENU);
    textoPlaneta("R  Limpar a fila", x, y + 148, 8, COR_MENU);
    textoPlaneta("Parede e lava apagam a fila.", x, y + 168, 8, COR_SUBTITULO);
}

static void desenharFila(void)
{
    for (int i = 0; i < ROBO_MAX_COMANDOS; i++) {
        float x = FILA_X + (float)i * FILA_PASSO, y = FILA_Y;
        if (i < laboratorio.fila.quantidade) {
            Color tinta = WHITE;
            if (i == 0 && laboratorio.fase == ROBO_EXECUTANDO) {
                /* O próximo a ser cumprido fica destacado. */
                DrawRectangle((int)x, (int)y, 32, 36, Fade(COR_MARCADOR, 0.25f));
                tinta = COR_MARCADOR;
            }
            desenharSeta(comandoNaPosicao(&laboratorio.fila, i), x + 16, y + 18, tinta);
        } else if (i == laboratorio.fila.quantidade && laboratorio.fase != ROBO_EXECUTANDO &&
                   laboratorio.fase != ROBO_VENCEU && fmodf(tempo, 0.8f) < 0.5f) {
            /* Cursor: a casa onde entra o próximo comando. */
            DrawRectangle((int)x + 6, (int)y + 28, 20, 2, COR_CIANO);
        }
    }
}

static void desenharDesafio(void)
{
    if (bancada.id != 0) {
        DrawTexture(bancada, 0, 0, WHITE);
    } else {
        ClearBackground(GetColor(0x17131FFF));
    }
    desenharArena();
    desenharRobozinho();
    desenharMonitor();
    desenharFila();

    DrawRectangle(0, 0, LARGURA_VIRTUAL, 34, Fade(GetColor(0x17131FFF), 0.9f));
    textoPlaneta("BANCADA DE COMANDOS", 16, 9, 16, COR_TITULO);
    textoPlaneta(TextFormat("CIRCUITO %d/%d", circuito + 1, LAB_TOTAL_CIRCUITOS), 392, 13, 8, COR_SUBTITULO);
    desenharProgresso(504, 13);
    desenharTradutor(600, 1, 1, progressoPartida()->temTradutor || progressoPartida()->roboConcluido);

    DrawRectangle(0, 346, LARGURA_VIRTUAL, 14, GetColor(0x17131FFF));
    textoPlaneta("SETAS/WASD Comando   ENTER Executar   ESC Sair da bancada", 16, 350, 8, COR_MENU);
}

void desenharTelaRobo(void)
{
    if (!iniciou) {
        return;
    }
    if (modo == ROBO_DESAFIO) {
        desenharDesafio();
        return;
    }
    ClearBackground(GetColor(0x16121FFF));
    desenharSala();
    desenharInterface();
    if (modo == ROBO_CONVERSA) {
        Falante cientista = { "ROBÔ CIENTISTA", folhaCientista, (Rectangle){ 4, 0, 36, 36 } };
        desenharFala(&cientista, falas[falaAtual],
                     TextFormat("%d/%d   E / ENTER  Continuar", falaAtual + 1, totalFalas),
                     falaAtual < totalFalas - 1);
    } else if (modo == ROBO_PREMIO) {
        desenharPremio();
    }
}

void encerrarTelaRobo(void)
{
    if (!iniciou) {
        return;
    }
    Texture2D texturas[] = { sala, frente, bancada, folhaCientista, pecas };
    for (int i = 0; i < TOTAL(texturas); i++) {
        if (texturas[i].id != 0) {
            UnloadTexture(texturas[i]);
        }
    }
    free(pixelsPiso);
    pixelsPiso = NULL;
    piso = (MapaExploracao){ 0 };
    encerrarExploradorPlaneta(&explorador);
    iniciou = false;
}
