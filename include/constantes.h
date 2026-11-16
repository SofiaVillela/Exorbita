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

#define Y_MENU           192
#define ESPACO_MENU      26   /* distância entre uma opção e a próxima (cabem 5 opções) */
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
#define GANHO_INTELIGENCIA_SER       15
#define GANHO_SORTE_SER              20
#define ENVELHECER_TUDO              30   /* anos a mais para quem pediu tudo */
#define TRACO_MAXIMO                 100
#define IDADE_REJUVENESCIDA          18
#define ESCALA_SER                   3   /* sprite de 64x100 desenhado 3x: 192x300 */

/* Planeta do gigante (gigante_logica.c).
   Cada infração soma pontos. Com GIGANTE_PONTOS_REACAO o gigante reage
   (o chão treme); com GIGANTE_LIMITE_INFRACOES ele pega o tripulante.
   Sem novas infrações por GIGANTE_TEMPO_CALMA segundos ele começa a se
   acalmar e perdoa 1 ponto a cada GIGANTE_TEMPO_PERDAO segundos. */
#define GIGANTE_LIMITE_INFRACOES   5
#define GIGANTE_PONTOS_REACAO      3
#define GIGANTE_PONTOS_PLANTA      1     /* pisou numa planta brilhante */
#define GIGANTE_PONTOS_HABITANTE   2     /* esbarrou em um habitante */
#define GIGANTE_TEMPO_CALMA        6.0f
#define GIGANTE_TEMPO_PERDAO       8.0f
#define GIGANTE_RAIO_PLANTA        10.0f /* distância dos pés que amassa a planta */
#define GIGANTE_RAIO_HABITANTE     17.0f /* distância dos pés que conta como esbarrão */
#define GIGANTE_TEMPO_ATORDOADO    1.6f  /* habitante parado depois do esbarrão */
#define GIGANTE_TEMPO_PULO         0.42f /* segundos no ar em cada pulo do habitante */
#define GIGANTE_TEMPO_CHAO         0.20f /* pausa no chão entre dois pulos */
#define GIGANTE_TEMPO_TREMOR       2.6f  /* a tela treme quando o gigante reage */
#define GIGANTE_TEMPO_LENTO        3.5f  /* e fica difícil andar por este tempo */
#define GIGANTE_FATOR_LENTO        0.45f
/* Chance de convencer o gigante depois de pego: uma parte do traço, em %. */
#define GIGANTE_APELO_CARISMA      100   /* "Explicar com calma": carisma inteiro */
#define GIGANTE_APELO_INTELIGENCIA 90    /* "As regras não estavam claras" */
#define GIGANTE_APELO_EGOISMO      85    /* "Pôr a culpa em outro" */
/* O posto do planalto só abre com os três cristais de luz. Nos
   destroços de uma sonda há uma célula que devolve à nave o
   combustível gasto para chegar aqui. */
#define GIGANTE_TOTAL_CRISTAIS     3
#define GIGANTE_COMBUSTIVEL_CELULA 20

/* Planeta sem ar (sem_ar_logica.c e pilha_mochila.c).
   O oxigênio do traje vai de OXIGENIO_MAXIMO a 0 e cai o tempo todo. */
#define MOCHILA_CAPACIDADE          4      /* itens que cabem na pilha */
#define SEM_AR_CONSUMO_ANDANDO      1.5f   /* oxigênio por segundo, parado ou andando */
#define SEM_AR_CONSUMO_CORRENDO     2.6f   /* correr chega antes, mas gasta mais */
#define SEM_AR_CONSUMO_JATO         14.0f  /* a mais, dentro de um jato de gás */
#define SEM_AR_RECARGA_CILINDRO     25
#define SEM_AR_OXIGENIO_BAIXO       25     /* daqui para baixo a tela avisa */
#define SEM_AR_CURIOSIDADE_OCULTO   60     /* curiosidade mínima para ver o cilindro escondido */
#define SEM_AR_JATO_QUIETO          2.4f   /* segundos de cada fase do respiradouro */
#define SEM_AR_JATO_AVISO           0.8f
#define SEM_AR_JATO_ATIVO           1.4f
#define SEM_AR_RAIO_JATO            26.0f  /* distância dos pés que o jato alcança */
#define SEM_AR_RAIO_ITEM            18.0f  /* distância para pegar um item */
#define SEM_AR_TEMPO_PORTAO         1.2f   /* segundos para o portão abrir */

/* Tradutor (tradutor.c): chance de a leitura de tom acertar.
   Vai de MIN (inteligência 0) a MAX (inteligência 100), em linha reta. */
#define TRADUTOR_PRECISAO_MIN  0.55f
#define TRADUTOR_PRECISAO_MAX  0.90f

/* Laboratório do robô (robo_logica.c). */
#define ROBO_MAX_COMANDOS  16     /* quantos comandos cabem na fila */
#define ROBO_TEMPO_PASSO   0.3f   /* segundos entre um comando e o próximo */
#define ROBO_PASSOS_DICA   4      /* passos mostrados para tripulante inteligente */

#endif
