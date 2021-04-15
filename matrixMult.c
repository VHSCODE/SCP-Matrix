#include "stdio.h"
#include "mpi.h"
#include "time.h"
#include "stdlib.h"

//#define DEBUG  //Usado para testear el programa en su fase de desarrollo. Activa varios logs y llena las matrices A y B con 1s
//#define DIBUJAR // Usado para dibujar las matrices A, B y C por pantalla. Recomendado desactivarlo para matrices muy grandes

void SUMMA(int *A_local, int *B_local, int *C_local, int N);
void multiplicar_matrices(int *C, int *A, int *B, int N);
void dibujar_matriz(int *vector, int N);
void convertNormalToBlocked(int *const A, int *Ab, int size, int numBlocks);
void convertBlockedToNormal(int *const Ab, int *A, int size, int numBlocks);
int main(int argc, char *argv[])
{

	srand(time(NULL)); //Inicializamos el generador de numeros aleatorios

	int world_size, world_rank;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int N = atoi(argv[1]); //Tamaño de la matriz N x N

	//### Comprobamos los prerequisitos ###
	if (N % world_size != 0)
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
	//#########

	//Matrices principales
	int *A;
	int *B;
	int *C;

	//Matrices en forma de bloque
	int *Ab;
	int *Bb;
	int *Cb;

	//Copias locales de las matrices
	int *A_local;
	int *B_local;
	int *C_local;

	//Asignamos memoria para las matrices principales
	if (world_rank == 0)
	{
		A = (int *)calloc(N * N, sizeof(int));
		B = (int *)calloc(N * N, sizeof(int));
		C = (int *)calloc(N * N, sizeof(int));
		int i;
#ifdef DEBUG
		for (i = 0; i < N * N; i++)
		{
			if (A[i] != 0 || B[i] != 0 || C[i] != 0)
			{
				printf("Error en la creacion de los matrices principales");
				fflush(stdout);
				MPI_Abort(MPI_COMM_WORLD, -1);
				return -1;
			}

			printf("Matrices Principales....[ok]\n\n");
			fflush(stdout);
		}
		for (i = 0; i < N * N; i++)
		{
			A[i] = 1;
			B[i] = 1;
		}
#endif

#ifndef DEBUG
		for (i = 0; i < N * N; i++)
		{
			A[i] = rand() % 101;
			B[i] = rand() % 101;
		}
#endif
		Ab = (int *)calloc(N * N, sizeof(int));
		convertNormalToBlocked(A, Ab, N, numero_bloques);

		Bb = (int *)calloc(N * N, sizeof(int));
		convertNormalToBlocked(B, Bb, N, numero_bloques);

		Cb = (int *)calloc(N * N, sizeof(int));
		convertNormalToBlocked(C, Cb, N, numero_bloques);
	}

	//Ahora repartimos los trozos de forma equitativa los trozos de la matriz a cada nodo

	int tamaño_submatriz = N / numero_bloques; //Con esto conseguimos el tamaño de la submatriz de cada nodo

	A_local = (int *)calloc(tamaño_submatriz * tamaño_submatriz, sizeof(int));
	B_local = (int *)calloc(tamaño_submatriz * tamaño_submatriz, sizeof(int));
	C_local = (int *)calloc(tamaño_submatriz * tamaño_submatriz, sizeof(int));

#ifdef DEBUG
	int i = 0;
	for (i = 0; i < tamaño_submatriz * tamaño_submatriz; i++)
	{
		if (A_local[i] != 0 || B_local[i] != 0 || C_local[i] != 0)
		{
			printf("Error en la creacion de los matrices locales");
			fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, -1);
			return -1;
		}
	}
	printf("Matrices Locales de Nodo %d....[ok]\n\n", world_rank);
	fflush(stdout);
