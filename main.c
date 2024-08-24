#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//costanti per controllo del comando
#define ADD_RECIPE_ID       0       //Id aggiungi_ricetta
#define REMOVE_RECIPE_ID    1       //Id rimuovi_ricetta
#define ADD_ING_ID          2       //Id rifornimento
#define ADD_ORDER_ID        3       //Id ordine

//Lunghezze
#define COMMAND_LENGTH      17      //lunghezza massima di un comando preso in input
#define ARG_LENGTH          256     //Lunghezza massima di un argomento (ingredienti, ricette...) di un comando preso in input

//Dimensioni strutture dati statiche
#define HASHTABLE_SIZE      5000

//Altri parametri
#define HASHPARAMETER       31  

//Forward declaration struct
typedef struct ingredient ingredient;
typedef struct recipe recipe;
typedef struct order order;
typedef struct courier_config courier_config;
typedef struct ingredientList_node ingredientList_node;
typedef struct recipeList_node recipeList_node;
typedef struct stockList_node stockList_node;
typedef struct lotList_node lotList_node;
typedef struct orderList_node orderList_node;
typedef struct lotListRef_node lotListRef_node;

//Struct per gli elementi della pasticceria
struct ingredient{
    char* name;
    unsigned int quantity;
};

struct recipe{
    char* name;
    ingredientList_node* ingHead;
};

struct order{
    char* recipeName;
    unsigned int arrivalTime;
    unsigned int requestedQuantity;
    unsigned int weight;
};

//Struct per la costruzione delle strutture dati
struct ingredientList_node{
    ingredient content;
    ingredientList_node* next;
};

struct lotList_node{
    ingredient content;
    int expiration;
    lotList_node* next;
};

struct lotListRef_node{
    lotList_node* lotRef;
    lotListRef_node* next;
};

//IDEA -> Potrei, ogni volta che aggiungo un lotto aggiornare una variabile che mi dice qual'è il lotto da usare
struct stockList_node{
    lotList_node* lot;
    stockList_node* next;
};

//IDEA -> Potrei, ogni volta che viene fatto un ordine non soddisfacibile subito, aggiungere una lista di ordini che mi dice quali sono gli ordini su questa ricetta
struct recipeList_node{
    recipe content;
    recipeList_node* next;
};

struct orderList_node{
    order content;
    orderList_node* next;
    ingredientList_node* ingList;
    unsigned int weight;
    unsigned int suspended;
};

//Configurazione corriere
struct courier_config{
    unsigned int clock;              //ogni quanto passa il corriere
    unsigned int maxQuantity;        //capienza  del corriere
};

//------------------------------------------------------------------------------------------------------------------------------------
//Prototipi
void                    aggiungi_ricetta();
void                    rifornimento();
void                    ordine();
void                    rimuovi_ricetta();

//Funzioni ausiliarie
int                     fullfillOrder(orderList_node*);
void                    useLots(orderList_node*, lotListRef_node*);

//Funzione di hash
unsigned int            hash(char*);

//Funzioni tabella hash ricette
int                     addToRecipeHT(recipeList_node*);
int                     addToBucketRecipe(recipeList_node*, unsigned int);
recipe*                 getRecipe(char*);
void                    removeRecipeNode(recipeList_node**, recipeList_node*, recipeList_node*);

//Funzioni tabella hash magazzino
void                    addToStockHT(stockList_node*);
void                    addToBucketStock(stockList_node*, unsigned int);
stockList_node*         getStock(char*);

//Funzioni per liste di ingredienti
void                    insertIngredientList(ingredientList_node**, ingredientList_node*);

//Funzioni per liste di lotti
void                    insertLotInOrder(lotList_node**, lotList_node*);
int                     checkAvailability(lotList_node*, unsigned int);
lotList_node*           getUsableLot(lotList_node*);

//Funzioni per liste di riferimenti a lotti
void                    insertLotRefInTail(lotListRef_node**, lotListRef_node*);

//Funzioni per liste di ordini
void                    insertOrderTail(orderList_node**, orderList_node*);
void                    insertCompletedOrder(orderList_node**, orderList_node*);
void                    removeOrderNode(orderList_node**, orderList_node*, orderList_node*);
unsigned int            findRecipeOrder(char*);

//Utilities
void                    elaborateCommand(char[]);
void                    printRecipeHT();
void                    printRecipeList(recipeList_node*);
void                    printIngredientList(ingredientList_node*);
void                    printStockHT();
void                    printStockLists(stockList_node*);
void                    printLotList(lotList_node*);
void                    printLotRefs(lotListRef_node*);
void                    printOrderList(orderList_node*);

