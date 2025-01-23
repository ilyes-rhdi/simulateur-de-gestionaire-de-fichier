#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "mylib.h"

GtkWidget *window;
GtkWidget *label_info;
static GtkWidget *text_view;
static GtkWidget *scrolled_window;
GtkWidget *entry_nom, *entry_taille, *entry_mode, *entry_sort;
GtkWidget *button_initialiser_ms;
Virtualdisk* ms = NULL;
Fichier* fichierCourant = NULL;
BufferTransmission buffer = {0};
static char stdout_buffer[4096];
static char stderr_buffer[4096];

// Pipes pour la redirection
static int stdout_pipe[2];
static int stderr_pipe[2];
void afficherMessage(const gchar *message) {
    gtk_label_set_text(GTK_LABEL(label_info), message);
}
// Fonction de lecture des pipes
static gboolean read_pipe(GIOChannel *channel, GIOCondition condition, gpointer data) {
    GIOStatus status;
    gsize bytes_read;
    gchar *buffer = (gchar *)data;
    
    status = g_io_channel_read_chars(channel, buffer, sizeof(stdout_buffer) - 1, &bytes_read, NULL);
    if (status == G_IO_STATUS_NORMAL && bytes_read > 0) {
        buffer[bytes_read] = '\0';
        afficherMessage(buffer);
    }
    return TRUE;
}

// Initialisation de la redirection
void init_output_redirection() {
    // Créer les pipes
    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Rediriger stdout et stderr
    if (dup2(stdout_pipe[1], STDOUT_FILENO) == -1 || dup2(stderr_pipe[1], STDERR_FILENO) == -1) {
        perror("dup2 failed");
        exit(EXIT_FAILURE);
    }
    
    // Configurer les channels
    GIOChannel *stdout_channel = g_io_channel_unix_new(stdout_pipe[0]);
    GIOChannel *stderr_channel = g_io_channel_unix_new(stderr_pipe[0]);

    // Vérifier si les channels ont été créés avec succès
    if (!stdout_channel || !stderr_channel) {
        g_printerr("Failed to create GIOChannel\n");
        exit(EXIT_FAILURE);
    }

    // Ajouter les watches
    if (!g_io_add_watch(stdout_channel, G_IO_IN, read_pipe, stdout_buffer)) {
        g_printerr("Failed to add watch for stdout\n");
        exit(EXIT_FAILURE);
    }
    if (!g_io_add_watch(stderr_channel, G_IO_IN, read_pipe, stderr_buffer)) {
        g_printerr("Failed to add watch for stderr\n");
        exit(EXIT_FAILURE);
    }
}


