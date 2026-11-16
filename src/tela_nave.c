#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "tela_nave.h"
#include "exploracao.h"
#include "mapa_estelar.h"
#include "partida.h"
#include "dialogo.h"
#include "pilha_mochila.h"
#include "sons.h"
#include "tela_final.h"
#include "tanque.h"

#include <math.h>
#include <stdlib.h>

typedef enum { EXPLORANDO, CONVERSANDO, TRIPULACAO, COMUNICACOES, MAPA, FIM } ModoNave;
enum { SEM_ALVO = -1, TERMINAL_EQUIPE = TOTAL_TRIPULANTES,
       TERMINAL_MAPA, TERMINAL_COMUNICACOES, TOTAL_TERMINAIS = 3 };

static Texture2D sala, tripulantes[TOTAL_TRIPULANTES];
static Texture2D folhaItens;   /* de onde vem o desenho do pedaço do mapa */
static PixelExploracao *pixelsPiso;
static MapaExploracao piso;
static ObstaculoExploracao obstaculos[TOTAL_TRIPULANTES];
static Rectangle terminais[TOTAL_TERMINAIS];
static bool pronta;
static PontoExploracao jogador;
static int direcao, quadro, alvo = SEM_ALVO;
static float tempoAnimacao, tempoMarcado, tempoAviso;
static int marcado = SEM_ALVO, inspecionado, interlocutor, pagina;
static ModoNave modo;

static const char *arquivosTripulantes[TOTAL_TRIPULANTES] = {
    "assets/imagens/tripulante_castanho.png",
    "assets/imagens/tripulante_rabo_de_cavalo.png",
    "assets/imagens/tripulante_cacheado.png",
    "assets/imagens/tripulante_ruivo.png",
    "assets/imagens/tripulante_loiro.png",
    "assets/imagens/tripulante_preto.png"
};
/* Onde cada tripulante começa. pesTripulantes muda durante o jogo
   (o Tab deixa cada um onde parou); pesIniciais guarda o começo. */
static const Vector2 pesIniciais[TOTAL_TRIPULANTES] = {
    { 120, 260 }, { 200, 300 }, { 270, 250 },
    { 380, 300 }, { 450, 255 }, { 540, 290 }
};
static Vector2 pesTripulantes[TOTAL_TRIPULANTES] = {
    { 120, 260 }, { 200, 300 }, { 270, 250 },
    { 380, 300 }, { 450, 255 }, { 540, 290 }
};
/* Falas de orientação; a história dos planetas continua independente. */
static const char *falas[TOTAL_TRIPULANTES][2] = {
    { "Bem-vindo à sala do painel.\nAgora você pode explorar a nave.",
      "O terminal da esquerda mostra a tripulação.\nUse-o para localizar alguém na sala." },
    { "Você pode falar com todos a bordo.\nAproxime-se e pressione E.",
      "Enquanto conversamos, seus passos ficam em pausa.\nPressione E ou Enter para continuar." },
    { "O mapa estelar está no terminal central.\nEle reúne os cinco destinos conhecidos.",
      "Use as setas para escolher o destino.\nEnter pousa nos planetas disponíveis." },
    { "Há espaço para passar entre os tripulantes.\nCuidado com a cadeira diante do painel.",
      "Use WASD ou as setas para andar.\nSegure Shift se quiser correr." },
    { "O terminal da direita cuida das comunicações.\nPor enquanto, nenhum canal está conectado.",
      "Você pode consultar seu estado no painel.\nDepois, volte para explorar a sala." },
    { "A nave mantém sua posição enquanto você\nconsulta os terminais.",
      "Esc fecha uma conversa ou um painel.\nNa sala, Esc retorna ao menu inicial." }
};

static void texto(const char *s, float x, float y, int tamanho, Color cor)
{
    DrawTextEx(fontePadrao(), s, (Vector2){ x, y }, (float)tamanho, 0, cor);
}

static void caixa(int x, int y, int largura, int altura)
{
    DrawRectangle(x + 3, y + 3, largura, altura, Fade(BLACK, 0.5f));
    DrawRectangle(x, y, largura, altura, GetColor(0x211C2EFF));
    DrawRectangleLines(x, y, largura, altura, COR_SUBTITULO);
    DrawRectangle(x + 2, y + 2, largura - 4, 1, GetColor(0x514464FF));
}

