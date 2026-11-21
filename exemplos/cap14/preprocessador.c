/* preprocessador.c — #define, macros e compilação condicional. */
#include <stdio.h>

#define VERSAO        "1.2"
#define VIDA_INICIAL  30
#define MAXIMO(a, b)  ((a) > (b) ? (a) : (b))   /* parênteses em tudo! */
#define QUADRADO(x)   ((x) * (x))

/* Compile com  gcc -DDEPURAR ...  para ligar as mensagens extras. */
#ifdef DEPURAR
    #define LOG(msg) printf("[depuração] %s (linha %d)\n", msg, __LINE__)
#else
    #define LOG(msg)                          /* some do programa */
#endif

int main(void)
{
    int a = 7, b = 12;
    LOG("começando");
    printf("Versão %s, vida inicial %d\n", VERSAO, VIDA_INICIAL);
    printf("O maior entre %d e %d é %d\n", a, b, MAXIMO(a, b));
    printf("QUADRADO(2 + 1) = %d\n", QUADRADO(2 + 1));
#if defined(_WIN32)
    printf("Compilado para Windows.\n");
#elif defined(__APPLE__)
    printf("Compilado para macOS.\n");
#else
    printf("Compilado para Linux (ou outro Unix).\n");
#endif
    LOG("terminando");
    return 0;
}
