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

Na nave, aproxime-se do painel central para abrir o mapa estelar. Escolha um planeta e pressione Enter para pousar. A Via Láctea só abre depois que os outros forem visitados.

## O que já está implementado

### Nave

- Exploração em visão superior com movimento em oito direções, corrida e colisão com paredes e objetos.
- Interação com tripulantes, painel de comunicações, terminal da equipe e mapa estelar.
- Diálogos com avanço por tecla e pausa do movimento durante a conversa.
- Cinco planetas no mapa, todos visitáveis, mais a Via Láctea.

### Planeta do robô e tradutor

- Sala do cientista e console do laboratório de comandos existente.
- Desafio em grade: setas/WASD adicionam comandos, Backspace remove, Enter executa, R reinicia e H mostra uma dica.
- Após vencer, volte a falar com o cientista para receber o tradutor. A vitória no laboratório, sozinha, não entrega o item.
- O tradutor permite compreender o visitante extraterrestre; a leitura do tom pode falhar.

### Planeta extraterrestre

- Cidade cyberpunk explorável, com visitante, mesa de cartas, obstáculos e saída para a nave.
- Conversa incompreensível antes de obter o tradutor e diálogo em português depois.
- Jogo de blefe: o visitante anuncia o valor de uma carta, que pode ser verdade ou mentira.
- Escolha uma carta com A/D, setas ou 1/2/3. Use W/S ou cima/baixo para escolher **Comparar** ou **Desconfiar**; Enter confirma.
- Comparar ganha se sua carta real for maior. Desconfiar ganha se o anúncio for falso.
- Cada lado tem três cartas. A partida termina quando alguém chega a dois pontos ou após três rodadas; Enter permite revanche.

### Planeta do gigante

- Um planalto de pedra com o rosto do gigante ao fundo. Os olhos dele seguem, devagar, quem desceu.
- Plantas brilhantes crescem pelo caminho e habitantes (bichinhos quadrados) andam aos pulos. Pisar numa planta vale 1 ponto de infração; esbarrar em um habitante vale 2.
- Com 3 pontos o gigante reage: o chão treme, pedras caem e fica difícil andar por alguns segundos.
- Sem novas infrações, ele se acalma e perdoa 1 ponto de tempos em tempos.
- Com 5 pontos o gigante pega o tripulante, que escolhe uma resposta. Cada uma depende de um traço: carisma, inteligência ou egoísmo. Se convencer, volta para a nave; se não, o tripulante não volta.
- Chegue ao posto do outro lado e use E para registrar o marco. E no círculo de pouso (ou Esc) volta à nave.

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

- `tests/` tem um teste para cada conjunto de regras (lista de planetas, robô, tradutor, Ser, gigante, sem ar). O comando para compilar e rodar está no começo de cada arquivo.
- `ferramentas/gerar_sem_ar.py` redesenha o cenário, a máscara e os itens do planeta sem ar (precisa de Python com numpy e Pillow). O jogo não precisa dele para rodar.
