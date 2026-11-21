/* ansi.c — conversando com o terminal "na unha", com sequências ANSI. */
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

int main(void)
{
    int cor;
#ifdef _WIN32
    /* No Windows, as sequências ANSI precisam ser ligadas uma vez. */
    HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    GetConsoleMode(saida, &modo);
    SetConsoleMode(saida, modo | 0x0004);   /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
    SetConsoleOutputCP(65001);              /* UTF-8 */
#endif
    printf("\x1b[2J");                      /* limpa a tela          */
    printf("\x1b[1;1H");                    /* cursor na linha 1, coluna 1 */
    printf("As 16 cores do terminal:\n\n");
    for (cor = 0; cor < 8; cor++)
        printf("\x1b[%dm %2d \x1b[0m", 40 + cor, 40 + cor);   /* fundos escuros */
    printf("\n");
    for (cor = 0; cor < 8; cor++)
        printf("\x1b[30;%dm %3d\x1b[0m", 100 + cor, 100 + cor); /* fundos claros */
    printf("\n\n");
    for (cor = 0; cor < 8; cor++)
        printf("\x1b[%dmTexto %d  \x1b[%dmClaro %d\x1b[0m\n", 30 + cor, 30 + cor,
               90 + cor, 90 + cor);

    /* uma moldura desenhada em qualquer lugar da tela */
    printf("\x1b[5;40H\x1b[93m╔══════════════╗");
    printf("\x1b[6;40H║ \x1b[97mOlá, terminal\x1b[93m║");
    printf("\x1b[7;40H╚══════════════╝\x1b[0m");
    printf("\x1b[20;1HAperte Enter para sair...");
    fflush(stdout);
    getchar();
    printf("\x1b[0m\x1b[2J\x1b[1;1H");     /* devolve a tela limpa */
    return 0;
}
