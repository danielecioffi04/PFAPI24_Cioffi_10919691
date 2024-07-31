#include <stdio.h>
#include <string.h>

//costanti per controllo del comando
#define ADD_RECIPE_ID       0       //Id aggiungi_ricetta
#define REMOVE_RECIPE_ID    1       //Id rimuovi_ricetta
#define ADD_ING_ID          2       //Id rifornimento
#define ADD_ORDER_ID        3       //Id ordine

//lunghezza massima di un comando preso in input
#define COMMAND_LENGTH  16

//Lunghezza massima di un argomento (ingredienti, ricette...) di un comando preso in input
#define ARG_LENGTH      255

//Struct per gli elementi della pasticceria
typedef struct{
    char name[ARG_LENGTH];
    int quantity;
    int expiration;
} ingredient;

typedef struct{
    char name[ARG_LENGTH];
    //aggiungere lista in ingredienti
} recipe;

typedef struct{
    recipe recipe;
    int arrival_time;
} order;

//Configurazione corriere
typedef struct{
    int clock;              //ogni quanto passa il corriere
    int max_quantity;       //capienza  del corriere
} courier_config;


//Variabili globali
courier_config  courierConfig;              //Configurazioni del corriere
int             time = 0;                   //Istante di tempo attuale

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


int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.max_quantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    printf("Fine comandi\n");
}