void afficherBlocgf(BufferTransmission *Buffer, Bloc *bloc, GtkTextBuffer *text_buffer) {
    if (bloc == NULL || Buffer == NULL) {
        gtk_text_buffer_set_text(text_buffer, "Erreur : Bloc non valide.\n", -1);
        return;
    }

    // Lire le bloc dans le buffer
    if (!LireBloc(Buffer, bloc)) {
        gtk_text_buffer_set_text(text_buffer, "Erreur : Échec de la lecture du bloc.\n", -1);
        return;
    }

    if (Buffer->B == NULL) {
        gtk_text_buffer_set_text(text_buffer, "Erreur : Données du bloc non disponibles dans le buffer.\n", -1);
        return;
    }

    // Afficher les métadonnées du bloc
    gchar texte[1024];
    g_snprintf(texte, sizeof(texte), "\nInformations du bloc:\n");
    gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);

    g_snprintf(texte, sizeof(texte), "Numéro du bloc: %d\n", Buffer->B->numBloc);
    gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
    g_snprintf(texte, sizeof(texte), "Nombre d'enregistrements: %d\n", Buffer->B->taille);
    gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
    g_snprintf(texte, sizeof(texte), "Est complet: %s\n", Buffer->B->estComplet ? "Oui" : "Non");
    gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);

    // Parcourir et afficher les enregistrements du bloc
    g_snprintf(texte, sizeof(texte), "\nDétails des enregistrements :\n");
    gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);

    const int TAILLE_MAX_BUFFER = 256;
    char bufferTemp[TAILLE_MAX_BUFFER];

    for (int i = 0; i < Buffer->B->taille; i++) {
        EnregistrementPhysique *enr = &Buffer->B->enregistrements[i];

        if (ecrireEnregistrement(bufferTemp, TAILLE_MAX_BUFFER, enr)) {
            g_snprintf(texte, sizeof(texte), "\nEnregistrement %d:\n", i + 1);
            gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
            g_snprintf(texte, sizeof(texte), "ID: %d\n", enr->entete.id);
            gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
            g_snprintf(texte, sizeof(texte), "Data1: %s\n", enr->data1);
            gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
            g_snprintf(texte, sizeof(texte), "Data2: %s\n", enr->data2);
            gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
            g_snprintf(texte, sizeof(texte), "Data3: %s\n", enr->data3);
            gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
        } else {
            g_snprintf(texte, sizeof(texte), "Erreur : Impossible de lire l'enregistrement %d.\n", i + 1);
            gtk_text_buffer_insert_at_cursor(text_buffer, texte, -1);
        }
    }
}
void afficherfichier(Fichier *fichier) {
    GtkTextBuffer *text_buffer;
    if (!text_view) {
        // Create TextView only once
        scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                     GTK_POLICY_AUTOMATIC,
                                     GTK_POLICY_AUTOMATIC);
        text_view = gtk_text_view_new();
        gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
        gtk_box_pack_start(GTK_BOX(gtk_bin_get_child(GTK_BIN(window))), 
                          scrolled_window, TRUE, TRUE, 0);
        gtk_widget_show_all(scrolled_window);
    }

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    if (!fichier) {
        gtk_text_buffer_set_text(text_buffer, "afficherFichier: fichier est NULL\n", -1);
        return;
    }

    gchar texte[1024];
    gchar buffer_tmp[256];

    g_snprintf(texte, sizeof(texte), "Fichier contient %d enregistrements:\n", 
               fichier->entete.nbEnregistrements);
    gtk_text_buffer_set_text(text_buffer, texte, -1);

    if (fichier->mode == Contigue) {
        for (int i = 0; i < fichier->nbBlocs; i++) {
            g_snprintf(buffer_tmp, sizeof(buffer_tmp), "Bloc %d:\n", i);
            gtk_text_buffer_insert_at_cursor(text_buffer, buffer_tmp, -1);
            afficherBlocgf(&buffer, &fichier->blocs[i], text_buffer);
        }
    } else if (fichier->mode == Chainee) {
        Bloc *current = fichier->blocs;
        int index = 0;
        while (current) {
            g_snprintf(buffer_tmp, sizeof(buffer_tmp), "Bloc %d:\n", index++);
            gtk_text_buffer_insert_at_cursor(text_buffer, buffer_tmp, -1);
            afficherBlocgf(&buffer, current, text_buffer);
            current = current->next;
        }
    }
}
void on_button_ouvrir_fermer_fichier_clicked(GtkWidget *widget, gpointer data) {
    if (fichierCourant == NULL) {
        GtkWidget *dialog, *entry_nom_fichier;
        dialog = gtk_dialog_new_with_buttons("Ouvrir un Fichier", GTK_WINDOW(window),
                                            GTK_DIALOG_MODAL, "Ouvrir", GTK_RESPONSE_ACCEPT,
                                            "Annuler", GTK_RESPONSE_REJECT,
                                            NULL);

        // Créer des champs de saisie pour cette fenêtre modale
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_container_add(GTK_CONTAINER(content_area), box);

        // Nom du fichier
        entry_nom_fichier = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom_fichier), "Nom du fichier");
        gtk_box_pack_start(GTK_BOX(box), entry_nom_fichier, FALSE, FALSE, 5);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_ACCEPT) {
            const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom_fichier));
            // Essayer d'ouvrir le fichier
            fichierCourant = ouvrirFichier(nom, ms);  // Fonction à définir dans "mylib.h"
            if (fichierCourant) {
                afficherMessage("Fichier ouvert avec succès.");
            } else {
                afficherMessage("Erreur lors de l'ouverture du fichier.");
            }
        }

        gtk_widget_destroy(dialog);
    } else {
        // Fermer le fichier si déjà ouvert
        fermerFichier(fichierCourant,ms);  // Fonction à définir dans "mylib.h"
        fichierCourant = NULL;
        afficherMessage("Fichier fermé.");
    }
}
// Fenêtre modale pour initialiser la mémoire virtuelle
void on_button_initialiser_ms_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *entry_taille_ms;
    dialog = gtk_dialog_new_with_buttons("Initialiser la Mémoire Virtuelle", GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL, "Initialiser", GTK_RESPONSE_ACCEPT,
                                        "Annuler", GTK_RESPONSE_REJECT,
                                        NULL);

    // Créer des champs de saisie pour cette fenêtre modale
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // Taille de la mémoire virtuelle
    entry_taille_ms = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_taille_ms), "Taille du disque virtuel");
    gtk_box_pack_start(GTK_BOX(box), entry_taille_ms, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        // Récupérer la taille de la mémoire virtuelle
        int taille = atoi(gtk_entry_get_text(GTK_ENTRY(entry_taille_ms)));
        ms = InitialiseMs(taille);  // Fonction à définir dans "mylib.h"
        if (!ms) {
            afficherMessage("Erreur lors de l'initialisation de la mémoire.");
        } else {
            afficherMessage("Mémoire virtuelle initialisée avec succès.");
            gtk_widget_hide(widget);
        }
    }
    
    // Fermer la fenêtre modale
    gtk_widget_destroy(dialog);
}

