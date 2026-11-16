/* fliperama.c: o fliperama do bar do planeta extraterrestre (ver fliperama.h).

   A máquina tem três telas: o título (esperando ESPAÇO), a partida e o
   fim de jogo. O jogo é desenhado dentro do recorte da tela do
   gabinete; o gabinete vem por cima e esconde o que passar da borda,
   como as naves que voltam por cima da tela. */

#include <math.h>
#include <stdint.h>
#include "raylib.h"
#include "constantes.h"
#include "cenario_planeta.h"
#include "fliperama_logica.h"
#include "fliperama.h"
#include "sons.h"

#define ARQUIVO_GABINETE  "assets/imagens/planetas/et_fliperama.png"
#define ARQUIVO_NAVES     "assets/imagens/et_naves.png"

/* Onde a tela do gabinete fica na tela do jogo. */
#define TELA_X            120
#define TELA_Y            40

#define TAMANHO_NAVE      32      /* cada quadro da folha de naves */
#define QUADRO_JOGADOR    0
#define QUADRO_INIMIGO    1       /* batedor 1-2, caça 3-4, comandante 5-6 */
#define QUADRO_EXPLOSAO   7       /* quatro quadros */
#define TOTAL_ESTRELAS    60

#define COR_CIANO_ET      GetColor(0x4FE0D8FF)
#define COR_MAGENTA_ET    GetColor(0xF08CC8FF)
#define COR_LARANJA_ET    GetColor(0xF2A541FF)
#define COR_FUNDO_TELA    GetColor(0x0B0912FF)

typedef enum { MAQUINA_TITULO, MAQUINA_JOGANDO, MAQUINA_FIM } EstadoMaquina;

static Texture2D gabinete, naves;
static bool carregado = false;
static EstadoMaquina estado;
static Fliperama jogo;
static int recorde = 0;          /* maior pontuação desde que o jogo abriu */
static bool bateuRecorde;
static float relogio;            /* para piscar textos e mover estrelas */

/* ---------- carregamento ---------- */

static Texture2D carregarImagem(const char *arquivo)
{
    Texture2D t = LoadTexture(arquivo);
    if (t.id != 0) {
        SetTextureFilter(t, TEXTURE_FILTER_POINT);
    }
    return t;
}

void carregarFliperama(void)
{
    if (carregado) {
        return;
    }
    gabinete = carregarImagem(ARQUIVO_GABINETE);
    naves = carregarImagem(ARQUIVO_NAVES);
    carregado = true;
}

void descarregarFliperama(void)
{
    if (!carregado) {
        return;
    }
    if (gabinete.id != 0) UnloadTexture(gabinete);
    if (naves.id != 0) UnloadTexture(naves);
    carregado = false;
}

void abrirFliperama(void)
{
    estado = MAQUINA_TITULO;
    relogio = 0;
    definirAmbiente(AMBIENTE_FLIPERAMA);    /* a música da máquina */
}

/* ---------- lógica ---------- */

static void comecarPartida(void)
{
    iniciarFliperama(&jogo, (uint32_t)GetRandomValue(1, 1000000));
    bateuRecorde = false;
    tocarSom(SOM_FLIP_INICIO);
    estado = MAQUINA_JOGANDO;
}

bool atualizarTelaFliperama(void)
{
    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    relogio += dt;
    if (IsKeyPressed(KEY_ESCAPE)) {
        return false;
    }

    if (estado == MAQUINA_TITULO) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            comecarPartida();
        }
        return true;
    }
    if (estado == MAQUINA_FIM) {
        atualizarFliperama(&jogo, dt, 0, 0, false);    /* as últimas explosões terminam */
        /* Uma pausa curta, para um tiro segurado não recomeçar sem querer. */
        if (jogo.tempoFase > 1.0f && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))) {
            comecarPartida();
        }
        return true;
    }

    float x = (float)((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)));
    float y = (float)((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) - (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)));
    /* As regras não sabem de som: compara o jogo antes e depois do
       quadro para descobrir o que aconteceu. */
    int tirosAntes = 0, tirosDepois = 0;
    for (int i = 0; i < FLIP_MAX_TIROS; i++) tirosAntes += jogo.tiros[i].ativo;
    int pontosAntes = jogo.pontos, vidasAntes = jogo.vidas;
    FaseFliperama faseAntes = jogo.fase;

    atualizarFliperama(&jogo, dt, x, y, IsKeyDown(KEY_SPACE));

    for (int i = 0; i < FLIP_MAX_TIROS; i++) tirosDepois += jogo.tiros[i].ativo;
    if (tirosDepois > tirosAntes) tocarSomVariado(SOM_FLIP_TIRO, 0.06f);
    if (jogo.pontos > pontosAntes) tocarSomVariado(SOM_FLIP_EXPLOSAO, 0.15f);
    if (jogo.vidas < vidasAntes) tocarSom(SOM_FLIP_ATINGIDO);
    if (jogo.fase == FLIP_ENTRE_ONDAS && faseAntes == FLIP_JOGANDO) {
        tocarSom(SOM_FLIP_ONDA);       /* limpou a onda */
    }
    if (jogo.fase == FLIP_FIM && faseAntes != FLIP_FIM) tocarSom(SOM_FLIP_FIM);
    if (jogo.pontos > recorde) {
        recorde = jogo.pontos;
        bateuRecorde = true;
    }
    if (jogo.fase == FLIP_FIM) {
        estado = MAQUINA_FIM;
    }
    return true;
}

