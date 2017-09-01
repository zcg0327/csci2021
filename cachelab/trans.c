/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
	if (M == 32 && N == 32) {
		/* use loop blocking and a row-wise access pattern of blocks
		 * and within the blocks.
		 * also handle diagonal entries separately by storing in a
		 * local variable and assigning later. */
		/* 8 was found to be the most miss-reducing block size. */
		int block = 8;
		int i,j,x,y;
		int tmp = 0;
		int diag = -1;
		for (i=0; i < N; i+=block) {
			for (j=0; j < M; j+= block) {
				/* inner block loops. */
				for (x = i; x < i+block; x++) {
					for (y = j; y < j+block; y++) {
						/* if @ diagonal in A, save to temp. */
						if (x == y) {
							tmp = A[x][y];
							diag++;
						}
						/* otherwise use regular transpose. */
						else {
							B[y][x] = A[x][y];
						}
					}
					/* place temp into B's diagonal if @ diagonal block.. */
					if (i == j) {
						B[diag][diag] = tmp;
					}
				}
			}
		}
	}

	if (M == 64 && N == 64) {
		/* unlike 32x32 and 61x67 versions, 64x64 utilizes manual assignment
		 * of values in A to local variables and then into B. */
		/* uses a column-wise block access pattern within the 64x64 matrices.
		 * uses a u-shaped pattern within the 8x8 block, transposing 1x4 
		 * sub-blocks at a time to utilize the local variables. */
    	int i, j, k;
    	int a,b,c,d,e,f,g,h;
    	for (i = 0; i < N; i += 8) {
        	for (j = 0; j < M; j += 8) {
				/* use local variables to reduce misses. */
           		for (k = 0; k < 8; k++) {
					/* store first 4 elements from current row of A. */
					/* naturally, use k in an increasing fashion. */
					a = A[j+k][i];
					b = A[j+k][i+1];
					c = A[j+k][i+2];
					d = A[j+k][i+3];
               		if (k == 0) {
						/* save upperrightmost 1x4 sub-block. */
						/* this should only be done once per block. */
						e = A[j][i+4];
						f = A[j][i+5];
						g = A[j][i+6];
						h = A[j][i+7];
                	}
					/* place elements transposed into B. */
                	B[i][j+k] = a;
                	B[i+1][j+k] = b;
   	            	B[i+2][j+k] = c;
   	            	B[i+3][j+k] = d;
   	        	}
	
				/* store last 4 elements from current column of A. 
				 * then place them correspondingly into B. */
				/* start at the bottom row of the block since this
				 * was most recently used. */
				/* note that this loop only iterates 7 times since 
				 * 4 elements are taken care of by vars e f g & h. */
        	    for (k = 7; k > 0; k--) {
					a = A[j+k][i+4];
					b = A[j+k][i+5];
					c = A[j+k][i+6];
					d = A[j+k][i+7];
    	            B[i+4][j+k] = a;
	                B[i+5][j+k] = b;
                	B[i+6][j+k] = c;
           	    	B[i+7][j+k] = d;
        	    }
				
				/* simply using 4 more local variables to specifically place
				 * these final 4 entries reduced misses by ~50.
				 * I cannot figure out why this works. */
				/* finish with elements in first column of block & last 4 rows. */
				B[i+4][j] = e;
				B[i+5][j] = f;
				B[i+6][j] = g;
				B[i+7][j] = h;
       		}
    	}  	
	}
	
	if (M == 61 && N == 67) {
		/* use loop blocking and a row-wise access pattern of blocks
		 * and within the blocks.
		 * also handle diagonal entries separately by storing in a
		 * local variable and assigning later. */
		/* 16 was found to be the most miss-reducing block size. */
		int block = 16;
		int i,j,x,y;
		int tmp = 0;
		int diag = -1;
		for (i=0; i < N; i+=block) {
			for (j=0; j < M; j+= block) {
				/* inner block loops. */
				for (x = i; (x < i+block) && (x < N); x++) {
					for (y = j; (y < j+block) && (y < M); y++) {
						/* if @ diagonal in A, save to temp. */
						if (x == y) {
							tmp = A[x][y];
							diag++;
						}
						/* otherwise use regular transpose. */
						else {
							B[y][x] = A[x][y];
						}
					}
					/* place temp into B's diagonal if @ diagonal block. */
					if (i == j) {
						B[diag][diag] = tmp;
					}
				}
			}
		}
	}
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

