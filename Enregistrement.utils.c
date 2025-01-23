#include "mylib.h"

bool ajouterEnregistrement(Virtualdisk* ms, Fichier* fichier, EnregistrementPhysique *enregistrement, BufferTransmission *buffer) {
    if (!ms || !fichier || !enregistrement || !buffer || 
        fichier->entete.nbEnregistrements >= (fichier->entete.capaciteMax * MAX_ENREGISTREMENTS_PAR_BLOC)) {
        return false;
    }


    enregistrement->entete.id = fichier->entete.nextID++;
    enregistrement->entete.suppprimer = false;

    Bloc *blocActuel = trouverBlocAvecEspace(fichier);
    if (!blocActuel) {
        AjouterBloc(ms, fichier);
        if (!(blocActuel = trouverBlocAvecEspace(fichier))) {
            return false;
        }
    }

    // Vérifier la capacité du bloc
    if (blocActuel->taille >= MAX_ENREGISTREMENTS_PAR_BLOC) {
           return false;
    }

    blocActuel->enregistrements[blocActuel->taille] = *enregistrement;
    blocActuel->taille++;
    fichier->entete.nbEnregistrements++;

    if (fichier->sort == Trie) {
        qsort(blocActuel->enregistrements, blocActuel->taille, 
              sizeof(EnregistrementPhysique), comparerEnregistrements);
    }
    char chemin_initial[256];
    getcwd(chemin_initial, sizeof(chemin_initial));
    char buf[256];
    if (chdir("MS") != 0) {
        perror("Impossible de changer de répertoire");
        return false;
    }else{
        // Créer et ouvrir le fichier (dans "ms" car le répertoire courant a changé)
        FILE *Fichier = fopen(fichier->nomFichier, "w");
        if (Fichier == NULL) {
            perror("Erreur lors de la création du fichier");
            return false;
        }
        if (ecrireEnregistrement(buf,256,enregistrement))
        {
            fprintf(Fichier,buf);
        }else{
            printf("err a l'ecriture dans la ms");
            return false;
        }
        chdir(chemin_initial);
    }

    return true;
}

int comparerEnregistrements(const void *a, const void *b) {
    const EnregistrementPhysique *enregA = (const EnregistrementPhysique*)a;
    const EnregistrementPhysique *enregB = (const EnregistrementPhysique*)b;
    
    if (!enregA->data3 || !enregB->data3 || !enregA->data1 || !enregB->data1) {
        return 0;
    }
    
    int comparaisonData3 = strcasecmp(enregA->data3, enregB->data3);
    return comparaisonData3 ? comparaisonData3 : strcasecmp(enregA->data1, enregB->data1);
}

static void supprimerDansBloc(Bloc *bloc, int index, bool suppression_physique) {
    if (suppression_physique) {
        memmove(&bloc->enregistrements[index],
                &bloc->enregistrements[index + 1],
                (bloc->taille - index - 1) * sizeof(EnregistrementPhysique));
        bloc->taille--;
    } else {
        bloc->enregistrements[index].entete.suppprimer = true;
    }
}

bool supprimerEnregistrement(Virtualdisk *ms,Fichier *fichier, int id, BufferTransmission *buffer, bool suppression_physique) {
    if (!fichier || !buffer) return false;

    bool found = false;
    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs && !found; i++) {
            Bloc *bloc = &fichier->blocs[i];
            for (int j = 0; j < bloc->taille; j++) {
                if (bloc->enregistrements[j].entete.id == id) {
                    supprimerDansBloc(bloc, j, suppression_physique);
                    found = true;
                    if (suppression_physique && bloc->taille == 0) {
                        libererBloc(fichier, bloc,ms);
                    }
                    break;
                }
            }
        }
    } else {
        Bloc *prev = NULL;
        Bloc *current = fichier->blocs;
        while (current && !found) {
            for (int j = 0; j < current->taille; j++) {
                if (current->enregistrements[j].entete.id == id) {
                    supprimerDansBloc(current, j, suppression_physique);
                    found = true;
                    if (suppression_physique && current->taille == 0) {
                        if (!prev) {
                            fichier->blocs = current->next;
                        } else {
                            prev->next = current->next;
                        }
                        libererBloc(fichier, current,ms);
                    }
                    break;
                }
            }
            prev = current;
            current = current->next;
        }
    }
    if (found)
    {
        fichier->entete.nbEnregistrements--;
    }
    
    return found;
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

bool enregistrementValide(const EnregistrementPhysique *enregistrement) {
    return enregistrement && enregistrement->entete.id >= 0 && 
           !enregistrement->entete.suppprimer &&
           enregistrement->data1 && enregistrement->data2 && enregistrement->data3;
}

bool lireEnregistrement(EnregistrementPhysique *enregistrement, const char *buffer) {
    if (!enregistrement || !buffer) return false;
    
    return sscanf(buffer, "%d|%[^|]|%[^|]|%s",
                 &enregistrement->entete.id,
                 enregistrement->data1,
                 enregistrement->data2,
                 enregistrement->data3) == 4;
}

bool ecrireEnregistrement(char *buffer, size_t size, EnregistrementPhysique *enregistrement) {
    if (!buffer || !enregistrement || !enregistrementValide(enregistrement)) return false;
    
    snprintf(buffer, size, "%d|%s|%s|%s",
            enregistrement->entete.id,
            enregistrement->data1,
            enregistrement->data2,
            enregistrement->data3);
}