static int corTerminal(Color cor)
{
    if (cor.r == 0 && cor.g == 255 && cor.b == 0) return 0;
    if (cor.r == 255 && cor.g == 0 && cor.b == 0) return 1;
    if (cor.r == 0 && cor.g == 0 && cor.b == 255) return 2;
    return -1;
}

static bool carregarPiso(void)
{
    Image imagem = LoadImage("assets/mascaras/sala_painel_mascara.png");
    if (!imagem.data) return false;
    if (imagem.width != LARGURA_VIRTUAL || imagem.height != ALTURA_VIRTUAL) {
        TraceLog(LOG_ERROR, "NAVE: a mascara deve acompanhar o cenario de 640x360");
        UnloadImage(imagem);
        return false;
    }
    Color *cores = LoadImageColors(imagem);
    size_t quantidade = (size_t)imagem.width * (size_t)imagem.height;
    pixelsPiso = malloc(quantidade * sizeof(*pixelsPiso));
    if (!cores || !pixelsPiso) {
        if (cores) UnloadImageColors(cores);
        free(pixelsPiso);
        pixelsPiso = NULL;
        UnloadImage(imagem);
        return false;
    }
    int minX[3] = { imagem.width, imagem.width, imagem.width };
    int minY[3] = { imagem.height, imagem.height, imagem.height };
    int maxX[3] = { -1, -1, -1 }, maxY[3] = { -1, -1, -1 };
    for (int y = 0; y < imagem.height; y++) {
        for (int x = 0; x < imagem.width; x++) {
            size_t i = (size_t)y * (size_t)imagem.width + (size_t)x;
            Color cor = cores[i];
            pixelsPiso[i] = (PixelExploracao){ cor.r, cor.g, cor.b, cor.a };
            int terminal = corTerminal(cor);
            if (terminal < 0) continue;
            if (x < minX[terminal]) minX[terminal] = x;
            if (y < minY[terminal]) minY[terminal] = y;
            if (x > maxX[terminal]) maxX[terminal] = x;
            if (y > maxY[terminal]) maxY[terminal] = y;
        }
    }
    for (int i = 0; i < TOTAL_TERMINAIS; i++) {
        terminais[i] = maxX[i] < 0 ? (Rectangle){ 0 } : (Rectangle){
            (float)minX[i], (float)minY[i],
            (float)(maxX[i] - minX[i] + 1), (float)(maxY[i] - minY[i] + 1)
        };
    }
    piso = (MapaExploracao){ pixelsPiso, imagem.width, imagem.height };
    UnloadImageColors(cores);
    UnloadImage(imagem);
    return true;
}

static bool folhaValida(Texture2D folha)
{
    return folha.id != 0 && folha.width >= LARGURA_PERSONAGEM * 5 &&
           folha.height >= ALTURA_PERSONAGEM * 4;
}

void iniciarTelaNave(void)
{
    sala = LoadTexture("assets/imagens/sala_painel_fundo.png");
    folhaItens = LoadTexture("assets/imagens/sem_ar_itens.png");
    if (folhaItens.id) SetTextureFilter(folhaItens, TEXTURE_FILTER_POINT);
    pronta = carregarPiso();
    pronta = pronta && sala.id != 0 && sala.width == LARGURA_VIRTUAL &&
             sala.height == ALTURA_VIRTUAL;
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        tripulantes[i] = LoadTexture(arquivosTripulantes[i]);
        pronta = folhaValida(tripulantes[i]) && pronta;
        obstaculos[i] = (ObstaculoExploracao){
            pesTripulantes[i].x - 11, pesTripulantes[i].y - 8, 22, 8
        };
    }
    int tripControlado = progressoPartida()->controlado;
    jogador = (PontoExploracao){ pesTripulantes[tripControlado].x, pesTripulantes[tripControlado].y };
    obstaculos[tripControlado].largura = 0;   /* não bate em si mesmo */
    pronta = pronta && posicaoLivreExploracao(&piso, jogador, obstaculos, TOTAL_TRIPULANTES);
    if (!pronta) TraceLog(LOG_ERROR, "NAVE: nao foi possivel preparar a exploracao");
    if (sala.id) SetTextureFilter(sala, TEXTURE_FILTER_POINT);
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (tripulantes[i].id) SetTextureFilter(tripulantes[i], TEXTURE_FILTER_POINT);
    }
    direcao = 1;
    modo = EXPLORANDO;
    iniciarMapaEstelar();
}

