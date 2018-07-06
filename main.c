/* A nondeterministic TM simulator implementation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    config snapshot;//memorizes the OC's current state and input string's head
    int *Accept;    //array of final states;
    int max;        //condition for termination if it's looping
}TM;

/* ---------- Private function declarations ---------- */
void init_TM(TM *);
void initInputHash(inputHash **S);
void createEndList(endList **base, char output, char hmove, int end);
inputHash *LIST_SEARCH(State *L, char k);
void LoadTM(TM *);
void LIST_INSERT(State *L, inputHash *x);
inputHash *INPUT_SEARCH(TM tm, int start, char input);      //solo per controllo

void ND_Step(config **c, TM tm);
char *LOAD_STRING(void);

/* ---------------------------------------------------------------------- */
void init_TM(TM *tm) {
    printf("Initializing the TM ...\n\n");
    tm->tr = (State **)malloc(sizeof(State *));   //array of ptrs to State
    tm->tr[0] = (State *)malloc(sizeof(State));
    tm->tr[0]->start = 0;
    tm->tr[0]->input_next = NULL;
    tm->Accept = (int *)malloc(sizeof(int));//init list of accepted states
    tm->snapshot.String = NULL;  //no input string loaded
    tm->snapshot.pos = -1;
    tm->snapshot.next = NULL;
    tm->snapshot.pos = 0;
    tm->snapshot.state = 0;
    printf("Initialization complete!\n\n");
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
void createEndList(endList **base, char output, char hmove, int end){
    *base = (endList *) malloc(sizeof(endList));
    (*base)->output = output;
    (*base)->hmove = hmove;
    (*base)->end = end;
    (*base)->next = NULL;
}

/* ---------------------------------------------------------------------- */
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

/* ---------------------------------------------------------------------- */
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

/* ---------------------------------------------------------------------- */
void LoadTM(TM *tm){
    char input, output, hmove, c, Line[4];
    int start, end, states = 0;
    int i;
    //State **old_addr = tm->tr;
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
            tm->tr = (State **)realloc(tm->tr, (start+2)*sizeof(State *));//realloc start+1 objects.
            /*if(tm->tr != old_addr)
            {
                printf("States: %d\told: %x, new: %x \n", states, (unsigned int) old_addr, (unsigned int) tm->tr);
                old_addr = tm->tr;
            }*/
            tm->tr[start+1] = NULL;//last object of dynamic array not to be realloced
            i = start;
            while(i > states)   //just malloc for the newly added states(cells)
            {   //check from the last cell and allocates new memory for it. It is assumed that the first positions have already their pointers.
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
            if(tempInputH != NULL)  //finishing one step of the tr function
                createEndList(&(tempInputH->end_next->next), output, hmove, end);
        c = getc(stdin);            //used to recognize the "acc" line
        ungetc(c, stdin);           //puts back the read character
    }while(c != 'a');
        printf("Transition part completed!\n");
    //
    fscanf(stdin, "%s", Line);      //"acc" line
    //printf("%s\n", Line);
    i = 0;                          //using it for accept states array
    do{
        fscanf(stdin, "%d\n", &(tm->Accept[i]));     //final state
        c = getc(stdin);            //used to recognize the "max" line
        ungetc(c, stdin);           //puts back the read character
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
config *MKconfig()
{
    config *c = (config *)malloc(sizeof(config));
    c->state = -1;
    c->String = NULL;
    c->pos = -1;
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

/* ---------------------------------------------------------------------- */
void ND_Transition(TM *tm)
{
    config *snapshot = &tm->snapshot;
    config *old_addr = snapshot;
    while (snapshot != NULL) {
        ND_Step(&snapshot, *tm);
        if(snapshot == old_addr)
            snapshot = snapshot->next;
    }
}

/* ---------------------------------------------------------------------- */
void ND_Step(config **c, TM tm){
    inputHash *inputH = NULL;
    config *oneconfig = NULL;
    config *root = NULL;
    endList *choice = NULL;
    char *string = NULL;
    int flag = 0;
    
    inputH = INPUT_SEARCH(tm, (*c)->state, (*c)->String[(*c)->pos]);//get the list of steps with input key; can navigate list with 'inputH->end_next->next'
    if(inputH != NULL)
    {
        choice = inputH->end_next;
        while(choice != NULL)       //all possible steps on the input
        {
            //printf("sizeof(char) = %lu\n", sizeof(char));
            //printf("strlen(c->String) = %lu\n",strlen(c->String));
            printf("BEFORE: %s\tS:%d\tH:%d\n", (*c)->String, (*c)->state, (*c)->pos);
            string = (char *) malloc(strlen((*c)->String)+2);
            //printf("%d",(int) strlen(c->String));
            memcpy(string, (*c)->String, strlen((*c)->String)+1);
            oneconfig = MKconfig();  //creates a config var to pass to tm at the end;
            oneconfig->state = choice->end;         //1)update state
            oneconfig->String =  string;
            oneconfig->pos = (*c)->pos;
            oneconfig->String[oneconfig->pos] = choice->output;   //2)overwrite
            switch (choice->hmove) {      //3)move head
                case 'L':
                    if(oneconfig->pos == 0)//pos doesn't change, remains 0
                    {
                        memcpy(&string[1], oneconfig->String, strlen(oneconfig->String)+1);//shifts string of one position to the right.
                        string[0] = '_';
                    }
                    else
                        oneconfig->pos -= 1 ;//updates pos of one position to the left
                    break;
                case 'R':
                    if(oneconfig->pos >= strlen(oneconfig->String))
                    {
                        string[strlen(string)] = '_';
                        strcat(string, "\0");
                    }
                    oneconfig->pos += 1;
                    break;
                case 'S'://does nothing
                    break;

                default:
                    printf("Default case: not readable head movement");
                    break;
            }
            printf("AFTER: %s\tS:%d\tH:%d\n", oneconfig->String, oneconfig->state, oneconfig->pos);
            //linking different paths as list;
            if(flag == 0)
            {
                root = oneconfig;
                flag = 1;
            }
            else
                ADDconfig(root, oneconfig);
            choice = choice->next;
            printf("\n");
        }
        free((*c)->String);
    }
    else
    {
        free((*c)->String);
        if((*c)->prev != NULL)
        {
            (*c)->prev =  (*c)->next;
            (*c)->next->prev = (*c)->prev;
        }
        else
        {
            root = (*c)->next;
            (*c)->next->prev = NULL;
        }
        free((*c));
    }
    (*c) = root;
    
}
 
/* ---------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    char c;
    TM machine;
    freopen(argv[1], "r", stdin); //check from lldb
    init_TM(&machine);
    LoadTM(&machine);
    c = fgetc(stdin);   //read character to see if it is EOF
    ungetc(c, stdin);   //puts back the read character
    
    machine.snapshot.String = LOAD_STRING();         //load one string
    
    machine.snapshot.pos = 1;  //strings have this format _"string"_, pos=1;
    
    ND_Transition(&machine);
    
    
    /*while(c != EOF)
    {
        machine.snapshot.String = LOAD_STRING();         //load one string

        machine.snapshot.pos = 1;  //strings have this format _"string"_, pos=1;
        
        ND_Transition(&machine);
        
        //VisualizeString(machine.snapshot.String);
        c = fgetc(stdin);                      //needed to check the first character of the string
        ungetc(c, stdin);
    }
     */
    if(c == EOF)
        printf("EOF reached: %d\n", c);
    /*
    free(machine.Accept);
    inputHash *temp1=machine.tr[0]->input_next, *old_addr1 = temp1;
    endList *old_addr2,*temp2;
    for(int j=0; j<=5; j++)
    {
        while (temp1!=NULL) {
            temp2 = temp1->end_next;
            old_addr2 = temp2;
            while(temp2 != NULL)
            {
                temp2 = temp2->next;
                free(old_addr2);
                old_addr2 = temp2;
            }
            temp1 = temp1->next;
            free(old_addr1);
            old_addr1 = temp1;
        }
        free(machine.tr[j]);
    }
    free(machine.tr);
    */
}
