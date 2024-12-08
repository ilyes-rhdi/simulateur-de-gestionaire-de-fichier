#include "tov.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

Virtualdisk* InitialiseMs(int nbloc) {
    // Allouer la mémoire pour la MS
    Virtualdisk* I = malloc(sizeof(Virtualdisk));
    if (I == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour Virtualdisk.\n");
        return NULL;
    }

    I->nb_blocs = nbloc;

    // Allouer la mémoire pour la table d'allocation
    bloc001* tableAllocation = malloc(nbloc * sizeof(bloc001));
    if (tableAllocation == NULL) {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour la table d'allocation.\n");
        free(I);
        return NULL;
    }

    // Initialiser la table d'allocation
    for (int i = 1; i < nbloc; i++) {
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

    newBloc->data = (void*)tableAllocation; // Premier bloc contient la table d'allocation
    newBloc->taille = 1; // On suppose qu'il occupe un espace pour un enregistrement
    newBloc->numBloc = 0;
    newBloc->estComplet = true;
    newBloc->next = NULL;

    I->bloc = newBloc; // Le premier bloc devient la tête de la liste
    return I;
}

void VidezMS(Virtualdisk* ms) {
    if (ms == NULL) {
        printf("Mémoire secondaire déjà vide\n");
        return;
    }

    bloc001* tableAllocation = (bloc001*)ms->bloc->data;
    ModifierTableAllocation(ms, -1); // Réinitialiser la table d'allocation

    for (int i = 1; i < ms->nb_blocs; i++) {
        if (tableAllocation[i].adresbloc != NULL) {
            tableAllocation[i].adresbloc->taille = 0;
            tableAllocation[i].adresbloc->numBloc = i;
            tableAllocation[i].adresbloc->estComplet = false;
            tableAllocation[i].adresbloc->next = NULL;
        }
    }
}

void ModifierTableAllocation(Virtualdisk* ms, int indexBloc) {
    if (ms == NULL) {
        printf("Erreur : Le disque virtuel est NULL.\n");
        return;
    }

    if (indexBloc < -1 || indexBloc >= ms->nb_blocs) {
        printf("Erreur : Index du bloc invalide.\n");
        return;
    }

    bloc001* tableAllocation = (bloc001*)ms->bloc->data;
    if (indexBloc == -1) { // Réinitialiser la table entière
        for (int i = 0; i < ms->nb_blocs; i++) {
            tableAllocation[i].isfull = 0;
            tableAllocation[i].adresbloc = NULL;
        }
    } else { // Modifier un bloc spécifique
        if (tableAllocation[indexBloc].adresbloc && tableAllocation[indexBloc].adresbloc->estComplet) {
            tableAllocation[indexBloc].isfull = 1;
        } else {
            tableAllocation[indexBloc].isfull = 0;
        }
    }

    printf("Table d'allocation modifiée. Bloc %d est maintenant %d.\n",
           indexBloc, tableAllocation[indexBloc].isfull);
}
void AjouterBloc(Virtualdisk* ms,Fichier *Fichier) {
    if (ms == NULL) {
        printf("Erreur : Mémoire secondaire non initialisée.\n");
        return;
    }

    if (Fichier->mode == Contigue) {
        // Accéder à la table d'allocation
        bloc001* tableAllocation = (bloc001*)ms->bloc[0].data;

        // Trouver ou je peut ajouter
        int j = 0;
        while (j < ms->nb_blocs && tableAllocation[j].adresbloc != NULL  ){
            j++;
        }

        if (j == Fichier->max_bloc) {
            printf("Impossible d'ajouter un bloc : fichier pleine.\n");
            return;
        }

        // Agrandir le tableau de blocs
        Bloc* newBlocs = malloc((Fichier->nbBlocs+ 1) * sizeof(Bloc));
        if (newBlocs == NULL) {
            printf("Erreur : Allocation mémoire échouée.\n");
            return;
        }

        // Copier les anciens blocs dans le nouveau tableau
        for (int i = 0; i < Fichier->nbBlocs ; i++) {
            newBlocs[i] = Fichier->blocs[i];
        }

        // Libérer l'ancien tableau
        free(Fichier->blocs);

        // Initialiser le nouveau bloc
        newBlocs [Fichier->nbBlocs].data = malloc(TAILLE_MAX_BLOC);
        if (newBlocs[Fichier->nbBlocs].data == NULL) {
            printf("Erreur : Allocation mémoire échouée pour le nouveau bloc.\n");
            free(newBlocs);
            return;
        }
        newBlocs[Fichier->nbBlocs].taille = 0;
        newBlocs[Fichier->nbBlocs].numBloc = Fichier->nbBlocs+ 1;
        newBlocs[Fichier->nbBlocs].estComplet = false;
        newBlocs[Fichier->nbBlocs].next = NULL;

        // Mettre à jour la table d'allocation
        tableAllocation[Fichier->nbBlocs].isfull = 1;
        tableAllocation[Fichier->nbBlocs].adresbloc = &newBlocs[j]; // Marquer comme utilisé


        // Mettre à jour la référence du tableau de blocs
        Fichier->blocs = newBlocs;
        Fichier->nbBlocs++;
    } else {  // Mode Chaînée
        // Parcourir la liste pour trouver le dernier bloc
        Bloc* head = Fichier->blocs;
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
            Fichier->blocs = newBloc;  // La liste était vide
        } else {
            prec->next = newBloc;
        }
        Fichier->nbBlocs++;
    }
}
Bloc* trouverBlocAvecEspace(Fichier* fichier) {
    if (fichier == NULL) return NULL;

    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs; i++) {
            if ((fichier->blocs[i].taille < TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT) &&
                !fichier->blocs[i].estComplet) {
                return &fichier->blocs[i];
            }
        }
    } else { // Mode chaîné
        Bloc* head = fichier->blocs;
        while (head != NULL) {
            if (head->taille < TAILLE_MAX_BLOC / TAILLE_MAX_ENREGISTREMENT && !head->estComplet) {
                return head;
            }
            head = head->next;
        }
    }

    return NULL;
}