static int alvoProximo(void)
{
    int controlado = progressoPartida()->controlado;
    int proximo = SEM_ALVO;
    float menor = 46 * 46;
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (i == controlado || !progressoPartida()->tripulacao[i].vivo) {
            continue;
        }
        float dx = jogador.x - pesTripulantes[i].x;
        float dy = jogador.y - pesTripulantes[i].y;
        float distancia = dx * dx + dy * dy;
        if (distancia <= 42 * 42 && distancia < menor) {
            menor = distancia;
            proximo = i;
        }
    }
    for (int i = 0; i < TOTAL_TERMINAIS; i++) {
        Rectangle r = terminais[i];
        if (r.width == 0) continue;
        float dx = jogador.x - fmaxf(r.x, fminf(jogador.x, r.x + r.width));
        float dy = jogador.y - fmaxf(r.y, fminf(jogador.y, r.y + r.height));
        float distancia = dx * dx + dy * dy;
        if (distancia < menor) {
            menor = distancia;
            proximo = TOTAL_TRIPULANTES + i;
        }
    }
    return proximo;
}

static void atualizarPassos(float dt)
{
    float dx = (float)((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) -
                       (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)));
    float dy = (float)((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) -
                       (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)));
    if (dx != 0) direcao = dx < 0 ? 2 : 3;
    else if (dy != 0) direcao = dy < 0 ? 1 : 0;
    float velocidade = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ?
                       VELOCIDADE_CORRIDA : VELOCIDADE_JOGADOR;
    PontoExploracao anterior = jogador;
    jogador = moverExploracao(&piso, jogador, dx, dy, velocidade, dt,
                             obstaculos, TOTAL_TRIPULANTES);
    if (jogador.x != anterior.x || jogador.y != anterior.y) {
        tempoAnimacao += dt * (velocidade / VELOCIDADE_JOGADOR);
        while (tempoAnimacao >= TEMPO_PASSO_PERSONAGEM) {
            tempoAnimacao -= TEMPO_PASSO_PERSONAGEM;
            quadro = quadro % 4 + 1;
            if (quadro == 1 || quadro == 3) tocarPasso(velocidade > VELOCIDADE_JOGADOR);
        }
    } else {
        quadro = 0;
        tempoAnimacao = 0;
    }
}

/* Tab: passa o controle para o próximo tripulante vivo.
   Quem estava sendo controlado fica parado onde está. */
static void trocarControlado(void)
{
    ProgressoPartida *jogo = progressoPartida();
    int atual = jogo->controlado;

    /* 1. quem sai fica onde está e volta a ser obstáculo */
    pesTripulantes[atual] = (Vector2){ jogador.x, jogador.y };
    obstaculos[atual] = (ObstaculoExploracao){ jogador.x - 11, jogador.y - 8, 22, 8 };

    /* 2. procura o próximo vivo, dando a volta no vetor:
          depois do último vem o primeiro */
    int proximo = atual;
    for (int passos = 0; passos < TOTAL_TRIPULANTES; passos++) {
        proximo = proximo + 1;
        if (proximo == TOTAL_TRIPULANTES) {
            proximo = 0;
        }
        if (jogo->tripulacao[proximo].vivo) {
            break;
        }
    }

    /* 3. quem entra deixa de ser obstáculo, e o jogador vai para o lugar dele */
    jogo->controlado = proximo;
    obstaculos[proximo].largura = 0;
    jogador = (PontoExploracao){ pesTripulantes[proximo].x, pesTripulantes[proximo].y };
    quadro = 0;
    direcao = 0;
}

/* Mortos não ficam na sala: não bloqueiam a passagem nem são desenhados.
   Quem está sendo controlado também não é obstáculo para si mesmo. */
static void atualizarObstaculos(void)
{
    ProgressoPartida *jogo = progressoPartida();
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (i == jogo->controlado || !jogo->tripulacao[i].vivo) {
            obstaculos[i].largura = 0;
        } else {
            obstaculos[i] = (ObstaculoExploracao){
                pesTripulantes[i].x - 11, pesTripulantes[i].y - 8, 22, 8
            };
        }
    }
}

