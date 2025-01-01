#include "mylib.h"
// pas le temps de utiliser le hashage
// nouvelle fonction d'initialisation de la table hashage:          (checked)
HashTable *initialiserHashTable(int taille) {
    HashTable *hashTable = malloc(sizeof(HashTable));
    if (!hashTable) {
        return NULL;
    }

    hashTable->taille = taille;
    hashTable->table = malloc(taille * sizeof(Element *));
    if (!hashTable->table) {
        free(hashTable);
        return NULL;
    }

    // Initialisation des buckets à NULL
    for (int i = 0; i < taille; i++) {
        hashTable->table[i] = NULL;
    }

    return hashTable;
}
// sont role est de transformer un ID en un indice
int hashFunction(int id, int tailleTable)
{
    // simple hach use of modulo
    // this will assure que l'indice is always dans les limites de la table de hachage (de 0 a tailleTable - 1).
    return id % tailleTable;
}
bool trouverDansHashTable(HashTable *hashTable, int id) {
    if (!hashTable) {
        return false;
    }

    int index = hashFunction(id, hashTable->taille);
    Element *courant = hashTable->table[index];

    // Parcourir la liste chaînée à ce bucket
    while (courant) {
        if (courant->id == id) {
            return true; // ID trouvé
        }
        courant = courant->suivant;
    }

    return false; // ID non trouvé
}
bool ajouterDansHashTable(HashTable *hashTable, int id) {
    if (!hashTable) {
        return false;
    }

    // Vérifier si l'ID existe déjà
    if (trouverDansHashTable(hashTable, id)) {
        return false; // ID déjà présent
    }

    // Calculer l'index
    int index = hashFunction(id, hashTable->taille);

    // Créer un nouvel élément
    Element *nouvelElement = malloc(sizeof(Element));
    if (!nouvelElement) {
        return false; // Échec de l'allocation
    }
    nouvelElement->id = id;
    nouvelElement->suivant = hashTable->table[index];

    // Ajouter au début de la liste chaînée
    hashTable->table[index] = nouvelElement;

    return true; // Ajout réussi
}
bool supprimerDansHashTable(HashTable *hashTable, int id) {
    if (!hashTable) {
        return false;
    }

    int index = hashFunction(id, hashTable->taille);
    Element *courant = hashTable->table[index];
    Element *precedent = NULL;

    // Parcourir la liste chaînée à ce bucket
    while (courant) {
        if (courant->id == id) {
            // Si l'élément est trouvé
            if (precedent) {
                // Retirer l'élément du milieu ou de la fin
                precedent->suivant = courant->suivant;
            } else {
                // Retirer le premier élément du bucket
                hashTable->table[index] = courant->suivant;
            }

            free(courant); // Libérer la mémoire de l'élément supprimé
            return true;   // Suppression réussie
        }

        precedent = courant;
        courant = courant->suivant;
    }

    return false; // ID non trouvé
}

void libererHashTable(HashTable *hashTable) {
    if (!hashTable) {
        return;
    }

    for (int i = 0; i < hashTable->taille; i++) {
        Element *courant = hashTable->table[i];
        while (courant) {
            Element *aSupprimer = courant;
            courant = courant->suivant;
            free(aSupprimer);
        }
    }

    free(hashTable->table);
    free(hashTable);
}

#include "mylib.h"