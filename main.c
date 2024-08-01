#include <stdio.h>
#include <string.h>

//costanti per controllo del comando
#define ADD_RECIPE_ID       0       //Id aggiungi_ricetta
#define REMOVE_RECIPE_ID    1       //Id rimuovi_ricetta
#define ADD_ING_ID          2       //Id rifornimento
#define ADD_ORDER_ID        3       //Id ordine

//Lunghezze
#define COMMAND_LENGTH      16      //lunghezza massima di un comando preso in input
#define ARG_LENGTH          255     //Lunghezza massima di un argomento (ingredienti, ricette...) di un comando preso in input

//Prototipi
void add_recipe();

void elaborateCommand(char[]);

//Forward declaration struct
typedef struct ingredient ingredient;
typedef struct ingredient_node ingredient_node;
typedef struct recipe recipe;
typedef struct order order;
typedef struct courier_config courier_config;

//Struct per gli elementi della pasticceria
struct ingredient{
    char name[ARG_LENGTH];
    int quantity;
    int expiration;
};

struct recipe{
    char name[ARG_LENGTH];
    ingredient_node* ingHead;
    ingredient_node* last;
};

struct order{
    recipe recipe;
    int arrivalTime;
};

//Struct per la costruzione delle strutture dati
struct ingredient_node{
    ingredient content;
    ingredient_node* next;
};

//Configurazione corriere
struct courier_config{
    int clock;              //ogni quanto passa il corriere
    int maxQuantity;        //capienza  del corriere
};


//Variabili globali
courier_config  courierConfig;              //Configurazioni del corriere
int             time = 0;                   //Istante di tempo attuale

int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.maxQuantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    printf("Fine comandi\n");
}

//Utilities
//Funzione che collega il comando alla rispettiva funzione da eseguire
void elaborateCommand(char command[]){
    int commandID;
    if(command[0] == 'a') commandID = ADD_RECIPE_ID;
    else if(command[0] == 'r'){
        if(command[2] == 'm') commandID = REMOVE_RECIPE_ID;
        else commandID = ADD_ING_ID;
    }
    else commandID = ADD_ORDER_ID;

    switch (commandID){
        case 0:
            printf("Comando rilevato: \"aggiungi_ricetta\"\n");
            add_recipe();
            break;
        
        case 1:
            printf("Comando rilevato: \"rimuovi_ricetta\"\n");
            break;

        case 2:
            printf("Comando rilevato: \"rifornimento\"\n");
            break;

        case 3:
            printf("Comando rilevato: \"ordine\"\n");
            break;
        
        default:
            break;
    }   
}

//Funzioni
void add_recipe(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    name[ARG_LENGTH];                       //Nome della ricetta
    char    eol = '0';                              //Var per controllare la fine della linea
    char    ingName[ARG_LENGTH];                    //Ingrediente
    int     ingQuantity;                            //Quantità ingrediente

    status = scanf("%s", name);                     //Lettura del nome della ricetta
    printf("Nome ricetta: %s\n", name);

    do{
        status = scanf("%s %d", ingName, &ingQuantity);
        printf("Nome ingrediente: %s\nQuantita' ingrediente: %d\n", ingName, ingQuantity);
        status = scanf("%c", &eol);
    }while(eol != '\n');

    if(status == 0) printf("hello");
}