void reiniciarTelaNave(void)
{
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        pesTripulantes[i] = pesIniciais[i];
    }
    int controlado = progressoPartida()->controlado;
    jogador = (PontoExploracao){ pesTripulantes[controlado].x, pesTripulantes[controlado].y };
    direcao = 1;
    quadro = 0;
    tempoAnimacao = 0;
    alvo = SEM_ALVO;
    marcado = SEM_ALVO;
    tempoMarcado = 0;
    tempoAviso = 0;
    inspecionado = 0;
    modo = EXPLORANDO;
    atualizarObstaculos();
}

void entrarTelaNave(void)
{
    ProgressoPartida *jogo = progressoPartida();
    int desceu = jogo->controlado;

    /* Quem desceu não voltou (armadilha do Ser): o controle passa
       para o próximo vivo, que aparece onde estava parado. */
    if (!jogo->tripulacao[desceu].vivo) {
        int proximo = -1;
        for (int passos = 1; passos <= TOTAL_TRIPULANTES; passos++) {
            int i = (desceu + passos) % TOTAL_TRIPULANTES;
            if (jogo->tripulacao[i].vivo) {
                proximo = i;
                break;
            }
        }
        if (proximo < 0) {
            modo = FIM;            /* ninguém restou a bordo */
            atualizarObstaculos();
            return;
        }
        jogo->controlado = proximo;
        jogador = (PontoExploracao){ pesTripulantes[proximo].x, pesTripulantes[proximo].y };
        quadro = 0;
        direcao = 0;
    }

    /* Combustível não dá para nenhum planeta liberado: a nave fica
       presa no espaço, e isso também é o final ruim. */
    if (!algumDestinoAlcancavel()) {
        marcarFinalSemCombustivel();
        modo = FIM;
        atualizarObstaculos();
        return;
    }

    modo = EXPLORANDO;
    atualizarObstaculos();
}

Tela atualizarTelaNave(void)
{
    atualizarFundo();
    if (!pronta) return IsKeyPressed(KEY_ESCAPE) ? TELA_INICIO : TELA_NAVE;
    float dt = GetFrameTime();
    if (dt > 0.1f) dt = 0.1f;
    if (tempoMarcado > 0) tempoMarcado -= dt;
    if (tempoAviso > 0) tempoAviso -= dt;
    if (modo == FIM) {
        return TELA_FINAL;   /* ninguém vivo ou sem combustível: final ruim */
    }
    if (modo == MAPA) {
        if (atualizarMapaEstelar()) {
            modo = EXPLORANDO;
            return destinoEscolhidoMapaEstelar();
        }
        return TELA_NAVE;
    }
    if (modo == CONVERSANDO) {
        if (IsKeyPressed(KEY_ESCAPE)) modo = EXPLORANDO;
        else if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER)) {
            if (++pagina >= 2) modo = EXPLORANDO;
        }
        return TELA_NAVE;
    }
    if (modo == TRIPULACAO) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E)) modo = EXPLORANDO;
        else if (IsKeyPressed(KEY_ENTER)) {
            marcado = inspecionado;
            tempoMarcado = 8;
            modo = EXPLORANDO;
        } else {
            int anterior = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A);
            int proximo = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D);
            inspecionado = (inspecionado + proximo - anterior + TOTAL_TRIPULANTES) % TOTAL_TRIPULANTES;
        }
        return TELA_NAVE;
    }
    if (modo == COMUNICACOES) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER))
            modo = EXPLORANDO;
        return TELA_NAVE;
    }
    if (IsKeyPressed(KEY_ESCAPE)) return TELA_INICIO;
    if (IsKeyPressed(KEY_TAB)) {
        trocarControlado();
    }
    atualizarPassos(dt);
    alvo = alvoProximo();
    if (IsKeyPressed(KEY_E)) {
        if (alvo >= 0 && alvo < TOTAL_TRIPULANTES) {
            interlocutor = alvo;
            pagina = 0;
            modo = CONVERSANDO;
        } else if (alvo == TERMINAL_EQUIPE) modo = TRIPULACAO;
        else if (alvo == TERMINAL_MAPA) {
            abrirMapaEstelar();
            modo = MAPA;
        } else if (alvo == TERMINAL_COMUNICACOES) modo = COMUNICACOES;
        else tempoAviso = 2.5f;
    }
    return TELA_NAVE;
}

