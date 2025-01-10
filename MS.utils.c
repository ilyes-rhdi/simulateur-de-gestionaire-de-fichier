#include "mylib.h"

Virtualdisk* InitialiseMs(int nbloc) {
    // Allouer la mémoire pour la ms
    Virtualdisk* I = malloc(sizeof(Virtualdisk));
    if (I == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour Virtualdisk.\n");
        return NULL;
    }

    I->nb_blocs = nbloc;
    I->nb=0;
    I->table_fichiers = malloc(nbloc*sizeof(Fichier));
    if (I->table_fichiers == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour la table d'allocation des fichier .\n");
        free(I);
        return NULL;
    }
    for (int i = 0; i < nbloc; i++) {
    I->table_fichiers[i].nomFichier[0] = '\0';
    I->table_fichiers[i].mode = Contigue;
    I->table_fichiers[i].sort = NoTrie;
    I->table_fichiers[i].nbBlocs = 0;
    I->table_fichiers[i].max_bloc = 0;
    I->table_fichiers[i].entete.nbEnregistrements = 0;
    I->table_fichiers[i].entete.capaciteMax = 0;
    I->table_fichiers[i].entete.nextID = 0;
    }

    // Allouer la mémoire pour la table d'allocation dans le premier bloc
    bloc001* tableAllocation = malloc(nbloc * sizeof(bloc001));
    if (tableAllocation == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour la table d'allocation.\n");
        free(I);
        return NULL;
    }

    // Initialiser la table d'allocation à -1 (tous les blocs sont libres au début)
    for (int i = 0; i < nbloc; i++) {
        tableAllocation[i].isfull = 0; // 0 = Libre
        tableAllocation[i].adresbloc = NULL;
    }

    Bloc* newBloc = malloc(sizeof(Bloc));
    if (newBloc == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour le premier bloc.\n");
        free(tableAllocation);
        free(I);
        return NULL;
    }

    newBloc->enregistrements = (void*)tableAllocation; // Premier bloc contient la table d'allocation
    newBloc->taille = 0;  // Pas encore d'enregistrements
    newBloc->numBloc = 0;
    newBloc->estComplet = true;
    newBloc->next = NULL;

    // Le premier bloc devient la tête de la liste chaînée
    I->bloc = newBloc;

    return I;
}

void VidezMS(Virtualdisk* ms) {
    if (ms == NULL) {
        printf("Memoire secondaire déjà vide\n");
        return;
    }

    bloc001* tableAllocation = (bloc001*)ms->bloc->enregistrements;
    if (tableAllocation == NULL) {
        printf("Erreur : Table d'allocation introuvable.\n");
        return;
    }

    // Vider la table d'allocation
    ModifierTableAllocation(ms, -1);

    // Libérer chaque bloc
    for (int i = 1; i < ms->nb_blocs; i++) {
        if (tableAllocation[i].adresbloc != NULL) {
            free(tableAllocation[i].adresbloc);
            tableAllocation[i].adresbloc = NULL;
        }
    }

    printf("Mémoire secondaire vidée avec succès.\n");
}

void ModifierTableAllocation(Virtualdisk* ms, int indexBloc) {
    if (ms == NULL) {
        printf("Erreur : Le disque virtuel est NULL.\n");
        return;
    }

    bloc001* tableAllocation = (bloc001*)ms->bloc->enregistrements;
    if (tableAllocation == NULL) {
        printf("Erreur : Table d'allocation introuvable.\n");
        return;
    }

    if (indexBloc == -5) {
        // Vider toute la table d'allocation
        for (int i = 0; i < ms->nb_blocs; i++) {
            tableAllocation[i].isfull = 0;
            tableAllocation[i].adresbloc = NULL;
        }
        printf("Table d'allocation videe.\n");
    } else if (indexBloc >= 0 && indexBloc < ms->nb_blocs) {
        // Modifier un bloc spécifique
        if (tableAllocation[indexBloc].adresbloc != NULL) {
            tableAllocation[indexBloc].isfull = tableAllocation[indexBloc].adresbloc->estComplet ? 1 : 0;
        } else {
            tableAllocation[indexBloc].isfull = 0;
        }

        printf("Table d'allocation modifiée. Bloc %d est maintenant %d.\n", indexBloc, tableAllocation[indexBloc].isfull);
    } else {
        printf("Erreur : Index du bloc invalide.\n");
    }
}

