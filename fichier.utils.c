#include "tov.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

// Initialisation du fichierTOV
Fichier *initialiserFichier(int capaciteMax, const char *nom, ModeOrganisationF sort, ModeOrganisationE mode) {
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
    fichier->max_bloc = (capaciteMax * TAILLE_MAX_ENREGISTREMENT) / TAILLE_MAX_BLOC + 1;
    fichier->entete.nbEnregistrements = 0;
    fichier->entete.capaciteMax = capaciteMax;
    fichier->entete.nextID = 0;

    // Initialisation des blocs selon le mode
    if (mode == Contigue) {
        fichier->blocs = calloc(fichier->max_bloc, sizeof(Bloc));
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
void libererFichier(Fichier *fichier) {
    if (fichier == NULL) {
        printf("libererFichier: fichier est NULL\n");
        return;
    }

    // Suppression du fichier physique (non nécessaire dans ce cas, c'est une simulation)
    if (access(fichier->nomFichier, F_OK) == 0) {
        if (remove(fichier->nomFichier) != 0) {
            fprintf(stderr, "Erreur : Échec de suppression du fichier %s.\n", fichier->nomFichier);
        }
    }

    // Libération mémoire selon le mode
    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs; i++) {
            free(fichier->blocs[i].enregistrements);
        }
        free(fichier->blocs);
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        while (current != NULL) {
            Bloc *next = current->next;
            free(current->enregistrements); // Libération des données du bloc
            free(current);       // Libération du bloc
            current = next;
        }
    }

    free(fichier); // Libération de la structure FichierTOV elle-même
    printf("libererFichier: mémoire libérée et fichier supprimé\n");
}

// Affichage du contenu du fichier
void afficherFichier(const Fichier *fichier) {
    if (fichier == NULL) {
        printf("afficherFichier: fichier est NULL\n");
        return;
    }

    printf("Fichier contient %d enregistrements:\n", fichier->entete.nbEnregistrements);

    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs; i++) {
            for (int j = 0; j < fichier->blocs[i].taille; j++) {
                printf("Enregistrement %d:\n", j);
                printf("data1: %s\n", fichier->blocs[i].enregistrements[j].data1);
                printf("data2: %s\n", fichier->blocs[i].enregistrements[j].data2);
                printf("data3: %s\n", fichier->blocs[i].enregistrements[j].data3);
            }
        }
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        int index = 0;
        while (current != NULL) {
            printf("Bloc %d:\n", index++);
            for (int i = 0; i < current->taille; i++) {
                printf("Enregistrement %d:\n", i);
                printf("data1: %s\n", current->enregistrements[i].data1);
                printf("data2: %s\n", current->enregistrements[i].data2);
                printf("data3: %s\n", current->enregistrements[i].data3);
            }
            current = current->next;
        }
    }
}

// Compactage du fichier TOV
bool Compactage(Fichier* fichier) {
    if (fichier == NULL) {
        printf("Compactage : fichier est NULL\n");
        return false;
    }

    // Allocation d'un tableau temporaire pour les enregistrements compactés
    EnregistrementPhysique* enregistrementsCompactes = malloc(fichier->entete.capaciteMax * sizeof(EnregistrementPhysique));
    if (enregistrementsCompactes == NULL) {
        printf("Compactage : échec d'allocation pour les enregistrements compactés\n");
        return false;
    }

    int nouveauxNbEnregistrements = 0;

    // Parcourir les blocs existants et copier les enregistrements valides dans le tableau compacté
    for (int i = 0; i < fichier->nbBlocs; i++) {
        Bloc* bloc = &fichier->blocs[i];
        for (int j = 0; j < bloc->taille; j++) {
            EnregistrementPhysique* enregistrement = &bloc->enregistrements[j];
            if (enregistrementValide(enregistrement)) {
                enregistrementsCompactes[nouveauxNbEnregistrements++] = *enregistrement;
            }
        }
    }

    // Réorganiser les blocs avec les enregistrements compactés
    int indexCompacte = 0;
    for (int i = 0; i < fichier->nbBlocs; i++) {
        Bloc* bloc = &fichier->blocs[i];
        bloc->taille = 0;  // Réinitialisation du bloc avant de réécrire les enregistrements

        // Ajouter des enregistrements au bloc tant qu'il y a de la place
        while (bloc->taille < TAILLE_MAX_BLOC && indexCompacte < nouveauxNbEnregistrements) {
            bloc->enregistrements[bloc->taille++] = enregistrementsCompactes[indexCompacte++];
        }
    }

    // Mise à jour du nombre d'enregistrements dans l'entête
    fichier->entete.nbEnregistrements = nouveauxNbEnregistrements;

    // Libération du tableau temporaire
    free(enregistrementsCompactes);

    return true;
}