static void personagem(Texture2D folha, Vector2 pes, int coluna, int linha)
{
    Rectangle origem = { (float)(coluna * LARGURA_PERSONAGEM),
                         (float)(linha * ALTURA_PERSONAGEM),
                         LARGURA_PERSONAGEM, ALTURA_PERSONAGEM };
    Vector2 canto = { floorf(pes.x) - LARGURA_PERSONAGEM / 2, floorf(pes.y) - ALTURA_PERSONAGEM };
    DrawTextureRec(folha, origem, canto, WHITE);
}

static void seta(int x, int y, Color cor)
{
    DrawRectangle(x - 4, y, 8, 2, cor);
    DrawRectangle(x - 3, y + 2, 6, 2, cor);
    DrawRectangle(x - 1, y + 4, 2, 2, cor);
}

static void desenharPessoas(void)
{
    int controlado = progressoPartida()->controlado;
    int ordem[TOTAL_TRIPULANTES + 1];
    float alturas[TOTAL_TRIPULANTES + 1];
    for (int i = 0; i <= TOTAL_TRIPULANTES; i++) {
        ordem[i] = i;
        alturas[i] = i == TOTAL_TRIPULANTES ? jogador.y : pesTripulantes[i].y;
        for (int j = i; j > 0 && alturas[ordem[j]] < alturas[ordem[j - 1]]; j--) {
            int troca = ordem[j]; ordem[j] = ordem[j - 1]; ordem[j - 1] = troca;
        }
    }
    const Tripulante *tripulacao = progressoPartida()->tripulacao;
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (i == controlado || !tripulacao[i].vivo) {
            continue;
        }
        DrawEllipse((int)pesTripulantes[i].x, (int)pesTripulantes[i].y - 3, 13, 4, Fade(BLACK, 0.3f));
    }
    DrawEllipse((int)jogador.x, (int)jogador.y - 3, 13, 4, Fade(BLACK, 0.4f));
    for (int i = 0; i <= TOTAL_TRIPULANTES; i++) {
        int indice = ordem[i];
        if (indice == TOTAL_TRIPULANTES) {
            /* quem eu controlo: anda com a folha dele */
            personagem(tripulantes[controlado], (Vector2){ jogador.x, jogador.y }, quadro, direcao);
        }else if (indice != controlado && tripulacao[indice].vivo) {
            /* os outros, parados de frente */
            personagem(tripulantes[indice], pesTripulantes[indice], 0, 0);
        }
    }
    seta((int)jogador.x, (int)jogador.y - ALTURA_PERSONAGEM - 10, COR_MARCADOR);
    if (marcado >= 0 && tempoMarcado > 0) {
        Vector2 pes = pesTripulantes[marcado];
        seta((int)pes.x, (int)pes.y - ALTURA_PERSONAGEM - 10, GetColor(0x68D8CDFF));
    }
}

/* Canto de baixo, à esquerda: o que a tripulação já trouxe para a nave.
   Em cima, o tradutor (apagado enquanto não foi obtido). Embaixo, os
   pedaços do mapa estelar: um quadradinho para cada, cheio quando veio. */
static void desenharPedacosDoMapa(void)
{
    int obtidos = pedacosMapaObtidos();
    bool temTradutor = progressoPartida()->temTradutor;
    caixa(16, 266, 172, 34);
    desenharTradutor(20, 267, 1, temTradutor);
    texto("TRADUTOR", 56, 273, 8, COR_SUBTITULO);
    if (temTradutor) {
        texto("a bordo", 56, 286, 8, GetColor(0x68D8CDFF));
    } else {
        texto("não obtido", 56, 286, 8, COR_MENU);
    }
    caixa(16, 304, 172, 34);
    if (folhaItens.id) {
        Rectangle origem = { (float)(ITEM_MAPA * 32), 0, 32, 32 };
        DrawTexturePro(folhaItens, origem, (Rectangle){ 20, 305, 32, 32 }, (Vector2){ 0, 0 }, 0, WHITE);
    }
    texto("PEDAÇOS DO MAPA", 56, 311, 8, COR_SUBTITULO);
    for (int i = 0; i < TOTAL_PEDACOS_MAPA; i++) {
        DrawRectangle(56 + i * 12, 324, 8, 8, GetColor(0x40334FFF));
        if (i < obtidos) {
            DrawRectangle(57 + i * 12, 325, 6, 6, COR_MARCADOR);
        }
    }
    texto(TextFormat("%d/%d", obtidos, TOTAL_PEDACOS_MAPA), (float)(60 + TOTAL_PEDACOS_MAPA * 12), 324, 8,
          obtidos == TOTAL_PEDACOS_MAPA ? COR_MARCADOR : COR_TITULO);
}