// Fenêtre modale pour la création du fichier
void on_button_creer_fichier_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *entry_nom_fichier, *entry_taille_fichier, *entry_mode_fichier, *entry_sort_fichier;
    dialog = gtk_dialog_new_with_buttons("Créer un Fichier", GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL, "Créer", GTK_RESPONSE_ACCEPT,
                                        "Annuler", GTK_RESPONSE_REJECT,
                                        NULL);

    // Créer des champs de saisie pour cette fenêtre modale
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // Nom du fichier
    entry_nom_fichier = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom_fichier), "Nom du fichier");
    gtk_box_pack_start(GTK_BOX(box), entry_nom_fichier, FALSE, FALSE, 5);

    // Taille du fichier
    entry_taille_fichier = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_taille_fichier), "Taille du fichier");
    gtk_box_pack_start(GTK_BOX(box), entry_taille_fichier, FALSE, FALSE, 5);

    // Mode (0: Contigue, 1: Chainee)
    entry_mode_fichier = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mode_fichier), "Mode (0: Contigue, 1: Chainee)");
    gtk_box_pack_start(GTK_BOX(box), entry_mode_fichier, FALSE, FALSE, 5);

    // Tri (0: Trie, 1: Non trie)
    entry_sort_fichier = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_sort_fichier), "Tri (0: Trie, 1: Non trie)");
    gtk_box_pack_start(GTK_BOX(box), entry_sort_fichier, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        // Récupérer les informations de la fenêtre modale
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom_fichier));
        int taille = atoi(gtk_entry_get_text(GTK_ENTRY(entry_taille_fichier)));
        int mode = atoi(gtk_entry_get_text(GTK_ENTRY(entry_mode_fichier)));
        int sort = atoi(gtk_entry_get_text(GTK_ENTRY(entry_sort_fichier)));

        if (!ms) {
            afficherMessage("Initialisez d'abord le disque virtuel.");
        } else {
            fichierCourant = initialiserFichier(taille, ms, nom, mode, sort);
            if (!fichierCourant) {
                afficherMessage("Erreur lors de la création du fichier.");
            } else {
                afficherMessage("Fichier créé avec succès.");
            }
        }
    }
    
    // Fermer la fenêtre modale
    gtk_widget_destroy(dialog);
}

