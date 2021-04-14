#include "stdio.h"
#include "mpi.h"
#include "time.h"
#include "random"


int* SUMMA(int* A, int* B, int* C, int N, MPI_Comm comm);
int main(int argc, char *argv[])
{
	srand(time(NULL));

	int world_size, world_rank;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int **A;
	int **B;
	int **C;

	int nrow = atoi(argv[1]);
	int ncol = atoi(argv[2]);

	//Comprobamos que se cumplan los requisitos para el uso de la utilidad.
	if (world_rank == 0)
	{
		if (nrow != ncol)
		{
			if (world_rank == 0)
			{
				printf("Esta utilidad solo puede manejar matrices cuadradas. Abortando...");
				MPI_Finalize();
				return 0;
			}
		}
		if (nrow % world_size != 0)
		{
			printf("El numero de nodos debe de ser divisible entre la dimension de la matriz a calcular. Abortando...");
			MPI_Finalize();
			return 0;
		}

		int numero_bloques = sqrt(world_size);

		if (numero_bloques * numero_bloques != world_size)
		{
			printf("El numero de nodos debe ser un Cuadrado perfecto .Abortando.");
		}
	}

	//Creamos la una topologia 2D con coordenadas cartesianas
	MPI_Comm comm_cartesiano;

	const int numero_dimensiones = 2;
	int dimensiones[2];
	dimensiones[0] = nrow;
	dimensiones[0] = ncol;
	const int periodicidad[2] = {0, 0}; //No queremos que haya un bucle en cada fila o columna
	int reordenacion = 0;				//Esto evita que mpi cree nuevos ranks para cada nodo.

	MPI_Cart_create(MPI_COMM_WORLD, numero_dimensiones, dimensiones, periodicidad, reordenacion, &comm_cartesiano);



	int tamaño_submatrix = nrow / sqrt(world_size); //Con esto conseguimos el tamaño de la submatriz de cada nodo
	//Creamos las matrices de forma dinamica
	A = malloc(tamaño_submatrix * sizeof(int *));
	B = malloc(tamaño_submatrix * sizeof(int *));
	C = malloc(tamaño_submatrix * sizeof(int *));
	
	int i;
	for (i = 0; i < nrow; i++)
	{
		A[i] = malloc(tamaño_submatrix * sizeof(int));
		B[i] = malloc(tamaño_submatrix * sizeof(int));
		C[i] = malloc(tamaño_submatrix * sizeof(int));
	}
	//Llenamos la A de numeros aleatorios :P
	if (world_rank == 0)
	{
		int i, j;
		for (i = 0; i < nrow; i++)
		{
			for (j = 0; j < ncol; j++)
			{
				A[i][j] = rand() % 101;
				B[i][j] = rand() % 101;
			}
		}
		int k;

		//Calculamos la multiplicacion usando el algoritmo SUMMA
		C = SUMMA(A,B,C,nrow,comm_cartesiano);
		
	}

	free(A);
	free(B);
	free(C);
	MPI_Finalize();
	return 0;
}

int* SUMMA(int* A, int* B, int* C, int N, MPI_Comm comm)
{
	int mis_coordenadas[2];
}