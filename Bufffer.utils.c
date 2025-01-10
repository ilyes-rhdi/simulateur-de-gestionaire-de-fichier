#include "mylib.h"

/*                                                            *  DONE  BY ilyes  *                                                        */


// utiliser le buffer pour gerer le tampon de transmission avec ces 2 fonctions   (checked)
void remplirBuffer(BufferTransmission *buffer, Bloc *bloc) {
    if (bloc == NULL || buffer ==NULL ) {
        fprintf(stderr, "Erreur : Paramètres invalides pour remplirBuffer.\n");
        return;
    }

    // Allouer de la mémoire pour le bloc dans le buffer
    buffer->B = malloc(sizeof(Bloc));
    if (buffer->B == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour buffer->B.\n");
        return;
    }

    // Copier les champs simples
    buffer->B->taille = bloc->taille;
    buffer->B->numBloc = bloc->numBloc;
    buffer->B->estComplet = bloc->estComplet;

    // Allouer et copier les enregistrements
    if (bloc->enregistrements != NULL && bloc->taille > 0) {
        buffer->B->enregistrements = malloc(bloc->taille * sizeof(EnregistrementPhysique));
        if (buffer->B->enregistrements == NULL) {
            fprintf(stderr, "Erreur : Allocation mémoire échouée pour enregistrements.\n");
            free(buffer->B);
            buffer->B = NULL;
            return;
        }

        char buff[256];
        for (int i = 0; i < bloc->taille; i++) {
            if (ecrireEnregistrement(buff, 256,&bloc->enregistrements[i])) {
                if (!lireEnregistrement(&buffer->B->enregistrements[i], buff))
                {
                    printf("erreur : l'enregistrement n'a pas pu etre ecris dans le buffer");
                }
            } else {
                fprintf(stderr, "Erreur : Échec de la lecture de l'enregistrement %d.\n", i);
                // Libérer les ressources partiellement allouées
                for (int j = 0; j < i; j++) {
                    free(&buffer->B->enregistrements[j]);
                }
                free(buffer->B->enregistrements);
                free(buffer->B);
                buffer->B = NULL;
                return;
            }
        }
    } else {
        buffer->B->enregistrements = NULL;
    }

    // Ne pas copier `next` par défaut
    buffer->B->next = NULL;
}


void viderBuffer(BufferTransmission *buffer)
{ //(checked)
    if (buffer == NULL)
        return;
    // set the first character of the buffer to '\0' pour indiquer an empty string
    buffer->B->enregistrements = NULL;
    buffer->B->estComplet=false;
    buffer->B->next=NULL;
    buffer->B->numBloc=-1;
    buffer->B->taille=0;
    buffer->B= NULL;
    // reset the size of the data in the buffer to 0
    buffer->taille = 0;
}

// utiliser les fonctions dans main
#include "mylib.h"