// Fenêtre modale pour ajouter un enregistrement
void on_button_ajouter_enregistrement_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *entry_nom, *entry_filier, *entry_section;
    dialog = gtk_dialog_new_with_buttons("Ajouter un Enregistrement", GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL, "Ajouter", GTK_RESPONSE_ACCEPT,
                                        "Annuler", GTK_RESPONSE_REJECT,
                                        NULL);

    // Créer des champs de saisie pour cette fenêtre modale
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // Nom et prénom
    entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom), "Nom et prénom");
    gtk_box_pack_start(GTK_BOX(box), entry_nom, FALSE, FALSE, 5);

    // Filière
    entry_filier = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_filier), "Filière");
    gtk_box_pack_start(GTK_BOX(box), entry_filier, FALSE, FALSE, 5);

    // Section
    entry_section = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_section), "Section");
    gtk_box_pack_start(GTK_BOX(box), entry_section, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        EnregistrementPhysique enr;
        strcpy(enr.data1, gtk_entry_get_text(GTK_ENTRY(entry_nom)));
        strcpy(enr.data2, gtk_entry_get_text(GTK_ENTRY(entry_filier)));
        strcpy(enr.data3, gtk_entry_get_text(GTK_ENTRY(entry_section)));

        if (!ajouterEnregistrement(ms, fichierCourant, &enr,&buffer)) {
            afficherMessage("Erreur lors de l'ajout de l'enregistrement.");
        } else {
            afficherMessage("Enregistrement ajouté avec succès.");
        }
    }
    
    // Fermer la fenêtre modale
    gtk_widget_destroy(dialog);
}
void on_button_afficher_fichier_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *label_fichier_info;
    dialog = gtk_dialog_new_with_buttons("Afficher Fichier", GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL, "Fermer", GTK_RESPONSE_ACCEPT,
                                        NULL);

    // Créer une zone d'affichage pour cette fenêtre modale
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // Vérifier si un fichier est ouvert
    if (fichierCourant == NULL) {
        label_fichier_info = gtk_label_new("Aucun fichier ouvert.");
        gtk_box_pack_start(GTK_BOX(box), label_fichier_info, FALSE, FALSE, 5);
    } else {
        char fichier_info[1024];
        // Appel de la fonction pour afficher le contenu du fichier (exemple)
        afficherfichier(fichierCourant);  // Fonction à définir dans "mylib.h"
        label_fichier_info = gtk_label_new(fichier_info);
        gtk_box_pack_start(GTK_BOX(box), label_fichier_info, FALSE, FALSE, 5);
    }

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Fermer la fenêtre modale
    gtk_widget_destroy(dialog);
}
// Fenêtre modale pour supprimer un enregistrement
void on_button_supprimer_enregistrement_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *entry_id;
    dialog = gtk_dialog_new_with_buttons("Supprimer un Enregistrement", GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL, "Supprimer", GTK_RESPONSE_ACCEPT,
                                        "Annuler", GTK_RESPONSE_REJECT,
                                        NULL);

    // Créer des champs de saisie pour cette fenêtre modale
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // ID à supprimer
    entry_id = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_id), "ID à supprimer");
    gtk_box_pack_start(GTK_BOX(box), entry_id, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        int id = atoi(gtk_entry_get_text(GTK_ENTRY(entry_id)));
        bool a = true; // Suppression physique
        if (!supprimerEnregistrement(ms, fichierCourant, id,&buffer , a)) {
            afficherMessage("Erreur lors de la suppression de l'enregistrement.");
        } else {
            afficherMessage("Enregistrement supprimé avec succès.");
        }
    }
    
    // Fermer la fenêtre modale
    gtk_widget_destroy(dialog);
}

