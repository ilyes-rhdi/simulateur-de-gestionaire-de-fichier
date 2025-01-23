#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 
void enregistrerFichier(int capaciteMax,Virtualdisk *ms,const char *nom, ModeOrganisationF sort, ModeOrganisationE mode){
    FILE *Fich;
    char chemin_initial[256];
    getcwd(chemin_initial, sizeof(chemin_initial));
    
    // Changer le répertoire courant vers "ms"
    if (chdir("MS") != 0) {
        perror("Impossible de changer de répertoire");
        return NULL;
    }else{
        FILE *fichier = fopen(nom, "r");

        if (fichier) {
            printf("Le fichier '%s' existe déjà.\n", nom);
            Fichier *f = initialiserFichier(capaciteMax,ms,nom,sort,mode);
            bool swapp=true;
            char buf[256];
            EnregistrementPhysique  enr;
            strcpy(enr.data1, "");
            strcpy(enr.data2, "");
            strcpy(enr.data3, "");
            while (swapp)
            {
                fscanf(fichier,buf);
                if (!lireEnregistrement(&enr,buf))
                {
                    swapp=false;
                }  
            }

            fclose(fichier); // Toujours fermer le fichier après ouverture
        } else {
        // Créer et ouvrir le fichier (dans "ms" car le répertoire courant a changé)
        Fich = fopen(nom, "w");
        if (Fich == NULL) {
            perror("Erreur lors de la création du fichier");
            return NULL;
        }
        chdir(chemin_initial);
        }
    }
}

// Initialisation du fichierTOV
Fichier *initialiserFichier(int capaciteMax,Virtualdisk *ms,const char *nom, ModeOrganisationF sort, ModeOrganisationE mode) {
    Fichier *fichier = malloc(sizeof(Fichier));
    if (fichier == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour le fichier.\n");
        return NULL;
    }
    
    // Initialisation des champs
    strncpy(fichier->nomFichier, nom, sizeof(fichier->nomFichier) - 1);
    fichier->nomFichier[sizeof(fichier->nomFichier) - 1] = '\0';
    fichier->mode = mode;
    fichier->sort = sort;
    fichier->nbBlocs = 0;
    fichier->max_bloc = capaciteMax;
    fichier->entete.nbEnregistrements = 0;
    fichier->entete.capaciteMax = (capaciteMax* MAX_ENREGISTREMENTS_PAR_BLOC);
    fichier->entete.nextID = 0;

    // Initialisation des blocs selon le mode
    if (mode == Contigue) {
        if (ms->nb+fichier->max_bloc > ms->nb_blocs)
        {
            printf("memoir secondaire plein le fichier ne peut pas etre cree avec cette taille \n");
            free(fichier);
            return NULL;
        }
        
        fichier->blocs = calloc(fichier->max_bloc, sizeof(Bloc));
        if (fichier->blocs == NULL) {
            fprintf(stderr, "Erreur : Allocation mémoire échouée pour les blocs contigus.\n");
            free(fichier);
            return NULL;
        }
        fichier->nbBlocs=fichier->max_bloc;
        ms->nb+=fichier->nbBlocs;
        for (int  i = 0; i < fichier->nbBlocs; i++)
        {
            fichier->blocs[i].enregistrements= malloc((TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT) * sizeof(EnregistrementPhysique));
            if (fichier->blocs[i].enregistrements == NULL)
            {
                printf("Erreur : Allocation mémoire échouée pour les enregistrements.\n");
                free(fichier->blocs[i].enregistrements);
                return NULL;
            }
            fichier->blocs[i].estComplet= false;
            fichier->blocs[i].next=NULL;
            fichier->blocs[i].taille = 0;
            fichier->blocs[i].numBloc = ms->nb + i - fichier->nbBlocs;
            
            ModifierTableAllocation(ms,fichier->blocs[i].numBloc);
        }      
        
    } else if (mode == Chainee) {
        int espaceDisponible = ms->nb_blocs - ms->nb;
        int blocsAlloues;
       if (fichier->max_bloc > espaceDisponible) {
        // Pas assez d'espace pour la taille demandée
            if (espaceDisponible <= 0) {
                printf("Mémoire secondaire pleine, le fichier ne peut pas être créé.\n");
                return NULL; // Ou une autre valeur d'erreur appropriée
            } else {
            // On peut créer le fichier mais avec moins de blocs que demandé
                printf("Le fichier %s va être initialisé avec seulement %d blocs au lieu de %d.\n", 
                fichier->nomFichier, espaceDisponible, fichier->max_bloc);
                blocsAlloues = espaceDisponible;
            }
        } else {
        // On a assez d'espace pour la taille demandée
                blocsAlloues = fichier->max_bloc;
        }
        ms->nb += blocsAlloues;
        fichier->max_bloc = blocsAlloues;
        fichier->blocs = NULL; // Les blocs seront alloués dynamiquement
    }
    int i =0;
    while (*ms->table_fichiers[i].nomFichier!='\0')
    {
        i++;
    }
    strncpy(ms->table_fichiers[i].nomFichier, nom, sizeof(ms->table_fichiers[i].nomFichier) - 1);
    ms->table_fichiers[i].nomFichier[sizeof(fichier->nomFichier) - 1] = '\0';
    ms->table_fichiers[i].mode = mode;
    ms->table_fichiers[i].sort = sort;
    ms->table_fichiers[i].nbBlocs = 0;
    ms->table_fichiers[i].max_bloc = capaciteMax;
    ms->table_fichiers[i].entete.nbEnregistrements = 0;
    ms->table_fichiers[i].entete.capaciteMax = capaciteMax * MAX_ENREGISTREMENTS_PAR_BLOC;
    ms->table_fichiers[i].entete.nextID = 0;
    ms->table_fichiers[i].blocs=NULL;

    return fichier;
}

