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
   printf("Data1: "); scanf("%s", enr.data1);
   printf("Data2: "); scanf("%s", enr.data2);
   printf("Data3: "); scanf("%s", enr.data3);
   return enr;
}

int main() {
   Virtualdisk* ms = NULL;
   Fichier* fichierCourant = NULL;
   BufferTransmission buffer = {0};
   int choix;
   
   while(1) {
       afficherMenu();
       scanf("%d", &choix);
       
       switch(choix) {
           case 1: {
               int nbBlocs;
               printf("Nombre de blocs: ");
               scanf("%d", &nbBlocs);
               ms = InitialiseMs(nbBlocs);
               printf("Disque virtuel initialisé\n");
               break;
           }
           
           case 2: {
               if (!ms) {
                   printf("Initialisez d'abord le disque\n");
                   break;
               }
               char nom[30];
               int mode, sort;
               printf("Nom du fichier: "); scanf("%s", nom);
               printf("Mode (0:Contigue, 1:Chainee): "); scanf("%d", &mode);
               printf("Tri (0:Trie, 1:NoTrie): "); scanf("%d", &sort);
               fichierCourant = initialiserFichier(MAX_ENREGISTREMENTS_PAR_BLOC, ms, nom, mode, sort);
               break;
           }
           
           case 3: {
               if (!fichierCourant) {
                   printf("Créez d'abord un fichier\n");
                   break;
               }
               EnregistrementPhysique enr = creerEnregistrement();
               ajouterEnregistrement(ms, fichierCourant, &enr, &buffer);
               break;
           }
           
           case 4: {
               if (!fichierCourant) break;
               int id;
               printf("ID à supprimer: "); scanf("%d", &id);
               supprimerEnregistrement(ms, fichierCourant, id, &buffer, true);
               break;
           }
           
           case 5: {
               if (!fichierCourant) break;
               int id;
               printf("ID à rechercher: "); scanf("%d", &id);
               EnregistrementPhysique* enr = rechercherEnregistrement(fichierCourant, id, NULL, NULL);
               if (enr) {
                   printf("Trouvé: %s %s %s\n", enr->data1, enr->data2, enr->data3);
               }
               break;
           }
           
           case 6: {
               if (!fichierCourant) break;
               afficherFichier(fichierCourant);
               break;
           }
           
           case 7: {
               if (!ms) break;
               if (Compactage(ms)) printf("Compactage réussi\n");
               break;
           }
           
           case 8: {
               if (!fichierCourant) break;
               if (Defragmentation(fichierCourant)) printf("Défragmentation réussie\n");
               break;
           }
           
           case 9:
               if (ms) VidezMS(ms);
               if (fichierCourant) libererFichier(fichierCourant);
               return 0;
       }
   }
}