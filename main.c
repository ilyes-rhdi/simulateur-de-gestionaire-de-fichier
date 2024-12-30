#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

int main() {
    // Initialisation du fichier TOV avec une capacité maximale de 10 enregistrements
    // et des modes de tri et d'organisation choisis
    Virtualdisk* ms = InitialiseMs(2);
    Fichier *fichier = initialiserFichier(1, ms, "ilyes", Trie, Chainee);

    // Vérification si le fichier a bien été initialisé
    if (fichier == NULL) {
        printf("Erreur lors de l'initialisation du fichier.\n");
        return 1;  // Terminer l'exécution si l'initialisation a échoué
    }

    // Création d'un enregistrement physique
    EnregistrementPhysique enregistrement;
    strcpy(enregistrement.data1, "ILyes rachedi");
    strcpy(enregistrement.data2, "2eme ingenieur informatique");
    strcpy(enregistrement.data3, "C");
    EnregistrementPhysique enregistrement1;
    strcpy(enregistrement1.data1, "karim rachedi");
    strcpy(enregistrement1.data2, "2eme ingenieur informatique");
    strcpy(enregistrement1.data3, "C");
    EnregistrementPhysique enregistrement2;
    strcpy(enregistrement2.data1, "tewfik rachedi");
    strcpy(enregistrement2.data2, "2eme ingenieur informatique");
    strcpy(enregistrement2.data3, "A");
    EnregistrementPhysique enregistrement3;
    strcpy(enregistrement3.data1, "tewfik rachedi");
    strcpy(enregistrement3.data2, "2eme ingenieur informatique");
    strcpy(enregistrement3.data3, "A");


    // Initialisation d'un buffer de transmission
    BufferTransmission buffer;
    buffer.taille = 256;

    // Ajouter un enregistrement dans le fichier
    if (ajouterEnregistrement(ms, fichier, &enregistrement, &buffer)) {
        printf("Enregistrement ajouté avec succès.\n");
    } else {
        printf("Erreur lors de l'ajout de l'enregistrement.\n");
    }

    if (ajouterEnregistrement(ms, fichier, &enregistrement1, &buffer)) {
        printf("Enregistrement ajouté avec succès.\n");
    } else {
        printf("Erreur lors de l'ajout de l'enregistrement.\n");
    }

    if (ajouterEnregistrement(ms, fichier, &enregistrement2, &buffer)) {
        printf("Enregistrement ajouté avec succès.\n");
    } else {
        printf("Erreur lors de l'ajout de l'enregistrement.\n");
    }
    if (ajouterEnregistrement(ms, fichier, &enregistrement3, &buffer)) {
        printf("Enregistrement ajouté avec succès.\n");
    } else {
        printf("Erreur lors de l'ajout de l'enregistrement.\n");
    }


    // Affichage des informations du fichier
    afficherFichier(fichier);

    //Recherche de l'enregistrement ajouté dans le fichier
    EnregistrementPhysique *resultat = rechercherEnregistrement(fichier, enregistrement.entete.id, "ilyes rachedi", "C");
    if (resultat != NULL) {
        printf("Enregistrement trouve : %s, %s, %s\n", resultat->data1, resultat->data2, resultat->data3);
    } else {
        printf("Enregistrement non trouve.\n");
    }

    if (supprimerEnregistrement(fichier, resultat->entete.id, &buffer,true)) {
        printf("Enregistrement supprimé\n");
    }

    afficherFichier(fichier);

    // Libération de la mémoire allouée pour le fichier
    libererFichier(fichier);

    return 0;
}
