#include "recursos.h"  
static Font fonte;
static Texture2D tradutor;     /* desenho do aparelho tradutor */

/*  a fonte padrão da raylib não tem acentos (ç, ã, é...). Então usamos a PixelifySans */
void carregarRecursos(void)
{
    /* Quantos caracteres vamos pedir. Primeiro grupo: 126-32+1 = 95.
       Segundo grupo: 255-160+1 = 96. O "+1" é porque os dois extremos
       entram na conta. Total: 191. */
    int quantidade = (126 - 32 + 1) + (255 - 160 + 1);

    /* Lista (vetor) com espaço para os 191 números. Começa vazia. */
    int codepoints[(126 - 32 + 1) + (255 - 160 + 1)];

    /* "n" é a próxima posição livre da lista. */
    int n = 0;

    /* Preenche a lista com 32, 33, 34 ... até 126. */
    for (int c = 32; c <= 126; c++) {
        codepoints[n] = c;   /* guarda o número na posição n */
        n++;                 /* anda uma posição */
    }

    /* Continua a lista com 160, 161 ... até 255. */
    for (int c = 160; c <= 255; c++) {
        codepoints[n] = c;
        n++;
    }

    /* Carrega o arquivo da fonte. Argumentos:
         caminho do arquivo .ttf
         16          tamanho com que a fonte é "assada" em imagem
         codepoints  a lista de caracteres que montamos acima
         quantidade  quantos itens tem a lista  */
    fonte = LoadFontEx("assets/fontes/PressStart2P-Regular.ttf", 8, codepoints, quantidade);

    /* Sem suavização: as letras ficam com pixels nítidos em vez de borradas. */
    SetTextureFilter(fonte.texture, TEXTURE_FILTER_POINT);

    tradutor = LoadTexture("assets/imagens/tradutor.png");
    if (tradutor.id != 0) {
        SetTextureFilter(tradutor, TEXTURE_FILTER_POINT);
    }
}

/* Libera a memória da fonte. Chamada no fim do jogo. */
void descarregarRecursos(void)
{
    UnloadFont(fonte);
    if (tradutor.id != 0) {
        UnloadTexture(tradutor);
    }
}

/* Devolve a fonte para quem pedir (o main.c usa isso para escrever texto). */
Font fontePadrao(void)
{
    return fonte;
}

void desenharTradutor(float x, float y, float escala, bool aceso)
{
    if (tradutor.id == 0) {
        return;
    }
    Color tinta = WHITE;
    if (!aceso) {
        tinta = GetColor(0x4A4460FF);     /* só a silhueta escura: ainda não obtido */
    }
    Rectangle origem = { 0, 0, (float)tradutor.width, (float)tradutor.height };
    Rectangle destino = { x, y, (float)tradutor.width * escala, (float)tradutor.height * escala };
    DrawTexturePro(tradutor, origem, destino, (Vector2){ 0, 0 }, 0, tinta);
}
