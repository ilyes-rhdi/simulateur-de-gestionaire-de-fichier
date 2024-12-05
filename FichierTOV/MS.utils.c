#include "tov.h"

Virtualdisk* InitialiseMs(int nbloc) {
    // Allouer la mémoire pour la structure Virtualdisk
    Virtualdisk* I = malloc(sizeof(Virtualdisk));
    if (I == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour Virtualdisk.\n");
        return NULL;
    }

    I->nb_blocs = nbloc;

    // Allouer la mémoire pour la table d'allocation dans le premier bloc
    bloc001* tableAllocation = malloc(nbloc * sizeof(bloc001));
    if (tableAllocation == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour la table d'allocation.\n");
        free(I);
        return NULL;
    }

    // Initialiser la table d'allocation à -1 (tous les blocs sont libres au début et pas allouer )
    for (int i = 1; i < nbloc; i++) {
        tableAllocation[i].isfull = 0; // 0 = Libre
        tableAllocation[i].adresbloc=NULL;
    }
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
        
        I->bloc = newBloc;

    return I;
}

void VidezMS(Virtualdisk* ms) {
    if (ms == NULL) {
        printf("Memoire secondaire déjà vide\n");
    } else{
        bloc001* tableAllocation = (bloc001*)ms->bloc->data;
        // Mode contigu : vider tous les blocs dans le tableau
        ModifierTableAllocation(ms,-1,0);
        for (int i = 1; i < ms->nb_blocs; i++) {
            tableAllocation[i].adresbloc->taille = 0;
            tableAllocation[i].adresbloc->numBloc = i;
            tableAllocation[i].adresbloc->estComplet = false; // Par défaut, le bloc est vide
            tableAllocation[i].adresbloc->next = NULL;        // Non utilisé en mode contigu
        }
    } 
 
}

void ModifierTableAllocation(Virtualdisk* ms, int indexBloc) {
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
   
        bloc001* tableAllocation = (bloc001*)ms->bloc->data;
    if (indexBloc == -1)
    {
        //en cas ou il faut vider toute la memoir secondaire
        for (int i = 0; i < ms->nb_blocs -1; i++)
        {
            tableAllocation[i].isfull=0;
            tableAllocation[i].adresbloc=NULL;
        } 
    }else
    {
       // Modifier la valeur de la table d'allocation pour l'index spécifié
       if (tableAllocation[indexBloc].adresbloc->estComplet)
       {
            tableAllocation[indexBloc].isfull = 1;
       }else{
             tableAllocation[indexBloc].isfull = 0;
       }
       
    }
    

    // Afficher l'état de la table d'allocation après modification
    printf("Table d'allocation modifiée. Bloc %d est maintenant %d.\n", indexBloc, nouvelleValeur);
}
void AjouterBloc(Virtualdisk* ms,FichierTOV *FichierTOV) {
    if (ms == NULL) {
        printf("Erreur : Mémoire secondaire non initialisée.\n");
        return;
    }

    if (FichierTOV->mode == Contigue) {
        // Accéder à la table d'allocation
        bloc001* tableAllocation = (bloc001*)ms->bloc[0].data;

        // Trouver ou je peut ajouter
        int j = 0;
        while (j < ms->nb_blocs && tableAllocation[j].adresbloc != NULL) {
            j++;
        }

        if (j == FichierTOV->nbBlocs) {
            printf("Impossible d'ajouter un bloc : fichier pleine.\n");
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
            newBlocs[i] = FichierTOV->blocs[i];
        }

        // Libérer l'ancien tableau
        free(FichierTOV->blocs);

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
        tableAllocation[j].isfull = 1;
        tableAllocation[j].adresbloc = &newBlocs[j]; // Marquer comme utilisé

        // Mettre à jour la référence du tableau de blocs
        FichierTOV->blocs = newBlocs;
    } else {  // Mode Chaînée
        // Parcourir la liste pour trouver le dernier bloc
        Bloc* head = FichierTOV->blocs;
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
            FichierTOV->blocs = newBloc;  // La liste était vide
        } else {
            prec->next = newBloc;
        }
    }
}
Bloc *trouverBlocAvecEspace(FichierTOV* fichier){
    if (fichier == NULL) {
        return NULL;
    }
    if (fichier->mode == Contigue)
    {
        for (int i = 0; i < fichier->nbBlocs - 1; i++)
    {
        if (fichier->blocs->taille < TAILLE_MAX_BLOC/TAILLE_MAX_ENREGISTREMENT && !fichier->blocs[i]->estComplet)
        {
            return fichier->blocs[i]; 
        }
            
    }
    }
    
 
    return NULL;
    
}