bool libererBloc(Fichier *fichier,Bloc *blocDirect) {
    if (fichier == NULL) {
        return false; // Vérification des paramètres
    }

    // Cas du mode contigu
    if (fichier->mode == Contigue) {
        if (blocDirect->numBloc < 0 || blocDirect->numBloc >= fichier->nbBlocs) {
            return false; // Numéro de bloc invalide
        }

        // Décaler les blocs suivants pour combler l'espace
        for (int i = blocDirect->numBloc; i < fichier->nbBlocs - 1; i++) {
            fichier->blocs[i] = fichier->blocs[i + 1];
        }

        fichier->nbBlocs--; // Réduction du nombre de blocs
        return true;
    }

    // Cas du mode chaîné
    if (fichier->mode == Chainee) {
        if (blocDirect == NULL) {
            return false; // Pointeur de bloc invalide
        }

        Bloc *prev = NULL;
        Bloc *current = fichier->blocs;

        // Rechercher le bloc dans la liste chaînée
        while (current != NULL) {
            if (current == blocDirect) {
                // Supprimer le bloc
                if (prev != NULL) {
                    prev->next = current->next;
                } else {
                    fichier->blocs = current->next; 
                }

                free(current); 
                return true;
            }
            fichier->nbBlocs--;
            prev = current;
            current = current->next;
        }
    }

    return false; 
}


// Initialisation du fichierTOV
Fichier *initialiserFichier(int capaciteMax, const char *nom, ModeOrganisationF sort, ModeOrganisationE mode) {
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
    fichier->max_bloc=(capaciteMax*TAILLE_MAX_ENREGISTREMENT)/TAILLE_MAX_BLOC + 1;
    fichier->entete.nbEnregistrements = 0;
    fichier->entete.capaciteMax = capaciteMax;
    fichier->entete.nextID = 0;
    fichier->enregistrements = (EnregistrementPhysique *)malloc(fichier->entete.capaciteMax * sizeof(EnregistrementPhysique));

    // Initialisation des blocs selon le mode
    if (mode == Contigue) {
        fichier->blocs = calloc(fichier->nbBlocs, sizeof(Bloc));
        if (fichier->blocs == NULL) {
            fprintf(stderr, "Erreur : Allocation mémoire échouée pour les blocs contigus.\n");
            free(fichier);
            return NULL;
        }
    } else if (mode == Chainee) {
        fichier->blocs = NULL; // Les blocs seront alloués dynamiquement
    }

    return fichier;
}