// Fenêtre modale pour rechercher un enregistrement
void on_button_rechercher_enregistrement_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *entry_id, *entry_nom, *entry_section;
    dialog = gtk_dialog_new_with_buttons("Rechercher un Enregistrement", GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL, "Rechercher", GTK_RESPONSE_ACCEPT,
                                        "Annuler", GTK_RESPONSE_REJECT,
                                        NULL);

    // Créer des champs de saisie pour cette fenêtre modale
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // ID de l'enregistrement
    entry_id = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_id), "ID de l'enregistrement");
    gtk_box_pack_start(GTK_BOX(box), entry_id, FALSE, FALSE, 5);

    // Nom
    entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom), "Nom");
    gtk_box_pack_start(GTK_BOX(box), entry_nom, FALSE, FALSE, 5);

    // Section
    entry_section = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_section), "Section");
    gtk_box_pack_start(GTK_BOX(box), entry_section, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        int id = atoi(gtk_entry_get_text(GTK_ENTRY(entry_id)));
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        const char *section = gtk_entry_get_text(GTK_ENTRY(entry_section));
        rechercherEnregistrement(fichierCourant, id, nom, section);
    }
    
    // Fermer la fenêtre modale
    gtk_widget_destroy(dialog);
}

// Fonction principale
int main(int argc, char *argv[]) {
    GtkWidget *box;
    static GtkWidget *button_initialiser_ms;
    
    if (!gtk_init_check(&argc, &argv)) {
        fprintf(stderr, "Failed to initialize GTK\n");
        return 1;
    }
    init_output_redirection();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    gtk_window_set_title(GTK_WINDOW(window), "Gestionnaire de Fichiers");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    if (!box) {
        fprintf(stderr, "Failed to create/add box\n");
        return 1;
    }

    label_info = gtk_label_new("Choisissez une action.");
    if (!label_info) {
        fprintf(stderr, "Failed to create label\n");
        return 1;
    }
    gtk_box_pack_start(GTK_BOX(box), label_info, FALSE, FALSE, 5);
    // Add new Open/Close File button
    GtkWidget *button_ouvrir_fermer = gtk_button_new_with_label("Ouvrir/Fermer Fichier");
    g_signal_connect(button_ouvrir_fermer, "clicked", G_CALLBACK(on_button_ouvrir_fermer_fichier_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_ouvrir_fermer, FALSE, FALSE, 5);

    button_initialiser_ms = gtk_button_new_with_label("Initialiser la Mémoire Virtuelle");
    g_signal_connect(button_initialiser_ms, "clicked", G_CALLBACK(on_button_initialiser_ms_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_initialiser_ms, FALSE, FALSE, 5);

    GtkWidget *button_creer_fichier = gtk_button_new_with_label("Créer un fichier");
    g_signal_connect(button_creer_fichier, "clicked", G_CALLBACK(on_button_creer_fichier_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_creer_fichier, FALSE, FALSE, 5);

    GtkWidget *button_ajouter_enregistrement = gtk_button_new_with_label("Ajouter un enregistrement");
    g_signal_connect(button_ajouter_enregistrement, "clicked", G_CALLBACK(on_button_ajouter_enregistrement_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_ajouter_enregistrement, FALSE, FALSE, 5);

    GtkWidget *button_supprimer_enregistrement = gtk_button_new_with_label("Supprimer un enregistrement");
    g_signal_connect(button_supprimer_enregistrement, "clicked", G_CALLBACK(on_button_supprimer_enregistrement_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_supprimer_enregistrement, FALSE, FALSE, 5);

    GtkWidget *button_rechercher_enregistrement = gtk_button_new_with_label("Rechercher un enregistrement");
    g_signal_connect(button_rechercher_enregistrement, "clicked", G_CALLBACK(on_button_rechercher_enregistrement_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_rechercher_enregistrement, FALSE, FALSE, 5);

    GtkWidget *button_afficher_fichier = gtk_button_new_with_label("Afficher Fichier");
    g_signal_connect(button_afficher_fichier, "clicked", G_CALLBACK(on_button_afficher_fichier_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_afficher_fichier, FALSE, FALSE, 5);

    gtk_widget_show_all(window);
    gtk_main();

    if (fichierCourant) {
        fermerFichier(fichierCourant, ms);
    }

    return 0;
}