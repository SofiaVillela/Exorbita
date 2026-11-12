/* constante.h: guarda as constantes do jogo (números que o grupo vai querer
   ajustar). Ficam todas aqui para não ficarem espalhadas pelo código. */

#ifndef CONSTANTES_H    
#define CONSTANTES_H

/* Tamanho da janela, em pixels. Desenha nesse tamanho pequeno e
   depois estica (ver main.c). */
#define LARGURA_VIRTUAL 384
#define ALTURA_VIRTUAL  224
#define ESCALA_INICIAL  3
#define FPS            60

#define MARGEM_ESQUERDA     16

#define TAMANHO_TITULO      16
#define ESPACAMENTO_TITULO  1
#define Y_TITULO            56

#define TAMANHO_SUBTITULO      8
#define ESPACAMENTO_SUBTITULO  0
#define Y_SUBTITULO            (Y_TITULO + TAMANHO_TITULO + 8)

#define COR_FUNDO      GetColor(0x25222BFF)
#define COR_TITULO     GetColor(0xEDE6F2FF)
#define COR_SUBTITULO  GetColor(0x9B8CE0FF)

#define Y_MENU           104
#define ESPACO_MENU      16   /* distância entre uma opção e a próxima */
#define RECUO_MENU       10   /* espaço entre o quadradinho e o texto */
#define TAMANHO_MENU     8
#define TAMANHO_MARCADOR 4

#define COR_MENU          GetColor(0xB8B0C8FF)
#define COR_SELECIONADO   GetColor(0xFFFFFFFF)
#define COR_MARCADOR      GetColor(0xE8C86AFF)
#define NUM_ESTRELAS  90

#define COR_ESTRELA_FRACA  GetColor(0x5A5468FF)
#define COR_ESTRELA_MEDIA  GetColor(0x9C95ADFF)
#define COR_ESTRELA_FORTE  GetColor(0xEDE6F2FF)

/* Velocidade de cada camada do fundo, em pixels por segundo. */
#define VELOCIDADE_NEBULOSA        1.0f
#define VELOCIDADE_ESTRELAS_LONGE  3.0f
#define VELOCIDADE_ESTRELAS_PERTO  7.0f

#define TEMPO_QUADRO     0.12f   /* segundos por quadro das animações */
#define TAMANHO_PLANETA  96      /* cada quadro do planeta tem 96x96 */
#define QUADROS_PLANETA  96      /* quadros de uma volta completa */
#define X_PLANETA_INICIO 176
#define Y_PLANETA_INICIO 16
#define ESCALA_PLANETA_INICIO 2

/* Tradutor (tradutor.c): chance de a leitura de tom acertar.
   Vai de MIN (inteligência 0) a MAX (inteligência 100), em linha reta. */
#define TRADUTOR_PRECISAO_MIN  0.55f
#define TRADUTOR_PRECISAO_MAX  0.90f

/* Laboratório do robô (robo_logica.c). */
#define ROBO_MAX_COMANDOS  16     /* quantos comandos cabem na fila */
#define ROBO_TEMPO_PASSO   0.3f   /* segundos entre um comando e o próximo */
#define ROBO_PASSOS_DICA   4      /* passos mostrados para tripulante inteligente */

#endif
