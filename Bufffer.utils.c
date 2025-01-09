#include "mylib.h"

/*                                                            *  DONE  BY ilyes  *                                                        */


// utiliser le buffer pour gerer le tampon de transmission avec ces 2 fonctions   (checked)
void remplirBuffer(BufferTransmission *buffer,Bloc *bloc)
{
    if (buffer == NULL || bloc == NULL)
        return;
    // copy data to the buffer, leaving space for the null terminator

    buffer->taille=bloc->taille;
    // Copier les champs simples directement
    buffer->B.taille = bloc->taille;
    buffer->B.numBloc = bloc->numBloc;
    buffer->B.estComplet = bloc->estComplet;

    // Gérer le champ `enregistrements` (copie profonde si nécessaire)
    if (bloc->enregistrements != NULL) {
        // Allouer de la mémoire pour `enregistrements`
        buffer->B.enregistrements = malloc(bloc->taille * sizeof(EnregistrementPhysique));
        if (buffer->B.enregistrements != NULL) {
            // Copier les données de `enregistrements`
            memcpy(buffer->B.enregistrements, bloc->enregistrements, bloc->taille * sizeof(EnregistrementPhysique));
        } else {
            // Si l'allocation échoue, on peut choisir de retourner ou de continuer sans copier
            buffer->B.enregistrements = NULL;
        }
    } else {
        buffer->B.enregistrements = NULL;
    }

    // Gérer le champ `next` (par défaut, ne pas copier le bloc suivant)
    buffer->B.next = NULL; // On peut mettre `next` à NULL pour éviter des conflits

}

void viderBuffer(BufferTransmission *buffer)
{ //(checked)
    if (buffer == NULL)
        return;
    // set the first character of the buffer to '\0' pour indiquer an empty string
    buffer->B.enregistrements = NULL;
    buffer->B.estComplet=false;
    buffer->B.next=NULL;
    buffer->B.numBloc=-1;
    buffer->B.taille=0;
    // reset the size of the data in the buffer to 0
    buffer->taille = 0;
}

// utiliser les fonctions dans main
#include "mylib.h"