#endif

	MPI_Scatter(Ab, tamaño_submatriz * tamaño_submatriz, MPI_INT, A_local, tamaño_submatriz * tamaño_submatriz, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(Bb, tamaño_submatriz * tamaño_submatriz, MPI_INT, B_local, tamaño_submatriz * tamaño_submatriz, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(Cb, tamaño_submatriz * tamaño_submatriz, MPI_INT, C_local, tamaño_submatriz * tamaño_submatriz, MPI_INT, 0, MPI_COMM_WORLD);

#ifdef DEBUG
	for (i = 0; i < tamaño_submatriz * tamaño_submatriz; i++)
	{
		if (A_local[i] != 1 || B_local[i] != 1 || C_local[i] != 0)
		{
			printf("Error en la reparticion de trozos de las matrices principales");
			fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, -1);
			return -1;
		}
	}

	printf("Reparticion de trozos en nodo %d...[ok]\n\n", world_rank);
	fflush(stdout);
#endif

#ifdef DIBUJAR
	if (world_rank == 0)
	{
		printf("Matriz A\n\n");
		fflush(stdout);
		dibujar_matriz(A, N);

		printf("Matriz B\n\n");
		fflush(stdout);
		dibujar_matriz(A, N);
	}
#endif
	MPI_Barrier(MPI_COMM_WORLD);

	//Calculamos la multiplicacion usando el algoritmo SUMMA
	SUMMA(A_local, B_local, C_local, N);

	//Recogemos los calculos parciales de cada nodo
	MPI_Gather(C_local, tamaño_submatriz * tamaño_submatriz, MPI_INT, Cb, tamaño_submatriz * tamaño_submatriz, MPI_INT, 0, MPI_COMM_WORLD);

	if (world_rank == 0)
	{
		convertBlockedToNormal(Cb, C, N, numero_bloques); //Convertimos la matriz C en bloque a su forma normal matricial normal.

		printf("Calculo terminado!!!\n\n\n");
		fflush(stdout);
		printf("Matriz C\n\n");
		fflush(stdout);

#ifdef DIBUJAR
		dibujar_matriz(C, N);
#endif
		//Ahora comprobamos si el calculo es correcto, usando la version secuencial de la multiplicacion
		int *C_comprobacion = (int *)calloc(N * N, sizeof(int));
		multiplicar_matrices(C_comprobacion, A, B, N);

		int i = 0;
		int flag = 0;
		for (i = 0; i < N * N; i++)
		{
			if (C[i] != C_comprobacion[i])
			{
				flag = 1;
				break;
			}
		}
		if (flag)
		{
			printf("ERROR : EL CALCULO NO ES CORRECTO\n\n");
			fflush(stdout);
		}
		else
		{
			printf("EL CALCULO ES CORRECTO\n\n");
			fflush(stdout);
		}
	}
	if (world_rank == 0)
	{
		free(A);
		free(B);
		free(C);

		free(Ab);
		free(Bb);
		free(Cb);
	}
	free(A_local);
	free(B_local);
	free(C_local);

	MPI_Finalize();

	return 0;
}

void SUMMA(int *A_local, int *B_local, int *C_local, int N)
{

	int world_size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Comm comm_cartesiano;
	int numero_bloques = sqrt(world_size);

	int numero_dimensiones = 2; //Array 2 dimensiones

	const int dimensiones[2] = {numero_bloques, numero_bloques}; //Numero de filas y columnas de la topologia

	const int periodicidad[2] = {0, 0}; //No queremos que haya un bucle en cada fila o columna

	int reordenacion = 0; //Esto evita que MPI cree nuevos ranks para cada nodo.

	//Creamos la una topologia 2D con coordenadas cartesianas
	MPI_Cart_create(MPI_COMM_WORLD, numero_dimensiones, dimensiones, periodicidad, reordenacion, &comm_cartesiano);

	int mis_coordenadas[2];
	MPI_Cart_coords(comm_cartesiano, rank, 2, mis_coordenadas);

	int fila, columna;

	fila = mis_coordenadas[0];
	columna = mis_coordenadas[1];

	//En el algoritmo SUMMA, es necesario hacer broadcast entre filas y columnas, por lo que creamos sus respectivos comunicadores.

	MPI_Comm comm_fila, comm_columna;
	int mantener_dimm[2] = {1, 0}; //Esto es usado en la siguiente funcion para indicar si la dimension i-esima se incluye en la submatriz.
	MPI_Cart_sub(comm_cartesiano, mantener_dimm, &comm_fila);

	mantener_dimm[0] = 0;
	mantener_dimm[1] = 1;

	MPI_Cart_sub(comm_cartesiano, mantener_dimm, &comm_columna);

	int *A_temporal;
	int *B_temporal;
	int tamaño_submatriz = N / numero_bloques;

	A_temporal = (int *)calloc(tamaño_submatriz * tamaño_submatriz, sizeof(int));
	B_temporal = (int *)calloc(tamaño_submatriz * tamaño_submatriz, sizeof(int));

#ifdef DEBUG
	int i = 0;
	for (i = 0; i < tamaño_submatriz * tamaño_submatriz; i++)
	{
		if (A_temporal[i] != 0 || B_temporal[i] != 0)
		{
			printf("Error en la creacion de los matrices temporales\n\n");
			fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, -1);
			return -1;
		}
	}
	printf("Matrices Temporales de Nodo %d....[ok]\n\n", rank);
	fflush(stdout);

