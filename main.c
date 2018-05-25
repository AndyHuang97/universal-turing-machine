#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void init_TM(TM *);
void Load_TM(TM *, char *);
void createSt(State *);

void init_TM(TM *tm)
{
    printf("Initializing the TM ...\n\n");
    tm->tr = (State *)malloc(sizeof(State));   //creates tm
    tm->Accept = (int *)malloc(sizeof(int));
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
    TM tm;
    State *tempTr;
    
    init_TM(&tm);
    Load_TM(&tm, argv[1]);
    
    //createSt(tm.tr);                                    //creates tr
    tempTr = tm.tr;
    
    return 0;
}
