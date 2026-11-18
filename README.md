# Exórbita

Jogo de exploração espacial em C e raylib. Esta versão inclui uma nave explorável e cinco destinos jogáveis: os planetas do Ser, do robô, sem ar, extraterrestre e do gigante.

## Controles gerais

| Teclas | Ação |
|---|---|
| WASD ou setas | Andar e navegar pelos destinos |
| Shift | Correr nos cenários |
| E | Interagir com personagens, consoles, terminais, marco ou portas |
| Enter | Confirmar, avançar diálogos ou pousar no destino selecionado |
| Esc | Voltar/fechar a tela ou retornar à nave, conforme o contexto |
| M | Silenciar ou voltar a tocar a música (em qualquer tela) |

Na nave, aproxime-se do painel central para abrir o mapa estelar. Escolha um planeta e pressione Enter para pousar. A Via Láctea só abre depois que os outros forem visitados.

## O que já está implementado

### Música

- Uma música calma de ficção espacial toca em laço o jogo inteiro: órgão de tubos, uma figura de duas notas que lembra um relógio e um eco longo.
- O canto de baixo à direita mostra uma nota musical com barrinhas que acompanham o volume da música. Com M, a nota fica riscada e um aviso confirma que a música foi silenciada; M de novo retoma de onde parou.

### Sons

- Cada lugar tem um som de ambiente, que troca aos poucos: o motor e os consoles da nave, a cidade à noite, as vozes do bar, a respiração dentro do traje no planeta sem ar, o vento e os bichos do planeta do gigante, as notas de vidro do santuário do Ser e os computadores do laboratório.
- Os passos mudam conforme o piso: metal, poeira, grama, asfalto, madeira e o chão do santuário.
- Descer num planeta toca a escotilha, o motor e o pouso do módulo; voltar toca a decolagem. A porta do bar range e o portão da estação se arrasta.
- No planeta do gigante, um zumbido com batidas fica mais forte e mais rápido quanto mais furioso ele está, e some quando ele se acalma. Há sons para a planta pisada, o habitante assustado, a reação, a captura e o perdão.
- Também têm som: itens, oxigênio, vazamento e alarme do traje, o Ser aparecendo, os comandos do robô, as cartas, as falas dos alienígenas, vitórias e derrotas.
- O fliperama tem música e efeitos próprios, no estilo das máquinas antigas; enquanto ele está ligado, a música do jogo se cala.
- A tecla M silencia só a música. Os sons continuam.

### Nave

- Exploração em visão superior com movimento em oito direções, corrida e colisão com paredes e objetos.
- Interação com tripulantes, painel de comunicações, terminal da equipe e mapa estelar.
- Diálogos com avanço por tecla e pausa do movimento durante a conversa.
- Cinco planetas no mapa, todos visitáveis, mais a Via Láctea.

### Planeta do robô e tradutor

- Um laboratório com tanques, telão e a bancada de comandos. O robô cientista flutua perto de uma redoma de vidro, onde o tradutor fica à mostra.
- Fale com o cientista para destravar a bancada. Nela há três circuitos, do mais simples ao mais difícil: o robozinho precisa ir do início à saída sem bater em parede e sem cair na lava.
- Setas/WASD colocam comandos na fila, Backspace apaga o último, Enter executa, R limpa e H mostra os primeiros passos. O robozinho deixa um rastro por onde passa; parede e lava apagam a fila.
- Vencidos os três circuitos, volte ao cientista: uma cena mostra a entrega do tradutor.
- O tradutor aparece no alto da tela nos planetas e na caixa de carga da nave: apagado enquanto não foi obtido, aceso depois.
- Ele permite compreender os alienígenas; a leitura do tom de voz pode falhar.
- Depois de concluir os circuitos, use E na bancada para abrir a Corrida dos Algoritmos: aposte em Bubble Sort ou Insertion Sort, acompanhe as comparações e movimentos na mesma sequência e tente acumular acertos e medalhas. Espaço pausa/retoma a corrida; N avança um passo quando pausada.
- E na porta (ou Esc) volta à nave.

### Planeta extraterrestre

- Cidade à noite, maior que a tela: duas ruas, um beco, uma praça com holograma e o pátio onde o módulo pousa. A câmera acompanha o tripulante, e um marcador na borda mostra onde ficou o módulo.
- Na rua de baixo há um morador. Sem tradutor, a fala dele é só símbolos. Com tradutor, dá para cumprimentar e perguntar sobre o viajante.
- O viajante (Claude) fica na porta do bar, na rua de cima, e carrega um pedaço do mapa. Com ou sem tradutor, ele chama para dentro do bar; dá para aceitar ou recusar.
- Dentro do bar ficam o balcão, um freguês e a mesa de cartas. Use E na mesa para ver as regras e começar.
- Blefe Orbital: o viajante anuncia o valor de uma carta, que pode ser verdade ou mentira. Escolha uma carta com A/D, setas ou 1/2/3. Use W/S ou cima/baixo para escolher **Comparar** ou **Desconfiar**; Enter confirma.
- Comparar ganha se sua carta real for maior. Desconfiar ganha se o anúncio for falso.
- Cada lado tem três cartas. A partida termina quando alguém chega a dois pontos.
- Vale uma partida por pouso. Quem vence leva o pedaço do mapa; quem perde só tenta de novo pousando outra vez. Esc duas vezes no meio da partida é desistir, e conta como derrota.
- Depois de ganhar o mapa, as partidas seguintes são amistosas.
- No canto do bar há um fliperama: E na frente dele liga a máquina. Patrulha Estelar é um jogo de naves: WASD move, Espaço atira e Esc volta ao bar. As naves inimigas balançam em formação e algumas mergulham atirando; mergulhando valem o dobro. São 3 vidas, e cada onda limpa traz outra, mais rápida. O recorde fica guardado enquanto o jogo estiver aberto.
- E no pé da escada do módulo (ou Esc, na rua) volta à nave.