#endif
	int k;

	for (k = 0; k < numero_bloques; k++)
	{

		if (fila == k) //Actualizamos la matriz A con la copia local de la matriz A
		{

			//Al haber hecho las matrices contiguas, podemos hacer uso de memcpy sin problemas
			memcpy(A_temporal, A_local, tamaño_submatriz * tamaño_submatriz * sizeof(int));

#ifdef DEBUG
			int i = 0;
			for (i = 0; i < tamaño_submatriz * tamaño_submatriz; i++)
			{
				if (A_local[i] != A_temporal[i])
				{
					printf("Error: Copia de matriz local A en matriz temporal erronea\n\n...");
					fflush(stdout);
					MPI_Abort(MPI_COMM_WORLD, -1);
					return -1;
				}
			}
			printf("Copia de matriz A_local%d....[ok]\n\n", rank);
			fflush(stdout);
#endif
		}

		int err = MPI_Bcast(A_temporal, tamaño_submatriz * tamaño_submatriz, MPI_INT, k, comm_fila); //Hacemos broadcast a toda la fila
		if (err != MPI_SUCCESS)
		{
			MPI_Abort(MPI_COMM_WORLD, -1);
			return;
		}
		if (columna == k) //Actualizamos la matriz B con la copia local de la matriz B
		{
			memcpy(B_temporal, B_local, tamaño_submatriz * tamaño_submatriz * sizeof(int));

#ifdef DEBUG
			int i = 0;
			for (i = 0; i < tamaño_submatriz * tamaño_submatriz; i++)
			{
				if (B_local[i] != B_temporal[i])
				{
					printf("Error: Copia de matriz local B en matriz temporal erronea\n...");
					fflush(stdout);
					MPI_Abort(MPI_COMM_WORLD, -1);
					return -1;
				}
			}
			printf("Copia de matriz B_local%d....[ok]\n\n", rank);
			fflush(stdout);
#endif
		}

		err = MPI_Bcast(B_temporal, tamaño_submatriz * tamaño_submatriz, MPI_INT, k, comm_columna); //Hacemos broadcast a toda la columna
		if (err != MPI_SUCCESS)
		{
			MPI_Abort(MPI_COMM_WORLD, -1);
			return;
		}
		multiplicar_matrices(C_local, A_temporal, B_temporal, tamaño_submatriz);
	}

#ifdef DEBUG
	printf("Nodo %d ha completado su parte del algoritmo SUMMA\n\n", rank);
	fflush(stdout);
#endif
	MPI_Comm_free(&comm_cartesiano);
	MPI_Comm_free(&comm_fila);
	MPI_Comm_free(&comm_columna);
	free(A_temporal);
	free(B_temporal);
}

//Multiplicacion de matrices
void multiplicar_matrices(int *C, int *A, int *B, int N)
{

	int k, i, j;
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{

			for (k = 0; k < N; k++)
			{
				C[i + j * N] += A[i + k * N] * B[k + j * N];
			}
		}
	}
}

void dibujar_matriz(int *vector, int N)
{
	int i, j;
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{
			printf("%d     ", vector[i + j * N]);
			fflush(stdout);
		}
		printf("\n");
		fflush(stdout);
	}
	printf("\n\n\n");
	fflush(stdout);
}

/** 
 * 
 * Convierte la matriz A a una matriz ab en formato de bloque, para poder usarlo en el algoritmo SUMMA
 * 
 * Gracias a https://kayaogz.github.io/teaching/app4-programmation-parallele-2018/tp/tp3/summa-solution-reference.c
 **/
void convertNormalToBlocked(
	int *const A,
	int *Ab,
	int size,
	int numBlocks)
{
	const int blockSize = size / numBlocks;
	int *curBlockElem = Ab;
	int bj, bi, j, i;
	for (bj = 0; bj < numBlocks; bj++)
	{
		for (bi = 0; bi < numBlocks; bi++)
		{
			int *blockBegin = A + bi * blockSize + bj * blockSize * size;
			for (j = 0; j < blockSize; j++)
			{
				for (i = 0; i < blockSize; i++)
				{
					(*curBlockElem) = blockBegin[i + j * size];
					curBlockElem++;
				}
			}
		}
	}
}
/** 
 * 
 * Convierte la matriz Ab a una matriz A en formato "column mayor", esto se usa para sacar el resultado final de la matriz C en bloque
 * 
 * Gracias a https://kayaogz.github.io/teaching/app4-programmation-parallele-2018/tp/tp3/summa-solution-reference.c
 **/
void convertBlockedToNormal(
	int *const Ab,
	int *A,
	int size,
	int numBlocks)
{
	const int blockSize = size / numBlocks;
	int *curBlockElem = Ab;
	int bj, bi, j, i;
	for (bj = 0; bj < numBlocks; bj++)
	{
		for (bi = 0; bi < numBlocks; bi++)
		{
			int *blockBegin = A + bi * blockSize + bj * blockSize * size;
			for (j = 0; j < blockSize; j++)
			{
				for (i = 0; i < blockSize; i++)
				{
					blockBegin[i + j * size] = (*curBlockElem);
					curBlockElem++;
				}
			}
		}
	}
}