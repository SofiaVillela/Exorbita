# Exórbita

Jogo de exploração espacial em C e raylib. Esta versão inclui uma nave explorável e três destinos jogáveis: o planeta do robô, o planeta extraterrestre e o planeta do gigante.

## Controles gerais

| Teclas | Ação |
|---|---|
| WASD ou setas | Andar e navegar pelos destinos |
| Shift | Correr nos cenários |
| E | Interagir com personagens, consoles, terminais, marco ou portas |
| Enter | Confirmar, avançar diálogos ou pousar no destino selecionado |
| Esc | Voltar/fechar a tela ou retornar à nave, conforme o contexto |

Na nave, aproxime-se do painel central para abrir o mapa estelar. Escolha Robô, Extraterrestre ou Gigante e pressione Enter para pousar. Ser e Sem Ar aparecem no mapa, mas ainda não são destinos jogáveis.

## O que já está implementado

### Nave

- Exploração em visão superior com movimento em oito direções, corrida e colisão com paredes e objetos.
- Interação com tripulantes, painel de comunicações, terminal da equipe e mapa estelar.
- Diálogos com avanço por tecla e pausa do movimento durante a conversa.
- Cinco destinos exibidos no mapa; Robô, Extraterrestre e Gigante podem ser visitados nesta versão.

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

- Quatro áreas conectadas em sequência: entrada, passagens, pátio e câmara.
- Olhos que abrem e fecham, varrem a área e detectam o jogador dentro do cone e da linha de visão.
- Pilares servem de cobertura e bloqueiam a visão.
- Correr observado, ficar exposto e entrar em zona restrita geram infrações e elevam o alerta.
- O alerta sobe em perigo e diminui em segurança. Ao atingir o limite, ocorre captura.
- Após a captura, confirme para voltar à entrada. Áreas visitadas, conclusão, tradutor e vitórias nas cartas permanecem.
- Visite as quatro áreas e use E no marco da câmara para concluir a exploração.

