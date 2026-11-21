MAKE IN C: JOGOS DE AVENTURAS — código-fonte do livro
=====================================================

Pastas
  motor/      o motor ASCII construído na Parte II (term, tela, ui, mapa...)
  exemplos/   os programas dos capítulos, um diretório por capítulo
  templo/     Projeto 1: O Templo da Serpente   (ação em tempo real)
  coroa/      Projeto 2: A Coroa de Ferro       (masmorra medieval)
  noir/       Projeto 3: Porto Sombrio          (investigação noir)
  estrela/    Projeto 4: Estrela Errante        (comércio e voo espacial)
  esqueleto/  o ponto de partida do SEU jogo (capítulo 33)

Como compilar um projeto (Linux, macOS, WSL ou w64devkit no Windows)
  cd templo
  make
  ./templo          (no Windows: templo.exe)

Sem make? O comando inteiro está no Makefile de cada projeto, por exemplo:
  gcc -std=c99 -Wall -Wextra -O2 -I../motor main.c niveis.c jogador.c
      monstros.c desenho.c recordes.c ../motor/term.c ../motor/tela.c
      ../motor/ui.c ../motor/rng.c ../motor/mapa.c ../motor/dados.c -o templo

Exemplos da Parte I (C puro):
  cd exemplos/cap05
  gcc -std=c99 -Wall -Wextra encruzilhada.c -o encruzilhada

Exemplos da Parte II (usam o motor):
  cd exemplos/cap20
  gcc -std=c99 -Wall -I../../motor arena.c ../../motor/*.c -o arena

Rode sempre os jogos de dentro da pasta deles: eles procuram os arquivos
de dados (níveis, diálogos, mapas) em caminhos relativos.

O terminal precisa ter pelo menos 80 colunas por 24 linhas.
No Windows, use o Windows Terminal (o padrão no Windows 11).
