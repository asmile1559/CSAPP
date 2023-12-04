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
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l, x0, x1, x2, x3, x4, x5, x6, x7;
    if (M == 32 && N == 32)
    {
        for (i = 0; i < 4 * 4; i++)
        {
            for (j = (i % 4) * 8; j < (i % 4) * 8 + 8; j++)
            {
                k = (i / 4) * 8;
                x0 = A[j][k];
                x1 = A[j][k + 1];
                x2 = A[j][k + 2];
                x3 = A[j][k + 3];
                x4 = A[j][k + 4];
                x5 = A[j][k + 5];
                x6 = A[j][k + 6];
                x7 = A[j][k + 7];

                B[k][j] = x0;
                B[k + 1][j] = x1;
                B[k + 2][j] = x2;
                B[k + 3][j] = x3;
                B[k + 4][j] = x4;
                B[k + 5][j] = x5;
                B[k + 6][j] = x6;
                B[k + 7][j] = x7;
            }
        }
    }
    else if (M == 64 && N == 64)
    {
        for (i = 0; i < 16 * 16; i++)
        {
            for (j = (i % 16) * 4; j < (i % 16) * 4 + 4; j++)
            {
                k = (i / 16) * 4;
                x0 = A[j][k];
                x1 = A[j][k + 1];
                x2 = A[j][k + 2];
                x3 = A[j][k + 3];

                B[k][j] = x0;
                B[k + 1][j] = x1;
                B[k + 2][j] = x2;
                B[k + 3][j] = x3;
            }
        }
    }
    else if (M == 61 && N == 67)
    {
        for (i = 0; i < N; i += 16)
        {
            for (j = 0; j < M; j += 16)
            {
                for (k = i; k < i + 16 && k < N; k++)
                {
                    for (l = j; l < j + 16 && l < M; l++)
                    {
                        B[l][k] = A[k][l];
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

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
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

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
