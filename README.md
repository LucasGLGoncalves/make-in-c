# 🎮 MAKE IN C: JOGOS DE AVENTURAS — Código-Fonte Oficial

Repositório oficial com o motor gráfico ASCII, exemplos dos capítulos e os projetos completos do livro **MAKE IN C: JOGOS DE AVENTURAS**, um guia prático para aprender **Linguagem C do zero ao nível avançado** construindo jogos completos diretamente no terminal.

![Demonstração dos Jogos](link-do-seu-gif-ou-print.gif)
*(Dica: Substitua a linha acima por um GIF curto mostrando um dos 4 jogos rodando no terminal!)*

---

## 📖 Sobre o Livro
Cansado de aprender programação apenas com exercícios teóricos e telas pretas sem graça? Em **MAKE IN C: JOGOS DE AVENTURAS**, você sai dos fundamentos básicos da linguagem e chega à construção de um motor gráfico próprio em modo texto e quatro jogos completos de gêneros diferentes.

### O que você vai dominar na prática:
* **Fundamentos de C puro:** Variáveis, controle de fluxo, funções, arrays, matrizes e manipulação de strings.
* **Programação Avançada:** Ponteiros, alocação dinâmica de memória, estruturas de dados, leitura de arquivos de mapas/diálogos e organização modular em múltiplos arquivos `.c` e `.h`.
* **Construção de um Motor ASCII (`motor/`):** Controle de terminal (`term`), renderização de tela (`tela`), interface de usuário (`ui`), geração de números aleatórios (`rng`), mapas (`mapa`) e dados (`dados`).

---

## 🛒 Onde adquirir o livro completo
O livro digital (e-book) traz toda a explicação passo a passo, arquitetura do código e a teoria por trás de cada linha deste repositório:

* 👉 **[Comprar na Amazon (Kindle / E-book)](COLE_AQUI_O_LINK_DA_AMAZON)**
* 👉 **[Comprar na Hotmart (PDF Direto)](COLE_AQUI_O_LINK_DA_HOTMART)** *(em breve)*

---

## 📂 Estrutura do Repositório

```text
├── motor/      # O motor ASCII construído na Parte II (term, tela, ui, mapa, rng, dados)
├── exemplos/   # Os programas dos capítulos, organizados um diretório por capítulo
├── templo/     # Projeto 1: O Templo da Serpente (ação em tempo real)
├── coroa/      # Projeto 2: A Coroa de Ferro (masmorra medieval)
├── noir/       # Projeto 3: Porto Sombrio (investigação noir)
├── estrela/    # Projeto 4: Estrela Errante (comércio e voo espacial)
└── esqueleto/  # O ponto de partida para criar o SEU próprio jogo (Capítulo 33)
```

---

## ⚙️ Como Compilar e Rodar

### Pré-requisitos
* **Compilador C (GCC)** com suporte ao padrão `C99` (Linux, macOS, WSL ou **w64devkit** no Windows).
* **Terminal:** O console precisa ter pelo menos **80 colunas por 24 linhas**. No Windows, utilize o **Windows Terminal** (padrão no Windows 11).

> ⚠️ **Importante:** Rode sempre os jogos de dentro da pasta deles, pois eles carregam os arquivos de dados (níveis, diálogos e mapas) usando caminhos relativos.

---

### 1. Compilando os Projetos Principais (`templo/`, `coroa/`, `noir/`, `estrela/`)

**Usando `make` (recomendado):**
```bash
cd templo
make
./templo          # No Windows: templo.exe
```

**Sem `make` (compilação manual via GCC):**
O comando completo está dentro do `Makefile` de cada projeto. Para compilar *O Templo da Serpente*, por exemplo:
```bash
cd templo
gcc -std=c99 -Wall -Wextra -O2 -I../motor main.c niveis.c jogador.c monstros.c desenho.c recordes.c ../motor/term.c ../motor/tela.c ../motor/ui.c ../motor/rng.c ../motor/mapa.c ../motor/dados.c -o templo
./templo
```

---

### 2. Compilando os Exemplos dos Capítulos

**Exemplos da Parte I (C puro):**
```bash
cd exemplos/cap05
gcc -std=c99 -Wall -Wextra encruzilhada.c -o encruzilhada
./encruzilhada
```

**Exemplos da Parte II (que utilizam o motor ASCII):**
```bash
cd exemplos/cap20
gcc -std=c99 -Wall -I../../motor arena.c ../../motor/*.c -o arena
./arena
```

---

## 📝 Licença
Os códigos-fonte deste repositório estão licenciados sob a **MIT License** — sinta-se à vontade para estudar, modificar e usar o motor e o `esqueleto/` como base para seus próprios jogos. O conteúdo textual, diagramas e estrutura didática do livro **MAKE IN C: JOGOS DE AVENTURAS** são protegidos por direitos autorais.
