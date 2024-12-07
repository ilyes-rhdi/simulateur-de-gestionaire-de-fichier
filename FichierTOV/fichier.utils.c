#include "tov.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

// Initialisation du fichierTOV
Fichier *initialiserFichierTOV(int capaciteMax, const char *nom, ModeOrganisationF sort, ModeOrganisationE mode) {
    Fichier *fichier = malloc(sizeof(Fichier));
    if (fichier == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour le fichier.\n");
        return NULL;
    }

    // Initialisation des champs
    strncpy(fichier->nomFichier, nom, sizeof(fichier->nomFichier) - 1);
    fichier->nomFichier[sizeof(fichier->nomFichier) - 1] = '\0';
    fichier->mode = mode;
    fichier->sort = sort;
    fichier->nbBlocs = 0;
    fichier->max_bloc=(capaciteMax*TAILLE_MAX_ENREGISTREMENT)/TAILLE_MAX_BLOC + 1;
    fichier->entete.nbEnregistrements = 0;
    fichier->entete.capaciteMax = capaciteMax;
    fichier->entete.nextID = 0;

    // Initialisation des blocs selon le mode
    if (mode == Contigue) {
        fichier->blocs = calloc(fichier->nbBlocs, sizeof(Bloc));
        if (fichier->blocs == NULL) {
            fprintf(stderr, "Erreur : Allocation mémoire échouée pour les blocs contigus.\n");
            free(fichier);
            return NULL;
        }
    } else if (mode == Chainee) {
        fichier->blocs = NULL; // Les blocs seront alloués dynamiquement
    }

    return fichier;
}

// Libération de la mémoire du fichier TOV
void libererFichierTOV(Fichier *fichier) {
    if (fichier == NULL) {
        printf("libererFichierTOV: fichier est NULL\n");
        return;
    }

    // Suppression du fichier physique
    if (access(fichier->nomFichier, F_OK) == 0) {
        if (remove(fichier->nomFichier) != 0) {
            fprintf(stderr, "Erreur : Échec de suppression du fichier %s.\n", fichier->nomFichier);
        }
    }

    // Libération mémoire selon le mode
    if (fichier->mode == Contigue) {
        free(fichier->blocs);
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        while (current != NULL) {
            Bloc *next = current->next;
            free(current->data); // Libération des données du bloc
            free(current);       // Libération du bloc
            current = next;
        }
    }

    free(fichier); // Libération de la structure FichierTOV elle-même
    printf("libererFichierTOV: mémoire libérée et fichier supprimé\n");
}

// Affichage du contenu du fichier TOV
void afficherFichierTOV(const Fichier *fichier) {
    if (fichier == NULL) {
        printf("afficherFichierTOV: fichier est NULL\n");
        return;
    }

    printf("Fichier TOV contient %d enregistrements:\n", fichier->entete.nbEnregistrements);

    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->entete.nbEnregistrements; i++) {
            printf("Enregistrement %d:\n", i);
            printf("data1: %s\n", fichier->enregistrements[i].data1);
            printf("data2: %s\n", fichier->enregistrements[i].data2);
            printf("data3: %s\n", fichier->enregistrements[i].data3);
        }
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        int index = 0;
        while (current != NULL) {
            printf("Bloc %d:\n", index++);
            printf("Data: %s\n", current->data);
            current = current->next;
        }
    }
}

// Compactage du fichier TOV
bool Compactage(Fichier *fichier) {
    if (fichier == NULL) {
        printf("Compactage: fichier est NULL\n");
        return false;
    }

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
    return true;
}