### Pedaços do mapa

- Há três pedaços do mapa estelar: um no planeta sem ar, um com o viajante do planeta extraterrestre e um no posto do planeta do gigante.
- Na nave, o canto de baixo à esquerda mostra quantos já foram trazidos, junto com o tradutor.

### Planeta do gigante

- Um planalto de pedra comprido, com quatro telas de largura: a câmera acompanha o tripulante para os lados. O rosto do gigante fica parado ao fundo, em qualquer ponto do planalto, e os olhos dele seguem, devagar, quem desceu.
- O pouso é no meio. A oeste ficam uma clareira, o bosque das plantas e os destroços de uma sonda; a leste, o ninho dos habitantes, duas pontes sobre o abismo e o altar.
- Plantas brilhantes crescem pelo caminho e habitantes (bichinhos quadrados) andam aos pulos. Pisar numa planta vale 1 ponto de infração; esbarrar em um habitante vale 2.
- Com 3 pontos o gigante reage: o chão treme, pedras caem e fica difícil andar por alguns segundos.
- Sem novas infrações, ele se acalma e perdoa 1 ponto de tempos em tempos.
- Com 5 pontos o gigante pega o tripulante, que escolhe uma resposta. Cada uma depende de um traço: carisma, inteligência ou egoísmo. Se convencer, volta para a nave; se não, o tripulante não volta.
- O posto, perto do pouso, guarda um pedaço do mapa e só abre com três cristais de luz: um no bosque, um no ninho e um no altar. E pega o cristal; o alto da tela mostra quantos já vieram, e as luzes do painel do posto também. Os cristais recolhidos continuam com a tripulação se for preciso pousar de novo.
- Nos destroços, no extremo oeste, há uma célula de combustível: ela devolve à nave os 20 gastos para chegar ao planeta. Só existe uma por partida.
- E no círculo de pouso (ou Esc) volta à nave.

### Planeta sem ar

- Superfície de poeira rosa maior que a tela: a câmera acompanha o tripulante, e um marcador na borda mostra onde ficou o módulo de pouso.
- Não há ar. O oxigênio do traje cai o tempo todo, mais depressa para quem corre (Shift). Cilindros espalhados recarregam; um deles só aparece para tripulantes curiosos.
- Respiradouros no chão borbulham e depois soltam um jato de gás. Ficar no jato faz o traje vazar.
- A mochila é uma pilha de 4 itens: tudo o que o tripulante encosta vai para o topo. E usa o item do topo e Q larga o item do topo. Sucata e rochas só ocupam espaço.
- O portão da estação abre em duas etapas, cada uma com um item no topo da mochila: primeiro a célula de energia liga o terminal, depois a placa de acesso destrava. Os dois ficam em cantos opostos do mapa.
- Dentro da estação está o pedaço do mapa. Leve-o ao módulo de pouso e use E para voltar à nave. Aqui Esc não volta para a nave: é preciso chegar ao módulo.
- Se o oxigênio acabar, o tripulante cai, a tela se fecha em volta dele e aparece o memorial. A nave passa o controle para outro tripulante.
- O portão continua como ficou entre uma visita e outra.

## Testes e ferramentas

- `tests/` tem um teste para cada conjunto de regras (lista de planetas, robô, tradutor, Ser, gigante, sem ar, fliperama). O comando para compilar e rodar está no começo de cada arquivo.
- `ferramentas/gerar_robo.py` redesenha o laboratório, o cientista, as peças dos circuitos e o tradutor.
- `ferramentas/gerar_gigante.py` amplia o planalto do gigante: mantém o desenho original no centro, constrói o resto em volta com as mesmas cores e separa o rosto do gigante numa camada de fundo (precisa também de scipy). As plantas brilhantes são os pixels verdes da máscara.
- `ferramentas/gerar_sons.py` sintetiza os efeitos e os sons de ambiente em `assets/sons/` (mesmos requisitos do script da música).
- `ferramentas/gerar_musica.py` compõe a música de fundo e grava `assets/musica/tema.qoa` (precisa de numpy, scipy e do jogo já compilado em `build/`, porque usa a raylib para converter o áudio).
- `ferramentas/gerar_extraterrestre.py` redesenha a cidade, o bar, os alienígenas, as cartas e o fliperama do planeta extraterrestre (precisa de Python com numpy e Pillow).
- `ferramentas/gerar_sem_ar.py` redesenha o cenário, a máscara e os itens do planeta sem ar (precisa de Python com numpy e Pillow). O jogo não precisa dele para rodar.
