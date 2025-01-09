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
#define MAX_ENREGISTREMENTS_PAR_BLOC (TAILLE_MAX_BLOC /TAILLE_MAX_ENREGISTREMENT )+1

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
    Bloc B; // Buffer pour stocker temporairement des données à transmettre
    int taille;               // Quantité de données actuellement stockées
} BufferTransmission;

typedef enum {
    Contigue,
    Chainee
} ModeOrganisationF;
typedef enum {
    Trie,
    NoTrie
} ModeOrganisationE;
// Structure représentant le fichier 
typedef struct {
    EnteteFichier entete;
    char nomFichier[30];  // Nom du fichier, modifiable
    ModeOrganisationE mode;
    ModeOrganisationF sort;
    Bloc *blocs;       // Tableau de blocs pour la gestion de la mémoire
    int nbBlocs;
    int max_bloc;      // Taille en blocs
} Fichier;
typedef struct {
    int nb_blocs;//nombre max de bloc 
    int nb;//nombre actuel de bloc 
    Bloc *bloc;
    Fichier *table_fichiers;  
    ModeOrganisationF mode;
} Virtualdisk;



// Prototypes des fonctions
Virtualdisk* InitialiseMs(int nbloc);
void VidezMS(Virtualdisk* ms);
void remplirBuffer(BufferTransmission *buffer,Bloc *bloc);
void viderBuffer(BufferTransmission *buffer);
void EcrireBloc(BufferTransmission*Buffer,Bloc * bloc);
void LireBloc(BufferTransmission *Buffer,Bloc *bloc);
void afficherBloc(BufferTransmission *Buffer, Bloc bloc);
void ModifierTableAllocation(Virtualdisk* ms, int indexBloc);
void RecupererInfoFichier(Virtualdisk *ms, const char* nom, Fichier* fichier);
void LireBlocDepuisMS(Virtualdisk *ms, int numBloc, Bloc *bloc);
bool fichierExisteDansMS(Virtualdisk *ms, const char* nom);
Fichier *initialiserFichier(int capaciteMax,Virtualdisk *ms, char *nom, ModeOrganisationF sort, ModeOrganisationE mode);
void libererFichier(Fichier *fichier);
void AjouterBloc(Virtualdisk* ms,Fichier *Fichier);
Bloc* trouverBlocAvecEspace(Fichier* fichier);
bool libererBloc(Fichier *fichier, Bloc *blocDirect,Virtualdisk *ms);
int comparerEnregistrements(const void *a, const void *b);
bool Compactage(Virtualdisk *ms);
bool fermerFichier(Fichier* fichier, Virtualdisk* ms);
bool Defragmentation(Fichier* fichier);
Fichier* ouvrirFichier(const char* nom, Virtualdisk *ms);
bool ajouterEnregistrement(Virtualdisk* ms, Fichier* fichier, EnregistrementPhysique *enregistrement, BufferTransmission *buffer);
bool supprimerEnregistrement(Virtualdisk* ms,Fichier *fichier, int id, BufferTransmission *buffer, bool suppression_physique);
EnregistrementPhysique* rechercheSequencielleDansBloc(Bloc *bloc, int id);
EnregistrementPhysique* rechercheBinaireDansBloc(Bloc *bloc, const char *name,const char *sec);
EnregistrementPhysique* rechercherEnregistrement(Fichier *fichier, int id, const char * name,const char *sec);
void afficherFichier(Fichier *fichier);
bool lireEnregistrement(EnregistrementPhysique *enregistrement, const char *buffer);
void ecrireEnregistrement(char *buffer, size_t size, EnregistrementPhysique *enregistrement);
bool enregistrementValide(const EnregistrementPhysique *enregistrement);
void RenameFichier(const char* name,const char* Newname,Virtualdisk *ms);
// Fonctions de gestion des buffers
void libererHashTable(HashTable *hashTable);
bool ajouterDansHashTable(HashTable *hashTable, int id);
bool supprimerDansHashTable(HashTable *hashTable, int id);
bool trouverDansHashTable(HashTable *hashTable, int id);
int hashFunction(int id, int tailleTable);
HashTable *initialiserHashTable(int taille);

#endif