/* ---------- desenho: peças ---------- */

/* Largura de um texto: 8 px por letra no tamanho 8, 16 no tamanho 16.
   Bytes de continuação do UTF-8 não contam como letra. */
static int larguraTexto(const char *s, int tamanho)
{
    int letras = 0;
    for (const char *c = s; *c != '\0'; c++) {
        if (((unsigned char)*c & 0xC0) != 0x80) {
            letras++;
        }
    }
    return letras * tamanho;
}

/* Texto centralizado na tela do fliperama, com sombra. */
static void textoCentro(const char *s, float y, int tamanho, Color cor)
{
    float x = TELA_X + (FLIP_LARGURA - (float)larguraTexto(s, tamanho)) / 2;
    textoPlaneta(s, x + 2, TELA_Y + y + 2, tamanho, GetColor(0x2A1230FF));
    textoPlaneta(s, x, TELA_Y + y, tamanho, cor);
}

static void desenharQuadro(int quadro, float x, float y, float escala, Color tinta)
{
    if (naves.id == 0) {
        DrawRectangle((int)(TELA_X + x - 8), (int)(TELA_Y + y - 8), 16, 16, tinta);
        return;
    }
    float lado = TAMANHO_NAVE * escala;
    Rectangle origem = { (float)(quadro * TAMANHO_NAVE), 0, TAMANHO_NAVE, TAMANHO_NAVE };
    Rectangle destino = { floorf(TELA_X + x - lado / 2), floorf(TELA_Y + y - lado / 2), lado, lado };
    DrawTexturePro(naves, origem, destino, (Vector2){ 0, 0 }, 0, tinta);
}

/* Estrelas em três camadas, descendo em velocidades diferentes: dá a
   sensação de a nave avançar pelo espaço. */
static void desenharEstrelas(void)
{
    static const float velocidade[] = { 14, 30, 60 };
    static const unsigned int cores[] = { 0x3A3452FF, 0x8A84A0FF, 0xF4ECD8FF };
    for (int i = 0; i < TOTAL_ESTRELAS; i++) {
        int camada = i % 3;
        uint32_t semente = (uint32_t)i * 2654435761u;
        float x = (float)(semente % FLIP_LARGURA);
        float y = fmodf((float)((semente >> 9) % FLIP_ALTURA) + relogio * velocidade[camada], FLIP_ALTURA);
        int lado = camada == 2 ? 2 : 1;
        DrawRectangle(TELA_X + (int)x, TELA_Y + (int)y, lado, lado, GetColor(cores[camada]));
    }
}

static void desenharTiros(void)
{
    for (int i = 0; i < FLIP_MAX_TIROS; i++) {
        const TiroFliperama *t = &jogo.tiros[i];
        if (!t->ativo) continue;
        int x = TELA_X + (int)t->x, y = TELA_Y + (int)t->y;
        DrawRectangle(x - 2, y - 6, 4, 12, Fade(COR_CIANO_ET, 0.35f));
        DrawRectangle(x - 1, y - 6, 2, 10, COR_CIANO_ET);
        DrawRectangle(x - 1, y - 6, 2, 3, WHITE);
    }
    for (int i = 0; i < FLIP_MAX_TIROS_INIMIGOS; i++) {
        const TiroFliperama *t = &jogo.tirosInimigos[i];
        if (!t->ativo) continue;
        int x = TELA_X + (int)t->x, y = TELA_Y + (int)t->y;
        /* Bolinha de energia que pisca entre magenta e laranja. */
        Color cor = ((int)(relogio * 12) % 2) ? COR_MAGENTA_ET : COR_LARANJA_ET;
        DrawRectangle(x - 3, y - 3, 6, 6, Fade(cor, 0.35f));
        DrawRectangle(x - 2, y - 2, 4, 4, cor);
        DrawRectangle(x - 1, y - 1, 2, 2, WHITE);
    }
}

