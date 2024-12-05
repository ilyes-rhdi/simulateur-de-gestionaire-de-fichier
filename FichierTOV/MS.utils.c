#include "tov.h"

Virtualdisk* InitialiseMs(int nbloc, ModeOrganisationF mode) {
    // Allouer la mémoire pour la structure Virtualdisk
    Virtualdisk* I = malloc(sizeof(Virtualdisk));
    if (I == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour Virtualdisk.\n");
        return NULL;
    }

    I->nb_blocs = nbloc;
    I->mode = mode;

    // Allouer la mémoire pour la table d'allocation dans le premier bloc
    int* tableAllocation = malloc(nbloc * sizeof(int));
    if (tableAllocation == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour la table d'allocation.\n");
        free(I);
        return NULL;
    }

    // Initialiser la table d'allocation à -1 (tous les blocs sont libres au début et pas allouer )
    for (int i = 1; i < nbloc; i++) {
        tableAllocation[i] = -1; // 0 = Libre , -1 = pas encore allouer 
    }

    if (mode == Contigue) {
        // Mode contigu : créer un tableau de blocs
        I->bloc = malloc(sizeof(Bloc));
        if (I->bloc == NULL) {
            fprintf(stderr, "Erreur : Allocation mémoire échouée pour le tableau de blocs.\n");
            free(tableAllocation);
            free(I);
            return NULL;
        }

        // Initialiser le premier bloc avec la table d'allocation
        I->bloc[0].data = (void*)tableAllocation; // Premier bloc contient la table d'allocation
        I->bloc[0].taille = nbloc * sizeof(int);  // Taille de la table d'allocation
        I->bloc[0].numBloc = 0;
        I->bloc[0].estComplet = true;
        I->bloc[0].next = NULL; // Non utilisé en mode contigu
    } else if (mode == Chainee) {
        // Mode chaîné : créer une liste chaînée de blocs
        I->bloc = NULL; // Pas de tableau, seulement un pointeur vers le premier bloc
        Bloc* precedent = NULL;
        Bloc* head = NULL;

        // Créer le premier bloc avec la table d'allocation
        Bloc* newBloc = malloc(sizeof(Bloc));
        if (newBloc == NULL) {
            fprintf(stderr, "Erreur : Allocation mémoire échouée pour le premier bloc.\n");
            free(tableAllocation);
            free(I);
            return NULL;
        }

        newBloc->data = (void*)tableAllocation; // Premier bloc contient la table d'allocation
        newBloc->taille = nbloc * sizeof(int);  // Taille de la table d'allocation
        newBloc->numBloc = 0;
        newBloc->estComplet = true;
        newBloc->next = NULL;

        // Le premier bloc devient la tête de la liste chaînée
        head = newBloc;
        I->bloc = head;

        precedent = newBloc;

    } else {
        fprintf(stderr, "Erreur : Mode d'organisation non valide.\n");
        free(tableAllocation);
        free(I);
        return NULL;
    }

    return I;
}

