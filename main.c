/* A nondeterministic TM simulator implementation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* ---------------------------------------------------------------------- */

typedef struct configuration{
    int state;          //state of the OC;
    char *String;    //input string;
    int pos;            //position of the head on the tape(string);
    struct configuration *next;     //for list of configurations.
    struct configuration *prev;
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
    int start;               //start state
    inputHash *input_next;
}State;
typedef struct machine{
    State **tr;
    config *snapshot;//memorizes the OC's current state and input string's head
    int *Accept;    //array of final states;
    int numAcc;
    long max;        //condition for termination if it's looping
}TM;

/* ---------- Private function declarations ---------- */
void init_TM(TM *);
void initInputHash(inputHash **);
endList *createEndList(char, char, int);
void ENDLIST_INSERT(endList **, endList *);
inputHash *LIST_SEARCH(State *, char);
int LoadTM(TM *);
void LIST_INSERT(State *, inputHash *);
inputHash *INPUT_SEARCH(TM , int , char );      //solo per controllo

config *MKconfig(void);
config *ND_Step(config **, TM , int *);
char *LOAD_STRING(void);

/* ---------------------------------------------------------------------- */
void init_TM(TM *tm) {
    //printf("Initializing the TM ...\n\n");
    tm->tr = (State **)malloc(sizeof(State *));   //array of ptrs to State
    tm->tr[0] = (State *)malloc(sizeof(State));
    tm->tr[0]->start = 0;
    tm->tr[0]->input_next = NULL;
    tm->Accept = (int *)malloc(sizeof(int));//init list of accepted states
    tm->numAcc = 1;
    tm->snapshot = MKconfig();
}

/* ---------------------------------------------------------------------- */
void initInputHash(inputHash **S) {
    *S = (inputHash *)malloc(sizeof(inputHash));
    (*S)->input= 0;
    (*S)->next = NULL;
    (*S)->prev = NULL;
    (*S)->end_next = NULL;
}

/* ---------------------------------------------------------------------- */
endList *createEndList(char output, char hmove, int end) {
    endList *new = NULL;
    new = (endList *) malloc(sizeof(endList));
    new->output = output;
    new->hmove = hmove;
    new->end = end;
    new->next = NULL;
    return new;
}

/* ---------------------------------------------------------------------- */
void ENDLIST_INSERT(endList **L, endList *el) {
    if(*L == NULL)
        *L = el;
    else {
        el->next = *L;
        *L = el;
    }
}
/* ---------------------------------------------------------------------- */
//SEARCH...
inputHash *LIST_SEARCH(State *L, char k) {
    inputHash *x = NULL;
    if(L != NULL) {
        x = L->input_next;
        while(x != NULL && x->input != k)
            x = x->next;
    }
    return x;
}

/* ---------------------------------------------------------------------- */
//INSERTION... (to head)
void LIST_INSERT(State *L, inputHash *x) {
    if(L->input_next != NULL) {
        x->next = L->input_next;
        if(L->input_next != NULL)
            L->input_next->prev = x;
    }
    L->input_next = x;
}

