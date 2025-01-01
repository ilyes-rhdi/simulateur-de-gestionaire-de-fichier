#include "mylib.h"

/*                                                            *  DONE  BY ilyes  *                                                        */


// utiliser le buffer pour gerer le tampon de transmission avec ces 2 fonctions   (checked)
void remplirBuffer(BufferTransmission *buffer,Bloc *bloc)
{
    if (buffer == NULL || bloc == NULL)
        return;
    // copy data to the buffer, leaving space for the null terminator
    buffer->B=*bloc;
    buffer->taille=bloc->taille;

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