static void desenharInimigos(void)
{
    int bater = (int)(relogio * 4) % 2;          /* asas e luzes alternam */
    for (int i = 0; i < jogo.totalInimigos; i++) {
        const InimigoFliperama *e = &jogo.inimigos[i];
        if (!e->vivo) continue;
        Color tinta = WHITE;
        if (e->tipo == 2 && e->vida == 1) {
            tinta = GetColor(0xFFB0A0FF);         /* comandante avariado: mais avermelhado */
        }
        desenharQuadro(QUADRO_INIMIGO + e->tipo * 2 + bater, e->x, e->y, 1, tinta);
    }
}

static void desenharExplosoes(void)
{
    for (int i = 0; i < FLIP_MAX_EXPLOSOES; i++) {
        const ExplosaoFliperama *x = &jogo.explosoes[i];
        if (!x->ativa) continue;
        float duracao = FLIP_TEMPO_EXPLOSAO * (x->grande ? 2 : 1);
        int quadro = (int)(x->tempo / duracao * 4);
        if (quadro > 3) quadro = 3;
        desenharQuadro(QUADRO_EXPLOSAO + quadro, x->x, x->y, x->grande ? 2.0f : 1.25f, WHITE);
    }
}

static void desenharJogador(void)
{
    if (estado == MAQUINA_FIM) {
        return;
    }
    /* Pisca enquanto está protegido, depois de perder uma vida. */
    if (jogo.invencivel > 0 && (int)(relogio * 10) % 2 == 0) {
        return;
    }
    /* Chama do motor, que treme. */
    int chama = 3 + (int)(relogio * 20) % 3;
    int x = TELA_X + (int)jogo.jogadorX, y = TELA_Y + (int)jogo.jogadorY + 14;
    DrawRectangle(x - 2, y, 4, chama, COR_LARANJA_ET);
    DrawRectangle(x - 1, y, 2, chama + 2, GetColor(0xFFD98AFF));
    desenharQuadro(QUADRO_JOGADOR, jogo.jogadorX, jogo.jogadorY, 1, WHITE);
}

/* Placar dentro da tela, como nas máquinas de verdade. */
static void desenharPlacar(void)
{
    textoPlaneta("PONTOS", TELA_X + 8, TELA_Y + 6, 8, COR_MAGENTA_ET);
    textoPlaneta(TextFormat("%06d", jogo.pontos), TELA_X + 8, TELA_Y + 16, 8, WHITE);
    const char *rotulo = "RECORDE";
    textoPlaneta(rotulo, TELA_X + FLIP_LARGURA - 8 - (float)larguraTexto(rotulo, 8), TELA_Y + 6, 8, COR_CIANO_ET);
    textoPlaneta(TextFormat("%06d", recorde), TELA_X + FLIP_LARGURA - 56, TELA_Y + 16, 8, WHITE);

    /* Vidas que sobram, em naves pequenas; a onda, no canto direito. */
    for (int i = 0; i < jogo.vidas - 1; i++) {
        desenharQuadro(QUADRO_JOGADOR, 14 + (float)i * 18, FLIP_ALTURA - 12, 0.5f, WHITE);
    }
    const char *onda = TextFormat("ONDA %d", jogo.onda);
    textoPlaneta(onda, TELA_X + FLIP_LARGURA - 8 - (float)larguraTexto(onda, 8), TELA_Y + FLIP_ALTURA - 16, 8,
                 COR_LARANJA_ET);
}

