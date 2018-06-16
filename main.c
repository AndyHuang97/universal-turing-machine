#include <stdio.h>
#include <stdlib.h>

typedef struct CHAR{
    char key;
    struct CHAR *next;
    struct CHAR *prev;
}CHAR;
typedef CHAR *STRING;

typedef struct configuration{
    int state;          //state of the OC;
    CHAR *InStr;        //input string;
    CHAR *pos;            //position of the head on the tape(string);
    struct configuration *next;
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
    endList *end_next;
}inputHash;
typedef struct State{        //Main hashtable ordered according to states
    int start;                   //start state
    inputHash *input_next;
}State;
typedef struct machine{
    State **tr;
    config snapshot;//memorizes the OC's current state and input string's head
    int *Accept;    //array of final states;
    int max;        //condition for termination if it's looping
}TM;

void init_TM(TM *);
void initInputHash(inputHash **S);
void createEndList(endList **base, char output, char hmove, int end);
inputHash *LIST_SEARCH(State *L, char k);
void LoadTM(TM *);
void LIST_INSERT(State *L, inputHash *x);
inputHash *INPUT_SEARCH(TM tm, int start, char input);      //solo per controllo
CHAR *createEL(void);
CHAR *INSERT_TO_STRING(STRING *, CHAR *);
STRING LOAD_STRING(void);
void VisualizeCHAR(CHAR *); //solo per visualizzazione

void init_TM(TM *tm) {
    printf("Initializing the TM ...\n\n");
    tm->tr = (State **)malloc(sizeof(State *));   //array of ptrs to State
    tm->tr[0] = (State *)malloc(sizeof(State));
    tm->tr[0]->start = 0;
    tm->tr[0]->input_next = NULL;
    tm->Accept = (int *)malloc(sizeof(int));//init list of accepted states
    tm->snapshot.InStr = NULL;  //no input string loaded
    tm->snapshot.pos = NULL;
    tm->snapshot.next = NULL;
    tm->snapshot.pos = 0;
    tm->snapshot.state = 0;
    printf("Initialization complete!\n\n");
}
void initInputHash(inputHash **S) {
    *S = (inputHash *)malloc(sizeof(inputHash));
    (*S)->input= 0;
    (*S)->next = NULL;
    (*S)->prev = NULL;
    (*S)->end_next = NULL;
}
void createEndList(endList **base, char output, char hmove, int end){
    *base = (endList *) malloc(sizeof(endList));
    (*base)->output = output;
    (*base)->hmove = hmove;
    (*base)->end = end;
    (*base)->next = NULL;
}
//SEARCH...
inputHash *LIST_SEARCH(State *L, char k) {
    inputHash *x = NULL;
    if(L != NULL)
    {
        x = L->input_next;
        while(x != NULL && x->input != k)
            x = x->next;
    }
    return x;
}
//INSERTION... (to head)
void LIST_INSERT(State *L, inputHash *x) {
    if(L->input_next != NULL)
    {
        x->next = L->input_next;
        if(L->input_next != NULL)
            L->input_next->prev = x;
    }
    L->input_next = x;
}

void LoadTM(TM *tm){
    char input, output, hmove, c, Line[4];
    int start, end, states = 0;
    int i;
    inputHash *tempInputH;
    
    //while(fgetc(stdin) != '\n'){}   //reads line with string "tr"
    fscanf(stdin, "%s", Line);
    //printf("tr\n");
    
    do
    {
        //reads the lines of the transition function
        //printf("input data: ");
        fscanf(stdin, "%d %c %c %c %d\n", &start, &input, &output, &hmove, &end);
        //printf("%d %c %c %c %d\n", start, input, output, hmove, end);
        
        if(start > states)
        {//if start>states then reallocate new memory for it
            tm->tr = (State **)realloc(tm->tr, (start+2)*sizeof(State *));//realloc start +1 objects.
            tm->tr[start+1] = NULL;//last object of dynamic array not to be realloced
            i = start;
            while(i > states)   //just malloc for the newly added states(cells)
            {//check from the last cell and allocates new memory for it. It is assumed that the first positions have already their pointers.
                tm->tr[i] = (State *)malloc(sizeof(State));
                tm->tr[i]->start = i;
                tm->tr[i]->input_next = NULL;
                i -= 1;
            }
            states = start;
        }
        
        tempInputH = LIST_SEARCH(tm->tr[start], input);
        if(tempInputH == NULL)
        {
            initInputHash(&tempInputH); //init an inputHash el
            tempInputH->input = input;
            LIST_INSERT(tm->tr[start], tempInputH);
            //finishing one step of the tr function
            createEndList(&(tm->tr[start]->input_next->end_next), output, hmove, end);
        }
        else
            if(tempInputH != NULL)
                //finishing one step of the tr function
                createEndList(&(tempInputH->end_next->next), output, hmove, end);
        c = getc(stdin);        //used to recognize the "acc" line
        ungetc(c, stdin);       //puts back the read character
    }while(c != 'a');
        printf("Transition part completed!\n");
    //
    fscanf(stdin, "%s", Line);     //"acc" line
    //printf("%s\n", Line);
    i = 0;                         //using it for accept states array
    do{
        fscanf(stdin, "%d\n", &(tm->Accept[i]));     //final state
        c = getc(stdin);        //used to recognize the "max" line
        ungetc(c, stdin);       //puts back the read character
        i++;
        if(c != 'm')
            tm->Accept = (int *)realloc(tm->Accept, (i+2)*sizeof(int));
    }while(c != 'm');
    
    /*for(int count = 0; count < i; count++)
        printf("%d\n", tm->Accept[count]);*/
    fscanf(stdin, "%s\n", Line);     //"max" line
    //printf("%s\n", Line);
    fscanf(stdin, "%d\n", &tm->max);       //max steps
    //printf("%d\n", tm->max);
    fscanf(stdin, "%s\n", Line);   //"run" line
    //printf("%s\n", Line);
}
//loading input string functions
CHAR *createEL()
{
    CHAR *x = (CHAR *)malloc(sizeof(CHAR));
    x->key = '_';
    x->next = NULL;
    x->prev = NULL;
    return x;
}
CHAR *INSERT_TO_STRING(STRING *string, CHAR *x){
    (*string)->next = x;
    return x;//da cambiare
}

