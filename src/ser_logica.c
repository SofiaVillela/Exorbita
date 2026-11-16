/* ser_logica.c: regras do planeta do Ser dos desejos (ver ser_logica.h). */

#include <stdio.h>
#include "constantes.h"
#include "ser_logica.h"

/* Vetor fixo com todos os desejos, na ordem em que aparecem na lista. */
static const Desejo desejos[TOTAL_DESEJOS] = {
    { DESEJO_TANQUE,       "Encher o tanque.",
      "Combustível completo. O Ser só dá uma vez." },
    { DESEJO_INTELIGENCIA, "Quero entender mais.",
      "+15 de inteligência." },
    { DESEJO_SORTE,        "Que a sorte me acompanhe.",
      "+20 de sorte." },
    { DESEJO_REJUVENESCER, "Devolva os anos perdidos.",
      "O mais velho a bordo volta a ter 18 anos." },
    { DESEJO_REVIVER,      "Traga de volta quem perdemos.",
      "Um tripulante morto volta à nave." },
    { DESEJO_CAMINHO,      "Mostre o caminho.",
      "Um planeta não visitado conta como visitado." },
    { DESEJO_TERRA,        "Quero ir para a Terra agora.",
      "Só para você. Agora." },
    { DESEJO_TUDO,         "Quero tudo.",
      "Tudo o que o Ser puder dar." }
};

/* Chance base de cada visita: 1ª, 2ª, 3ª, 4ª em diante. */
static const int chancesBase[4] = { CHANCE_SER_1, CHANCE_SER_2, CHANCE_SER_3, CHANCE_SER_4 };

const Desejo *desejoDoTipo(TipoDesejo tipo)
{
    if (tipo < 0 || tipo >= TOTAL_DESEJOS) {
        return &desejos[0];
    }
    return &desejos[tipo];
}

int chanceSer(int visitasAnteriores, int sorte)
{
    if (visitasAnteriores <= 0) {
        return CHANCE_SER_1;   /* primeira visita: garantido */
    }
    int indice = visitasAnteriores;
    if (indice > 3) {
        indice = 3;            /* da 4ª visita em diante, a última chance */
    }
    int chance = chancesBase[indice] + (sorte - 50) / 2;
    if (chance < 0) {
        chance = 0;
    }
    if (chance > 100) {
        chance = 100;
    }
    return chance;
}

bool serApareceu(int chance, int sorteio)
{
    return sorteio <= chance;
}

int primeiroMorto(const Tripulante tripulacao[])
{
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (!tripulacao[i].vivo) {
            return i;
        }
    }
    return -1;
}

/* Índice do tripulante vivo mais velho (no empate, o primeiro), ou -1
   se não há ninguém vivo. Busca do maior em um vetor. */
static int maisVelhoVivo(const Tripulante tripulacao[])
{
    int maisVelho = -1;
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (tripulacao[i].vivo &&
            (maisVelho < 0 || tripulacao[i].idade > tripulacao[maisVelho].idade)) {
            maisVelho = i;
        }
    }
    return maisVelho;
}

/* Soma um ganho a um traço sem passar de TRACO_MAXIMO. */
static int somarTraco(int valor, int ganho)
{
    valor += ganho;
    if (valor > TRACO_MAXIMO) {
        valor = TRACO_MAXIMO;
    }
    return valor;
}

/* Regra de cada desejo: true se o Ser oferece este desejo a quem desceu. */
static bool desejoLiberado(const ProgressoPartida *jogo, int quemDesceu,
                           Planeta *planetas, TipoDesejo tipo)
{
    const Tripulante *t = &jogo->tripulacao[quemDesceu];

    if (tipo == DESEJO_TANQUE) {
        return !jogo->tanqueSerUsado;   /* só uma vez por partida */
    }
    if (tipo == DESEJO_INTELIGENCIA) {
        return t->tracos.inteligencia < TRACO_MAXIMO;
    }
    if (tipo == DESEJO_SORTE) {
        return t->tracos.sorte < TRACO_MAXIMO;
    }
    if (tipo == DESEJO_REJUVENESCER) {
        int maisVelho = maisVelhoVivo(jogo->tripulacao);
        return t->tracos.carisma >= MINIMO_CARISMA_REJUVENESCER &&
               maisVelho >= 0 &&
               jogo->tripulacao[maisVelho].idade > IDADE_REJUVENESCIDA;
    }
    if (tipo == DESEJO_REVIVER) {
        return t->tracos.sorte >= MINIMO_SORTE_REVIVER &&
               primeiroMorto(jogo->tripulacao) >= 0;
    }
    if (tipo == DESEJO_CAMINHO) {
        return t->tracos.curiosidade >= MINIMO_CURIOSIDADE_CAMINHO &&
               primeiroNaoVisitado(planetas) != NULL;
    }
    if (tipo == DESEJO_TERRA) {
        return t->tracos.egoismo >= MINIMO_EGOISMO_ARMADILHA;
    }
    if (tipo == DESEJO_TUDO) {
        return t->tracos.impulsividade >= MINIMO_IMPULSIVIDADE_TUDO;
    }
    return false;
}

