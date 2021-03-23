#include "stdio.h"
#include "mpi.h"
#include "time.h"
#include "random"
int main(int argc, char* argv[])
{
	srand(time(NULL));

	int world_size, world_rank;
	MPI_Init(&argc,&argv);
	
	MPI_Comm_size(MPI_COMM_WORLD,&world_size);
	MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);

	//Comunicadores de fila y columna


	int nrow = atoi(argv[1]);
	int ncol = atoi(argv[2]);
	if(nrow != ncol)
	{
		if(world_rank == 0)
			printf("Esta utilidad solo puede manejar matrices cuadradas. Sorry :(");
	}
	else
	{
		//Creamos las matrices de forma dinamica
		//FIXME optimizar el uso de memoria. Los nodos no deberian reservar toda la memoria
		int **A = malloc(nrow * sizeof(int *));
		int **B = malloc(nrow * sizeof(int *));
		int **C = malloc(nrow * sizeof(int *));
		
		int i;
		for(i = 0; i < nrow; i++)
		{
			A[i] = malloc(ncol  * sizeof(int));
			B[i] = malloc(ncol  * sizeof(int));
			C[i] = malloc(ncol  * sizeof(int));
		}


		//Llenamos la A de numeros aleatorios :P
		if(world_rank == 0)
		{
			int i,j;
			for(i = 0; i < nrow; i++)
			{
				for(j = 0 ; j < ncol; j++)
				{
					A[i][j] = rand() % 101;
					B[i][j] = rand() % 101;
				}
			}
		}

		int k;

		do
		{

		}while (k < nrow);
	}

	MPI_Finalize();
	return 0;
}
