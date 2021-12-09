/* Shared Memory Struct
----------------------------------------------------------------
Description: Matrix M holds the 5 input integers, Matrix F is a multipurpose register. */
//Global definitions
#define SET_SIZE 5
#define REG_SIZE 4

struct matrix {
    //Input data set
    int M[SET_SIZE];
    //process flag matrix (there will always be one less process than the data set)
    int F[REG_SIZE];
};