// Libération de la mémoire du fichier TOV
void libererFichier(Fichier *fichier) {
    if (fichier == NULL) {
        printf("libererFichier: fichier est NULL\n");
        return;
    }

    // Suppression du fichier physique (non nécessaire dans ce cas, c'est une simulation)
    if (access(fichier->nomFichier, F_OK) == 0) {
        if (remove(fichier->nomFichier) != 0) {
            fprintf(stderr, "Erreur : Échec de suppression du fichier %s.\n", fichier->nomFichier);
        }
    }

    // Libération mémoire selon le mode
    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs; i++) {
            free(fichier->blocs[i].enregistrements);
        }
        free(fichier->blocs);
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        while (current != NULL) {
            Bloc *next = current->next;
            free(current->enregistrements); // Libération des données du bloc
            free(current);       // Libération du bloc
            current = next;
        }
    }

    free(fichier); // Libération de la structure FichierTOV elle-même
    printf("libererFichier: mémoire libérée et fichier supprimé\n");
}

// Affichage du contenu du fichier
void afficherFichier(Fichier *fichier) {
    if (!fichier) {
        printf("afficherFichier: fichier est NULL\n");
        return;
    }
    BufferTransmission*Buffer = calloc(1,sizeof(BufferTransmission));
    char buffer[256];
    printf("Fichier contient %d enregistrements:\n", fichier->entete.nbEnregistrements);

    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs; i++) {
            printf("Bloc %d:\n", i);
            afficherBloc(Buffer,&fichier->blocs[i]);
        }
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        int index = 0;
        while (current) {
            printf("Bloc %d:\n", index++);
            afficherBloc(Buffer,current);
            current = current->next;
        }
    }
}