//------------------------------------------------------------------------------------------------------------------------------------
//Variabili globali
courier_config      courierConfig;              //Configurazioni del corriere
recipeList_node**   recipeHashTable;            //HashTable per ricette
stockList_node**    stockHashTable;             //HashTable in cui elemento è un albero di lotti ordinati per data di scadenza
orderList_node*     completedOrders = NULL;     //Ordini completati
orderList_node*     suspendendOrders = NULL;    //Ordini in sospeso
int                 time = 0;                   //Istante di tempo attuale

//====================================================================================================================================
int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire

    //Inizializzazione strutture dati statiche
    recipeHashTable = (recipeList_node**)malloc(HASHTABLE_SIZE * sizeof(recipeList_node*));
    stockHashTable = (stockList_node**)malloc(HASHTABLE_SIZE * sizeof(stockList_node*));
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.maxQuantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    //Stampe per verifiche
    printf("Ordini in sospeso:\n");
    printOrderList(suspendendOrders);
    printf("Ordini completati:\n");
    printOrderList(completedOrders);
    printf("\n");

    //Free della memoria usata per le strutture dati statiche
    free(recipeHashTable);
    free(stockHashTable);
}
//====================================================================================================================================

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
            //Aggiungi ricetta
            aggiungi_ricetta();
            time++;
            break;
        
        case 1:
            //Rimuovi_ricetta
            rimuovi_ricetta();
            time++;
            break;

        case 2:
            //Rifornimento
            rifornimento();
            time++;
            break;

        case 3:
            //Ordine
            ordine();
            time++;
            break;
        
        default:
            break;
    }   
}

void printRecipeHT(){
    int i;
    for(i = 0 ; i < HASHTABLE_SIZE ; i++){
        if(recipeHashTable[i] != NULL){
            printf("HT[%d] ", i);
            printRecipeList(recipeHashTable[i]);
            printf("\n");
            fflush(stdout);
        }
    }
}

void printRecipeList(recipeList_node* x){
    if(x == NULL)return;
    printf("-> %s ", x->content.name);
    printf("(INGREDIENTI: ");
    printIngredientList(x->content.ingHead);
    printf(")");
    printRecipeList(x->next);
}

void printIngredientList(ingredientList_node* x){
    if(x == NULL)return;
    printf("-> %s %d", x->content.name, x->content.quantity);
    printIngredientList(x->next);
}

void printStockHT(){
    int i;
    for(i = 0 ; i < HASHTABLE_SIZE ; i++){
        if(stockHashTable[i] != NULL){
            printf("HT[%d] ", i);
            printStockLists(stockHashTable[i]);
            printf("\n");
            fflush(stdout);
        }
    }
}

void printStockLists(stockList_node* head){
    if(head == NULL) return;

    printf("==> %s \n", head->lot->content.name);
    printLotList(head->lot);
    printStockLists(head->next);
}

void printLotList(lotList_node* head){
    if(head == NULL){
        printf("\n");
        return;
    }

    printf("-> %s %d %d ", head->content.name, head->content.quantity, head->expiration);
    printLotList(head->next);
}

void printLotRefs(lotListRef_node* head){
    if(head == NULL){
        printf("\n");
        return;
    }

    printf("-> %s %d %d ", head->lotRef->content.name, head->lotRef->content.quantity, head->lotRef->expiration);
    printLotRefs(head->next);
}

void printOrderList(orderList_node* head){
    if(head == NULL){
        printf("\n");
        return;
    }

    printf("-> %s %d A=%d - W=%d", head->content.recipeName, head->content.requestedQuantity, head->content.arrivalTime, head->weight);
    printOrderList(head->next);
}

