#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//costanti per controllo del comando
#define ADD_RECIPE_ID       0       //Id aggiungi_ricetta
#define REMOVE_RECIPE_ID    1       //Id rimuovi_ricetta
#define ADD_ING_ID          2       //Id rifornimento
#define ADD_ORDER_ID        3       //Id ordine

//Lunghezze
#define COMMAND_LENGTH      16      //lunghezza massima di un comando preso in input
#define ARG_LENGTH          255     //Lunghezza massima di un argomento (ingredienti, ricette...) di un comando preso in input

//Forward declaration struct
typedef struct ingredient ingredient;
typedef struct ingredient_node ingredient_node;
typedef struct recipe recipe;
typedef struct order order;
typedef struct courier_config courier_config;
typedef struct recipe_node recipe_node;

//Struct per gli elementi della pasticceria
struct ingredient{
    char name[ARG_LENGTH];
    int quantity;
    int expiration;
};

struct recipe{
    char name[ARG_LENGTH];
    ingredient_node* ingList;
    int weight;
};

struct order{
    recipe recipe;
    int arrivalTime;
};

//Struct per la costruzione delle strutture dati
struct ingredient_node{
    ingredient content;
    ingredient_node* parent;
    ingredient_node* next;              //Usato per la creazione di liste di ingredienti nelle ricette
    ingredient_node* right;
    ingredient_node* left;
    char color;
};

struct recipe_node{
    recipe content;
    recipe_node* left;
    recipe_node* right;
    recipe_node* parent;
    char color;
};

//Configurazione corriere
struct courier_config{
    int clock;              //ogni quanto passa il corriere
    int maxQuantity;        //capienza  del corriere
};

//Prototipi---------------------------------------------------------------------------------------------------------------------------
void    add_recipe();
void    remove_recipe();
void    rifornimento();

//Utilities
void    printTreeRec(recipe_node*, int);
void    printTreeIng(ingredient_node*, int);
void    elaborateCommand(char[]);

//Funzioni per albero ricette
int     insertRecipe(recipe_node**, recipe_node*);
void    leftRotateRec(recipe_node**, recipe_node*);
void    rightRotateRec(recipe_node**, recipe_node*);
void    insertRecipeFixUp(recipe_node**, recipe_node*);

//Funzioni per albero magazzino
int     insertIngredient(ingredient_node**, ingredient_node*);
void    leftRotateIng(ingredient_node**, ingredient_node*);
void    rightRotateIng(ingredient_node**, ingredient_node*);
void    insertIngredientFixUp(ingredient_node**, ingredient_node*);

//Funzioni per gestione delle liste di ingredienti
void    addToList(ingredient_node**, ingredient_node*);
void    printList(ingredient_node*);
void    addInOrderList(ingredient_node**, ingredient_node*, ingredient_node*, ingredient_node*);

//------------------------------------------------------------------------------------------------------------------------------------

//Variabili globali
int                 time = 0;                   //Istante di tempo attuale
courier_config      courierConfig;              //Configurazioni del corriere
recipe_node*        recipeList = NULL;          //Albero delle ricette
ingredient_node*    stock = NULL;               //Albero magazzino

//MAIN ===============================================================================================================================
int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.maxQuantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    return 1;
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
            //Aggiungi_ricetta
            add_recipe();
            break;
        
        case 1:
            //Rimuovi_ricetta
            //remove_recipe();
            break;

        case 2:
            //Rifornimento
            rifornimento();
            break;

        case 3:
            //Ordine
            break;
        
        default:
            break;
    }   
}

//Stampa un albero in modo "carino"
void printTreeRec(recipe_node* node, int depth) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    if(node != recipeList && node->parent->left != NULL && node == node->parent->left) printf("L - %s - %c\n", node->content.name, node->color);
    else printf("R - %s - %c\n", node->content.name, node->color);

    printTreeRec(node->left, depth + 1);

    printTreeRec(node->right, depth + 1);
}