void AjouterBloc(Virtualdisk* ms, Fichier* fichier) {
    if (ms == NULL || fichier == NULL) {
        printf("Erreur : Paramètres invalides.\n");
        return;
    }
    if (fichier->nbBlocs>=fichier->max_bloc)
    {
        printf("Fichier %s est complet\n",fichier->nomFichier);
        return;
    }
    
    if (fichier->mode == Chainee)
    {
    Bloc* dernier = fichier->blocs;
    while (dernier != NULL && dernier->next != NULL) {
        dernier = dernier->next;
    }

    Bloc* newBloc = malloc(sizeof(Bloc));
    if (newBloc == NULL) {
        printf("Erreur : Allocation mémoire échouée pour le nouveau bloc.\n");
        return;
    }

    newBloc->enregistrements = malloc((TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT + 1 ) * sizeof(EnregistrementPhysique));
    if (newBloc->enregistrements == NULL) {
        printf("Erreur : Allocation mémoire échouée pour les enregistrements.\n");
        free(newBloc);
        return;
    }
    ms->nb_blocs++;
    newBloc->taille = 0;
    newBloc->numBloc = ms->nb;
    newBloc->estComplet = false;
    newBloc->next = NULL;

    if (dernier == NULL) {
        fichier->blocs = newBloc;
    } else {
        dernier->next = newBloc;
    }

    fichier->nbBlocs++;
    }
}

Bloc* trouverBlocAvecEspace(Fichier* fichier) {
    if (fichier == NULL) {
        return NULL;
    }

    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs; i++) {
            if (fichier->blocs[i].taille < (TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT) && !fichier->blocs[i].estComplet) {
                return &fichier->blocs[i];
            }
        }
    } else {
        Bloc* current = fichier->blocs;
        while (current != NULL) {
            if (current->taille < (TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT) && !current->estComplet) {
                return current;
            }
            current = current->next;
        }
    }

    return NULL;
}

bool libererBloc(Fichier* fichier, Bloc* blocDirect,Virtualdisk *ms) {
    if (fichier == NULL || blocDirect == NULL) {
        return false;
    }

    if (fichier->mode == Contigue) {
        if (blocDirect->numBloc < 0 || blocDirect->numBloc >= fichier->nbBlocs) {
            return false;
        }

        for (int i = blocDirect->numBloc; i < fichier->nbBlocs - 1; i++) {
            fichier->blocs[i] = fichier->blocs[i + 1];
        }

        fichier->nbBlocs--;
        return true;
    } else if (fichier->mode == Chainee) {
        Bloc* prev = NULL;
        Bloc* current = fichier->blocs;

        while (current != NULL) {
            if (current == blocDirect) {
                if (prev != NULL) {
                    prev->next = current->next;
                } else {
                    fichier->blocs = current->next;
                }

                free(current->enregistrements);
                free(current);
                fichier->nbBlocs--;
                return true;
            }
            prev = current;
            current = current->next;
        }
    } 
    ms->nb_blocs--;
    return false;
}

bool LireBloc(BufferTransmission *Buffer, Bloc *bloc) {
    if (bloc == NULL) {
        printf("Erreur : Bloc vide.\n");
        return false;
    }
    // Appeler remplirBuffer pour copier les données
    remplirBuffer(Buffer, bloc);

    // Si remplirBuffer a une logique d'échec, on peut gérer ici.
    if (Buffer->B == NULL) {
        printf("Erreur : Impossible de remplir le buffer.\n");
        return false;
    }

    return true;
}
void EcrireBloc(BufferTransmission*Buffer,Bloc * bloc){
    if (!bloc || !Buffer)
    {
        printf("bloc ou buffer vide");
        return;
    }
    bloc=Buffer->B ;
    viderBuffer(Buffer);
    
}
void afficherBloc(BufferTransmission *Buffer, Bloc *bloc) {
    if (bloc == NULL || Buffer == NULL) {
        printf("Erreur : Bloc non valide.\n");
        return;
    }

    // Lire le bloc dans le buffer
    if (!LireBloc(Buffer, bloc)) {
        printf("Erreur : Échec de la lecture du bloc.\n");
        return;
    }

    if (Buffer->B == NULL) {
        printf("Erreur : Données du bloc non disponibles dans le buffer.\n");
        return;
    }

    // Afficher les métadonnées du bloc
    printf("\nInformations du bloc:\n");
    printf("Numéro du bloc: %d\n", Buffer->B->numBloc);
    printf("Nombre d'enregistrements: %d\n", Buffer->B->taille);
    printf("Est complet: %s\n", Buffer->B->estComplet ? "Oui" : "Non");

    // Parcourir et afficher les enregistrements du bloc
    printf("\nDétails des enregistrements :\n");
    const int TAILLE_MAX_BUFFER = 256;
    char bufferTemp[TAILLE_MAX_BUFFER];

    for (int i = 0; i < Buffer->B->taille; i++) {
        EnregistrementPhysique *enr = &Buffer->B->enregistrements[i];

        if (ecrireEnregistrement(bufferTemp,TAILLE_MAX_BUFFER,enr)) {
            printf("\nEnregistrement %d:\n", i + 1);
            printf("ID: %d\n", enr->entete.id);
            printf("Data1: %s\n", enr->data1);
            printf("Data2: %s\n", enr->data2);
            printf("Data3: %s\n", enr->data3);
        } else {
            printf("Erreur : Impossible de lire l'enregistrement %d.\n", i + 1);
        }
    }
}
