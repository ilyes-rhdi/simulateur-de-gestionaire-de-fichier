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
    blocActuel->taille++; // Incrément du nombre d'enregistrements
    blocActuel->enregistrements[blocActuel->taille]= *enregistrement;
    fichier->entete.nbEnregistrements++;
    if (fichier->sort==Trie)
    {
          qsort(blocActuel->enregistrements,blocActuel->taille, sizeof(EnregistrementPhysique), comparerEnregistrements);
    }
    
    return true;
}
int comparerEnregistrements(const void *a, const void *b) {
    EnregistrementPhysique *enregA = (EnregistrementPhysique*)a;
    EnregistrementPhysique *enregB = (EnregistrementPhysique*)b;
    
    // Comparaison d'abord par data3
    int comparaisonData3 = stricmp(enregA->data3, enregB->data3);
    
    if (comparaisonData3 != 0) {
        return comparaisonData3; // Si data3 est différent, on retourne directement
    }
    
    // Si data3 est égal, on compare ensuite par data1
    return stricmp(enregA->data1, enregB->data1);
}


bool supprimerEnregistrement_Physique(Fichier *fichier, int id, BufferTransmission *buffer) {
    if (fichier == NULL || buffer == NULL) {
        return false;
    }

    bool found = false;

    if (fichier->mode == Contigue) {
        // Mode contigu : blocs gérés séquentiellement
        for (int i = 0; i < fichier->nbBlocs; i++) {
            Bloc *bloc = &fichier->blocs[i];

            for (int j = 0; j < bloc->taille; j++) {
                if (bloc->enregistrements[j].entete.id == id) {
                    // Supprimer l'enregistrement en décalant les éléments suivants
                    memmove(
                        &bloc->enregistrements[j],
                        &bloc->enregistrements[j + 1],
                        (bloc->taille - j - 1) * sizeof(EnregistrementPhysique)
                    );
                    bloc->taille--; // Mise à jour du nombre d'enregistrements
                    found = true;

                    // Libérer le bloc si vide
                    if (bloc->taille == 0) {
                        libererbloc(fichier, bloc);
                    }
                    break;
                }
            }

            if (found) break; // Arrêter si l'enregistrement est supprimé
        }
    } else {
        // Mode chaîné : blocs liés par des pointeurs
        Bloc *prev = NULL;
        Bloc *current = fichier->blocs;

        while (current != NULL) {
            for (int j = 0; j < current->taille; j++) {
                if (current->enregistrements[j].entete.id == id) {
                    // Supprimer l'enregistrement en décalant les éléments suivants
                    memmove(
                        &current->enregistrements[j],
                        &current->enregistrements[j + 1],
                        (current->taille - j - 1) * sizeof(EnregistrementPhysique)
                    );
                    current->taille--; // Mise à jour du nombre d'enregistrements
                    found = true;

                    // Libérer le bloc si vide
                    if (current->taille == 0) {
                        if (prev == NULL) {
                            fichier->blocs = current->next; // Premier bloc
                        } else {
                            prev->next = current->next; // Bloc intermédiaire ou final
                        }
                        libererbloc(fichier, current);
                    }
                    break;
                }
            }

            if (found) break; // Arrêter si l'enregistrement est supprimé

            // Passer au bloc suivant
            prev = current;
            current = current->next;
        }
    }

    return found;
}

bool supprimerEnregistrement_Logique(Fichier *fichier, int id, BufferTransmission *buffer) {
    if (fichier == NULL || buffer == NULL) {
        return false;
    }

    bool found = false;

    if (fichier->mode == Contigue) {
        // Mode contigu : blocs gérés séquentiellement
        for (int i = 0; i < fichier->nbBlocs; i++) {
            Bloc *bloc = &fichier->blocs[i];

            for (int j = 0; j < bloc->taille; j++) {
                if (bloc->enregistrements[j].entete.id == id) {
                    bloc->enregistrements[j].entete.suppprimer=true;
                    found = true;

                    break;
                }
            }
            if (found) break; // Arrêter si l'enregistrement est supprimé
        }
    } else {
        // Mode chaîné : blocs liés par des pointeurs
        Bloc *prev = NULL;
        Bloc *current = fichier->blocs;

        while (current != NULL) {
            for (int j = 0; j < current->taille; j++) {
                if (current->enregistrements[j].entete.id == id) {
                    current->enregistrements[j].entete.suppprimer=true;
                    found = true;
                    break;
                }
            }

            if (found) break; // Arrêter si l'enregistrement est supprimé

            // Passer au bloc suivant
            prev = current;
            current = current->next;
        }
    }

    return found;
}


// Fonction de validation
bool enregistrementValide(const EnregistrementPhysique *enregistrement) {
    return enregistrement != NULL && enregistrement->entete.id >= 0 && enregistrement->entete.suppprimer ;
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
EnregistrementPhysique *rechercherEnregistrement(Fichier *fichier, int id, const char *cle,const char *sec) {
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
                EnregistrementPhysique *result = rechercheBinaireDansBloc(bloc, cle,sec);
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
                EnregistrementPhysique *result = rechercheBinaireDansBloc(current, cle,sec);
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

    for (int i = 0; i < bloc->taille; i++) {
        if (bloc->enregistrements[i].entete.id == id) {
            return bloc->enregistrements + i ;
        }
    }

    return NULL; // Non trouvé
}

EnregistrementPhysique *rechercheBinaireDansBloc(Bloc *bloc, const char *name, const char *sec) {
    // Vérification des paramètres
    if (bloc == NULL || name == NULL || sec == NULL) {
        return NULL;
    }

    int debut = 0;
    int fin = bloc->taille - 1;

    // Boucle pour la recherche binaire
    while (debut <= fin) {
        int milieu = (debut + fin) / 2;

        // Récupération de l'enregistrement au milieu
        EnregistrementPhysique *enr = &bloc->enregistrements[milieu];

        // Comparaison avec sec
        int comparaison = stricmp(enr->data3, sec);
        if (comparaison == 0) {
            // Si sec correspond, comparer avec name
            int comparaison1 = stricmp(enr->data1, name);
            if (comparaison1 == 0) {
                // Si les deux conditions correspondent, retourner l'enregistrement
                return enr;
            } else if (comparaison1 < 0) {
                debut = milieu + 1;
            } else {
                fin = milieu - 1;
            }
        } else if (comparaison < 0) {
            debut = milieu + 1;
        } else {
            fin = milieu - 1;
        }
    }

    // Non trouvé
    return NULL;
}
