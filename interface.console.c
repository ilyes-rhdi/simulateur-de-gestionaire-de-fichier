#include "mylib.h"

void afficherMenu() {
    printf("\n=== Gestionnaire de Fichiers ===\n");
    printf("1. Initialiser le disque virtuel\n");
    printf("2. Créer un fichier\n");
    printf("3. Ajouter un enregistrement\n"); 
    printf("4. Supprimer un enregistrement\n");
    printf("5. Rechercher un enregistrement\n");
    printf("6. Afficher un fichier\n");
    printf("7. Compactage\n");
    printf("8. Défragmentation\n");
    printf("9. Quitter\n");
    printf("Choix : ");
}

EnregistrementPhysique creerEnregistrement() {
    EnregistrementPhysique enr;
    printf("Nom et prenom: ");
    scanf("%s",enr.data1);
    printf("Filier : ");
    scanf("%s",enr.data2);  
    printf("Section: ");
    scanf("%s",enr.data3);
    return enr;
}

int main() {
    Virtualdisk* ms = NULL;
    Fichier* fichierCourant = NULL;
    BufferTransmission buffer = {0};
    int choix;

    while (true) {
        afficherMenu();
        scanf("%d", &choix);

        switch (choix) {
            case 1: {
                int nbBlocs;
                printf("Nombre de blocs: ");
                scanf("%d", &nbBlocs);
                ms = InitialiseMs(nbBlocs);
                if (!ms) {
                    printf("Erreur lors de l'initialisation du disque virtuel.\n");
                } else {
                    printf("Disque virtuel initialise avec succès.\n");
                }
                break;
            }

            case 2: {
                if (!ms) {
                    printf("Initialisez d'abord le disque virtuel.\n");
                    break;
                }
                char nom[30];
                int mode, sort, taille;
                printf("Taille du fichier (en blocs): "); scanf("%d", &taille);
                printf("Nom du fichier: "); scanf("%s", nom);
                printf("Mode (0: Contigue, 1: Chainee): "); scanf("%d", &mode);
                printf("Tri (0: Trie, 1: Non trie): "); scanf("%d", &sort);

                fichierCourant = initialiserFichier(taille, ms, nom, mode, sort);
                if (!fichierCourant) {
                    printf("Erreur lors de la création du fichier.\n");
                } else {
                    printf("Fichier créé avec succès.\n");
                }
                break;
            }

            case 3: {
                if (!fichierCourant) {
                    printf("Créez d'abord un fichier.\n");
                    break;
                }
                EnregistrementPhysique enr = creerEnregistrement();
                if (!ajouterEnregistrement(ms, fichierCourant, &enr, &buffer)) {
                    printf("Erreur lors de l'ajout de l'enregistrement.\n");
                } else {
                    printf("Enregistrement ajouté avec succès.\n");
                }
                break;
            }

            case 4: {
                if (!fichierCourant) {
                    printf("Créez d'abord un fichier.\n");
                    break;
                }
                int id;
                bool a;
                printf("Suppresion (0: logique, 1: physique): "); scanf("%d", &a);
                printf("ID à supprimer: ");
                scanf("%d", &id);
                if (!supprimerEnregistrement(ms, fichierCourant, id, &buffer, a)) {
                    printf("Erreur lors de la suppression de l'enregistrement.\n");
                } else {
                    printf("Enregistrement supprimé avec succès.\n");
                }
                break;
            }

            case 5: {
                if (!fichierCourant) {
                    printf("Créez d'abord un fichier.\n");
                    break;
                }
                int id;
                char name[40];
                char* Section;
                printf("ID à rechercher: ");
                scanf("%d", &id);
                printf("Nom et  Section de la personne a chercher : ");
                scanf("%s%s", &name,&Section);

                EnregistrementPhysique* enr = rechercherEnregistrement(fichierCourant, id, name, Section);
                if (enr) {
                    printf("Trouvé: %s %s %s\n", enr->data1, enr->data2, enr->data3);
                } else {
                    printf("Enregistrement non trouvé.\n");
                }
                break;
            }

            case 6: {
                if (!fichierCourant) {
                    printf("Créez d'abord un fichier.\n");
                    break;
                }
                afficherFichier(fichierCourant);
                break;
            }

            case 7: {
                if (!ms) {
                    printf("Initialisez d'abord le disque virtuel.\n");
                    break;
                }
                if (Compactage(ms)) {
                    printf("Compactage réussi.\n");
                } else {
                    printf("Erreur lors du compactage.\n");
                }
                break;
            }

            case 8: {
                if (!fichierCourant) {
                    printf("Créez d'abord un fichier.\n");
                    break;
                }
                if (Defragmentation(fichierCourant)) {
                    printf("Défragmentation réussie.\n");
                } else {
                    printf("Erreur lors de la défragmentation.\n");
                }
                break;
            }

            case 9:
                if (ms) VidezMS(ms);
                if (fichierCourant) libererFichier(fichierCourant);
                return 0;

            default:
                printf("Choix invalide. Réessayez.\n");
        }
    }
}
