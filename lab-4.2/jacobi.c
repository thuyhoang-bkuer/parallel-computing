#include <string.h>
#include <stdio.h>
#include <stdlib.h>



double reduceOp(char op, double* arr, int w) {
	double ret = 0.0;
	for (int i = 0; i < w; i++) {
		ret += op == '+' ? arr[i] : -arr[i];
	}
	return ret;
}

double* reduceVec(char op, double* left, double* right, int w) {
	double* ret = (double *) malloc(sizeof(double) * w);
	for (int i = 0; i < w; i++) {
		ret[i] = op == '+' ? (left[i] + right[i]): (left[i] - right[i]);
	}
	return ret;
}

double* mulMatVec(double **mat, double *vec, int w) {
	double * ret = (double*) malloc(w * sizeof(double));

	for (int i = 0; i < w; i++) {
		ret[i] = 0.0;
		for (int j = 0; j < w; j++) {
			ret[i] += mat[i][j];
		}
	}
	return ret;
}


void printVector(double *vec, int w) {
	printf("[ ");
	for (int i = 0; i < w; i++) {
		printf("%.2lf ", vec[i]);
	}
	printf("]\n");
}

void printMatrix(double** mat, int w, int h) {
	for (int i = 0; i < w; i++) {
		printf("[ ");
		for (int j = 0; j < h; j++) {
			printf("%.2lf ", mat[i][j]);
		}
		printf("]\n");
	}
}

int main(int argc, char** argv)
{	
	char *filename = "sample.mat";

	double **A, *x, *b;
	int wA, hA;
	int i, j;

	// Read data
	FILE *file = fopen(filename, "r");

	if (!file) {
		printf("File Not Found!\n");
		return -1;
	}
	fscanf(file, "%d %d", &wA, &hA);
	
	if (wA != hA) {
		printf("Jacobi method is square matrix required ");
		return -1;
	}
	
	A = (double **)malloc(wA * sizeof(double *));
	x = (double *)calloc(wA, sizeof(double));
	b = (double *)malloc(hA * sizeof(double));

	for (i = 0; i < wA; i++)
	{
		A[i] = (double *)malloc(hA * sizeof(double));
		for (j = 0; j < hA; j++)
		{
			fscanf(file, "%lf", &A[i][j]);
		}
		fscanf(file, "%lf", &b[i]);
		x[i] = 1.0;
	}

	fclose(file);
	printVector(b, hA);
	printMatrix(A, wA, hA);
	

	// Jacobi iterative method
	double error = 0.0;
	double signma = 0.0;
	do {
		for (i = 0; i < wA; i++) {
			for (j = 0; j < hA; j++) {
				signma = 0.0;
				if (i != j) 
					signma += A[i][j]*x[j];
			}
			x[i] = (b[i] - x[i]) / A[i][i];
		}
		
		error = reduceOp('+', reduceVec('-', mulMatVec(A, x, hA), b, hA), hA) / (double) hA;
		printf(">>> %lf\n", error);
	}
	while (error > 1e-5);

	printVector(x, hA);

	return 0;
}