//aggiungi_ricetta
void aggiungi_ricetta(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea

    char            name[ARG_LENGTH];                       //Nome della ricetta
    char            ingName[ARG_LENGTH];                    //Ingrediente
    unsigned int    ingQuantity;                            //Quantità ingrediente

    status = scanf("%s", name);                     //Lettura del nome della ricetta

    //Creazione ricetta
    recipeList_node* newRecipe = (recipeList_node*)malloc(sizeof(recipeList_node));
    newRecipe->content.name = (char*)malloc(strlen(name) + 1);
    strcpy(newRecipe->content.name, name);

    int res = addToRecipeHT(newRecipe);
    if(res == 0){
        printf("ignorato\n");
        free(newRecipe);
        do{
            status = scanf("%s %d", ingName, &ingQuantity);
            status = scanf("%c", &eol);
        } while (eol != '\n');
        return;        //Ricetta già presente
    }

    printf("aggiunta\n");
    do{
        //Lettura ingredienti
        status = scanf("%s %d", ingName, &ingQuantity);
        
        //Creazione ingrediente
        ingredientList_node* newIng = (ingredientList_node*)malloc(sizeof(ingredientList_node));
        newIng->content.name = (char*)malloc(strlen(ingName) + 1);
        strcpy(newIng->content.name, ingName);
        newIng->content.quantity = ingQuantity;

        insertIngredientList(&(newRecipe->content.ingHead), newIng);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    if(status == 0) printf("error\n");
}

int addToRecipeHT(recipeList_node* x){
    unsigned int index = hash(x->content.name);
    int res = 0;

    if(recipeHashTable[index] == NULL){
        recipeHashTable[index] = x;
        res = 1;
    }
    //Collisione
    else res = addToBucketRecipe(x, index);

    if(res == 0) return 0;
    else return 1;
}

//rifornimento - RICORDARSI DI GUARDARE QUANDO DUE LOTTI HANNO LA STESSA SCADENZA ALLORA BASTA AGGIUNGERE LA QUANTITA'
void rifornimento(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea

    char            ingName[ARG_LENGTH];
    unsigned int    ingQuantity;
    unsigned int    ingExpiration;

    do{
        //Lettura ingredienti
        status = scanf("%s %d %d", ingName, &ingQuantity, &ingExpiration);

        //Creazione lotto
        stockList_node* newItemHT = (stockList_node*)malloc(sizeof(stockList_node));
        lotList_node* newLot = (lotList_node*)malloc(sizeof(lotList_node));
        newLot->content.name = (char*)malloc(strlen(ingName) + 1);
        strcpy(newLot->content.name, ingName);
        newLot->content.quantity = ingQuantity;
        newLot->expiration = ingExpiration;

        newItemHT->lot = newLot;

        addToStockHT(newItemHT);
        
        status = scanf("%c", &eol);
    }while(eol != '\n');

    printf("rifornito\n");

    orderList_node* curr = suspendendOrders;
    orderList_node* prev = NULL;
    while(curr){
        orderList_node* next = curr->next; // Salva il prossimo nodo prima di rimuovere curr
        int res = fullfillOrder(curr);

        if (res == 1) removeOrderNode(&suspendendOrders, prev, curr);
        else prev = curr;

        curr = next;
    }

    if(status == 0) printf("error\n");
}

void addToStockHT(stockList_node* x){
    unsigned int index = hash(x->lot->content.name);

    if(stockHashTable[index] == NULL) stockHashTable[index] = x;
    //Collisione
    else addToBucketStock(x, index);
}

//ordine
void ordine(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;

    char    orderRecipe[ARG_LENGTH];
    int     requestedQuantity;

    status = scanf("%s %d", orderRecipe, &requestedQuantity);

    //Creazione ordine
    orderList_node* newOrder = (orderList_node*)malloc(sizeof(orderList_node));
    newOrder->content.recipeName = (char*)malloc(strlen(orderRecipe)+ 1);
    strcpy(newOrder->content.recipeName, orderRecipe);
    newOrder->content.arrivalTime = time;
    newOrder->content.requestedQuantity = requestedQuantity;

    fullfillOrder(newOrder);

    if(status == 0) printf("error\n");
}

int fullfillOrder(orderList_node* x){
    //Cerca ricetta
    recipe* orderRecipe = getRecipe(x->content.recipeName);
    
    if(orderRecipe == NULL){
        if(x->suspended == 0) printf("rifiutato\n");
        return 0;
    }

    if(x->suspended == 0) printf("accettato\n");
    //Ricetta trovata
    x->ingList = orderRecipe->ingHead;
    ingredientList_node*    ingList = orderRecipe->ingHead;
    lotListRef_node*        lots = NULL;  //Lista di riferimenti con i lotti da usare
    unsigned int            weight = 0;
    unsigned int            requestedQuantity = x->content.requestedQuantity;
    
    while(ingList){
        stockList_node* stock = getStock(ingList->content.name);
        if(stock == NULL){
            //Lista ordini in sospeso
            if(x->suspended == 0) insertOrderTail(&suspendendOrders, x);
            x->suspended = 1;
            return 0;
        }

        //Primo lotto non scaduto
        lotList_node* lot = getUsableLot(stock->lot);
        if(lot == NULL){
            //Lista ordini in sospeso
            if(x->suspended == 0) insertOrderTail(&suspendendOrders, x);
            x->suspended = 1;
            return 0;
        }

        //Controlla che ci siano effettivamente tutti gli ingredienti per soddisfare l'ordine
        int availability = checkAvailability(lot, x->content.requestedQuantity * ingList->content.quantity);
        if(availability == 0){
            //Lista ordini in sospeso
            if(x->suspended == 0) insertOrderTail(&suspendendOrders, x);
            x->suspended = 1;
            return 0;
        }

        //Salvo il primo lotto usabile per dopo
        lotListRef_node* lotRef = (lotListRef_node*)malloc(sizeof(lotListRef_node)); //DA LIBERARE!
        lotRef->lotRef = lot;

        insertLotRefInTail(&lots, lotRef);

        weight += ingList->content.quantity * requestedQuantity;

        ingList = ingList->next;
    }

    x->weight = weight;

    //Uso i lotti
    useLots(x, lots);

    orderList_node* xCompleted = (orderList_node*)malloc(sizeof(orderList_node));
    xCompleted->content = x->content;
    xCompleted->ingList = x->ingList;
    xCompleted->suspended = 0;
    xCompleted->weight = x->weight;

    insertCompletedOrder(&completedOrders, xCompleted); //Aggiunge l'ordine alla lista degli ordini completati

    if(x->suspended == 0) free(x); //Se l'ordine non era sospeso allora basta liberare x, altrimenti va gestito altrove

    return 1;
}

//TODO - Eliminare lotti vuoti?
void useLots(orderList_node* x, lotListRef_node* lot){
    ingredientList_node*    currIng = x->ingList;
    lotListRef_node*        currLotRef = lot;
    lotList_node*           currLot = currLotRef->lotRef;

    while(currIng){
        unsigned int requestedQuantity = currIng->content.quantity * x->content.requestedQuantity;
        unsigned int sum = 0;

        while(currLot){
            if(currLot->content.quantity >= requestedQuantity){
                currLot->content.quantity -= requestedQuantity;
                break;
            }
            else{
                sum += currLot->content.quantity;
                requestedQuantity -= currLot->content.quantity;
                currLot->content.quantity = 0;
            }

            if(sum >= requestedQuantity) break;
            currLot = currLot->next;
        }
    
        if(currIng->next){
            currLotRef = currLotRef->next;
            currLot = currLotRef->lotRef;
        }
        currIng = currIng->next;
    } 
}

//rimuovi_ricetta
void rimuovi_ricetta(){
    int status;
    char name[ARG_LENGTH];

    status = scanf("%s\n", name);

    int index = hash(name);
    if(recipeHashTable[index] == NULL){
        printf("non presente\n");
        return;
    }

    recipeList_node* curr = recipeHashTable[index];
    recipeList_node* prev = NULL;
    while(curr){
        recipeList_node* next = curr->next;
        
        if(strcmp(curr->content.name, name) == 0){
            if(findRecipeOrder(name) == 0){
                printf("rimossa\n");
                removeRecipeNode(&recipeHashTable[index], prev, curr);
            }
            else printf("ordini in sospeso %s\n", name);
            
            printf("Ordini sospesi\n");
            printOrderList(suspendendOrders);
            printf("Ordini completati\n");
            printOrderList(completedOrders);
            printf("\n");
            return;
        }
        else prev = curr;

        curr = next;
    }

    printf("non presente\n");

    if(status == 0) printf("error\n");
}

//Funzione di hash
unsigned int hash(char* x){
    unsigned int res = 0;

    while(*x != '\0'){
        res = res * HASHPARAMETER + (unsigned char)(*x);
        x++;
    }

    return res % HASHTABLE_SIZE;
}

//Funzioni per stockHashTable
void addToBucketStock(stockList_node* x, unsigned int index){
    stockList_node* curr = stockHashTable[index];

    while(curr){
        if(strcmp(x->lot->content.name, curr->lot->content.name) == 0){
            //Trvoato ingrediente in magazzino, aggiungere lotto alla lista dei lotti
            insertLotInOrder(&(curr->lot), x->lot);
            return;
        }
        curr = curr->next;
    }

    //Ingrediente non trovato, aggiungo nuovo ingrediente nella bucketList
    x->next = stockHashTable[index];
    stockHashTable[index] = x;
}

stockList_node* getStock(char* x){
    unsigned int index = hash(x);

    if(stockHashTable[index] == NULL) return NULL;

    stockList_node* curr = stockHashTable[index];

    while(curr){
        if(strcmp(curr->lot->content.name, x) == 0){
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

//Funzioni per recipeHashTable
int addToBucketRecipe(recipeList_node* x, unsigned int index){
    recipeList_node* curr = recipeHashTable[index];
    
    while(curr){
        if(strcmp(curr->content.name, x->content.name) == 0){
            return 0;
        }
        curr = curr->next;
    }

    x->next = recipeHashTable[index];
    recipeHashTable[index] = x;
    return 1;
}

recipe* getRecipe(char* x){
    unsigned int index = hash(x);

    if(recipeHashTable[index] == NULL) return NULL;

    recipeList_node* curr = recipeHashTable[index];
    while(curr){
        if(strcmp(curr->content.name, x) == 0){
            return &(curr->content);
        }
        curr = curr->next;
    }

    return NULL;
}

void removeRecipeNode(recipeList_node** head, recipeList_node* prev, recipeList_node* x){
    if (prev == NULL) {
        *head = (*head)->next;
    }
    else{
        prev->next = x->next;
    }

    free(x);
}

//Funzioni per liste di ingredienti
void insertIngredientList(ingredientList_node** head, ingredientList_node* x){
    if(*head == NULL){
        *head = x;
        return;
    }

    x->next = *head;
    *head = x;
}

//Funzioni per liste di lotti -- TODO: RIMUOVERE LOTTI SCADUTI QUANDO SI INCONTRANO
void insertLotInOrder(lotList_node** head, lotList_node* x) {
    if (*head == NULL || x->expiration <= (*head)->expiration) {
        x->next = *head;
        *head = x;
        return;
    }

    lotList_node* curr = *head;

    while (curr && curr->next && x->expiration > curr->next->expiration) curr = curr->next;

    x->next = curr->next;
    curr->next = x;
}

lotList_node* getUsableLot(lotList_node* head){
    while(head){
        if(head->expiration > time) return head;
        head = head->next;
    }

    return NULL;
}

int checkAvailability(lotList_node* x, unsigned int requestedQuantity){
    int sum = 0;

    while(x){
        sum += x->content.quantity;
        if(sum >= requestedQuantity) return 1;
        x = x->next;
    }

    return 0;
}

//Funzioni per liste di riferimenti a lotti
void insertLotRefInTail(lotListRef_node** head, lotListRef_node* x){
    if(*head == NULL){
        *head = x;
        return;
    }

    lotListRef_node* curr = *head;
    while(curr->next) curr = curr->next;

    curr->next = x;
}

//Funzioni per liste di ordini
void insertOrderTail(orderList_node** head, orderList_node* x){
    if(*head == NULL){
        *head = x;
        return;
    }

    orderList_node* curr = *head;
    while(curr->next){
        curr = curr->next;
    }

    curr->next = x;
}

void insertCompletedOrder(orderList_node** head, orderList_node* x){
    if (*head == NULL || x->weight > (*head)->weight || (x->weight == (*head)->weight && x->content.arrivalTime < (*head)->content.arrivalTime)){
        x->next = *head;
        *head = x;
        return;
    }

    orderList_node* curr = *head;
    while (curr->next != NULL && (x->weight < curr->next->weight || (x->weight == curr->next->weight && x->content.arrivalTime > curr->next->content.arrivalTime))){
        curr = curr->next;
    }

    x->next = curr->next;
    curr->next = x;
}

void removeOrderNode(orderList_node** head, orderList_node* prev, orderList_node* x){
    if (prev == NULL) {
        *head = (*head)->next;
    }
    else{
        prev->next = x->next;
    }

    free(x);
}

unsigned int findRecipeOrder(char* name){
    orderList_node* suspendedCurr = suspendendOrders;
    orderList_node* completedCurr = completedOrders;

    while(suspendedCurr){
        if(strcmp(suspendedCurr->content.recipeName, name) == 0) return 1;
        suspendedCurr = suspendedCurr->next;
    }

    while(completedCurr){
        if(strcmp(completedCurr->content.recipeName, name) == 0) return 1;
        completedCurr = completedCurr->next;
    }

    return 0;
}