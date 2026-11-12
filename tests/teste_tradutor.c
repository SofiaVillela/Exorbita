/* teste_tradutor.c: testa o tradutor sem abrir o jogo.

   Fica fora de src/ de propósito: o CMake só junta os .c de src/ no jogo.
   Compilar e rodar, a partir da pasta do projeto:
     gcc -std=c99 -Wall -Wextra -Iinclude src/tradutor.c tests/teste_tradutor.c -o build/teste_tradutor
     ./build/teste_tradutor
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tradutor.h"

static int falhas = 0;
#define CONFERIR(cond, msg) do { if (!(cond)) { printf("FALHOU: %s\n", msg); falhas++; } else printf("ok: %s\n", msg); } while (0)

static void testesUnitarios(void)
{
    FalaTraduzida s;
    FalaAlien blefe   = { "Tenho a carta mais alta.", true };
    FalaAlien sincero = { "Tenho a carta mais alta.", false };

    /* Com tradutor, o texto chega igual. */
    traduzir(&blefe, true, 50, 0.0f, &s);
    CONFERIR(strcmp(s.texto, blefe.fala) == 0, "com tradutor o texto chega igual");

    /* As quatro combinações de verdade x acerto. sorteio 0 sempre acerta, 0.99 sempre erra. */
    traduzir(&blefe,   true, 50, 0.0f,  &s); CONFERIR(s.leitura == LEITURA_SUSPEITA, "blefe + acerto = suspeita");
    traduzir(&blefe,   true, 50, 0.99f, &s); CONFERIR(s.leitura == LEITURA_CALMA,    "blefe + erro = calma (blefe passou)");
    traduzir(&sincero, true, 50, 0.0f,  &s); CONFERIR(s.leitura == LEITURA_CALMA,    "sincero + acerto = calma");
    traduzir(&sincero, true, 50, 0.99f, &s); CONFERIR(s.leitura == LEITURA_SUSPEITA, "sincero + erro = suspeita (alarme falso)");

    /* Sem tradutor: sem leitura e sem nenhuma letra legível. */
    traduzir(&blefe, false, 100, 0.0f, &s);
    CONFERIR(s.leitura == LEITURA_NENHUMA, "sem tradutor nao ha leitura");
    int temLetra = 0;
    for (const char *p = s.texto; *p; p++) if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) temLetra = 1;
    CONFERIR(!temLetra, "sem tradutor nenhuma letra legivel sobra");
    CONFERIR(strchr(s.texto, ' ') && s.texto[strlen(s.texto) - 1] == '.', "espacos e pontuacao ficam");
    printf("     cifrado: %s\n", s.texto);

    /* Mesma palavra, com ou sem acento, vira os mesmos símbolos. */
    FalaTraduzida a, b;
    FalaAlien f1 = { "mão", false }, f2 = { "MAO", false };
    traduzir(&f1, false, 0, 0, &a); traduzir(&f2, false, 0, 0, &b);
    CONFERIR(strcmp(a.texto, b.texto) == 0, "acento e maiuscula viram o mesmo simbolo");

    /* Texto maior que o buffer: corta sem quebrar letra e sempre termina em '\0'. */
    char longo[400]; longo[0] = '\0';
    for (int i = 0; i < 60; i++) strcat(longo, "ã");   /* 120 bytes */
    for (int i = 0; i < 60; i++) strcat(longo, "é");   /* +120 bytes */
    FalaAlien fl = { longo, false };
    traduzir(&fl, true, 0, 0, &s);
    size_t n = strlen(s.texto);
    CONFERIR(n < TRADUTOR_TAM_TEXTO, "texto longo cabe no buffer");
    CONFERIR(n % 2 == 0, "texto longo nao corta letra de 2 bytes no meio");
    traduzir(&fl, false, 0, 0, &s);
    CONFERIR(strlen(s.texto) < TRADUTOR_TAM_TEXTO && strlen(s.texto) % 2 == 0, "cifrado longo tambem corta certo");

    /* Fala vazia ou NULL não quebra. */
    FalaAlien vazia = { "", true }, nula = { NULL, true };
    traduzir(&vazia, false, 0, 0, &s); CONFERIR(s.texto[0] == '\0', "fala vazia");
    traduzir(&nula,  true,  0, 0, &s); CONFERIR(s.texto[0] == '\0', "fala NULL");

    /* Inteligência fora de 0-100 é limitada. */
    CONFERIR(precisaoTradutor(-20) == precisaoTradutor(0) && precisaoTradutor(300) == precisaoTradutor(100), "inteligencia limitada a 0-100");
}

/* Simula muitas jogadas e mede: quando o tradutor alerta, quantas vezes era blefe? */
static void simular(float taxaBlefe)
{
    const int N = 200000;
    const int ints[] = { 35, 50, 65, 88 };
    printf("\nTaxa de blefe %.0f%%   (sem tradutor, o jogador so sabe que %.0f%% das falas sao blefe)\n", taxaBlefe * 100, taxaBlefe * 100);
    printf("  inteligencia  precisao  P(blefe | alerta)  P(blefe | calma)  alertas falsos\n");
    for (int k = 0; k < 4; k++) {
        int alertas = 0, alertaCerto = 0, calmas = 0, calmaBlefe = 0;
        for (int i = 0; i < N; i++) {
            FalaAlien f = { "x", (rand() / (float)RAND_MAX) < taxaBlefe };
            FalaTraduzida s;
            traduzir(&f, true, ints[k], rand() / ((float)RAND_MAX + 1), &s);
            if (s.leitura == LEITURA_SUSPEITA) { alertas++; if (f.blefando) alertaCerto++; }
            else { calmas++; if (f.blefando) calmaBlefe++; }
        }
        printf("  %12d  %7.0f%%  %16.0f%%  %15.0f%%  %13.0f%%\n", ints[k], precisaoTradutor(ints[k]) * 100,
               100.0 * alertaCerto / alertas, 100.0 * calmaBlefe / calmas, 100.0 * (alertas - alertaCerto) / alertas);
    }
}

int main(void)
{
    srand(42);
    testesUnitarios();
    simular(0.30f);
    simular(0.40f);
    printf("\n%s (%d falha%s)\n", falhas ? "HA FALHAS" : "TUDO CERTO", falhas, falhas == 1 ? "" : "s");
    return falhas ? 1 : 0;
}