// Libération de la mémoire du fichier TOV
void libererFichier(Fichier *fichier) {
    if (fichier == NULL) {
        printf("libererFichier: fichier est NULL\n");
        return;
    }

    // Suppression du fichier physique
    if (access(fichier->nomFichier, F_OK) == 0) {
        if (remove(fichier->nomFichier) != 0) {
            fprintf(stderr, "Erreur : Échec de suppression du fichier %s.\n", fichier->nomFichier);
        }
    }

    // Libération mémoire selon le mode
    if (fichier->mode == Contigue) {
        free(fichier->blocs);
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        while (current != NULL) {
            Bloc *next = current->next;
            free(current->data); // Libération des données du bloc
            free(current);       // Libération du bloc
            current = next;
        }
    }

    free(fichier); // Libération de la structure FichierTOV elle-même
    printf("libererFichier: mémoire libérée et fichier supprimé\n");
}

// Affichage du contenu du fichier TOV
void afficherFichier(const Fichier *fichier) {
    if (fichier == NULL) {
        printf("afficherFichierTOV: fichier est NULL\n");
        return;
    }

    printf("Fichier TOV contient %d enregistrements:\n", fichier->entete.nbEnregistrements);

    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->entete.nbEnregistrements; i++) {
            printf("Enregistrement %d:\n", i);
            printf("data1: %s\n", fichier->enregistrements[i].data1);
            printf("data2: %s\n", fichier->enregistrements[i].data2);
            printf("data3: %s\n", fichier->enregistrements[i].data3);
        }
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        int index = 0;
        while (current != NULL) {
            printf("Bloc %d:\n", index++);
            printf("Data: %s\n", current->data);
            current = current->next;
        }
    }
}

// Compactage du fichier TOV
bool Compactage(Fichier *fichier) {
    if (fichier == NULL) {
        printf("Compactage: fichier est NULL\n");
        return false;
    }

    const char *nomFichierTemp = "tempFichierTOV.tov";
    FILE *fichierPhysique = fopen(fichier->nomFichier, "r");
    FILE *fichierTemp = fopen(nomFichierTemp, "w");

    if (fichierPhysique == NULL || fichierTemp == NULL) {
        if (fichierPhysique) fclose(fichierPhysique);
        if (fichierTemp) fclose(fichierTemp);
        return false;
    }

    EnregistrementPhysique enregistrement;
    int nouveauxNbEnregistrements = 0;

    // Lecture et réécriture des enregistrements valides
    while (lireEnregistrement(fichierPhysique, &enregistrement)) {
        if (enregistrementValide(&enregistrement)) {
            ecrireEnregistrement(fichierTemp, &enregistrement);
            nouveauxNbEnregistrements++;
        }
    }

    fclose(fichierPhysique);
    fclose(fichierTemp);

    // Remplacement de l'ancien fichier par le fichier compacté
    if (remove(fichier->nomFichier) != 0) {
        printf("Compactage: échec de suppression du fichier original\n");
        return false;
    }
    if (rename(nomFichierTemp, fichier->nomFichier) != 0) {
        printf("Compactage: échec de renommage du fichier temporaire\n");
        return false;
    }

    fichier->entete.nbEnregistrements = nouveauxNbEnregistrements;
    return true;
}
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
        AjouterBloc(ms, fichier);
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
                    libererBloc(fichier,bloc);
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
                    libererBloc(fichier,current);
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

        int comparaison = strcmp(enr->data1, cle);
        if (comparaison == 0 ) {
            return enr; // Trouvé
        } else if (comparaison < 0) {
            debut = milieu + 1;
        } else {
            fin = milieu - 1;
        }
    }

    return NULL; // Non trouvé
}
void remplirBuffer(BufferTransmission *buffer, const char *data)
{
    if (buffer == NULL || data == NULL)
        return;
    // copy data to the buffer, leaving space for the null terminator
    strncpy(buffer->data, data, TAILLE_BUFFER - 1);
    // explicitly set the last character to '\0' for null termination
    buffer->data[TAILLE_BUFFER - 1] = '\0';
    // Update the size of the data in the buffer
    buffer->taille = strlen(buffer->data);
}

