/* musica.c: música de fundo e ícone da tecla M (ver musica.h). */

#include <math.h>
#include "raylib.h"
#include "constantes.h"
#include "cenario_planeta.h"
#include "musica.h"

#define ARQUIVO_MUSICA   "assets/musica/tema.qoa"
#define VOLUME_MUSICA    0.7f
#define TOTAL_BARRAS     4
#define TEMPO_AVISO      2.2f

/* Canto de baixo à direita, na mesma faixa das dicas de teclas. */
#define ICONE_X          592
#define ICONE_Y          346
#define ICONE_LARGURA    48
#define ICONE_ALTURA     14

static Music musica;
static bool carregada = false;
static bool silenciada = false;
static float tempo;
static float tempoAviso;
static float fatorAlvo = 1, fatorAgora = 1;   /* ver definirVolumeMusica */
static float barras[TOTAL_BARRAS];   /* altura de cada barrinha, de 0 a 1 */

/* Volume do trecho que acabou de ir para a caixa de som. É escrito pela
   linha de execução do áudio e lido pelo jogo; um float basta aqui,
   porque um valor de um quadro atrasado não faz diferença na tela. */
static volatile float nivelAgora = 0;

/* A raylib chama esta função com cada pedaço do áudio antes de tocar:
   amostras em float, esquerda e direita intercaladas. */
static void medirVolume(void *dados, unsigned int quadros)
{
    const float *amostras = (const float *)dados;
    float soma = 0;
    for (unsigned int i = 0; i < quadros * 2; i++) {
        soma += amostras[i] * amostras[i];
    }
    if (quadros > 0) {
        nivelAgora = sqrtf(soma / (float)(quadros * 2));
    }
}

void iniciarMusica(void)
{
    InitAudioDevice();
    if (!IsAudioDeviceReady() || !FileExists(ARQUIVO_MUSICA)) {
        TraceLog(LOG_WARNING, "MUSICA: sem audio ou sem %s; o jogo segue sem musica", ARQUIVO_MUSICA);
        return;
    }
    musica = LoadMusicStream(ARQUIVO_MUSICA);
    if (musica.frameCount == 0) {
        return;
    }
    musica.looping = true;
    SetMusicVolume(musica, VOLUME_MUSICA);
    AttachAudioStreamProcessor(musica.stream, medirVolume);
    PlayMusicStream(musica);
    carregada = true;
}

void atualizarMusica(void)
{
    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    tempo += dt;
    if (tempoAviso > 0) {
        tempoAviso -= dt;
    }
    if (IsKeyPressed(KEY_M)) {
        silenciada = !silenciada;
        tempoAviso = TEMPO_AVISO;
        /* Pausar em vez de parar: ao voltar, a música segue de onde estava. */
        if (carregada && silenciada) {
            PauseMusicStream(musica);
        } else if (carregada) {
            ResumeMusicStream(musica);
        }
    }
    if (carregada) {
        /* O volume pedido por definirVolumeMusica chega aos poucos. */
        if (fatorAgora != fatorAlvo) {
            float passo = dt * 1.5f;
            if (fatorAgora < fatorAlvo) {
                fatorAgora = fminf(fatorAlvo, fatorAgora + passo);
            } else {
                fatorAgora = fmaxf(fatorAlvo, fatorAgora - passo);
            }
            SetMusicVolume(musica, VOLUME_MUSICA * fatorAgora);
        }
        UpdateMusicStream(musica);
    }

    /* Cada barrinha segue o volume com um balanço próprio, para não
       subirem todas juntas. Sobem rápido e descem devagar. */
    float nivel = 0;
    if (carregada && !silenciada) {
        nivel = nivelAgora * 7 * fatorAgora;     /* a música é baixa: o volume médio fica perto de 0,1 */
        if (nivel > 1) {
            nivel = 1;
        }
    }
    for (int i = 0; i < TOTAL_BARRAS; i++) {
        float balanco = 0.55f + 0.45f * sinf(tempo * (2.1f + 0.9f * (float)i) + (float)i * 1.7f);
        float alvo = nivel * balanco;
        float rapidez = alvo > barras[i] ? 12.0f : 3.0f;
        barras[i] += (alvo - barras[i]) * fminf(1, dt * rapidez);
    }
}

/* Nota musical (colcheia) de 7x10 pixels, com o canto de cima em (x, y). */
static void desenharNota(int x, int y, Color cor)
{
    DrawRectangle(x + 4, y, 1, 8, cor);          /* haste */
    DrawRectangle(x + 5, y + 1, 1, 1, cor);      /* bandeirola */
    DrawRectangle(x + 6, y + 2, 1, 2, cor);
    DrawRectangle(x + 5, y + 4, 1, 1, cor);
    DrawRectangle(x + 1, y + 7, 3, 3, cor);      /* cabeça */
    DrawRectangle(x, y + 8, 1, 1, cor);
}

void desenharMusica(void)
{
    Color fundo = GetColor(0x17131FFF);
    Color ligada = GetColor(0x68D8CDFF);         /* o verde-água dos terminais */
    DrawRectangle(ICONE_X, ICONE_Y, ICONE_LARGURA, ICONE_ALTURA, fundo);
    DrawRectangle(ICONE_X, ICONE_Y, 1, ICONE_ALTURA, GetColor(0x514464FF));

    int x = ICONE_X + 8, y = ICONE_Y + 2;
    if (silenciada || !carregada) {
        desenharNota(x, y, COR_MENU);
        /* risco na diagonal sobre a nota */
        for (int i = 0; i < 9; i++) {
            DrawRectangle(x - 1 + i, y + i, 1, 1, COR_ALERTA);
        }
        textoPlaneta("M", (float)(x + 14), (float)(ICONE_Y + 4), 8, COR_MENU);
    } else {
        desenharNota(x, y, COR_MARCADOR);
        /* ondinhas: barrinhas que acompanham o volume */
        for (int i = 0; i < TOTAL_BARRAS; i++) {
            int altura = 1 + (int)(barras[i] * 9);
            DrawRectangle(x + 12 + i * 5, ICONE_Y + 12 - altura, 3, altura, ligada);
        }
    }

    if (tempoAviso > 0) {
        const char *texto = "MÚSICA LIGADA";
        Color cor = ligada;
        if (silenciada) {
            texto = "MÚSICA SILENCIADA";
            cor = COR_ALERTA;
        }
        int letras = 13;             /* a fonte tem 8 px por letra; "Ú" conta como uma */
        if (silenciada) {
            letras = 17;
        }
        int w = letras * 8 + 28;
        int bx = LARGURA_VIRTUAL - 8 - w, by = ICONE_Y - 28;
        caixaPlaneta(bx, by, w, 22);
        DrawRectangle(bx + 8, by + 8, 6, 6, cor);
        textoPlaneta(texto, (float)(bx + 20), (float)(by + 7), 8, cor);
    }
}

void definirVolumeMusica(float fator)
{
    if (fator < 0) fator = 0;
    if (fator > 1) fator = 1;
    fatorAlvo = fator;
}

void encerrarMusica(void)
{
    if (carregada) {
        DetachAudioStreamProcessor(musica.stream, medirVolume);
        StopMusicStream(musica);
        UnloadMusicStream(musica);
        carregada = false;
    }
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
}
