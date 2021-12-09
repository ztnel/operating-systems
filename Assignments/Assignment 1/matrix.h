/* Shared Memory Struct
----------------------------------------------------------------
Description: Matrix holds 3 4x4 matrices; inputs M, N and output Q */
struct matrix {
    //Input Matrix M
    int M[100][100];

    //Input Matrix N
    int N[100][100];

    //Output Matrix Q
    int Q[100][100];
};