/* Canto de baixo, à direita: o tanque de combustível em pé, com o
   número ao lado. Fica na mesma altura das caixas da esquerda. */
static void desenharCombustivel(void)
{
    int combustivel = progressoPartida()->combustivel;
    caixa(572, 266, 52, 72);
    desenharTanque(580, 272, 18, 60, combustivel, 0);
    const char *numero = TextFormat("%d", combustivel);
    float largura = MeasureTextEx(fontePadrao(), numero, 8, 0).x;
    texto(numero, 612 - largura / 2, 296, 8, combustivel <= 20 ? COR_ALERTA : COR_TITULO);
}

static void desenharIndicacoes(void)
{
    caixa(16, 12, 184, 34);
    texto("SALA DO PAINEL", 26, 20, 8, COR_TITULO);
    texto("Explore a nave", 26, 33, 8, COR_SUBTITULO);
    caixa(398, 12, 226, 46);
    texto("WASD / SETAS  Mover", 408, 20, 8, COR_MENU);
    texto("SHIFT  Correr TAB Trocar", 408, 33, 8, COR_MENU);
    texto("E  Interagir   ESC  Menu", 408, 46, 8, COR_TITULO);
    if (alvo == SEM_ALVO) {
        if (tempoAviso > 0) {
            caixa(108, 72, 424, 24);
            texto("Aproxime-se de um tripulante ou terminal.", 120, 80, 8, COR_MENU);
        }
        return;
    }
    const char *mensagem;
    Vector2 centro;
    if (alvo < TOTAL_TRIPULANTES) {
        mensagem = "E  Conversar";
        centro = (Vector2){ pesTripulantes[alvo].x, pesTripulantes[alvo].y - 80 };
    } else {
        static const char *nomes[] = { "E  Tripulação", "E  Mapa estelar", "E  Comunicações" };
        Rectangle r = terminais[alvo - TOTAL_TRIPULANTES];
        mensagem = nomes[alvo - TOTAL_TRIPULANTES];
        centro = (Vector2){ r.x + r.width / 2, r.y - 54 };
        DrawRectangle((int)r.x, (int)r.y, (int)r.width, 2, COR_MARCADOR);
    }
    int largura = (int)MeasureTextEx(fontePadrao(), mensagem, 8, 0).x + 16;
    int x = (int)centro.x - largura / 2;
    if (x < 8) x = 8;
    if (x + largura > LARGURA_VIRTUAL - 8) x = LARGURA_VIRTUAL - largura - 8;
    caixa(x, (int)centro.y, largura, 22);
    texto(mensagem, (float)x + 8, centro.y + 7, 8, COR_MARCADOR);
}

static void desenharConversa(void)
{
    /* Rosto: a parte de cima do primeiro quadro da folha (de frente). */
    Falante falante = { progressoPartida()->tripulacao[interlocutor].nome,
                        tripulantes[interlocutor], (Rectangle){ 4, 0, 36, 36 } };
    const char *acao = "Continuar";
    bool temMais = true;
    if (pagina == 1) {
        acao = "Encerrar";
        temMais = false;
    }
    desenharFala(&falante, falas[interlocutor][pagina],
                 TextFormat("%d/2   E / ENTER  %s", pagina + 1, acao), temMais);
}

/* Traço principal de cada um, na mesma ordem de tripulacao.c. */
static const char *tracoPrincipal[TOTAL_TRIPULANTES] = {
    "Impulsivo", "Inteligente", "Curioso", "Carismática", "Sortuda", "Egoísta"
};

/* Escreve centralizado em x. */
static void textoCentrado(const char *s, float x, float y, Color cor)
{
    float largura = MeasureTextEx(fontePadrao(), s, 8, 0).x;
    texto(s, x - largura / 2, y, 8, cor);
}

