#ifndef MYLIB_H
#define MYLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TAILLE_MAX_ENREGISTREMENT 256
#define TAILLE_BUFFER 768
#define TAILLE_MAX_BLOC 768
#define SEPARATEUR '|'

typedef struct {
    int id; // Utilisé pour identifier de manière unique chaque enregistrement
    bool suppprimer ;
} EnteteEnregistrement;
// Ajout de nouveaux champs à l'enregistrement physique
typedef struct {
    EnteteEnregistrement entete;
    char data1[TAILLE_MAX_ENREGISTREMENT];
    char data2[TAILLE_MAX_ENREGISTREMENT];
    char data3[TAILLE_MAX_ENREGISTREMENT];
} EnregistrementPhysique;
typedef struct Bloc {
    EnregistrementPhysique *enregistrements;
    int taille;             // Espace utilisé dans le bloc en eregistrement 
    int numBloc;            // Numéro du bloc pour identification
    bool estComplet;       // Indicateur si le bloc est plein
    struct Bloc* next;     // Pointeur vers le bloc suivant
} Bloc;

// Structure pour implémenter une table de hachage
typedef struct Element {
    int id; // La clé (ID de l'enregistrement)
    struct Element *suivant; // Pointeur vers l'élément suivant en cas de collision
} Element;

typedef struct HashTable {
    Element **table; // Tableau de pointeurs vers les listes chaînées
    int taille;      // Taille du tableau (nombre de "buckets")
} HashTable;

// Structure pour les informations globales du fichier TOV
typedef struct {
    int nbEnregistrements; // Nombre actuel d'enregistrements
    int capaciteMax;       // Nombre maximum d'enregistrements que le fichier peut contenir
    int nextID;            // ID global pour le suivi
} EnteteFichier;

typedef struct {
    Bloc* adresbloc;
    int isfull;
} bloc001;

typedef struct {
    char data[TAILLE_BUFFER]; // Buffer pour stocker temporairement des données à transmettre
    int taille;               // Quantité de données actuellement stockées
} BufferTransmission;

typedef enum {
    Contigue,
    Chainee
} ModeOrganisationF;

typedef struct {
    int nb_blocs;
    Bloc *bloc;
    ModeOrganisationF mode;
} Virtualdisk;

typedef enum {
    Trie,
    NoTrie
} ModeOrganisationE;

// Structure représentant le fichier TOV
typedef struct {
    EnteteFichier entete;
    char nomFichier[256];  // Nom du fichier, modifiable
    ModeOrganisationE mode;
    ModeOrganisationF sort;
    EnregistrementPhysique *enregistrements;  // Tableau d'enregistrements physiques
    HashTable *table;  // Table de hachage
    Bloc *blocs;       // Tableau de blocs pour la gestion de la mémoire
    int nbBlocs;
    int max_bloc;      // Taille en blocs
} Fichier;

// Prototypes des fonctions
Virtualdisk* InitialiseMs(int nbloc);
void VidezMS(Virtualdisk* ms);
void remplirBuffer(BufferTransmission *buffer, const char *data);
void viderBuffer(BufferTransmission *buffer);
void ModifierTableAllocation(Virtualdisk* ms, int indexBloc);
Fichier *initialiserFichier(int capaciteMax,Virtualdisk *ms, char *nom, ModeOrganisationF sort, ModeOrganisationE mode);
void libererFichier(Fichier *fichier);
void AjouterBloc(Virtualdisk* ms,Fichier *Fichier);
Bloc* trouverBlocAvecEspace(Fichier* fichier);
bool libererBloc(Fichier *fichier, Bloc *blocDirect);
int comparerEnregistrements(const void *a, const void *b);
bool Compactage(Fichier *fichier);
bool ajouterEnregistrement(Virtualdisk* ms, Fichier* fichier, EnregistrementPhysique *enregistrement, BufferTransmission *buffer);
bool supprimerEnregistrement_Physique(Fichier *fichier, int id, BufferTransmission *buffer);
bool supprimerEnregistrement_Logique(Fichier *fichier, int id, BufferTransmission *buffer);
EnregistrementPhysique* rechercheSequencielleDansBloc(Bloc *bloc, int id);
EnregistrementPhysique* rechercheBinaireDansBloc(Bloc *bloc, const char *name,const char *sec);
EnregistrementPhysique* rechercherEnregistrement(Fichier *fichier, int id, const char * name,const char *sec);
void afficherFichier(const Fichier *fichier);

bool lireEnregistrement(FILE *fichier, EnregistrementPhysique *enregistrement);
void ecrireEnregistrement(FILE *fichier, EnregistrementPhysique *enregistrement);
bool enregistrementValide(const EnregistrementPhysique *enregistrement);
// Fonctions de gestion des buffers
void libererHashTable(HashTable *hashTable);
bool ajouterDansHashTable(HashTable *hashTable, int id);
bool supprimerDansHashTable(HashTable *hashTable, int id);
bool trouverDansHashTable(HashTable *hashTable, int id);
int hashFunction(int id, int tailleTable);
HashTable *initialiserHashTable(int taille);
unsigned long CalculerTailleEnregistrement(const EnregistrementPhysique *enregistrement);

#endif
