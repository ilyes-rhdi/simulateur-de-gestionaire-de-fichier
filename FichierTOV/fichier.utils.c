#include "tov.h"
// initialisation du fichierTOV                           (checked)
void initialiserFichierTOV(FichierTOV *fichier, int capaciteMax, const char *nom, ModeOrganisation mode) {
    if (fichier == NULL) {
        printf("initialiserFichierTOV: fichier est NULL\n");
        return;
    }


    // Initialisation des champs
    strncpy(fichier->nomFichier, nom, sizeof(fichier->nomFichier) - 1);
    fichier->nomFichier[sizeof(fichier->nomFichier) - 1] = '\0';
    fichier->mode = mode;
    fichier->nbBlocs = (capaciteMax*TAILLE_MAX_ENREGISTREMENT)/TAILLE_MAX_BLOC  + 1 ;
    fichier->entete.nbEnregistrements = 0;
    fichier->entete.capaciteMax = capaciteMax;
    fichier->entete.nextID = 0;

    // Allocation mémoire selon le mode d'organisation
    if (mode == Interne) {
        fichier->enregistrements = malloc(capaciteMax * sizeof(EnregistrementPhysique));
        if (fichier->enregistrements == NULL) {
            printf("initialiserFichierTOV: échec d'allocation mémoire\n");
            return;
        }
    } else if (mode == Global) {
        fichier->table = malloc(sizeof(HashTable));
        if (fichier->table == NULL) {
            printf("initialiserFichierTOV: échec d'allocation mémoire pour la table de hachage\n");
            free(fichier->enregistrements);
            return;
        }
        initialiserHashTable(fichier->table, capaciteMax);
    }

    printf("initialiserFichierTOV: initialisation réussie\n");
}

// Libération de la mémoire du fichier TOV
void libererFichierTOV(FichierTOV *fichier) {
    if (fichier == NULL) {
        printf("libererFichierTOV: fichier est NULL\n");
        return;
    }

    // Suppression du fichier physique
    if (access(fichier->nomFichier, F_OK) == 0) {
    remove(fichier->nomFichier);
    }

    // Libération mémoire selon le mode
    if (fichier->mode == Interne) {
        free(fichier->enregistrements);
        fichier->enregistrements = NULL;
    } else if (fichier->mode == Global) {
        libererHashTable(fichier->table);
        free(fichier->table);
        fichier->table = NULL;
    }

    fichier->entete.nbEnregistrements = 0;
    printf("libererFichierTOV: mémoire libérée et fichier supprimé\n");
}

// Affichage du contenu du fichier TOV
void afficherFichierTOV(const FichierTOV *fichier) {
    if (fichier == NULL) return;

    printf("Fichier TOV contient %d enregistrements:\n", fichier->entete.nbEnregistrements);
    for (int i = 0; i < fichier->entete.nbEnregistrements; i++) {
        printf("Enregistrement %d:\n", i);
        printf("data1: %s\n", fichier->enregistrements[i].data1);
        printf("data2: %s\n", fichier->enregistrements[i].data2);
        printf("data3: %s\n", fichier->enregistrements[i].data3);
    }
}

// Compactage du fichier TOV
bool Compactage(FichierTOV *fichier) {
    if (fichier == NULL) return false;

    const char *nomFichierTemp = "tempFichierTOV.tov";
    FILE *fichierPhysique = fopen(fichier->nomFichier, "r");
    FILE *fichierTemp = fopen(nomFichierTemp, "w");

    if (fichierPhysique == NULL || fichierTemp == NULL) {
        if (fichierPhysique) fclose(fichierPhysique);
        if (fichierTemp) fclose(fichierTemp);
        return false;
    }

    EnregistrementPhysique enregistrement;
    int nouveauxNbEnregistrements = 0;

    // Lecture et réécriture des enregistrements valides
    while (lireEnregistrement(fichierPhysique, &enregistrement)) {
        if (enregistrementValide(&enregistrement)) {
            ecrireEnregistrement(fichierTemp, &enregistrement);
            nouveauxNbEnregistrements++;
        }
    }

    fclose(fichierPhysique);
    fclose(fichierTemp);

    // Remplacement de l'ancien fichier par le fichier compacté
    
    if (remove(fichier->nomFichier) != 0) {
    printf("Compactage: échec de suppression du fichier original\n");
    return false;
    }
    if (rename(nomFichierTemp, fichier->nomFichier) != 0) {
       printf("Compactage: échec de renommage du fichier temporaire\n");
    return false;
    }


    fichier->entete.nbEnregistrements = nouveauxNbEnregistrements;

    if (fichier->mode == Interne) {
        reorganiserTableauInterne(fichier);
    } else if (fichier->mode == Global) {
        reinitialiserHashTable(fichier);
    }

    return true;
}

// Réorganisation du tableau interne
void reorganiserTableauInterne(FichierTOV *fichier) {
    int index = 0;
    for (int i = 0; i < fichier->entete.nbEnregistrements; ++i) {
        if (fichier->enregistrements[i].entete.id >= 0) {
            fichier->enregistrements[index++] = fichier->enregistrements[i];
        }
    }
    memset(&fichier->enregistrements[index], 0,
           (fichier->entete.nbEnregistrements - index) * sizeof(EnregistrementPhysique));
}

#include "tov.h"