void printTreeIng(ingredient_node* node, int depth) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    if(node != stock && node->parent->left != NULL && node == node->parent->left) printf("L - %s - %c ----", node->content.name, node->color);
    else printf("R - %s - %c ----", node->content.name, node->color);

    printList(node);

    printTreeIng(node->left, depth + 1);

    printTreeIng(node->right, depth + 1);
}

//Stampa lista
void printList(ingredient_node* head){
    while(head != NULL){
        printf(" -> %s %d %d", head->content.name, head->content.quantity, head->content.expiration);
        head = head->next;
    }
    printf("\n\n");
}

//Funzioni
void add_recipe(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea
    int     res;                                    //Risultato dell'aggiunta della ricetta all'albero

    //Allocazione e inizializzazione nuova ricetta da inserire nell'albero
    recipe_node* newNode = (recipe_node*)malloc(sizeof(recipe_node));
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->parent = NULL;
    newNode->content.ingList = NULL;
    newNode->content.weight = 0;

    //Lettura nome della ricetta e aggiunta del nodo all'albero se non presente
    status = scanf("%s", newNode->content.name);                    //Lettura del nome della ricetta
    res = insertRecipe(&recipeList, newNode);

    //Stampa risultato aggiunta
    if(res == 0){
        printf("ignorato\n");
        free(newNode);
        return;
    }
    else printf("aggiunta\n");

    //Creazione lista di ingredienti
    do{
        ingredient_node* newIng = (ingredient_node*)malloc(sizeof(ingredient_node));
        ingredient tmp;
        status = scanf(" %s %d", tmp.name, &tmp.quantity);
        newIng->content = tmp;
        newNode->content.weight += tmp.quantity;                    //Incrementa il peso della ricetta
        addToList(&newNode->content.ingList, newIng);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    if(status == 0) printf("Error");
}

/*
void remove_recipe(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    name[ARG_LENGTH];                       //Nome della ricetta da eliminare

    //Lettura nome della ricetta da eliminare
    status = scanf("%s\n", name);

    //Continuare quando si sono implementati gli ordini
}
*/

void rifornimento(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea

    //Lettura args
    do{
        //Allocazione e inizializzazione nuovo ingrediente da inserire nell'albero
        ingredient_node* newNode = (ingredient_node*)malloc(sizeof(ingredient_node));
        newNode->left = NULL;
        newNode->right = NULL;
        newNode->next = NULL;
        ingredient tmp;

        status = scanf(" %s %d %d", tmp.name, &tmp.quantity, &tmp.expiration);
        newNode->content = tmp;
        insertIngredient(&stock, newNode);
        status = scanf("%c", &eol);
    }while(eol != '\n');

    printTreeIng(stock, 0);

    if(status == 0) printf("Error");
}

//Funzioni per albero di ricette
int insertRecipe(recipe_node** head, recipe_node* newNode){
    recipe_node* x = *head;
    recipe_node* y = NULL;

    while(x != NULL){
        y = x;
        int cmp = strcmp(newNode->content.name, x->content.name);
        if(cmp < 0) x = x->left;
        else if(cmp == 0) return 0;       //Ricetta già presente in lista
        else x = x->right;
    }

    newNode->parent = y;
    if(y == NULL){
        *head = newNode;        //Albero vuoto
    }
    else if(strcmp(newNode->content.name, y->content.name) < 0) y->left = newNode;
    else y->right = newNode;
    newNode->color = 'r';
    insertRecipeFixUp(head, newNode);
    return 1;
}

void insertRecipeFixUp(recipe_node** head, recipe_node* node){
    if(node == (*head)){
        node->color = 'b';
        return;
    }

    recipe_node* x = node->parent;          //Padre
    recipe_node* y = NULL;                  //Nonno
    recipe_node* z = NULL;                  //Zio
    
    if(x != NULL) y = x->parent;

    if(x->color == 'r'){
        if(y != NULL && x == y->left){
            z = y->right;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertRecipeFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->right){
                    node = x;
                    leftRotateRec(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                rightRotateRec(head, x->parent);
            }
        }
        else{
            z = y->left;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertRecipeFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->left){
                    node = x;
                    rightRotateRec(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                leftRotateRec(head, x->parent);
            }
        }
    }
}

void leftRotateRec(recipe_node** head, recipe_node* node){
    recipe_node* y = node->right;
    node->right = y->left;

    if(y->left != NULL){
        y->left->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->left) node->parent->left = y;
    else node->parent->right = y;
    y->left = node;
    node->parent = y;
}

void rightRotateRec(recipe_node** head, recipe_node* node){
    recipe_node* y = node->left;
    node->left = y->right;
    
    if(y->right != NULL){
        y->right->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->right) node->parent->right = y;
    else node->parent->left = y;
    y->right = node;
    node->parent = y;
}

//Funzioni per albero magazzino
int insertIngredient(ingredient_node** head, ingredient_node* newNode){
    ingredient_node* x = *head;
    ingredient_node* y = NULL;

    while(x != NULL){
        y = x;
        int cmp = strcmp(newNode->content.name, x->content.name);
        if(cmp < 0) x = x->left;
        else if(cmp == 0){
            addInOrderList(head, newNode, x, NULL);
            return 0;
        }
        else x = x->right;
    }

    newNode->parent = y;
    if(y == NULL){
        *head = newNode;        //Albero vuoto
    }
    else if(strcmp(newNode->content.name, y->content.name) < 0) y->left = newNode;
    else y->right = newNode;
    newNode->color = 'r';
    insertIngredientFixUp(head, newNode);
    return 1;
}

void insertIngredientFixUp(ingredient_node** head, ingredient_node* node){
    if(node == (*head)){
        node->color = 'b';
        return;
    }

    ingredient_node* x = node->parent;          //Padre
    ingredient_node* y = NULL;                  //Nonno
    ingredient_node* z = NULL;                  //Zio
    
    if(x != NULL) y = x->parent;

    if(x->color == 'r'){
        if(y != NULL && x == y->left){
            z = y->right;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertIngredientFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->right){
                    node = x;
                    leftRotateIng(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                rightRotateIng(head, x->parent);
            }
        }
        else{
            z = y->left;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertIngredientFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->left){
                    node = x;
                    rightRotateIng(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                leftRotateIng(head, x->parent);
            }
        }
    }
}

void leftRotateIng(ingredient_node** head, ingredient_node* node){
    ingredient_node* y = node->right;
    node->right = y->left;

    if(y->left != NULL){
        y->left->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->left) node->parent->left = y;
    else node->parent->right = y;
    y->left = node;
    node->parent = y;
}

void rightRotateIng(ingredient_node** head, ingredient_node* node){
    ingredient_node* y = node->left;
    node->left = y->right;
    
    if(y->right != NULL){
        y->right->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->right) node->parent->right = y;
    else node->parent->left = y;
    y->right = node;
    node->parent = y;
}

//Funzioni per liste di ingredienti
void addToList(ingredient_node** head, ingredient_node* newNode){       //Inserimento in testa
    if(*head == NULL) {
        *head = newNode;
        return;
    }

    ingredient_node* tmp = *head;
    *head = newNode;
    newNode->next = tmp;
}

void addInOrderList(ingredient_node** head, ingredient_node* newNode, ingredient_node* curr, ingredient_node* prev){
    //Aggiungere in testa
    if(curr == *head && newNode->content.expiration < curr->content.expiration){
        newNode->right = curr->right;
        newNode->left = curr->left;
        newNode->next = curr;
        curr->right = NULL;
        curr->left = NULL;
        *head = newNode;
        return;
    }

    //Aggiungere "dentro" la lista
    if(curr != NULL && newNode->content.expiration > curr->content.expiration) addInOrderList(head, newNode, curr->next, curr);
    
    if(curr != NULL){
        if(newNode->content.expiration == curr->content.expiration){
            curr->content.quantity += newNode->content.quantity;
            free(newNode);
        }
        else if(newNode->content.expiration < curr->content.expiration){
            prev->next = newNode;
            newNode->next = curr->next;
        }
    }
    //Aggiungere in coda
    else{
        prev->next = newNode;
    }
}
