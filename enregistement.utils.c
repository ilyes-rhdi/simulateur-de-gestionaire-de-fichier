#include "tov.h"

bool ajouterEnregistrement(Virtualdisk* ms, Fichier* fichier, EnregistrementPhysique *enregistrement, BufferTransmission *buffer) {
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


        fichier->enregistrements[fichier->entete.nbEnregistrements] = *enregistrement;
    fichier->entete.nbEnregistrements++;
    if (fichier->sort==Trie)
    {
          qsort(fichier->enregistrements, fichier->entete.nbEnregistrements, sizeof(EnregistrementPhysique), comparerEnregistrements);
    }
    
    return true;
}
int comparerEnregistrements(const void *a, const void *b) {
    EnregistrementPhysique *enregA = (EnregistrementPhysique*)a;
    EnregistrementPhysique *enregB = (EnregistrementPhysique*)b;
    
    // Comparaison d'abord par data3
    int comparaisonData3 = strcmp(enregA->data3, enregB->data3);
    
    if (comparaisonData3 != 0) {
        return comparaisonData3; // Si data3 est différent, on retourne directement
    }
    
    // Si data3 est égal, on compare ensuite par data1
    return strcmp(enregA->data1, enregB->data1);
}


bool supprimerEnregistrement(Fichier *fichier, int id, BufferTransmission *buffer) {
    if (fichier == NULL || buffer == NULL) {
        return false;
    }

    bool found = false;

    // Parcourir les blocs pour trouver et supprimer l'enregistrement
    if (fichier->mode==Contigue)
    {
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
                if (bloc->taille==0)
                {
                    libererbloc(fichier,bloc);
                }
                break;
            }
        }

        if (found) break; // Arrêter si l'enregistrement a été trouvé
    }
    }else{
        Bloc *prev = NULL;
        Bloc *current = fichier->blocs;

        while (current != NULL && !found) {
            char *blocData = current->data;

            for (int offset = 0; offset < current->taille; offset++) {
                EnregistrementPhysique *enr = (EnregistrementPhysique *)(blocData + (offset * TAILLE_MAX_ENREGISTREMENT));
                if (enr->entete.id == id) {
                    // Décaler les enregistrements suivants
                    memmove(
                        blocData + (offset * TAILLE_MAX_ENREGISTREMENT),
                        blocData + ((offset + 1) * TAILLE_MAX_ENREGISTREMENT),
                        (current->taille - offset - 1) * TAILLE_MAX_ENREGISTREMENT
                    );
                    current->taille--; // Mise à jour du nombre d'enregistrements dans le bloc
                    found = true;
                if (current->taille==0)
                {
                    libererbloc(fichier,current);
                }
                
                break;
            }
        }
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


void ecrireEnregistrement(FILE *fichier, EnregistrementPhysique *enregistrement) {
    fprintf(fichier, "%d|%s|%s|%s\n",
            enregistrement->entete.id,
            enregistrement->data1,
            enregistrement->data2,
            enregistrement->data3);
}
EnregistrementPhysique *rechercherEnregistrement(Fichier *fichier, int id, const char *cle) {
    if (fichier == NULL || cle == NULL) {
        return NULL; // Vérification des paramètres
    }

    // Recherche dans les blocs
    if (fichier->mode == Contigue) {
        // Mode contigu
        for (int i = 0; i < fichier->nbBlocs; i++) {
            Bloc *bloc = &fichier->blocs[i];

            // Si trié, utiliser une recherche binaire
            if (fichier->sort == Trie) {
                EnregistrementPhysique *result = rechercheBinaireDansBloc(bloc, cle);
                if (result != NULL) {
                    return result;
                }
            } else { // Sinon, recherche séquentielle
                EnregistrementPhysique *result = rechercheSequencielleDansBloc(bloc, id);
                if (result != NULL) {
                    return result;
                }
            }
        }
    } else {
        // Mode chaîné
        Bloc *current = fichier->blocs;
        while (current != NULL) {
            // Si trié, utiliser une recherche binaire
            if (fichier->sort == Trie) {
                EnregistrementPhysique *result = rechercheBinaireDansBloc(current, cle);
                if (result != NULL) {
                    return result;
                }
            } else { // Sinon, recherche séquentielle
                EnregistrementPhysique *result = rechercheSequencielleDansBloc(current, id);
                if (result != NULL) {
                    return result;
                }
            }
            current = current->next; // Passer au bloc suivant
        }
    }

    return NULL; // Si non trouvé
}
EnregistrementPhysique *rechercheSequencielleDansBloc(Bloc *bloc, int id) {
    if (bloc == NULL) {
        return NULL;
    }

    for (int offset = 0; offset < bloc->taille; offset++) {
        EnregistrementPhysique *enr = (EnregistrementPhysique *)(bloc->data + (offset * TAILLE_MAX_ENREGISTREMENT));
        if (enr->entete.id == id) {
            return enr;
        }
    }

    return NULL; // Non trouvé
}

EnregistrementPhysique *rechercheBinaireDansBloc(Bloc *bloc, const char *cle) {
    if (bloc == NULL || cle == NULL) {
        return NULL;
    }

    int debut = 0;
    int fin = bloc->taille - 1;

    while (debut <= fin) {
        int milieu = (debut + fin) / 2;
        EnregistrementPhysique *enr = (EnregistrementPhysique *)(bloc->data + (milieu * TAILLE_MAX_ENREGISTREMENT));

        int comparaison = strcmp(enr->data3, cle);
        if (comparaison == 0) {
            return enr; // Trouvé
        } else if (comparaison < 0) {
            debut = milieu + 1;
        } else {
            fin = milieu - 1;
        }
    }

    return NULL; // Non trouvé
}