STRING LOAD_STRING(){
    char c;
    CHAR *x = NULL, *temp = NULL;
    STRING string = NULL;
    int first = 0;
    
    c = fgetc(stdin);
    while(c != '\n' && c != EOF)
    {
        if(first == 0)
        {
            string = createEL(); //first input char
            string->key = c;
            first = 1;
            temp = string;
        }
        else
        {
            x = createEL();
            x->key = c;
            temp->next = x;
            x->prev = temp;
            temp = temp->next;
        }
        c = fgetc(stdin);
    }
    return string;
}

void VisualizeCHAR(CHAR *L){
    CHAR *temp = L;
    if(L == NULL)
        printf("NULL\n");
    while(temp != NULL){
        printf("%c", temp->key);
        temp = temp->next;
    }
    printf("\n");
}
//solo per controllo
inputHash *INPUT_SEARCH(TM tm, int start, char input) {
    inputHash *x = tm.tr[start]->input_next;
    while(x != NULL && x->input != input)
        x = x->next;
    return x;
}
config *MKconfig()
{
    config *c = (config *)malloc(sizeof(config));
    c->InStr = NULL;
    c->next = NULL;
    c->pos = NULL;
    c->state = -1;
    return c;
}
void ADDconfig(config **root, config *leaf){
    config *temp = *root;
    while(temp != NULL)
        temp = temp->next;
    temp->next = leaf;
}
void ND_Transition(TM tm)
{
    
}
void ND_Step(config c, TM tm){
    inputHash *inputH = NULL;
    config *oneconfig = NULL;
    endList *choice = NULL;
    inputH = INPUT_SEARCH(tm, tm.snapshot.state, tm.snapshot.InStr->key);//get the list of steps with input key; can navigate list with 'inputH->end_next->next'
    if(inputH != NULL)
    {
        choice = inputH->end_next;
        while(choice != NULL)
        {
            oneconfig = MKconfig();  //creates a config var to pass to tm at the end;
            oneconfig->state = choice->end;         //update state
            oneconfig->pos->key = choice->output;   //overwrite
            switch (inputH->end_next->hmove) {      //move head;
                case 'L':
                    if(oneconfig->pos->prev == NULL)
                    {
                        oneconfig->pos->prev = createEL();
                        oneconfig->pos->prev->next = oneconfig->pos;
                        oneconfig->InStr = oneconfig->pos->prev;
                    }
                    oneconfig->pos = oneconfig->pos->prev;
                    break;
                case 'R':
                    if(oneconfig->pos->next == NULL)
                    {
                        oneconfig->pos->next = createEL();
                        oneconfig->pos->next->prev = oneconfig->pos;
                    }
                    oneconfig->pos = oneconfig->pos->next;
                    break;
                case 'S'://does nothing
                    break;
    
                default:
                    printf("Default case: not readable head movement");
                    break;
            }
            //linking different paths as list;
            
        }
    }
    else
    {
        
    }
}
int main(int argc, char *argv[])
{
    char c;
    TM machine;
    //freopen(argv[7], "r", stdin); //check from lldb
    init_TM(&machine);
    LoadTM(&machine);
    c = fgetc(stdin);
    ungetc(c, stdin);
    while(c != EOF)
    {
        machine.snapshot.InStr = LOAD_STRING();
    
        machine.snapshot.pos = machine.snapshot.InStr;//start:pos points to same addr as InStr
        VisualizeCHAR(machine.snapshot.InStr);
        c = fgetc(stdin);
        ungetc(c, stdin);
    }
    if(c == EOF)
        printf("EOF reached: %d\n", c);
}

