#include "tov.h"

/*                                                            *  DONE  BY ilyes  *                                                        */




// utiliser le buffer pour gerer le tampon de transmission avec ces 2 fonctions   (checked)
void remplirBuffer(BufferTransmission *buffer, const char *data)
{
    if (buffer == NULL || data == NULL)
        return;
    // copy data to the buffer, leaving space for the null terminator
    strncpy(buffer->data, data, TAILLE_BUFFER - 1);
    // explicitly set the last character to '\0' for null termination
    buffer->data[TAILLE_BUFFER - 1] = '\0';
    // Update the size of the data in the buffer
    buffer->taille = strlen(buffer->data);
}

void viderBuffer(BufferTransmission *buffer)
{ //(checked)
    if (buffer == NULL)
        return;
    // set the first character of the buffer to '\0' pour indiquer an empty string
    buffer->data[0] = '\0';
    // reset the size of the data in the buffer to 0
    buffer->taille = 0;
}

// une fonction pas demandee , j vais l'utiliser juste pour calculer la taille des diff enregistrements
unsigned long CalculerTailleEnregistrement(const EnregistrementPhysique *enregistrement)
{
    if (enregistrement == NULL)
        return 0;

    return sizeof(EnteteEnregistrement) + strlen(enregistrement->data1);
}
Bloc *trouverBlocAvecEspace(FichierTOV *fichier, size_t tailleEnregistrement) {
    for (int i = 0; i < fichier->nbBlocs; i++) {
        if (fichier->blocs[i].capacite - fichier->blocs[i].taille >= tailleEnregistrement) {
            return &fichier->blocs[i];
        }
    }
    return NULL; // Aucun bloc avec assez d'espace
}

// utiliser les fonctions dans main
#include "tov.h"


// utiliser les fonctions dans main
#include "tov.h"
