#include "stdio.h"
#include "mpi.h"
#include "time.h"
#include "stdlib.h"


#define DEBUG 0


int **SUMMA(int **A, int **B, int **C, int N, MPI_Comm comm, int rank, int world_size);
void multiplicar_matrices(int **C, int **A, int **B, int N);
int **sumar_matriz(int **A, int **B, int N);
void dibujar_matriz(int **matriz, int N);
void crear_matriz2D_contigua(int*** matriz, int N);


int main(int argc, char *argv[])
{
	srand(time(NULL));

	int world_size, world_rank;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


	int nrow = atoi(argv[1]);
	int ncol = atoi(argv[2]);

	//Comprobamos que se cumplan los requisitos para el uso de la utilidad.

	if (nrow != ncol)
	{
		if (world_rank == 0)
		{
			printf("Esta utilidad solo puede manejar matrices cuadradas. Abortando...\n");
			fflush(stdout);
		}
		MPI_Finalize();
		return 0;
	}
	if (nrow % world_size != 0)
	{
		if (world_rank == 0)
		{
			printf("El numero de nodos debe de ser divisible entre la dimension de la matriz a calcular. Abortando...");
			fflush(stdout);
		}
		MPI_Finalize();
		return 0;
	}

	int numero_bloques = sqrt(world_size);

	if (numero_bloques * numero_bloques != world_size)
	{
		if (world_rank == 0)
		{
			printf("El numero de nodos debe ser un Cuadrado perfecto .Abortando.");
			fflush(stdout);
		}
		MPI_Finalize();
		return 0;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	//Creamos la una topologia 2D con coordenadas cartesianas
	MPI_Comm comm_cartesiano;

	int numero_dimensiones = 2;
	const int dimensiones[2] = {numero_bloques,numero_bloques};
	const int periodicidad[2] = {0, 0}; //No queremos que haya un bucle en cada fila o columna
	int reordenacion = 0;				//Esto evita que mpi cree nuevos ranks para cada nodo.

	MPI_Cart_create(MPI_COMM_WORLD, numero_dimensiones, dimensiones, periodicidad, reordenacion, &comm_cartesiano);

	int tamaño_submatrix = nrow / sqrt(world_size); //Con esto conseguimos el tamaño de la submatriz de cada nodo
	
	
	
	//Creamos las matrices de forma dinamica
	/**
	A = malloc(tamaño_submatrix * sizeof(int *));
	B = malloc(tamaño_submatrix * sizeof(int *));
	C = malloc(tamaño_submatrix * sizeof(int *));
	**/

	int **A;
	int **B;
	int **C;
	crear_matriz2D_contigua(&A,tamaño_submatrix);
	crear_matriz2D_contigua(&B,tamaño_submatrix);
	crear_matriz2D_contigua(&C,tamaño_submatrix);
	int i = 0;
	int j = 0;
	for(i; i < tamaño_submatrix; i++)
		for(j; j < tamaño_submatrix; j++)
			C[i][j] = 0;
	/**
	int i;
	for (i = 0; i < tamaño_submatrix; i++)
	{
		A[i] = malloc(tamaño_submatrix * sizeof(int));
		B[i] = malloc(tamaño_submatrix * sizeof(int));
		C[i] = malloc(tamaño_submatrix * sizeof(int));
	}
		printf("Creando las matrices principales...\n");
		fflush(stdout);
	**/
	//Llenamos las matrices de numeros aleatorios :P
		for (i = 0; i < tamaño_submatrix; i++)
		{
			for (j = 0; j < tamaño_submatrix; j++)
			{
				if(DEBUG == 1)
				{
					A[i][j] = 1;
					B[i][j] = 1;
				}
				else
				{
					A[i][j] = rand() % 101;
					B[i][j] = rand() % 101;
				}
				
			}
		}

	MPI_Barrier(MPI_COMM_WORLD);
	//Calculamos la multiplicacion usando el algoritmo SUMMA
	C = SUMMA(A, B, C, tamaño_submatrix, comm_cartesiano, world_rank, world_size);
	if (world_rank == 0)
	{
		dibujar_matriz(C, tamaño_submatrix);
	}
	free(A);
	free(B);
	free(C);
	MPI_Finalize();
	return 0;
}

int **SUMMA(int **A, int **B, int **C, int N, MPI_Comm comm, int rank, int world_size)
{
	int mis_coordenadas[2];
	MPI_Cart_coords(comm, rank, 2, mis_coordenadas);

	int fila, columna;

	fila = mis_coordenadas[0];
	columna = mis_coordenadas[1];

	//En el algoritmo SUMMA, es necesario hacer broadcast entre filas y columnas, por lo que creamos sus respectivos comunicadores.


	MPI_Comm comm_fila, comm_columna;
	int mantener_dimm[2] = {1, 0}; //Esto es usado en la siguiente funcion para indicar si la entrada i-esima se incluye en la submatriz.
	MPI_Cart_sub(comm, mantener_dimm, &comm_fila);

	mantener_dimm[0] = 0;
	mantener_dimm[1] = 1;

	MPI_Cart_sub(comm, mantener_dimm, &comm_columna);

	//Creamos las copias locales de A, B y C, usadas para operar con ellas.
	
	/**
	int **A_local = malloc(N * sizeof(int *));
	int **B_local = malloc(N * sizeof(int *));
	int **C_temporal = malloc(N * sizeof(int *));
	**/
	int** A_local;
	int** B_local;
	int** C_temporal;
	crear_matriz2D_contigua(&A_local,N);
	crear_matriz2D_contigua(&B_local,N);
	crear_matriz2D_contigua(&C_temporal,N);  //Esta matriz sera la solucion de cada submatriz, para luego juntarlas en la matriz final C
	int i = 0;
	int j = 0;
	for(i; i < N; i++)
		for(j; j < N; j++)
			C_temporal[i][j] = 0;
	
	/**
	int i;
	for (i = 0; i < N; i++)
	{
		A[i] = malloc(N * sizeof(int));
		B[i] = malloc(N * sizeof(int));
		C[i] = malloc(N * sizeof(int));
	}
		printf("Creando las matrices locales...\n");
		fflush(stdout);
		
	//Copiamos los datos a estas matrices locales
**/
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
		{
			A_local[i][j] = A[i][j];
			B_local[i][j] = B[i][j];
			C[i][j] = 0;
		}
		
	int numero_bloques = sqrt(world_size);

	int k;
	for (k = 0; k < numero_bloques; k++)
	{

		if (columna == k) //Actualizamos la matriz A con la copia local de la matriz A
		{
			int j, i;
			for (i = 0; i < N; i++)
				for (j = 0; j < N; j++)
					A[i][j] = A_local[i][j];
		}

		MPI_Bcast(&(A[0][0]), N * N, MPI_INT, k, comm_fila);
		if (fila == k) //Actualizamos la matriz B con la copia local de la matriz B
		{
			int j, i;
			for (i = 0; i < N; i++)
				for (j = 0; j < N; j++)
					B[i][j] = B_local[i][j];
		}
		
		MPI_Bcast(&(B[0][0]), N * N, MPI_INT, k, comm_columna);



		multiplicar_matrices(C_temporal, A, B, N);

		
		C = sumar_matriz(C, C_temporal, N);
		
	}

	free(A_local);
	free(B_local);
	free(C_temporal);
	return C;
}

//Multiplicacion de matrices
void multiplicar_matrices(int **C, int **A, int **B, int N)
{
	int a, i, j;
	for (a = 0; a < N; a++)
	{
		for (i = 0; i < N; i++)
		{
			int suma = 0;
			for (j = 0; j < N; j++)
			{
				suma += A[i][j] * B[j][a];
			}
			C[i][a] = suma;
		}
	}
}

int **sumar_matriz(int **A, int **B, int N)
{
	int i, j;

	//Creamos una matriz para guardar el resultado
	int **C;
	crear_matriz2D_contigua(&C,N);
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			C[i][j] = A[i][j] + B[i][j];

	return C;
}

void dibujar_matriz(int **matriz, int N)
{
	int i, j;
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{
			printf("%d     ", matriz[i][j]);
		}
		printf("\n");
	}
}


/**
 * Esta funcion crea una matriz 2D contiguamente en memoria, para que MPI pueda hacer broadcast
 */
void crear_matriz2D_contigua(int*** matriz, int N)
{
    int *p = (int *)malloc(N*N*sizeof(int));
    if (!p) return;

    
    (*matriz) = (int **)malloc(N*sizeof(int*));
    if (!(*matriz)) {
       free(p);
       return;
    }

    
	int i;
    for (i=0; i<N; i++) 
       (*matriz)[i] = &(p[i*N]);
	

	return;
}