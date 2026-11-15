/* constantes.h: guarda as constantes do jogo (números que o grupo vai querer
   ajustar). Ficam todas aqui para não ficarem espalhadas pelo código. */

#ifndef CONSTANTES_H
#define CONSTANTES_H

/* Tamanho interno do jogo, em pixels. Tudo é desenhado nesse tamanho
   e depois ampliado para a janela com escala inteira (ver main.c).
   640x360 em escala 2 = 1280x720; em escala 3 = 1920x1080. */
#define LARGURA_VIRTUAL 640
#define ALTURA_VIRTUAL  360
#define ESCALA_INICIAL  2
#define FPS             60

/* ---------- Tela de início ---------- */
#define MARGEM_ESQUERDA     40

/* A fonte (Press Start 2P) tem grade de 8 px: tamanhos múltiplos de 8. */
#define TAMANHO_TITULO      32
#define ESPACAMENTO_TITULO  2
#define Y_TITULO            104

#define TAMANHO_SUBTITULO      16
#define ESPACAMENTO_SUBTITULO  1
#define Y_SUBTITULO            (Y_TITULO + TAMANHO_TITULO + 12)

#define Y_MENU           200
#define ESPACO_MENU      28   /* distância entre uma opção e a próxima */
#define RECUO_MENU       18   /* espaço entre o quadradinho e o texto */
#define TAMANHO_MENU     16
#define TAMANHO_MARCADOR 8

#define COR_FUNDO      GetColor(0x25222BFF)
#define COR_TITULO     GetColor(0xEDE6F2FF)
#define COR_SUBTITULO  GetColor(0x9B8CE0FF)

#define COR_MENU          GetColor(0xB8B0C8FF)
#define COR_SELECIONADO   GetColor(0xFFFFFFFF)
#define COR_MARCADOR      GetColor(0xE8C86AFF)

/* ---------- Fundo do espaço ---------- */
/* Velocidade de cada camada, em pixels por segundo. */
#define VELOCIDADE_NEBULOSA        1.5f
#define VELOCIDADE_ESTRELAS_LONGE  5.0f
#define VELOCIDADE_ESTRELAS_PERTO  11.0f

/* ---------- Animações e planetas ---------- */
#define TEMPO_QUADRO     0.12f   /* segundos por quadro das animações */
#define TAMANHO_PLANETA  96      /* cada quadro do planeta tem 96x96 */
#define QUADROS_PLANETA  96      /* quadros de uma volta completa */
#define X_PLANETA_INICIO 328
#define Y_PLANETA_INICIO 36
#define ESCALA_PLANETA_INICIO 3  /* 96 x 3 = 288 px */

/* ---------- Personagens (astronauta e tripulantes) ---------- */
#define LARGURA_PERSONAGEM 44   /* cada quadro da folha tem 44x58 */
#define ALTURA_PERSONAGEM  58
#define TOTAL_TRIPULANTES  6
#define OXIGENIO_MAXIMO    100  /* oxigênio cheio; em 0 o tripulante morre */
#define VELOCIDADE_JOGADOR 112.0f  /* pixels por segundo */
#define VELOCIDADE_CORRIDA  176.0f
#define TEMPO_PASSO_PERSONAGEM 0.10f

/* ---------- Painel de planetas (mapa_estelar.c) ---------- */
/* Combustível: toda viagem gasta o custo do planeta. A tripulação está
   em outra galáxia, e voltar para casa é voltar à Via Láctea. Por isso,
   depois de qualquer viagem, precisa sobrar o custo da Via Láctea.
   Com estes valores, os 5 planetas (80) mais a Via Láctea (20) usam
   exatamente 100. COMBUSTIVEL_MAXIMO é a capacidade do tanque (o desejo
   "encher o tanque" do Ser leva até ele); com 100, um tanque cheio
   sempre basta para terminar a rota. O inicial pode ser menor. */
#define COMBUSTIVEL_INICIAL    100
#define COMBUSTIVEL_MAXIMO     100
#define CUSTO_SER              10
#define CUSTO_ROBO             15
#define CUSTO_SEM_AR           15
#define CUSTO_EXTRATERRESTRE   20
#define CUSTO_GIGANTE          20
#define CUSTO_VIA_LACTEA       20

/* Desenho do painel. Planetas são folhas de 96x96 (TAMANHO_PLANETA). */
#define ESCALA_PLANETA_PAINEL  2     /* selecionado: 96 x 2 = 192 px */
#define X_CENTRO_PAINEL        320   /* centro do planeta selecionado */
#define Y_CENTRO_PAINEL        160
#define ESPACO_SETA_PAINEL     40    /* distância das setas < > até o planeta */
#define TEMPO_AVISO_PAINEL     2.5f  /* segundos que um aviso fica na tela */
#define MARGEM_PAINEL          24    /* distância do texto até a borda da tela */
#define COR_ALERTA             GetColor(0xEE9A8AFF)  /* pêssego do guia */

/* Planeta do Ser dos desejos (ser_logica.c).
   Chance de o Ser aparecer, em %, conforme a visita: a 1ª é garantida;
   da 2ª em diante soma (sorte - 50) / 2 de quem desceu, entre 0 e 100.
   Da 5ª visita em diante vale a última chance. */
#define CHANCE_SER_1    100
#define CHANCE_SER_2    50
#define CHANCE_SER_3    25
#define CHANCE_SER_4    12
#define MINIMO_CARISMA_REJUVENESCER  50
#define MINIMO_SORTE_REVIVER         55
#define MINIMO_EGOISMO_ARMADILHA     70   /* "Quero ir para a Terra agora" */
#define MINIMO_IMPULSIVIDADE_TUDO    70   /* "Quero tudo" */
#define MINIMO_CURIOSIDADE_CAMINHO   60   /* "Mostre o caminho" */
#define MINIMO_INTELIGENCIA_VOZES    70   /* "Quero entender as outras vozes" */
#define GANHO_INTELIGENCIA_SER       15
#define GANHO_SORTE_SER              20
#define ENVELHECER_TUDO              30   /* anos a mais para quem pediu tudo */
#define TRACO_MAXIMO                 100
#define IDADE_REJUVENESCIDA          18
#define ESCALA_SER                   3   /* sprite de 64x100 desenhado 3x: 192x300 */

/* Tradutor (tradutor.c): chance de a leitura de tom acertar.
   Vai de MIN (inteligência 0) a MAX (inteligência 100), em linha reta. */
#define TRADUTOR_PRECISAO_MIN  0.55f
#define TRADUTOR_PRECISAO_MAX  0.90f

/* Laboratório do robô (robo_logica.c). */
#define ROBO_MAX_COMANDOS  16     /* quantos comandos cabem na fila */
#define ROBO_TEMPO_PASSO   0.3f   /* segundos entre um comando e o próximo */
#define ROBO_PASSOS_DICA   4      /* passos mostrados para tripulante inteligente */

#endif
