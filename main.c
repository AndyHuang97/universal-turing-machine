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
    struct inputHash *prev;
    struct endList *end_next;
}inputHash;
typedef struct State{        //Main hashtable ordered according to states
    int start;                   //start state
    struct inputHash *input_next;
}State;
typedef struct machine{
    State **tr;
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
/*
 Functions initializations...
 
 */
void init_TM(TM *, int);
int PARENT(int);
int LEFT(int);
int RIGHT(int);
void MAX_HEAPIFY(array *, int, char **);
void BUILD_MAX_HEAP(array *, char **);
void HeapSort1(array *, char **);

int sortTr(char *);
inputHash *LIST_SEARCH(State *L, char k);
void LIST_INSERT(State *L, inputHash *x);
void Load_TM(TM *, char *);
void createSt(State *);

//------------------------------------------------------------
//INITIALIZATION functions;
void initInputHash(inputHash **S){
    *S = (inputHash *)malloc(sizeof(inputHash));
    (*S)->input= 0;
    (*S)->next = NULL;
    (*S)->prev = NULL;
    (*S)->end_next = NULL;
}
void init_TM(TM *tm, int hashDim)
{
    printf("Initializing the TM ...\n\n");
    tm->tr = (State **)malloc(hashDim * sizeof(State*));   //creates tm
    tm->Accept = (int *)malloc(sizeof(int));
}
//------------------------------------------------------------
//HEAPSORT function and support functions
int PARENT(int i){
    return i/2;
}
int LEFT(int i){
    return 2*i + 1;
}
int RIGHT(int i){
    return 2*i + 2;
}
void MAX_HEAPIFY(array *A, int i, char **line){
    int max;
    char* tempLine;
    int temp;
    int l = LEFT(i);
    int r = RIGHT(i);
    
    if(l < A->heapSize && A->heapArray[l] > A->heapArray[i])
        max = l;
    else max = i;
    if(r < A->heapSize && A->heapArray[r] > A->heapArray[max])
        max = r;
    if( max != i)
    {
        //printf("A[%d] <-> A[%d]\n\n", i+1, max+1);
        //swap A[i] <-> A[max] ma anche di line
        temp = A->heapArray[i];
        tempLine = line[i];
        A->heapArray[i] = A->heapArray[max];
        line[i] = line[max];
        A->heapArray[max] = temp;
        line[max] = tempLine;
        //end swap.
        /*  //testing
        for(i = 0; i < A->heapSize; i++)
        {
            printf("%d) %d\t", i+1, A->heapArray[i]);
            //printf("%i:\t", (unsigned int) Line[i]);
            printf("%s", line[i]);
        }
        printf("\n-------------------\n");
        //end testing.
         */
        MAX_HEAPIFY(A, max, line);
    }
    
}
void BUILD_MAX_HEAP(array *A, char** line)      //Working!
{
    int i;
    A->heapSize = A->Length;
    for(i = A->Length/2; i >= 0; i--)
        MAX_HEAPIFY(A, i, line);
}
void HeapSort1(array *A, char **line)
{
    int i;
    char* tempLine;
    int temp;
    BUILD_MAX_HEAP(A, line);
    //printf("MAX-HEAP COMPLETED!\n-------------------\n");
    for(i = A->Length - 1; i >= 1; i--)
    {
        //swap A[0] <-> A[i]
        //printf("A[1] <-> A[%d]\n\n", i+1);
        temp = A->heapArray[0];
        tempLine = line[0];
        A->heapArray[0] = A->heapArray[i];
        line[0] = line[i];
        A->heapArray[i] = temp;
        line[i] = tempLine;
        A->heapSize = A->heapSize - 1;
        /*  //testing
        for(i = 0; i < A->heapSize; i++)
        {
            printf("%d) %d\t", i+1, A->heapArray[i]);
            //printf("%i:\t", (unsigned int) Line[i]);
            printf("%s\n", line[i]);
        }
        printf("\nEND SWAP 1\n-------------------\n");
        */
        //end swap.
        MAX_HEAPIFY(A, 0, line);
    }
}
//HEAPSORT end.
//------------------------------------------------------------
//Sorting the transition function
int sortTr(char *InputFile){
    FILE *fp1;
    //FILE *fp2 = NULL;
    Heap mheap;
    char c;
    int states = 0, count = 0, i = 0, position = 0;
    array A;
    char **heapLine = NULL;
    char *Line = NULL;
    //char outputFile[25] = "tr_sorted.txt";
    
    fp1 = fopen(InputFile, "r+");
    //fp1 = fopen("input2.txt", "r+");
    if(fp1 == NULL)
        printf("Unable to open input file...\n\nLoading aborted!\n\n");
    else
    {
        printf("Input file: %s\n", InputFile);
        printf("Success in opening input file!\n");
        Line = (char *)malloc(sizeof(char));
        fgets(Line, 100, fp1);       //first line of the file (tr)
        //printf("%s", Line);
        do
        {
            states++;
            //printf("%d) ", states);
            /*
            fscanf(fp1, "%d %c %c %c %d\n", &mheap.start, &mheap.input, &mheap.output, &mheap.hmove, &mheap.end);
            //printf("%d %c %c %c %d\n", mheap.start, mheap.input, mheap.output, mheap.hmove, mheap.end);
             */
            fgets(Line, 100, fp1);
            //printf("%s",Line);
            c = fgetc(fp1);          //"first character of the line
            if(c != 'a')
                fseek(fp1, -1, SEEK_CUR);
        }while(c != 'a');
        //printf("\n=========\n");
        A.heapArray = (int *) malloc(states*sizeof(int));  //array of the positions of the lines
        A.Length = states;
        A.heapSize = states;
        heapLine = (char **) malloc(states*sizeof(char *)); //array of associated lines
        
        fseek(fp1, 3, SEEK_SET);    //salto riga di tr;
        do
        {
            i++;
            //printf("%d) ", i);
            
            //printf("diff: %d\nSEEK_CUR: %d\nSEEK_SET: %d\n", diff, SEEK_CUR, SEEK_SET);
            position = (int) ftell(fp1);
            //printf("Offset from SEEK_SET: %d\n", position);
            fscanf(fp1, "%d", &A.heapArray[i-1]);
            fseek(fp1, position, SEEK_SET);
            heapLine[i-1] = (char *)malloc(sizeof(char));
            fgets(heapLine[i-1], 100, fp1);
            //printf("%d\t", A.heapArray[i-1]);
            //printf("%s", heapLine[i-1]);
            c = fgetc(fp1);          //first character of the line
            if(c != 'a')
                fseek(fp1, -1, SEEK_CUR);
        }while(c != 'a');
        /*
        // controllo per vedere se gli stati sono stati memorizzati per bene
        for(i = 0; i < states; i++)
        {
            printf("%3d) State:%3d\t", i+1, A.heapArray[i]);
            printf("Transition:%s", *(heapLine + i));
        }
        */
        printf("\n-------------------\n");
        HeapSort1(&A, heapLine);
        printf("HEAP-SORT COMPLETED!\n-------------------\n");
        /*
        // controllo per vedere se gli stati sono stati memorizzati per bene
        for(i = 0; i < states; i++)
        {
            printf("%3d) State:%3d\t", i+1, A.heapArray[i]);
            printf("Transition:%s", *(heapLine + i));
        }
         */
        fseek(fp1, 3, SEEK_SET);
        int tempstart = -1;
        do
        {
            fscanf(fp1, "%d %c %c %c %d\n", &mheap.start, &mheap.input, &mheap.output, &mheap.hmove, &mheap.end);
            printf("%d %c %c %c %d\n", mheap.start, mheap.input, mheap.output, mheap.hmove, mheap.end);
            
            //checking if the transition is in the same state
            
            if(tempstart == -1)//first loop
            {   //initialize tr[state] and hashInput;
                tempstart = mheap.start;
                count++;
                
                //initInputHash(&tm->tr[state-1]->input_next);
            }
            else
                if(tempstart != mheap.start)//change state when there is a new one
                {
                    count++;
                    tempstart = mheap.start;
                }
            //------------------------------------------------
            c = fgetc(fp1);          //"first character of the line
            if(c != 'a')
                fseek(fp1, -1, SEEK_CUR);
        }while(c != 'a');
        printf("NUMBER OF STATES = %d\n", count);
        fseek(fp1, 3, SEEK_SET);
        for(i = 0; i < states; i++)//scrittura su file
        {
            fprintf(fp1, "%s", heapLine[i]);
        }
    }
    fclose(fp1);
    /*fp2 = fopen(outputFile, "w");
    if(fp2 == NULL)
        printf("Unable to open output file...\n\nLoading aborted!\n\n");
    else
    {
        fprintf(fp2, "tr\n");
        //fseek(fp1, 3, SEEK_SET);
        for(i = 0; i < states; i++)
        {
            fprintf(fp2, "%s", heapLine[i]);
        }
    }
     */
    printf("NUMBER OF TRANSITIONS: %d\n", states);
    return count;
}
//------------------------------------------------------------
//SEARCH...
inputHash *LIST_SEARCH(State *L, char k){
    inputHash *x = L->input_next;
    while(x != NULL && x->input != k)
        x = x->next;
    return x;
}
//INSERTION... (to head)
void LIST_INSERT(State *L, inputHash *x){
    if(L->input_next != NULL)
    {
        x->next = L->input_next;
        if(L->input_next != NULL)
            L->input_next->prev = x;
    }
    L->input_next = x;
}
//------------------------------------------------------------
// Loading TM from commandline after sorting the transition function
void Load_TM(TM *tm, char *filename)
{
    FILE *fp;
    int c, state = 0, tempstart = -1, start;
    //int i = 0;
    char input, output, hmove;
    int end;
    char *Line = (char *)malloc(sizeof(char));
    inputHash *tempInputH;
    
    printf("Loading the TM from file ...\n\n");
    fp = fopen(filename, "r");
    //fp = fopen("input2.txt", "r");
    if(fp == NULL)
        printf("Unable to open input file...\n\nLoading aborted!\n\n");
    else
    {
        printf("Loading to memory...\n\n");
        fseek(fp, 3, SEEK_SET);       //first line of the file (tr)
        printf("%s", Line);
        do
        {
            fscanf(fp, "%d %c %c %c %d\n", &start, &input, &output, &hmove, &end);
            //printf("%d %c %c %c %d\n", start, input, output, hmove, end);
            
            //checking if the transition is in the same state
            if(tempstart == -1)//first loop
            {   //initialize tr[state] and hashInput;
                tempstart = start;
                state++;
                tm->tr[state-1] = (State *)malloc(sizeof(State));
                tm->tr[state-1]->start = start;
                tm->tr[state-1]->input_next = NULL;
                //initInputHash(&tm->tr[state-1]->input_next);
            }
            else
                if(tempstart != start)//change state when there is a new one
                {
                    state++;
                    tm->tr[state-1] = (State *)malloc(sizeof(State));
                    tm->tr[state-1]->start = start;
                    tm->tr[state-1]->input_next = NULL;
                    tempstart = start;
                }
            //------------------------------------------------------------
            //checking if the current input is already present in the list
            tempInputH = LIST_SEARCH(tm->tr[state-1], input);
            if(tempInputH == NULL)
            {
                initInputHash(&tempInputH);
                tempInputH->input = input;
                LIST_INSERT(tm->tr[state-1], tempInputH);
                
                tm->tr[state-1]->input_next->end_next = (endList *) malloc(sizeof(endList));
                tm->tr[state-1]->input_next->end_next->output = output;
                tm->tr[state-1]->input_next->end_next->hmove = hmove;
                tm->tr[state-1]->input_next->end_next->end = end;
                tm->tr[state-1]->input_next->end_next->next = NULL;
            }
            else
                if(tempInputH != NULL)
                {
                    tempInputH->end_next->next = (endList *) malloc(sizeof(endList));
                    tempInputH->end_next->next->output = output;
                    tempInputH->end_next->next->hmove = hmove;
                    tempInputH->end_next->next->end = end;
                    tempInputH->end_next->next->next = NULL;
                }
            /*
            while(tm->tr[state-1]->input_next != NULL)
            {
                if(tm->tr[state-1]->input_next->next == NULL)
                    tm->tr[state-1]->input_next->next = tempInputH;
                tm->tr[state-1]->input_next = tm->tr[state-1]->input_next->next;
            }
             */
            //tm->tr[state-1].input_next->input = input;
            c = fgetc(fp);          //"first character of the line
            if(c != 'a')
                fseek(fp, -1, SEEK_CUR);
        }while(c != 'a');
        
        fseek(fp, -1, SEEK_CUR);
        fscanf(fp, "%s", Line);     //"acc" line
        //printf("%s\n", Line);
        fscanf(fp, "%d\n", tm->Accept);     //final state (only one just for now)
        //printf("%d\n", *tm->Accept);
        fscanf(fp, "%s\n", Line);     //"max" line
        //printf("%s\n", Line);
        fscanf(fp, "%d\n", &tm->max);       //max steps
        //printf("%d\n", tm->max);
        fscanf(fp, "%s\n", Line);   //"run" line
        //printf("%s\n", Line);
        /*
        i = 0;
        while(!feof(fp))
        {
            i++;
            fscanf(fp, "%s\n", Line);   //one input string
            printf("[%d] Input string: %s\n", i, Line);
        }
         */
        printf("Loading completed!\n\n");
        fclose(fp);
        
    }
    
}
//------------------------------------------------------------
//SEARCH in TM functions
//Starting from state "start"(int) and head on "input"(char), find the step of the transition
inputHash *INPUT_SEARCH(TM tm, int start, char input) {
    inputHash *x = tm.tr[start]->input_next;
    while(x != NULL && x->input != input)
        x = x->next;
    return x;
}

int main(int argc, char *argv[]) {
    int hashDim, start;
    char input;
    TM tm;
    inputHash *tempInput;
    //State **tempTr;
    
    if(argc > 3 || argc < 2)
    {
        printf("\nWARNING:  Invalid number of arguments!\n\n");
        printf("Usage: ./tm < <inputfile> <input to check>\n\n");
        
        return -1;
    }
    hashDim = sortTr(argv[1]);
    
    init_TM(&tm, hashDim);
    
    Load_TM(&tm, argv[1]);
    
    //checking if the stored data is correct... madness
    start = 0;
    int i = 0;
    input = *argv[2];        //change input to view;
    while(i < 30 && start < hashDim)
    {
        tempInput = INPUT_SEARCH(tm, start, input);
        if(tempInput != NULL)
        {
            while(tempInput->end_next != NULL)
            {
                printf("%3d) State:%3d |Input: %c\t", i+1, start, input);
                if(tempInput->end_next != NULL)
                    printf("Transition:%d %c %c %c %d\n", tm.tr[start]->start, tempInput->input, tempInput->end_next->output, tempInput->end_next->hmove, tempInput->end_next->end);
                else
                    printf("Transition1: [BLANK]\n");
                tempInput->end_next = tempInput->end_next->next;
                i++;
            }
        }
        else
        {
            printf("%3d) State:%3d |Input: %c\t", i+1, start, input);
            printf("Transition2: [BLANK]\n");
            i++;
        }
        start++;
        //i++;
    }
    return 0;
}
