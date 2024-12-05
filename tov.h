#ifndef TOV_H
#define TOV_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TAILLE_MAX_ENREGISTREMENT 256
#define TAILLE_BUFFER 512
#define TAILLE_MAX_BLOC 2560
#define SEPARATEUR '|'



/*sert a fournir des informations de base sur chaque enregistrement comme son id qui est important
pour le suivi des enregistrements dans le fichier et la table de hachage*/

typedef struct {
    int id; //ce champ est utilise pour identifier de maniere unique chaque enregistrement dans le fichier TOV (its used in most functions)
} EnteteEnregistrement;
typedef struct {
    char *data;             // Données du bloc
    int taille;             // espace utiliser dans le block
    int numBloc;            // Numéro de bloc pour une identification
    bool estComplet;       // Indicateur pour savoir si le bloc est plein
    Bloc* next;
} Bloc;



//adding new champs à l'enregistrement physique
typedef struct {
    EnteteEnregistrement entete;
    char data1[TAILLE_MAX_ENREGISTREMENT];
    char data2[TAILLE_MAX_ENREGISTREMENT];
    char data3[TAILLE_MAX_ENREGISTREMENT];
} EnregistrementPhysique;



//cette structure est utilisee pour implementer une table de hachage
typedef struct Element {
    int id; // La clé (ID de l'enregistrement)
    struct Element *suivant; // Pointeur vers l'élément suivant en cas de collision
} Element;

typedef struct HashTable {
    Element **table; // Tableau de pointeurs vers les listes chaînées
    int taille;      // Taille du tableau (nombre de "buckets")
} HashTable;



//cette structure represente les informations globales concernant le fichier TOV
typedef struct {
    int nbEnregistrements; //ce champ stocke le nombre actuel d'enregistrements dans le fichier TOV
    int capaciteMax;       //le nombre maximum d'enregistrements que le fichier peut contenir
    int nextID;            // nouveau champ pour suivre l'id global
} EnteteFichierTOV;


typedef struct {
    Bloc* adresbloc;
    int isfull;
}bloc001;
//Structure te3 buffer de transmission
typedef struct {
    //c un tableau de caracteres used pour stocker temporairement des donnees a transmettre
    char data[TAILLE_BUFFER];
    // Ce champ is used pour suivre la quantite de donnees actuellement stockees dans le tampon
    int taille;
} BufferTransmission;

typedef enum {
    Contigue,
    Chainee
}ModeOrganisationF;

typedef struct
{
    int nb_blocs;
    Bloc *bloc;
    ModeOrganisationF mode;
}Virtualdisk;


typedef enum {
    Trie,
    NoTrie
}ModeOrganisationE;

//cette structure represente le fichier TOV
typedef struct {
    EnteteFichierTOV entete;
    char nomFichier[256];  // Nom du fichier, modifiable à tout moment
    ModeOrganisationE mode;
    ModeOrganisationF sort;  
    //c un pointeur vers un tableau d'enregistrements physiques , ce tableau stocke les enregistrements individuels contenus dans le fichier TOV
    EnregistrementPhysique *enregistrements;
    HashTable *table;
    Bloc *blocs;     // Tableau de blocs pour la gestion de la mémoire
    int nbBlocs;  // taille en blocs
} FichierTOV;



// Prototypes de fonctions pour la gestion du fichier TOV

//Verifiez si fichier n'est pas NULL
void initialiserFichierTOV(FichierTOV *fichier, int capaciteMax);

//Verifiez si fichier n'est pas NULL
void libererFichierTOV(FichierTOV *fichier);

//Verifiez si fichier et le Buffer ne sont pas NULL
bool ajouterEnregistrement(FichierTOV *fichier, HashTable *hashTable, EnregistrementPhysique *enregistrement,BufferTransmission *buffer);

//Verifiez si fichier n'est pas NULL
bool supprimerEnregistrement(FichierTOV *fichier, HashTable *hashTable, int id,BufferTransmission *buffer);

//Verifiez si fichier n'est pas NULL
EnregistrementPhysique *rechercherEnregistrement(FichierTOV *fichier, HashTable *hashTable, int id); //changed to fit the new function

//Verifiez si fichier n'est pas NULL
void afficherFichierTOV(const FichierTOV *fichier);


// pour hed les 2 procedures verifiez les tailles bch n'evitiw le depassement de buffer (hws 3liha)
void remplirBuffer(BufferTransmission *buffer, const char *data);
void viderBuffer(BufferTransmission *buffer);


/*cette fonction calcule la taille de chaque enregistrement , heka n9dro ndiro le test w nchofo beli sah
la taille des enregistrements est variable w ttbdl d'apres wch ndkhlo hna*/
unsigned long CalculerTailleEnregistrement(const EnregistrementPhysique *enregistrement);


#endif