Gestionnaire de Fichiers Virtuel

Description:

Ce projet implémente un simulateur de gestion de fichiers virtuels. Il permet de manipuler des fichiers organisés en mode contigu ou chaîné sur un disque virtuel. Le programme offre une interface utilisateur avec diverses fonctionnalités pour créer, modifier, consulter et gérer les fichiers dans un environnement simulé.

Fonctionnalités

Initialiser le disque virtuel : Crée un disque virtuel prêt à recevoir des fichiers.

Créer un fichier : Ajoute un fichier au disque virtuel.

Ajouter un enregistrement : Insère un enregistrement dans un fichier existant.

Supprimer un enregistrement : Supprime un enregistrement spécifique d'un fichier.

Rechercher un enregistrement : Recherche un enregistrement par clé dans un fichier.

Afficher un fichier : Montre le contenu d'un fichier.

Compactage : Réduit la fragmentation dans le disque virtuel en réorganisant les blocs.

Défragmentation : Réorganise les blocs alloués pour optimiser l'espace disque.

Ouvrir un fichier : Charge un fichier dans la mémoire pour le modifier ou le lire.

Fermer un fichier : Sauvegarde les modifications apportées à un fichier et libère la mémoire.

Renommer un fichier : Change le nom d'un fichier existant.

Quitter : Termine le programme.

Prérequis

Compilateur C (gcc recommandé).

Système d'exploitation Linux ou Windows.

Structure du Projet

interface.console.c ou interface.c (si gtk marche) : Contient la logique principale et l'interface utilisateur.

mylib.h : Gère les structs et les header.

hashage.utils.c : Implémente les fonctions relatives au table de hasahge .

MS.utils.c :  Implémente les fonctions relatives au disque virtuel.

fichier.utils.c :  Gère les fichiers et leurs opérations.

Eneregistrement.utils.c : Gère les enregistrements et leurs opérations .

Bufffer.utils.c : Gère les buffer et leurs opérations .

Instructions d'Utilisation

Compilation

Utilisez la commande suivante pour compiler le programme :

gcc -o programme interface.console.c hashage.utils.c MS.utils.c fichier.utils.c Eneregistrement.utils.c Bufffer.utils.c

Exécution

Lancez le programme à l'aide de la commande suivante :

./programme

Navigation dans le Menu

Suivez les instructions affichées à l'écran.

Entrez le numéro correspondant à l'option désirée.

Pour quitter, choisissez l'option 12. Quitter.

Fonctionnement des Options Clés

Ouvrir un Fichier

Sélectionnez l'option 9. Ouvrir un fichier.

Entrez le nom du fichier à ouvrir.

Le fichier est chargé en mémoire pour modifications.

Fermer un Fichier

Assurez-vous qu'un fichier est ouvert.

Sélectionnez l'option 10. Fermer un fichier.

Les modifications sont sauvegardées, et la mémoire est libérée.

Renommer un Fichier

Sélectionnez l'option 11. Renommer un fichier.

Entrez le nom actuel du fichier.

Indiquez le nouveau nom souhaité.

Le fichier est renommé dans le disque virtuel.

Exemple d'Interaction

=== Gestionnaire de Fichiers ===
1. Initialiser le disque virtuel
2. Créer un fichier
...
9. Ouvrir un fichier
10. Fermer un fichier
11. Renommer un fichier
12. Quitter
Choix : 9
Nom du fichier à ouvrir : mon_fichier
Fichier 'mon_fichier' ouvert avec succès.

Limitations Connues

Les fichiers doivent respecter certaines contraintes de taille pour être gérés correctement.

La gestion des erreurs est limitée dans certains cas.

Auteur

Ce projet a été développé pour simuler un système de gestion de fichiers dans un disque virtuel à des fins éducatives.