static void desenharTripulacao(void)
{
    const Tripulante *tripulacao = progressoPartida()->tripulacao;
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.7f));
    caixa(24, 44, 592, 272);
    texto("TRIPULAÇÃO", 44, 60, 16, COR_TITULO);

    int vivos = 0;
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (tripulacao[i].vivo) {
            vivos++;
        }
    }
    texto(TextFormat("%d de %d a bordo", vivos, TOTAL_TRIPULANTES), 44, 84, 8, COR_SUBTITULO);

    /* Retrato, nome e traço de cada um. */
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        int x = 46 + i * 94;
        float centro = (float)x + 35;
        DrawRectangle(x, 104, 70, 84, GetColor(0x30283FFF));
        if (i == inspecionado) DrawRectangleLines(x, 104, 70, 84, COR_MARCADOR);
        if (tripulacao[i].vivo) {
            personagem(tripulantes[i], (Vector2){ centro, 172 }, 0, 0);
        } else {
            DrawTextureRec(tripulantes[i],
                           (Rectangle){ 0, 0, LARGURA_PERSONAGEM, ALTURA_PERSONAGEM },
                           (Vector2){ centro - LARGURA_PERSONAGEM / 2, 172 - ALTURA_PERSONAGEM },
                           Fade(WHITE, 0.25f));
            textoCentrado("MORTO", centro, 138, COR_ALERTA);
        }
        Color corNome = COR_MENU;
        if (i == inspecionado) {
            corNome = COR_MARCADOR;
        }
        textoCentrado(tripulacao[i].nome, centro, 196, corNome);
        textoCentrado(tracoPrincipal[i], centro, 210, GetColor(0x68D8CDFF));
    }

    /* Linha do tripulante escolhido. */
    const Tripulante *t = &tripulacao[inspecionado];
    const char *situacao = ".";
    if (!t->vivo) {
        situacao = ", morto.";
    } else if (inspecionado == progressoPartida()->controlado) {
        situacao = ", é quem desce.";
    }
    texto(TextFormat("%s, %d anos%s", t->nome, t->idade, situacao), 44, 240, 8, COR_TITULO);
    texto("Os traços podem influenciar os resultados.", 44, 256, 8, COR_SUBTITULO);

    texto("SETAS  Escolher   ENTER  Localizar   ESC  Voltar", 44, 290, 8, COR_MENU);
}

static void desenharComunicacoes(void)
{
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.7f));
    caixa(56, 88, 528, 188);
    texto("COMUNICAÇÕES", 80, 110, 16, COR_TITULO);
    DrawRectangle(80, 146, 6, 6, COR_MARCADOR);
    texto("CANAL EM ESPERA", 96, 146, 8, COR_MARCADOR);
    texto(progressoPartida()->temTradutor ?
          "Tradutor a bordo.\n\nEle será usado ao conversar com o visitante.\nA leitura do tom de voz pode falhar." :
          "Nenhuma transmissão recebida.\n\nO cientista recebe visitantes em seu planeta.\nUse o mapa estelar para pousar e conhecê-lo.",
          80, 174, 8, COR_MENU);
    texto("E / ENTER / ESC  Voltar à nave", 80, 250, 8, COR_TITULO);
}

void desenharTelaNave(void)
{
    if (!pronta) {
        texto("Não foi possível carregar a sala.\nESC  Voltar ao menu", 40, 160, 16, COR_TITULO);
        return;
    }
    desenharFundo();
    DrawTexture(sala, 0, 0, WHITE);
    desenharPessoas();
    if (modo == EXPLORANDO) {
        desenharIndicacoes();
        desenharPedacosDoMapa();
        desenharCombustivel();
    }
    if (modo == CONVERSANDO) desenharConversa();
    else if (modo == TRIPULACAO) desenharTripulacao();
    else if (modo == COMUNICACOES) desenharComunicacoes();
    else if (modo == MAPA) desenharMapaEstelar();
}

void encerrarTelaNave(void)
{
    encerrarMapaEstelar();
    if (sala.id) UnloadTexture(sala);
    if (folhaItens.id) UnloadTexture(folhaItens);
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (tripulantes[i].id) UnloadTexture(tripulantes[i]);
    }
    free(pixelsPiso);
    pixelsPiso = NULL;
    piso = (MapaExploracao){ 0 };
    pronta = false;
}
