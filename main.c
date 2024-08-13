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
};

struct recipe{
    char name[ARG_LENGTH];
    ingredient_node* ingList;
};

struct order{
    recipe recipe;
    int arrivalTime;
};

//Struct per la costruzione delle strutture dati
struct ingredient_node{         //Usato per liste di ingredienti nelle ricette
    ingredient content;
    ingredient_node* next;
};

struct recipe_node{             //Usato per costruire albero di ricette
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

//Utilities
void    printTree(recipe_node*, int);
void    elaborateCommand(char[]);

//Funzioni per albero ricette
int     insertRecipe(recipe_node**, recipe_node*);
void    leftRotate(recipe_node**, recipe_node*);
void    rightRotate(recipe_node**, recipe_node*);
void    insertRecipeFixUp(recipe_node**, recipe_node*);

//Funzioni per gestione delle liste di ingredienti
void    addToList(ingredient_node**, ingredient_node*);
void    printList(ingredient_node*);

//------------------------------------------------------------------------------------------------------------------------------------

//Variabili globali
int             time = 0;                   //Istante di tempo attuale
courier_config  courierConfig;              //Configurazioni del corriere
recipe_node*    recipeList = NULL;          //Albero delle ricette

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

    printTree(recipeList, 0);

    printf("Fine comandi\n");
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
            break;

        case 2:
            //Rifornimento
            break;

        case 3:
            //Ordine
            break;
        
        default:
            break;
    }   
}

//Stampa un albero in modo "carino"
void printTree(recipe_node* node, int depth) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    if(node != recipeList && node->parent->left != NULL && node == node->parent->left) printf("L - %s - %c\n", node->content.name, node->color);
    else printf("R - %s - %c\n", node->content.name, node->color);

    printTree(node->left, depth + 1);

    printTree(node->right, depth + 1);
}

//Stampa lista
void printList(ingredient_node* head){
    while(head != NULL){
        printf("-> %s %d ", head->content.name, head->content.quantity);
        head = head->next;
    }
    printf("\n\n");
}

//Funzioni
void add_recipe(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea
    //char    ingName[ARG_LENGTH];                    //Ingrediente
    //int     ingQuantity;                            //Quantità ingrediente
    int     res;                                    //Risultato dell'aggiunta della ricetta all'albero

    //Allocazione e inizializzazione nuova ricetta da inserire nell'albero
    recipe_node* newNode = (recipe_node*)malloc(sizeof(recipe_node));
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->parent = NULL;
    newNode->content.ingList = NULL;

    //Lettura nome della ricetta e aggiunta del nodo all'albero se non presente
    status = scanf("%s", newNode->content.name);                    //Lettura del nome della ricetta
    res = insertRecipe(&recipeList, newNode);

    //Stampa risultato aggiunta
    if(res == 0){
        printf("ignorato\n\n");
        free(newNode);
        return;
    }
    else printf("aggiunta\n");

    //Lettura args  (TODO! CREARE LISTA DEGLI INGREDIENTI)
    do{
        ingredient_node* newIng = (ingredient_node*)malloc(sizeof(ingredient_node));
        ingredient tmp;
        status = scanf(" %s %d", tmp.name, &tmp.quantity);
        newIng->content = tmp;
        addToList(&newNode->content.ingList, newIng);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    //printList(newNode->content.ingList);

    if(status == 0) printf("hello");
}

//Aggiunge la ricetta all'albero delle ricette, posso passare recipe_node* newNode senza problemi poichè allocato dinamicamente nell'head 
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
                    leftRotate(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                rightRotate(head, x->parent);
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
                    rightRotate(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                leftRotate(head, x->parent);
            }
        }
    }
}

void leftRotate(recipe_node** head, recipe_node* node){
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

void rightRotate(recipe_node** head, recipe_node* node){
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

//Funzioni su liste di ingredienti
void addToList(ingredient_node** head, ingredient_node* newNode){       //Inserimento in testa
    if(*head == NULL) {
        *head = newNode;
        return;
    }

    ingredient_node* tmp = *head;
    *head = newNode;
    newNode->next = tmp;
}