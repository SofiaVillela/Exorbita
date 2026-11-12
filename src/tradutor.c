/* tradutor.c: ver tradutor.h. */

#include <string.h>
#include "constantes.h"
#include "tradutor.h"

/* ---------- UTF-8 ----------
   Uma letra como "ã" ocupa 2 bytes. Se o texto fosse tratado byte a byte,
   o "ã" seria cortado no meio e viraria lixo na tela. Por isso o texto é
   lido e escrito por codepoint (o número da letra na tabela Unicode). */

/* Lê um codepoint a partir de s e diz em *tam quantos bytes ele ocupou.
   Byte inválido vira '?' e ocupa 1 byte, para o laço nunca travar. */
static int lerCodepoint(const unsigned char *s, int *tam)
{
    if (s[0] < 0x80) { *tam = 1; return s[0]; }
    if ((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
        *tam = 2; return ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
    }
    if ((s[0] & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
        *tam = 3; return ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
    }
    if ((s[0] & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
        *tam = 4; return ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
    }
    *tam = 1; return '?';
}

/* Escreve o codepoint cp no fim de saida, se couber (deixando lugar para o '\0').
   Devolve false se não coube: aí o texto para ali, sem cortar letra no meio. */
static bool escreverCodepoint(char *saida, int *usados, int tamanho, int cp)
{
    unsigned char b[4];
    int n;
    if (cp < 0x80)         { b[0] = cp; n = 1; }
    else if (cp < 0x800)   { b[0] = 0xC0 | (cp >> 6);  b[1] = 0x80 | (cp & 0x3F); n = 2; }
    else if (cp < 0x10000) { b[0] = 0xE0 | (cp >> 12); b[1] = 0x80 | ((cp >> 6) & 0x3F); b[2] = 0x80 | (cp & 0x3F); n = 3; }
    else                   { b[0] = 0xF0 | (cp >> 18); b[1] = 0x80 | ((cp >> 12) & 0x3F); b[2] = 0x80 | ((cp >> 6) & 0x3F); b[3] = 0x80 | (cp & 0x3F); n = 4; }

    if (*usados + n + 1 > tamanho) return false;
    memcpy(saida + *usados, b, n);
    *usados += n;
    saida[*usados] = '\0';
    return true;
}

/* ---------- alfabeto alien ----------
   Cada letra vira sempre o mesmo símbolo. Assim a mesma palavra sempre
   tem a mesma "cara", e o jogador atento pode reconhecer padrões.
   Os símbolos são os codepoints 0xA1 a 0xBF (¡ ¢ £ ¤ ¥ ...), que a fonte
   já carrega (recursos.c pede 160-255). 0xAD fica de fora: é o hífen
   invisível e não apareceria na tela. */
static const int SIMBOLOS[] = {
    0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,     0xAE,0xAF,
    0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF
};
#define TOTAL_SIMBOLOS ((int)(sizeof SIMBOLOS / sizeof SIMBOLOS[0]))   /* 30 */

/* Letra base de cada codepoint de 0xC0 a 0xFF ('*' = não é letra: × e ÷).
   Assim "ã", "á" e "a" viram o mesmo símbolo. */
static const char BASE_LATIN1[] =
    "aaaaaaaceeeeiiiidnooooo*ouuuuyts"    /* 0xC0 - 0xDF */
    "aaaaaaaceeeeiiiidnooooo*ouuuuyty";   /* 0xE0 - 0xFF */

/* Devolve a letra base (a-z) ou 0 se cp não for letra. */
static char letraBase(int cp)
{
    if (cp >= 'a' && cp <= 'z') return (char)cp;
    if (cp >= 'A' && cp <= 'Z') return (char)(cp - 'A' + 'a');
    if (cp >= 0xC0 && cp <= 0xFF && BASE_LATIN1[cp - 0xC0] != '*') return BASE_LATIN1[cp - 0xC0];
    return 0;
}

/* Multiplicar por 7 e tirar o resto por 30 embaralha as letras sem repetir
   símbolo (7 e 30 não têm divisor comum), então a -> 0, b -> 7, c -> 14 ... */
static int simboloDe(char letra)
{
    return SIMBOLOS[((letra - 'a') * 7) % TOTAL_SIMBOLOS];
}

/* Copia o texto (traduzido ou cifrado) codepoint a codepoint. */
static void converter(const char *fala, bool cifrar, char *saida, int tamanho)
{
    const unsigned char *p = (const unsigned char *)(fala ? fala : "");
    int usados = 0;
    saida[0] = '\0';

    while (*p) {
        int tam;
        int cp = lerCodepoint(p, &tam);
        p += tam;

        if (cifrar) {
            char base = letraBase(cp);
            if (base) cp = simboloDe(base);
            /* espaço, pontuação e números ficam iguais: a frase mantém o formato */
        }
        if (!escreverCodepoint(saida, &usados, tamanho, cp)) break;
    }
}

/* ---------- leitura de tom ---------- */

float precisaoTradutor(int inteligencia)
{
    if (inteligencia < 0) inteligencia = 0;
    if (inteligencia > 100) inteligencia = 100;
    return TRADUTOR_PRECISAO_MIN
         + (TRADUTOR_PRECISAO_MAX - TRADUTOR_PRECISAO_MIN) * inteligencia / 100.0f;
}

void traduzir(const FalaAlien *f, bool temTradutor, int inteligencia,
              float sorteio, FalaTraduzida *saida)
{
    converter(f->fala, !temTradutor, saida->texto, TRADUTOR_TAM_TEXTO);

    if (!temTradutor) {
        saida->leitura = LEITURA_NENHUMA;
        return;
    }

    /* Acertou: mostra a verdade. Errou: mostra o contrário.
         blefando + acertou -> SUSPEITA   (acerto)
         blefando + errou   -> CALMA      (blefe passou)
         sincero  + acertou -> CALMA      (acerto)
         sincero  + errou   -> SUSPEITA   (alarme falso) */
    bool acertou = sorteio < precisaoTradutor(inteligencia);
    bool pareceBlefe = acertou ? f->blefando : !f->blefando;
    saida->leitura = pareceBlefe ? LEITURA_SUSPEITA : LEITURA_CALMA;
}

const char *textoLeitura(Leitura l)
{
    /* Sem travessão (—): ele é U+2014 e a fonte não tem esse caractere. */
    switch (l) {
        case LEITURA_CALMA:    return "tom de voz: firme";
        case LEITURA_SUSPEITA: return "tom de voz: instável - possível blefe";
        default:               return "";
    }
}
