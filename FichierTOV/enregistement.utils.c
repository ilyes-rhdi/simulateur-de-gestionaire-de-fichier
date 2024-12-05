#include "tov.h"

bool ajouterEnregistrement(Virtualdisk* ms,FichierTOV* fichier,EnregistrementPhysique *enregistrement, BufferTransmission *buffer) {
    if (fichier == NULL || enregistrement == NULL || buffer == NULL || ms == NULL) {
        return false;
    }

    if (fichier->entete.nbEnregistrements >= fichier->entete.capaciteMax) {
        return false;
    }

    enregistrement->entete.id = fichier->entete.nextID++;
    enregistrement->suivant = NULL;
    Bloc *blocActuel = trouverBlocAvecEspace(fichier);
    // Gestion des blocs
    if (fichier->nbBlocs == 0 || blocActuel == NULL) {
        Ajouterbloc(ms,fichier);
    } else {
        blocActuel = &fichier->blocs[fichier->nbBlocs - 1];
    }

    memcpy(blocActuel->data + blocActuel->taille, enregistrement, sizeof(EnregistrementPhysique));
    blocActuel->taille += sizeof(EnregistrementPhysique);

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

    fichier->entete.nbEnregistrements++;
    return true;
}

// Fonction de suppression
bool supprimerEnregistrement(FichierTOV *fichier, int id, BufferTransmission *buffer) {
    if (fichier == NULL || buffer == NULL) {
        return false;
    }

    bool found = false;

    // Suppression dans les blocs
    for (int i = 0; i < fichier->nbBlocs; i++) {
        Bloc *bloc = &fichier->blocs[i];
        char *blocData = bloc->data;

        for (int offset = 0; offset < bloc->taille; offset += sizeof(EnregistrementPhysique)) {
            EnregistrementPhysique *enr = (EnregistrementPhysique *)(blocData + offset);
            if (enr->entete.id == id) {
                // Décaler les données suivantes
                memmove(blocData + offset,
                        blocData + offset + sizeof(EnregistrementPhysique),
                        bloc->taille - offset - sizeof(EnregistrementPhysique));
                bloc->taille -= sizeof(EnregistrementPhysique);
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (found) {
        // Mise à jour interne
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
        // Mise à jour globale
        else if (fichier->mode == Global) {
            Supprimerduhash(fichier->table, id);
        }
    }

    return found;
}

// Fonction de recherche
EnregistrementPhysique *rechercherEnregistrement(FichierTOV *fichier, int id) {
    if (fichier == NULL) {
        return NULL;
    }

    for (int i = 0; i < fichier->nbBlocs; i++) {
        Bloc *bloc = &fichier->blocs[i];
        char *blocData = bloc->data;

        for (int offset = 0; offset < bloc->taille; offset += sizeof(EnregistrementPhysique)) {
            EnregistrementPhysique *enr = (EnregistrementPhysique *)(blocData + offset);
            if (enr->entete.id == id) {
                return enr;
            }
        }
    }

    return NULL; // Non trouvé
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

include "tov.h"