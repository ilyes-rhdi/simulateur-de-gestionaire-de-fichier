#include "tov.h"

bool ajouterEnregistrement(Virtualdisk* ms, FichierTOV* fichier, EnregistrementPhysique *enregistrement, BufferTransmission *buffer) {
    if (fichier == NULL || enregistrement == NULL || buffer == NULL || ms == NULL) {
        return false;
    }

    // Vérification de la capacité maximale du fichier
    if (fichier->entete.nbEnregistrements >= fichier->entete.capaciteMax) {
        return false;
    }

    // Attribution d'un ID unique à l'enregistrement
    enregistrement->entete.id = fichier->entete.nextID++;

    // Trouver un bloc avec de l'espace
    Bloc *blocActuel = trouverBlocAvecEspace(fichier);

    // Si aucun bloc avec espace n'est trouvé, ajouter un nouveau bloc
    if (blocActuel == NULL) {
        Ajouterbloc(ms, fichier);
        blocActuel = trouverBlocAvecEspace(fichier);
        if (blocActuel == NULL) {
            return false; // Échec si aucun bloc valide n'est trouvé
        }
    }

    // Ajouter l'enregistrement au bloc actuel
    void* positionAjout = blocActuel->data + (blocActuel->taille * TAILLE_MAX_ENREGISTREMENT);
    memcpy(positionAjout, enregistrement, TAILLE_MAX_ENREGISTREMENT);
    blocActuel->taille++; // Incrément du nombre d'enregistrements

    // Mode interne
    if (fichier->mode == Interne) {
        fichier->enregistrements[fichier->entete.nbEnregistrements] = *enregistrement;
    }
    // Mode global
    else if (fichier->mode == Global) {
        if (!ajouterDansHashTable(fichier->table, enregistrement->entete.id)) {
            return false;
        }
    }

    // Mise à jour du nombre total d'enregistrements
    fichier->entete.nbEnregistrements++;

    return true;
}

bool supprimerEnregistrement(FichierTOV *fichier, int id, BufferTransmission *buffer) {
    if (fichier == NULL || buffer == NULL) {
        return false;
    }

    bool found = false;

    // Parcourir les blocs pour trouver et supprimer l'enregistrement
    for (int i = 0; i < fichier->nbBlocs; i++) {
        Bloc *bloc = &fichier->blocs[i];
        char *blocData = bloc->data;

        for (int offset = 0; offset < bloc->taille; offset++) {
            EnregistrementPhysique *enr = (EnregistrementPhysique *)(blocData + (offset * TAILLE_MAX_ENREGISTREMENT));
            if (enr->entete.id == id) {
                // Décaler les enregistrements suivants
                memmove(
                    blocData + (offset * TAILLE_MAX_ENREGISTREMENT),
                    blocData + ((offset + 1) * TAILLE_MAX_ENREGISTREMENT),
                    (bloc->taille - offset - 1) * TAILLE_MAX_ENREGISTREMENT
                );
                bloc->taille--; // Mise à jour du nombre d'enregistrements
                found = true;
                break;
            }
        }

        if (found) break; // Arrêter si l'enregistrement a été trouvé
    }

    if (found) {
        // Mise à jour en mode interne
        if (fichier->mode == Interne) {
            for (int i = 0; i < fichier->entete.nbEnregistrements; i++) {
                if (fichier->enregistrements[i].entete.id == id) {
                    for (int j = i; j < fichier->entete.nbEnregistrements - 1; j++) {
                        fichier->enregistrements[j] = fichier->enregistrements[j + 1];
                    }
                    fichier->entete.nbEnregistrements--;
                    break;
                }
            }
        }
        // Mise à jour en mode global
        else if (fichier->mode == Global) {
            Supprimerduhash(fichier->table, id);
        }
    }

    return found;
}


// Fonction de validation
bool enregistrementValide(const EnregistrementPhysique *enregistrement) {
    return enregistrement != NULL && enregistrement->entete.id >= 0;
}

// Fonction de lecture
bool lireEnregistrement(FILE *fichier, EnregistrementPhysique *enregistrement) {
    return fscanf(fichier, "%d|%[^|]|%[^|]|%s\n",
                  &enregistrement->entete.id,
                  enregistrement->data1,
                  enregistrement->data2,
                  enregistrement->data3) == 4;
}

// Fonction d'écriture
void ecrireEnregistrement(FILE *fichier, EnregistrementPhysique *enregistrement) {
    fprintf(fichier, "%d|%s|%s|%s\n",
            enregistrement->entete.id,
            enregistrement->data1,
            enregistrement->data2,
            enregistrement->data3);
}
EnregistrementPhysique *rechercherEnregistrement(FichierTOV *fichier, HashTable *hashTable, int id)
{
    if (fichier == NULL || hashTable == NULL)
    {
        return NULL;
    }
    if (fichier->mode==Interne)
    {
        
    }else{ 
            
    
    /*calcule l'indice dans la table de hachage pour l'id donne en utilisant la fonction de hachage*/
        int index = hashFunction(id, hashTable->taille);

    // verifying if id is stocked at the this index
        if (hashTable->table[index] == id)
        {
             // find l'enregistrement dans le tableau "enregistrements" de "fichier"
            for (int i = 0; i < fichier->entete.nbEnregistrements; i++)
            {
                if (fichier->enregistrements[i].entete.id == id)
                {
                    return &fichier->enregistrements[i];
                } 
            }
        }
    }
    return NULL; // enregistrement not found
}