// Defragmentation du fichier 
bool Defragmentation(Fichier* fichier) {
    if (fichier == NULL) {
        printf("Compactage : fichier est NULL\n");
        return false;
    }

    // Allocation d'un tableau temporaire pour les enregistrements compactés
    EnregistrementPhysique* enregistrementsCompactes = malloc(fichier->entete.capaciteMax * sizeof(EnregistrementPhysique));
    if (enregistrementsCompactes == NULL) {
        printf("Compactage : échec d'allocation pour les enregistrements compactés\n");
        return false;
    }

    int nouveauxNbEnregistrements = 0;

    // Parcourir les blocs existants et copier les enregistrements valides dans le tableau compacté
    for (int i = 0; i < fichier->nbBlocs; i++) {
        Bloc* bloc = &fichier->blocs[i];
        for (int j = 0; j < bloc->taille; j++) {
            EnregistrementPhysique* enregistrement = &bloc->enregistrements[j];
            if (enregistrementValide(enregistrement)) {
                enregistrementsCompactes[nouveauxNbEnregistrements++] = *enregistrement;
            }
        }
    }

    // Réorganiser les blocs avec les enregistrements compactés
    int indexCompacte = 0;
    for (int i = 0; i < fichier->nbBlocs; i++) {
        Bloc* bloc = &fichier->blocs[i];
        bloc->taille = 0;  // Réinitialisation du bloc avant de réécrire les enregistrements

        // Ajouter des enregistrements au bloc tant qu'il y a de la place
        while (bloc->taille < TAILLE_MAX_BLOC && indexCompacte < nouveauxNbEnregistrements) {
            bloc->enregistrements[bloc->taille++] = enregistrementsCompactes[indexCompacte++];
        }
    }

    // Mise à jour du nombre d'enregistrements dans l'entête
    fichier->entete.nbEnregistrements = nouveauxNbEnregistrements;

    // Libération du tableau temporaire
    free(enregistrementsCompactes);

    return true;
}
bool Compactage(Virtualdisk* ms) {
    if (!ms) {
        printf("Compactage : disque virtuel est NULL\n");
        return false;
    }

    // Récupérer la table d'allocation depuis le premier bloc
    bloc001* tableAllocation = (bloc001*)ms->bloc->enregistrements;
    
    // D'abord, défragmenter chaque fichier
    for (int i = 0; i < ms->nb; i++) {
        Defragmentation(&ms->table_fichiers[i]);
    }

    // Compteur pour la nouvelle position des blocs
    int nouvellePosition = 1;  // On commence à 1 car le bloc 0 est réservé pour la table d'allocation

    // Parcourir tous les fichiers pour les juxtaposer
    for (int i = 0; i < ms->nb; i++) {
        Fichier* fichierCourant = &ms->table_fichiers[i];
        
        if (fichierCourant->mode == Contigue) {
            for (int j = 0; j < fichierCourant->nbBlocs; j++) {
                // Si le bloc n'est pas déjà à la bonne position
                if (nouvellePosition != fichierCourant->blocs[j].numBloc) {
                    // Déplacer le bloc dans la table d'allocation
                    tableAllocation[nouvellePosition].adresbloc = tableAllocation[fichierCourant->blocs[j].numBloc].adresbloc;
                    tableAllocation[nouvellePosition].isfull = tableAllocation[fichierCourant->blocs[j].numBloc].isfull;
                    
                    // Libérer l'ancienne position
                    tableAllocation[fichierCourant->blocs[j].numBloc].adresbloc = NULL;
                    tableAllocation[fichierCourant->blocs[j].numBloc].isfull = 0;
                    
                    // Mettre à jour le numéro de bloc dans le fichier
                    fichierCourant->blocs[j].numBloc = nouvellePosition;
                }
                nouvellePosition++;
            }
        } 
        else if (fichierCourant->mode == Chainee) {
            Bloc* blocCourant = fichierCourant->blocs;
            while (blocCourant != NULL) {
                if (nouvellePosition != blocCourant->numBloc) {
                    // Déplacer le bloc dans la table d'allocation
                    tableAllocation[nouvellePosition].adresbloc = tableAllocation[blocCourant->numBloc].adresbloc;
                    tableAllocation[nouvellePosition].isfull = tableAllocation[blocCourant->numBloc].isfull;
                    
                    // Libérer l'ancienne position
                    tableAllocation[blocCourant->numBloc].adresbloc = NULL;
                    tableAllocation[blocCourant->numBloc].isfull = 0;
                    
                    // Mettre à jour le numéro de bloc
                    blocCourant->numBloc = nouvellePosition;
                }
                nouvellePosition++;
                blocCourant = blocCourant->next;
            }
        }
    }

    // Mise à jour du nombre de blocs utilisés
    ms->nb = nouvellePosition;
    return true;
}
Fichier* ouvrirFichier(const char* nom, Virtualdisk *ms) {
    if (!nom || !ms) {
        fprintf(stderr, "Erreur : Nom de fichier ou disque virtuel invalide.\n");
        return NULL;
    }

    // Allouer la structure Fichier
    Fichier* fichier = malloc(sizeof(Fichier));
    if (!fichier) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour le fichier.\n");
        return NULL;
    }

    // Chercher dans la table d'allocation du disque virtuel
    if (!fichierExisteDansMS(ms, nom)) {
        fprintf(stderr, "Erreur : Le fichier %s n'existe pas dans le disque virtuel.\n", nom);
        free(fichier);
        return NULL;
    }

    // Récupérer les informations du fichier depuis le disque virtuel
    RecupererInfoFichier(ms, nom, fichier);

    // Allocation et récupération des blocs selon le mode
    if (fichier->mode == Contigue) {
        fichier->blocs = calloc(fichier->max_bloc, sizeof(Bloc));
        if (!fichier->blocs) {
            fprintf(stderr, "Erreur : Allocation mémoire échouée pour les blocs.\n");
            free(fichier);
            return NULL;
        }

        // Récupérer les blocs depuis le disque virtuel
        for (int i = 0; i < fichier->nbBlocs; i++) {
            fichier->blocs[i].enregistrements = malloc((TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT) * sizeof(EnregistrementPhysique));
            if (!fichier->blocs[i].enregistrements) {
                // Nettoyer la mémoire
                for (int j = 0; j < i; j++) {
                    free(fichier->blocs[j].enregistrements);
                }
                free(fichier->blocs);
                free(fichier);
                return NULL;
            }

            // Lire le bloc depuis le disque virtuel
            LireBlocDepuisMS(ms, fichier->blocs[i].numBloc, &fichier->blocs[i]);
        }
    } else if (fichier->mode == Chainee) {
        // Pour le mode chaîné
        Bloc* dernierBloc = NULL;
        int numBlocCourant = fichier->blocs->numBloc;

        while (numBlocCourant != -1) {
            Bloc* nouveauBloc = malloc(sizeof(Bloc));
            if (!nouveauBloc) {
                libererFichier(fichier);
                return NULL;
            }

            nouveauBloc->enregistrements = malloc((TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT) * sizeof(EnregistrementPhysique));
            if (!nouveauBloc->enregistrements) {
                free(nouveauBloc);
                libererFichier(fichier);
                return NULL;
            }

            // Lire le bloc depuis le disque virtuel
            LireBlocDepuisMS(ms, numBlocCourant, nouveauBloc);

            // Chaîner le nouveau bloc
            nouveauBloc->next = NULL;
            if (dernierBloc) {
                dernierBloc->next = nouveauBloc;
            } else {
                fichier->blocs = nouveauBloc;
            }
            dernierBloc = nouveauBloc;
            Bloc *L =fichier->blocs;
            // Récupérer le numéro du prochain bloc

            numBlocCourant = L->next->numBloc ;
            L=L->next;
        }
    }

    return fichier;
}
bool fichierExisteDansMS(Virtualdisk *ms, const char* nom) {
    if (!ms || !nom) return false;
    
    // Parcourir la table des fichiers du disque virtuel
    for (int i = 0; i < ms->nb_blocs; i++) {
        if (strcmp(ms->table_fichiers[i].nomFichier, nom) == 0) {
            return true;
        }
    }
    return false;
}
void RecupererInfoFichier(Virtualdisk *ms, const char* nom, Fichier* fichier) {
    if (!ms || !nom || !fichier) return;
    
    // Trouver le fichier dans la table des fichiers
    for (int i = 0; i < ms->nb_blocs; i++) {
        if (strcmp(ms->table_fichiers[i].nomFichier, nom) == 0) {
            // Copier les informations de base
            strncpy(fichier->nomFichier, nom, sizeof(fichier->nomFichier) - 1);
            fichier->mode = ms->table_fichiers[i].mode;
            fichier->sort = ms->table_fichiers[i].sort;
            fichier->nbBlocs = ms->table_fichiers[i].nbBlocs;
            fichier->max_bloc = ms->table_fichiers[i].max_bloc;
            fichier->entete = ms->table_fichiers[i].entete;
            fichier->blocs=ms->table_fichiers[i].blocs;
            return;
        }
    }
}
void LireBlocDepuisMS(Virtualdisk *ms, int numBloc, Bloc *bloc) {
    if (!ms || !bloc || numBloc < 0 || numBloc >= ms->nb) return;
    bloc001* tableAllocation = (bloc001*)ms->bloc->enregistrements;
    // Vérifier si le bloc est alloué
    if (!tableAllocation[numBloc].adresbloc) {
        fprintf(stderr, "Erreur : Tentative de lecture d'un bloc non alloué (%d)\n", numBloc);
        return;
    }
    BufferTransmission *Buffer;
    LireBloc(Buffer,tableAllocation[numBloc].adresbloc);
    if (!Buffer)
    {
        printf("Erreur : Tentative de lecture du bloc (%d)  a echouer \n", numBloc);
        return;
    }
    // Copier les enregistrements
    EcrireBloc(Buffer,bloc);
    if (!bloc)
    {
        printf("Erreur : Tentative d'ecreture dans le  bloc (%d) a echouer \n", numBloc);
        return;
    }
}
void RenameFichier(const char* name,const char* Newname,Virtualdisk *ms){
    if (!name || !Newname || !ms )
    {
     printf("err d'argument");
     return;
    }
    Fichier * f = ouvrirFichier(name,ms);
    if (!f)
    {
        printf("error a l'ouverture du ficher ou fichier innexcistant  ");
        return;
    }
    strncpy(f->nomFichier,Newname,29);
}
bool fermerFichier(Fichier* fichier, Virtualdisk* ms) {
   if (!fichier || !ms) return false;
   
   // Mettre à jour les métadonnées
   for (int i = 0; i < ms->nb_blocs; i++) {
       if (strcmp(ms->table_fichiers[i].nomFichier, fichier->nomFichier) == 0) {
           strncpy(ms->table_fichiers[i].nomFichier,fichier->nomFichier,29);
            ms->table_fichiers[i].mode=fichier->mode;
            ms->table_fichiers[i].sort=fichier->sort;
            ms->table_fichiers[i].nbBlocs=fichier->nbBlocs;
            ms->table_fichiers[i].max_bloc = fichier->max_bloc ;
            ms->table_fichiers[i].entete = fichier->entete ;
            ms->table_fichiers[i].blocs = fichier->blocs;
           return true;
       }
   }
   return false; 
}