int desejosDisponiveis(const ProgressoPartida *jogo, int quemDesceu,
                       Planeta *planetas, TipoDesejo disponiveis[])
{
    int total = 0;
    /* Percorre o vetor fixo e copia só os liberados (filtro). */
    for (int i = 0; i < TOTAL_DESEJOS; i++) {
        if (desejoLiberado(jogo, quemDesceu, planetas, desejos[i].tipo)) {
            disponiveis[total] = desejos[i].tipo;
            total++;
        }
    }
    return total;
}

const char *aplicarDesejo(ProgressoPartida *jogo, int quemDesceu,
                          Planeta *planetas, TipoDesejo tipo)
{
    static char texto[256];
    Tripulante *t = &jogo->tripulacao[quemDesceu];

    if (tipo == DESEJO_TANQUE) {
        jogo->combustivel = COMBUSTIVEL_MAXIMO;
        jogo->tanqueSerUsado = true;
        snprintf(texto, sizeof texto,
                 "A luz desce até a nave.\nO tanque está cheio: combustível %d.",
                 jogo->combustivel);
    } else if (tipo == DESEJO_INTELIGENCIA) {
        t->tracos.inteligencia = somarTraco(t->tracos.inteligencia, GANHO_INTELIGENCIA_SER);
        snprintf(texto, sizeof texto,
                 "O orbe toca a testa de %s.\nTudo parece um pouco mais claro.\nInteligência: %d.",
                 t->nome, t->tracos.inteligencia);
    } else if (tipo == DESEJO_SORTE) {
        t->tracos.sorte = somarTraco(t->tracos.sorte, GANHO_SORTE_SER);
        snprintf(texto, sizeof texto,
                 "Uma faísca dourada fica presa no traje de %s.\nSorte: %d.",
                 t->nome, t->tracos.sorte);
    } else if (tipo == DESEJO_REJUVENESCER) {
        int maisVelho = maisVelhoVivo(jogo->tripulacao);
        if (maisVelho >= 0) {
            Tripulante *velho = &jogo->tripulacao[maisVelho];
            int antes = velho->idade;
            velho->idade = IDADE_REJUVENESCIDA;
            snprintf(texto, sizeof texto,
                     "O manto do Ser alcança a nave.\n%s tinha %d anos. Agora tem %d.",
                     velho->nome, antes, velho->idade);
        } else {
            snprintf(texto, sizeof texto, "Nada acontece.");
        }
    } else if (tipo == DESEJO_REVIVER) {
        int morto = primeiroMorto(jogo->tripulacao);
        if (morto >= 0) {
            jogo->tripulacao[morto].vivo = true;
            jogo->tripulacao[morto].oxigenio = OXIGENIO_MAXIMO;
            snprintf(texto, sizeof texto,
                     "O orbe se apaga por um instante.\n%s respira de novo e volta à nave.",
                     jogo->tripulacao[morto].nome);
        } else {
            snprintf(texto, sizeof texto, "Não há ninguém para trazer de volta.");
        }
    } else if (tipo == DESEJO_CAMINHO) {
        Planeta *p = primeiroNaoVisitado(planetas);
        if (p != NULL) {
            p->visitado = true;
            /* Mesma regra da viagem: abre o que dependia desse planeta. */
            if (atualizarDesbloqueios(planetas, mapaEstelarCompleto()) > 0) {
                snprintf(texto, sizeof texto,
                         "O Ser aponta para o céu.\n%s aparece no mapa como visitado.\n\nNovos caminhos se abrem no mapa.",
                         p->nome);
            } else {
                snprintf(texto, sizeof texto,
                         "O Ser aponta para o céu.\n%s aparece no mapa como visitado.",
                         p->nome);
            }
        } else {
            snprintf(texto, sizeof texto, "Não há mais caminhos para mostrar.");
        }
    } else if (tipo == DESEJO_TERRA) {
        t->vivo = false;
        snprintf(texto, sizeof texto,
                 "A luz puxa %s para cima, rápido demais.\nO santuário some. Só resta o espaço.\n\n%s não voltou para a nave.",
                 t->nome, t->nome);
    } else if (tipo == DESEJO_TUDO) {
        /* O Ser dá o tanque cheio, mas cobra em anos. Quem passa de
           IDADE_MAXIMA não aguenta. */
        jogo->combustivel = COMBUSTIVEL_MAXIMO;
        t->idade += ENVELHECER_TUDO;
        if (t->idade > IDADE_MAXIMA) {
            t->vivo = false;
            snprintf(texto, sizeof texto,
                     "O orbe escurece. \"Tudo tem um preço.\"\nO tanque da nave se enche.\nCom %d anos, o corpo de %s não aguenta.\n\n%s não voltou para a nave.",
                     t->idade, t->nome, t->nome);
        } else {
            snprintf(texto, sizeof texto,
                     "O orbe escurece. \"Tudo tem um preço.\"\nO tanque da nave se enche.\n%s envelhece %d anos de uma vez: agora tem %d.",
                     t->nome, ENVELHECER_TUDO, t->idade);
        }
    } else {
        snprintf(texto, sizeof texto, "Nada acontece.");
    }
    return texto;
}