void VidezMS(Virtualdisk* ms) {
    if (ms == NULL) {
        printf("Memoire secondaire déjà vide\n");
    } else if (ms->mode == Contigue) {
        // Mode contigu : vider tous les blocs dans le tableau
        ModifierTableAllocation(ms,-1,0)
        for (int i = 1; i < ms->nb_blocs; i++) {
            ms->bloc[i].taille = 0;
            ms->bloc[i].numBloc = i;
            ms->bloc[i].estComplet = false; // Par défaut, le bloc est vide
            ms->bloc[i].next = NULL;        // Non utilisé en mode contigu
        }
    } else if (ms->mode == Chainee) {
        // Mode chaîné : vider tous les blocs dans la liste chaînée
        // en initialise  toute les valuers la table d'allocation a 0
        ModifierTableAllocation(ms,-1,0)
        
        Bloc* currentBloc = ms->bloc;
        while (currentBloc != NULL) {
            currentBloc->taille = 0;
            currentBloc->estComplet = false;
            currentBloc = currentBloc->next; // Passer au bloc suivant
        }
    } else {
        printf("Erreur : Mode d'organisation non valide\n");
    }
}
void ModifierTableAllocation(Virtualdisk* ms, int indexBloc, int nouvelleValeur) {
    if (ms == NULL) {
        printf("Erreur : Le disque virtuel est NULL.\n");
        return;
    }

    // Vérifier si l'index est valide
    if (indexBloc < -1 || indexBloc >= ms->nb_blocs) {
        printf("Erreur : Index du bloc invalide.\n");
        return;
    }

    // Accéder à la table d'allocation dans le premier bloc
    if (ms->mode==Contigue)
    {
        int* tableAllocation = (int*)ms->bloc[0].data;
    }else{
        int* tableAllocation = (int*)ms->bloc->data;
    }
    if (indexBloc == -1)
    {
        //en cas ou il faut vider toute la memoir secondaire
        for (int i = 0; i < ms->nb_blocs -1; i++)
        {
            tableAllocation[i]=nouvelleValeur;
        }
        
    }else
    {
       // Modifier la valeur de la table d'allocation pour l'index spécifié
       tableAllocation[indexBloc] = nouvelleValeur;
    }
    

    // Afficher l'état de la table d'allocation après modification
    printf("Table d'allocation modifiée. Bloc %d est maintenant %d.\n", indexBloc, nouvelleValeur);
}
void AjouterBloc(Virtualdisk* ms,FichierTOV *FichierTOV) {
    if (ms == NULL) {
        printf("Erreur : Mémoire secondaire non initialisée.\n");
        return;
    }

    if (ms->mode == Contigue) {
        // Accéder à la table d'allocation
        int* tableAllocation = (int*)ms->bloc[0].data;

        // Trouver ou je peut ajouter
        int j = 0;
        while (j < ms->nb_blocs && tableAllocation[j] != -1) {
            j++;
        }

        if (j == ms->nb_blocs) {
            printf("Impossible d'ajouter un bloc : mémoire pleine.\n");
            return;
        }

        // Agrandir le tableau de blocs
        Bloc* newBlocs = malloc((j + 1) * sizeof(Bloc));
        if (newBlocs == NULL) {
            printf("Erreur : Allocation mémoire échouée.\n");
            return;
        }

        // Copier les anciens blocs dans le nouveau tableau
        for (int i = 0; i < j; i++) {
            newBlocs[i] = ms->bloc[i];
        }

        // Libérer l'ancien tableau
        free(ms->bloc);

        // Initialiser le nouveau bloc
        newBlocs[j].data = malloc(TAILLE_MAX_BLOC);
        if (newBlocs[j].data == NULL) {
            printf("Erreur : Allocation mémoire échouée pour le nouveau bloc.\n");
            free(newBlocs);
            return;
        }
        newBlocs[j].taille = 0;
        newBlocs[j].numBloc = j;
        newBlocs[j].estComplet = false;
        newBlocs[j].next = NULL;

        // Mettre à jour la table d'allocation
        tableAllocation[j] = 1; // Marquer comme utilisé

        // Mettre à jour la référence du tableau de blocs
        ms->bloc = newBlocs;
        i=0;
        while (FichierTOV->nbBlocs -1 >i && FichierTOV->blocs[i]!= NULL)
        {
              i+=1;
        }
        if (i == FichierTOV->nbBlocs - 1 )
        {
            printf("fichier plein");
            return;
        }else{
            FichierTOV->blocs[i]=&newBlocs[j];
            FichierTOV->nbBlocs++;
        }
    } else {  // Mode Chaînée
        // Parcourir la liste pour trouver le dernier bloc
        Bloc* head = ms->bloc;
        Bloc* prec = NULL;

        while (head != NULL) {
            prec = head;
            head = head->next;
        }

        // Allouer et initialiser un nouveau bloc
        Bloc* newBloc = malloc(sizeof(Bloc));
        if (newBloc == NULL) {
            printf("Erreur : Allocation mémoire échouée pour le nouveau bloc.\n");
            return;
        }

        newBloc->data = malloc(TAILLE_MAX_BLOC);
        if (newBloc->data == NULL) {
            printf("Erreur : Allocation mémoire échouée pour les données du bloc.\n");
            free(newBloc);
            return;
        }

        newBloc->taille = 0;
        newBloc->numBloc = (prec == NULL) ? 0 : prec->numBloc + 1;
        newBloc->estComplet = false;
        newBloc->next = NULL;

        // Ajouter le nouveau bloc à la liste
        if (prec == NULL) {
            ms->bloc = newBloc;  // La liste était vide
        } else {
            prec->next = newBloc;
        }
                i=0;
        while (FichierTOV->nbBlocs -1 >i && FichierTOV->blocs[i]!= NULL)
        {
              i+=1;
        }
        if (i == FichierTOV->nbBlocs - 1 )
        {
            printf("fichier plein");
            return;
        }else{
            FichierTOV->blocs[i]=newBlocs;
            FichierTOV->nbBlocs++;
        }
    }
}
Bloc *trouverBlocAvecEspace(FichierTOV* fichier){
    if (fichier == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < fichier->nbBlocs - 1; i++)
    {
        if (fichier->blocs[i]->taille < TAILLE_MAX_BLOC/TAILLE_MAX_ENREGISTREMENT && !fichier->blocs[i]->estComplet)
        {
            return fichier->blocs[i]; 
        }
            
    }
    return NULL;
    
}

