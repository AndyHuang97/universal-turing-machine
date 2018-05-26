/*
 Description: Simulator of a Nondeterministic Turing Machine;
 Usage: ./tm <inputfile> <outputfile>
 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//------------------------------------------------------------
//TM's data structures
typedef struct configuration{
    int state;          //state of the OC;
    char *InStr;        //input string;
    int pos;            //position of the head on the tape(string);
}config;
typedef struct endList{     //
    char output;    //written char;
    char hmove;     //{R,L, S} head moves;
    int end;        //end state
    struct endList *next;
}endList;
typedef struct inputHash{    //Hashtable for input
    char input;              //read char;
    struct inputHash *next;
    struct endList *end_next;
}inputHash;
typedef struct State{        //Main hashtable ordered according to states
    int start;                   //start state
    struct State *next;
    struct inputHash *input_next;
}State;
typedef struct machine{
    State *tr;
    config snapshot;//memorizes the OC's current state and input string's head
    int *Accept;    //array of final states;
    int max;        //condition for termination if it's looping
}TM;
//------------------------------------------------------------
//sorting data structures
typedef struct Heap{
    int start;
    char input;
    char output;
    char hmove;
    int end;
}Heap;
typedef struct array{
    int *heapArray;
    int Length;
    int heapSize;
}array;
//------------------------------------------------------------
void init_TM(TM *);
int PARENT(int);
int LEFT(int);
int RIGHT(int);
void MAX_HEAPIFY(array *, int, char **);
void BUILD_MAX_HEAP(array *, char **);
void HeapSort1(array *, char **);

void sortTr(char *);
void Load_TM(TM *, char *);
void createSt(State *);

void init_TM(TM *tm)
{
    printf("Initializing the TM ...\n\n");
    tm->tr = (State *)malloc(sizeof(State));   //creates tm
    tm->Accept = (int *)malloc(sizeof(int));
}
//HEAPSORT function and support functions
int PARENT(int i){
    return i/2;
}
int LEFT(int i){
    return 2*i;
}
int RIGHT(int i){
    return 2*i + 1;
}
void MAX_HEAPIFY(array *A, int i, char **line){
    int max;
    char* tempLine;
    int temp;
    int l = LEFT(i);
    int r = RIGHT(i);
    
    if(l <= A->heapSize && A->heapArray[l] > A->heapArray[i])
        max = l;
    else max = i;
    if(r <= A->heapSize && A->heapArray[r] > A->heapArray[i])
        max = r;
    if( max != i)
    {
        //swap A[i] <-> A[max] ma anche di line
        temp = A->heapArray[i];
        tempLine = line[i];
        A->heapArray[i] = A->heapArray[max];
        line[i] = line[max];
        A->heapArray[max] = temp;
        line[max] = tempLine;
        //end swap.
        MAX_HEAPIFY(A, max, line);
    }
}
void BUILD_MAX_HEAP(array *A, char** line)
{
    int i;
    A->heapSize = A->Length;
    for(i = A->Length/2; i >= 1; i--)
        MAX_HEAPIFY(A, i, line);
}
void HeapSort1(array *A, char **line)
{
    int i;
    char* tempLine;
    int temp;
    BUILD_MAX_HEAP(A, line);
    for(i = A->Length/2; i >= 0; i--)
    {
        temp = A->heapArray[0];
        tempLine = line[0];
        A->heapArray[0] = A->heapArray[i];
        line[0] = line[i];
        A->heapArray[i] = temp;
        line[i] = tempLine;
        MAX_HEAPIFY(A, 0, line);
    }
    
}
//HEAPSORT end.
void sortTr(char *InputFile){
    FILE *fp1, *fp2;
    Heap mheap;
    char outputFile[25] = "tr_sorted.txt";
    char c;
    int states = 0, i = 0;
    array A;
    char **heapLine = NULL;
    char *Line = NULL;
    
    fp1 = fopen("input1.txt", "r");
    if(fp1 == NULL)
        printf("Unable to open input file...\n\nLoading aborted!\n\n");
    else
    {
        printf("Input file: %s\n", InputFile);
        printf("Success in opening input file!\n");
        Line = (char *)malloc(sizeof(char));
        fgets(Line, 100, fp1);       //first line of the file (tr)
        printf("%s", Line);
        
        do
        {
            states++;
            //printf("%d) ", states);
            fscanf(fp1, "%d %c %c %c %d\n", &mheap.start, &mheap.input, &mheap.output, &mheap.hmove, &mheap.end);
            //printf("%d %c %c %c %d\n", mheap.start, mheap.input, mheap.output, mheap.hmove, mheap.end);
            c = fgetc(fp1);          //"first character of the line
            if(c != 'a')
                fseek(fp1, -1, SEEK_CUR);
        }while(c != 'a');
        
        A.heapArray = (int *)malloc(states*sizeof(int));  //array of the positions of the lines
        A.Length = states;
        A.heapSize = states;
        heapLine = (char **)malloc(states*sizeof(char *)); //array of associated lines
        
        fseek(fp1, 3, SEEK_SET);    //salto riga di tr;
        do
        {
            i++;
            //printf("%d) ", i);
            fscanf(fp1, "%d", &A.heapArray[i-1]);
            fseek(fp1, -1, SEEK_CUR);
            heapLine[i-1] = (char *)malloc(sizeof(char));
            fgets(heapLine[i-1], 100, fp1);
            //printf("%d\t", A.heapArray[i-1]);
            //printf("%s", heapLine[i-1]);
            c = fgetc(fp1);          //first character of the line
            if(c != 'a')
                fseek(fp1, -1, SEEK_CUR);
        }while(c != 'a');
        // controllo per vedere se gli stati sono stati memorizzati per bene
        for(i = 0; i < states; i++)
        {
            printf("%d) %d\t", i+1, A.heapArray[i]);
            printf("%i:\t", (unsigned int) *(heapLine + i));
            printf("%s\n", *(heapLine + i));
        }
        printf("\n---------------\n");
        HeapSort1(&A, heapLine);
        // controllo per vedere se gli stati sono stati memorizzati per bene
        for(i = 0; i < states; i++)
        {
            printf("%d) %d\t", i+1, A.heapArray[i]);
            printf("%s", heapLine[i]);
        }
    }
    fp2 = fopen(outputFile, "w");
    if(fp2 == NULL)
        printf("Unable to open output file...\n\nLoading aborted!\n\n");
    else
    {
        
    }
}
void Load_TM(TM *tm, char *filename)           // loading TM from commandline
{
    FILE *fp;
    int start, end, c, i = 0;
    char input, output, hmove;
    char *Line = (char *)malloc(sizeof(char));
    
    printf("Loading the TM from file ...\n\n");
    fp = fopen(filename, "r");
    if(fp == NULL)
        printf("Unable to open input file...\n\nLoading aborted!\n\n");
    else
    {
        printf("Input file: %s\n", filename);
        printf("Success in opening input file!\n");
        fgets(Line, 100, fp);       //first line of the file (tr)
        printf("%s", Line);
        
        do
        {
            fscanf(fp, "%d %c %c %c %d\n", &start, &input, &output, &hmove, &end);
            printf("%d %c %c %c %d\n", start, input, output, hmove, end);
            c = fgetc(fp);          //"first character of the line
            if(c != 'a')
                fseek(fp, -1, SEEK_CUR);
        }while(c != 'a');
        
        fseek(fp, -1, SEEK_CUR);
        fscanf(fp, "%s", Line);     //"acc" line
        printf("%s\n", Line);
        fscanf(fp, "%d\n", tm->Accept);     //final state (only one just for now)
        printf("%d\n", *tm->Accept);
        fscanf(fp, "%s\n", Line);     //"max" line
        printf("%s\n", Line);
        fscanf(fp, "%d\n", &tm->max);       //max steps
        printf("%d\n", tm->max);
        fscanf(fp, "%s\n", Line);   //"run" line
        printf("%s\n", Line);
        
        while(!feof(fp))
        {
            i++;
            fscanf(fp, "%s\n", Line);   //one input string
            printf("[%d] Input string: %s\n", i, Line);
        }
        printf("Loading completed!\n");
    }
    
}
void createSt(State *tr){
    tr->input_next = (inputHash *)malloc(sizeof(inputHash));
    tr->input_next->end_next = (endList *)malloc(sizeof(endList));
}

int main(int argc, char *argv[]) {
    /*
    if(argc > 3 || argc < 2)
    {
        printf("\nWARNING:  Invalid number of arguments!\n\n");
        printf("Usage: ./tm < <inputfile> <outputfile>\n\n");
        
        return -1;
    }*/
    sortTr(argv[1]);
    /*
    TM tm;
    State *tempTr;
    
    init_TM(&tm);
    Load_TM(&tm, argv[1]);
    
    //createSt(tm.tr);                                    //creates tr
    tempTr = tm.tr;
    */
    return 0;
}
