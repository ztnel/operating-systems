/* Shared Memory Struct
----------------------------------------------------------------
Description: Matrix holds 3 4x4 matrices; inputs M, N and output Q */
struct matrix {
    //Input Matrix M
    int M[4][4];

    //Input Matrix N
    int N[4][4];

    //Output Matrix Q
    int Q[4][4];
};