void viderBuffer(BufferTransmission *buffer)
{ //(checked)
    if (buffer == NULL)
        return;
    // set the first character of the buffer to '\0' pour indiquer an empty string
    buffer->data[0] = '\0';
    // reset the size of the data in the buffer to 0
    buffer->taille = 0;
}
// nouvelle fonction d'initialisation de la table hashage:          (checked)
HashTable *initialiserHashTable(int taille) {
    HashTable *hashTable = malloc(sizeof(HashTable));
    if (!hashTable) {
        return NULL;
    }

    hashTable->taille = taille;
    hashTable->table = malloc(taille * sizeof(Element *));
    if (!hashTable->table) {
        free(hashTable);
        return NULL;
    }

    // Initialisation des buckets à NULL
    for (int i = 0; i < taille; i++) {
        hashTable->table[i] = NULL;
    }

    return hashTable;
}
// sont role est de transformer un ID en un indice
int hashFunction(int id, int tailleTable)
{
    // simple hach use of modulo
    // this will assure que l'indice is always dans les limites de la table de hachage (de 0 a tailleTable - 1).
    return id % tailleTable;
}
bool trouverDansHashTable(HashTable *hashTable, int id) {
    if (!hashTable) {
        return false;
    }

    int index = hashFunction(id, hashTable->taille);
    Element *courant = hashTable->table[index];

    // Parcourir la liste chaînée à ce bucket
    while (courant) {
        if (courant->id == id) {
            return true; // ID trouvé
        }
        courant = courant->suivant;
    }

    return false; // ID non trouvé
}
bool ajouterDansHashTable(HashTable *hashTable, int id) {
    if (!hashTable) {
        return false;
    }

    // Vérifier si l'ID existe déjà
    if (trouverDansHashTable(hashTable, id)) {
        return false; // ID déjà présent
    }

    // Calculer l'index
    int index = hashFunction(id, hashTable->taille);

    // Créer un nouvel élément
    Element *nouvelElement = malloc(sizeof(Element));
    if (!nouvelElement) {
        return false; // Échec de l'allocation
    }
    nouvelElement->id = id;
    nouvelElement->suivant = hashTable->table[index];

    // Ajouter au début de la liste chaînée
    hashTable->table[index] = nouvelElement;

    return true; // Ajout réussi
}
bool supprimerDansHashTable(HashTable *hashTable, int id) {
    if (!hashTable) {
        return false;
    }

    int index = hashFunction(id, hashTable->taille);
    Element *courant = hashTable->table[index];
    Element *precedent = NULL;

    // Parcourir la liste chaînée à ce bucket
    while (courant) {
        if (courant->id == id) {
            // Si l'élément est trouvé
            if (precedent) {
                // Retirer l'élément du milieu ou de la fin
                precedent->suivant = courant->suivant;
            } else {
                // Retirer le premier élément du bucket
                hashTable->table[index] = courant->suivant;
            }

            free(courant); // Libérer la mémoire de l'élément supprimé
            return true;   // Suppression réussie
        }

        precedent = courant;
        courant = courant->suivant;
    }

    return false; // ID non trouvé
}

void libererHashTable(HashTable *hashTable) {
    if (!hashTable) {
        return;
    }

    for (int i = 0; i < hashTable->taille; i++) {
        Element *courant = hashTable->table[i];
        while (courant) {
            Element *aSupprimer = courant;
            courant = courant->suivant;
            free(aSupprimer);
        }
    }

    free(hashTable->table);
    free(hashTable);
}