static void desenharTitulo(void)
{
    textoCentro("PATRULHA", 46, 16, COR_CIANO_ET);
    textoCentro("ESTELAR", 68, 16, COR_MAGENTA_ET);
    /* Tabela de pontos, com as naves animadas. */
    int bater = (int)(relogio * 4) % 2;
    static const char *valores[] = { "10", "20", "50" };
    for (int tipo = 0; tipo < 3; tipo++) {
        float y = 120 + (float)tipo * 28;
        desenharQuadro(QUADRO_INIMIGO + tipo * 2 + bater, 150, y, 1, WHITE);
        textoPlaneta(TextFormat("= %s PONTOS", valores[tipo]), TELA_X + 172, TELA_Y + y - 4, 8, COR_MENU);
    }
    textoCentro("mergulhando valem o dobro", 206, 8, COR_SUBTITULO);
    if (fmodf(relogio, 1.0f) < 0.65f) {
        textoCentro("ESPAÇO para começar", 240, 8, COR_MARCADOR);
    }
    if (recorde > 0) {
        textoCentro(TextFormat("RECORDE %06d", recorde), 270, 8, COR_CIANO_ET);
    }
}

static void desenharFim(void)
{
    DrawRectangle(TELA_X, TELA_Y + 96, FLIP_LARGURA, 100, Fade(COR_FUNDO_TELA, 0.75f));
    textoCentro("FIM DE JOGO", 108, 16, COR_ALERTA);
    textoCentro(TextFormat("PONTOS %06d", jogo.pontos), 138, 8, WHITE);
    if (bateuRecorde && fmodf(relogio, 0.6f) < 0.4f) {
        textoCentro("NOVO RECORDE!", 154, 8, COR_MARCADOR);
    }
    if (jogo.tempoFase > 1.0f) {
        textoCentro("ESPAÇO jogar de novo", 176, 8, COR_MENU);
    }
}

/* Linhas escuras e brilho nas bordas: parece um monitor de tubo. */
static void desenharEfeitoDeTubo(void)
{
    for (int y = 0; y < FLIP_ALTURA; y += 2) {
        DrawRectangle(TELA_X, TELA_Y + y, FLIP_LARGURA, 1, Fade(BLACK, 0.22f));
    }
    for (int k = 0; k < 6; k++) {
        Color sombra = Fade(BLACK, 0.10f);
        DrawRectangleLines(TELA_X + k, TELA_Y + k, FLIP_LARGURA - 2 * k, FLIP_ALTURA - 2 * k, sombra);
    }
    /* Uma faixa clara que desce devagar, como a varredura do tubo. */
    float faixa = fmodf(relogio * 40, FLIP_ALTURA + 40) - 20;
    DrawRectangle(TELA_X, TELA_Y + (int)faixa, FLIP_LARGURA, 6, Fade(WHITE, 0.025f));
}

/* Laterais do gabinete: os controles à esquerda, a máquina à direita. */
static void desenharLaterais(void)
{
    caixaPlaneta(4, 120, 76, 112);
    textoPlaneta("TECLAS", 12, 128, 8, COR_SUBTITULO);
    textoPlaneta("WASD", 12, 146, 8, COR_MARCADOR);
    textoPlaneta("mover", 12, 156, 8, COR_MENU);
    textoPlaneta("ESPAÇO", 12, 174, 8, COR_MARCADOR);
    textoPlaneta("atirar", 12, 184, 8, COR_MENU);
    textoPlaneta("ESC", 12, 202, 8, COR_MARCADOR);
    textoPlaneta("sair", 12, 212, 8, COR_MENU);

    caixaPlaneta(560, 120, 76, 70);
    textoPlaneta("FICHA", 568, 128, 8, COR_SUBTITULO);
    textoPlaneta("grátis", 568, 146, 8, COR_MENU);
    textoPlaneta("para", 568, 156, 8, COR_MENU);
    textoPlaneta("visitas", 568, 166, 8, COR_MENU);
}

void desenharTelaFliperama(void)
{
    ClearBackground(GetColor(0x1A1422FF));
    DrawRectangle(TELA_X, TELA_Y, FLIP_LARGURA, FLIP_ALTURA, COR_FUNDO_TELA);
    desenharEstrelas();

    if (estado == MAQUINA_TITULO) {
        desenharTitulo();
    } else {
        desenharInimigos();
        desenharTiros();
        desenharJogador();
        desenharExplosoes();
        desenharPlacar();
        if (jogo.fase == FLIP_ENTRE_ONDAS && fmodf(relogio, 0.5f) < 0.35f) {
            textoCentro(TextFormat("ONDA %d", jogo.onda + 1), 130, 16, COR_LARANJA_ET);
        }
        if (estado == MAQUINA_FIM) {
            desenharFim();
        }
    }
    desenharEfeitoDeTubo();

    /* O gabinete por cima esconde o que passar da borda da tela. */
    if (gabinete.id != 0) {
        DrawTexture(gabinete, 0, 0, WHITE);
    }
    desenharLaterais();
}