/* ---------------------------------------------------------------------- */
int LoadTM(TM *tm) {
    char input, output, hmove, c, Line[4];
    int start, end, states = 0, max = 0;
    int i, intercept;
    //State **old_addr = tm->tr;
    inputHash *tempInputH;
    endList *new = NULL;

    intercept = scanf("%s", Line);
    do {
        //reads the lines of the transition function
        //printf("input data: ");
        intercept = scanf("%d %c %c %c %d\n", &start, &input, &output, &hmove, &end);
        //printf("%d %c %c %c %d\n", start, input, output, hmove, end);
        if(max < start || max < end) {
            if(start > end)
                max = start;
            else
                max = end;
        }
        if(max > states) {//if start>states then reallocate new memory for it
            tm->tr = (State **)realloc(tm->tr, (max+2)*sizeof(State *));//realloc max+1 objects.
            tm->tr[max+1] = NULL;//last object of dynamic array not to be realloced
            i = max;
            while(i > states) {   //just malloc for the newly added states(cells)
              //check from the last cell and allocates new memory for it. It is assumed that the first positions have already their pointers.
                tm->tr[i] = (State *)malloc(sizeof(State));
                tm->tr[i]->start = i;
                tm->tr[i]->input_next = NULL;
                i -= 1;
            }
            states = max;
        }
        tempInputH = LIST_SEARCH(tm->tr[start], input);
        if(tempInputH == NULL) {
            initInputHash(&tempInputH); //init an inputHash element
            tempInputH->input = input;
            LIST_INSERT(tm->tr[start], tempInputH);
            //finishing one step of the tr function
            new = createEndList(output, hmove, end);
            ENDLIST_INSERT(&(tm->tr[start]->input_next->end_next), new);
        }
        else
            if(tempInputH != NULL)  //finishing one step of the tr function
            {
                new = createEndList(output, hmove, end);
                ENDLIST_INSERT(&(tempInputH->end_next), new);
            }
        c = getc(stdin);            //used to recognize the "acc" line
        ungetc(c, stdin);           //puts back the read character
    }while(c != 'a');
    //printf("Transition part completed!\n");
    intercept = scanf("%s", Line);      //"acc" line
    //printf("%s\n", Line);
    i = 0;                          //using it for accept states array
    do {
        intercept = scanf("%d\n", &(tm->Accept[i]));     //final state
        c = getc(stdin);            //used to recognize the "max" line
        ungetc(c, stdin);           //puts back the read character
        i++;
        if(c != 'm')
        {
            tm->Accept = (int *)realloc(tm->Accept, (i+2)*sizeof(int));
            tm->numAcc += 1;
        }
    }while(c != 'm');
    /*for(int count = 0; count < i; count++)
        printf("%d\n", tm->Accept[count]);*/
    intercept = scanf("%s\n", Line);     //"max" line
    //printf("%s\n", Line);
    intercept = scanf("%li\n", &tm->max);       //max steps
    //printf("%d\n", tm->max);
    intercept = scanf("%s\n", Line);   //"run" line
    //printf("%s\n", Line);
    return max;
}

/* ---------------------------------------------------------------------- */
//loading input string functions
char *LOAD_STRING(){
    int dim = 1;
    char *string = (char *)malloc(2*sizeof(char));
    char c;
    
    string[0] = '_';
    string[1] = '\0';
    c = getc(stdin);
    while(c != '\n' && c != EOF)
    {
        string[dim] = c;
        dim++;
        string = (char *)realloc(string, (dim+2)*sizeof(char));
        string[dim] = '_';
        c = getc(stdin);
        if(c == '\r')
            c = getc(stdin);
    }
    string[dim+1] = '\0';
    return string;
}

/* ---------------------------------------------------------------------- */
void VisualizeString(char* string) {
    int i = 1;
    while(string[i] != '_')
    {
        printf("%c",string[i]);
        i++;
    }
    printf("\n");
    free(string);
}
/* ---------------------------------------------------------------------- */
//solo per controllo (per ora)
inputHash *INPUT_SEARCH(TM tm, int start, char input) {
    inputHash *x = tm.tr[start]->input_next;
    while(x != NULL && x->input != input)
        x = x->next;
    return x;
}

/* ---------------------------------------------------------------------- */
//initialize a new config
config *MKconfig(){
    config *c = (config *)malloc(sizeof(config));
    c->state = 0;
    c->String = NULL;
    c->pos = 0;
    c->next = NULL;
    c->prev = NULL;
     return c;
}

/* ---------------------------------------------------------------------- */
//adding a new confi to list
void ADDconfig(config *root, config *leaf){
    config *temp = root;
    while(temp->next != NULL)
        temp = temp->next;
    temp->next = leaf;
    leaf->prev = temp;
}
config *MergeConfig(config **config1, config **config2){
    config *temp = *config1;
    if(*config1 != NULL && *config2 != NULL)
    {
        while(temp->next != NULL)
            temp = temp->next;
        temp->next = *config2;
        (*config2)->prev = temp;
    }
    else
        if(*config1 == NULL)
            *config1 = *config2;
    return *config1;
}
/* ---------------------------------------------------------------------- */
void ND_Transition(TM *tm){
    int i = 0, accept = 0;
    config *snapshot = tm->snapshot;
    config *old_addr = NULL, *merge1 = NULL, *merge2 = NULL;
    
    while (i < tm->max && !accept && tm->snapshot != NULL) {
        snapshot = tm->snapshot;
        old_addr = snapshot->next;
        //printf("%d)\n",i);
        merge1 = ND_Step(&snapshot, *tm, &accept);
        while(old_addr != NULL && !accept) {
            snapshot = old_addr;
            old_addr = snapshot->next;
            merge2 = ND_Step(&snapshot, *tm, &accept);
            merge1 = MergeConfig(&merge1, &merge2);
        }
        tm->snapshot = merge1;
        if(!accept)
            i++;
    }
    if(accept)
        printf("1");
    if(i == tm->max)
        printf("U");
    if(tm->snapshot == NULL)
        printf("0");
    tm->snapshot = MKconfig();
}

