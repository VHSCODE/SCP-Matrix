#include "stdio.h"
#include "stdlib.h"
/**
 * Programa para testear si las matrices dinamicas funcionan
 **/
int main(int argc, char *argv[])
{
    int nrow, ncol;

    nrow = atoi(argv[1]);
    ncol = atoi(argv[2]);

    int **A = malloc(nrow * sizeof(int *));
    int **B = malloc(nrow * sizeof(int *));
    int **C = malloc(nrow * sizeof(int *));

    int i;
    for (i = 0; i < nrow; i++)
    {
        A[i] = malloc(ncol * sizeof(int));
        B[i] = malloc(ncol * sizeof(int));
        C[i] = malloc(ncol * sizeof(int));
    }

    int j = 0;
    i = 0;

    for(i; i < nrow; i++)
    {
        for (j; j < ncol; j++)
        {
            A[i][j] = 1;
            B[i][j] = 1;
            C[i][j] = 1;
        }

    }
    j = 0;
    i = 0;
    for(i; i < nrow; i++)
    {
        for (j; j < ncol; j++)
        {
            if(A[i][j] != 1 || B[i][j] != 1 || C[i][j] != 1)
            {
                printf("Error: La matrix no tiene el valor correcto en la posicion (%d, %d)",i,j);
            }
            printf("A %d\n", A[i][j]);
            printf("B %d\n", B[i][j]);
            printf("C %d\n", C[i][j]);
        }

    }

    free(A);
    free(B);
    free(C);
        
}