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

// utiliser les fonctions dans main
#include "tov.h"