/* ---------------------------------------------------------------------- */
config *ND_Step(config **c, TM tm, int *accept){
    inputHash *inputH = NULL;
    config *oneconfig = NULL;
    config *root = NULL;
    endList *choice = NULL;
    char *string = NULL;
    int flag = 0;
    
    inputH = INPUT_SEARCH(tm, (*c)->state, (*c)->String[(*c)->pos]);//get the list of steps with input key; can navigate list with 'inputH->end_next->next'
    if(inputH != NULL) {
        choice = inputH->end_next;
        while(choice != NULL) {      //all possible steps on the input
            //printf("BEFORE:\t%s\tS:%d\tH:%d\n", (*c)->String, (*c)->state, (*c)->pos);
            string = (char *) malloc(strlen((*c)->String)+5);
            memcpy(string, (*c)->String, strlen((*c)->String)+1);//copio la stringa + '\0'
            oneconfig = MKconfig();  //creates a config var to pass to tm at the end;
            
            for(int i = 0; i < tm.numAcc; i++) //checks if it's finale state;
                if(choice->end == tm.Accept[i]) {
                    *accept = 1;
                    //return *c;
                }
            oneconfig->state = choice->end;         //1)update state
            oneconfig->String =  string;
            oneconfig->pos = (*c)->pos;
            oneconfig->String[oneconfig->pos] = choice->output;   //2)overwrite
            switch (choice->hmove) {      //3)move head
                case 'L':
                    if(oneconfig->pos >= strlen(oneconfig->String)-1) {
                        string[strlen(string)+1] = '\0';
                        string[strlen(string)] = '_';
                    }
                    if(oneconfig->pos == 0) {//pos doesn't change, remains 0
                        memmove(&string[1], string, strlen(string)+1);//shifts string of one position to the right.
                        string[0] = '_';
                    }
                    else
                        oneconfig->pos -= 1 ;//updates pos of one position to the left
                    break;
                case 'R':
                    if(oneconfig->pos == 0) {//pos doesn't change, remains 0
                        memmove(&string[1], string, strlen(string)+1);//shifts string of one position to the right.
                        string[0] = '_';
                        oneconfig->pos += 1;
                    }
                    //printf("strlen(oneconfig->String): %lu \n",strlen(oneconfig->String));
                    if(oneconfig->pos >= strlen(string)-1) {
                        string[strlen(string)] = '_';
                        string[strlen(string)+1] = '\0';
                    }
                    oneconfig->pos += 1;
                    break;
                case 'S'://does nothing
                    break;

                default:
                    printf("Default case: not readable head movement");
                    break;
            }
            //printf("AFTER:\t%s\tS:%d\tH:%d\n", oneconfig->String, oneconfig->state, oneconfig->pos);
            //linking different paths as list;
            if(flag == 0) {
                root = oneconfig;
                flag = 1;
            }
            else
                ADDconfig(root, oneconfig);
            choice = choice->next;
            //printf("\n");
        }
        free((*c)->String); //freeing head of preceding list;
        free(*c);
        *c = root;
        return root;
    }
    else {
        //printf("STRING:\t%s\tS:%d\tH:%d\tDELETED\n\n",(*c)->String,(*c)->state,(*c)->pos);
        free((*c)->String);
        (*c)->String = NULL;
        if((*c)->prev != NULL) {
            (*c)->prev =  (*c)->next;
            if((*c)->next != NULL)
                (*c)->next->prev = (*c)->prev;
        }
        else {
            root = (*c)->next;
            if((*c)->next != NULL)
                (*c)->next->prev = NULL;
        }
        free(*c);
        *c = root;
        return NULL;
    }
}
 
/* ---------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    //clock_t tStart = clock();
    int max;
    char c;
    TM machine;
    
    freopen(argv[1], "r", stdin); //check from lldb
    init_TM(&machine);
    max = LoadTM(&machine);
    c = getc(stdin);   //read character to see if it is EOF
    ungetc(c, stdin);   //puts back the read character
    while(c != EOF) {
        machine.snapshot->String = LOAD_STRING();         //load one string
        machine.snapshot->pos = 1;  //strings have this format _"string"_, pos=1;
        ND_Transition(&machine);
        c = getc(stdin);           //needed to check the first character of the string
        if(c != EOF)
            printf("\n");
        ungetc(c, stdin);
    }
    free(machine.Accept);
    machine.Accept = NULL;
    //printf("\nTime taken: %2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    return 0;
}
