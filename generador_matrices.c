#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

int main(int argc, char *argv[])
{

    srand(time(NULL));

    if (argc < 3)
    {
        printf("Uso: generador_matrices <dimension> <nombre_fichero_salida>\n");
        return;
    }
    int N = atoi(argv[1]);

    char *nombre_archivo_salida = argv[2];

    int *matriz = calloc(N * N, sizeof(int));

    int i = 0;
    for (i; i < N * N; i++)
    {
        matriz[i] = rand() % 101;
    }

    escribir_matriz_a_archivo(nombre_archivo_salida, matriz, N);

    free(matriz);
}

void escribir_matriz_a_archivo(char *nombre_fichero, int *matriz, int N)
{
    FILE *fp_a = fopen(nombre_fichero, "w");
    if (fp_a == NULL)
    {
        printf("Error creando el archivo destino\n");
        exit(-1);
    }
    char *linea;

    int i, j;

    for (i = 0; i < N; i++)
    {
        linea = calloc(N * 2, sizeof(char)); //Dos veces el numero de elementos para prevenir errores de memoria
        for (j = 0; j < N; j++)
        {
            char str[20] = {0};

            if ((j + 1 == N))
                sprintf(str, "%d", matriz[i + j * N]);
            else
                sprintf(str, "%d ", matriz[i + j * N]);
            fflush(stdout);
            strcat(linea, str);
        }
        if (i + 1 != N)
            strcat(linea, "\n");
        fputs(linea, fp_a);
    }

    fclose(fp_a);
    free(linea);
}