/* tanque.h: tanque de combustível desenhado em pixel art.

   Um tanque em pé, com tampa em cima e o combustível em blocos
   empilhados de baixo para cima: cada bloco vale COMBUSTIVEL_POR_BLOCO.
   Quando o combustível gasta, os blocos somem do topo, como uma pilha. */

#ifndef TANQUE_H
#define TANQUE_H

#define COMBUSTIVEL_POR_BLOCO 10

/* Desenha o tanque dentro do retângulo (x, y, largura, altura), tampa
   incluída. "gasto" é quanto uma viagem vai tirar: esses blocos do topo
   piscam em pêssego. Passe 0 para não mostrar previsão. */
void desenharTanque(int x, int y, int largura, int altura, int combustivel, int gasto);

#endif
