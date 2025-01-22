#include <gtk/gtk.h>
#include "mylib.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *file_list;
    GtkWidget *status_bar;
    
    // Champs pour la création/ouverture de fichier
    GtkWidget *entry_filename;
    GtkWidget *spin_file_size;
    GtkWidget *combo_organization;  // Contigue/Chainee
    GtkWidget *combo_sort;         // Trie/Non trie
    
    // Champs pour les enregistrements
    GtkWidget *entry_nom_prenom;    // data1
    GtkWidget *entry_filiere;       // data2
    GtkWidget *entry_section;       // data3
    GtkWidget *spin_id;            // Pour la recherche/suppression par ID
    GtkWidget *check_physical_delete; // Pour la suppression physique/logique
    
    // Système de fichiers
    Virtualdisk *ms;
    Fichier *current_file;
    BufferTransmission *buffer;
} AppData;

static void create_gui(AppData *app) {
    // Création de la fenêtre principale
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Gestionnaire de Fichiers");
    gtk_container_set_border_width(GTK_CONTAINER(app->window), 10);
    gtk_widget_set_size_request(app->window, 1000, 700);

    // Layout principal
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app->window), main_box);

    // Création de la barre d'outils
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);

    // Zone des paramètres du fichier
    GtkWidget *file_frame = gtk_frame_new("Paramètres du fichier");
    GtkWidget *file_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(file_frame), file_box);
    gtk_box_pack_start(GTK_BOX(main_box), file_frame, FALSE, FALSE, 0);

    // Nom du fichier
    GtkWidget *filename_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(file_box), filename_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(filename_box), gtk_label_new("Nom du fichier:"), FALSE, FALSE, 0);
    app->entry_filename = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(filename_box), app->entry_filename, TRUE, TRUE, 0);

    // Taille du fichier
    GtkWidget *size_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(file_box), size_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(size_box), gtk_label_new("Taille (blocs):"), FALSE, FALSE, 0);
    app->spin_file_size = gtk_spin_button_new_with_range(1, 1000, 1);
    gtk_box_pack_start(GTK_BOX(size_box), app->spin_file_size, TRUE, TRUE, 0);

    // Mode d'organisation
    GtkWidget *org_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(file_box), org_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(org_box), gtk_label_new("Organisation:"), FALSE, FALSE, 0);
    app->combo_organization = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_organization), "Contigue");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_organization), "Chainee");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_organization), 0);
    gtk_box_pack_start(GTK_BOX(org_box), app->combo_organization, TRUE, TRUE, 0);

    // Mode de tri
    GtkWidget *sort_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(file_box), sort_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sort_box), gtk_label_new("Tri:"), FALSE, FALSE, 0);
    app->combo_sort = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_sort), "Trie");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_sort), "Non trie");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_sort), 0);
    gtk_box_pack_start(GTK_BOX(sort_box), app->combo_sort, TRUE, TRUE, 0);

    // Zone des enregistrements
    GtkWidget *record_frame = gtk_frame_new("Enregistrement");
    GtkWidget *record_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(record_frame), record_box);
    gtk_box_pack_start(GTK_BOX(main_box), record_frame, FALSE, FALSE, 0);

    // Nom et prénom
    GtkWidget *name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(record_box), name_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(name_box), gtk_label_new("Nom et prénom:"), FALSE, FALSE, 0);
    app->entry_nom_prenom = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(name_box), app->entry_nom_prenom, TRUE, TRUE, 0);

    // Filière
    GtkWidget *filiere_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(record_box), filiere_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(filiere_box), gtk_label_new("Filière:"), FALSE, FALSE, 0);
    app->entry_filiere = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(filiere_box), app->entry_filiere, TRUE, TRUE, 0);

    // Section
    GtkWidget *section_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(record_box), section_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section_box), gtk_label_new("Section:"), FALSE, FALSE, 0);
    app->entry_section = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(section_box), app->entry_section, TRUE, TRUE, 0);

    // ID pour la recherche/suppression
    GtkWidget *id_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(record_box), id_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(id_box), gtk_label_new("ID:"), FALSE, FALSE, 0);
    app->spin_id = gtk_spin_button_new_with_range(0, 1000000, 1);
    gtk_box_pack_start(GTK_BOX(id_box), app->spin_id, TRUE, TRUE, 0);

    // Checkbox pour suppression physique/logique
    app->check_physical_delete = gtk_check_button_new_with_label("Suppression physique");
    gtk_box_pack_start(GTK_BOX(record_box), app->check_physical_delete, FALSE, FALSE, 0);

    // Boutons d'action
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 0);

    GtkWidget *btn_create = gtk_button_new_with_label("Créer Fichier");
    gtk_box_pack_start(GTK_BOX(button_box), btn_create, TRUE, TRUE, 0);
    g_signal_connect(btn_create, "clicked", G_CALLBACK(initialiserFichier(spin)), app);

    GtkWidget *btn_open = gtk_button_new_with_label("Ouvrir Fichier");
    gtk_box_pack_start(GTK_BOX(button_box), btn_open, TRUE, TRUE, 0);
    g_signal_connect(btn_open, "clicked", G_CALLBACK(ouvrirFichier()), app);

    GtkWidget *btn_close = gtk_button_new_with_label("Fermer Fichier");
    gtk_box_pack_start(GTK_BOX(button_box), btn_close, TRUE, TRUE, 0);
    g_signal_connect(btn_close, "clicked", G_CALLBACK(on_close_file_clicked), app);

    GtkWidget *btn_rename = gtk_button_new_with_label("Renommer Fichier");
    gtk_box_pack_start(GTK_BOX(button_box), btn_rename, TRUE, TRUE, 0);
    g_signal_connect(btn_rename, "clicked", G_CALLBACK(on_rename_file_clicked), app);

    GtkWidget *button_box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), button_box2, FALSE, FALSE, 0);

    GtkWidget *btn_add = gtk_button_new_with_label("Ajouter Enregistrement");
    gtk_box_pack_start(GTK_BOX(button_box2), btn_add, TRUE, TRUE, 0);
    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_record_clicked), app);

    GtkWidget *btn_delete = gtk_button_new_with_label("Supprimer Enregistrement");
    gtk_box_pack_start(GTK_BOX(button_box2), btn_delete, TRUE, TRUE, 0);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_delete_record_clicked), app);

    GtkWidget *btn_search = gtk_button_new_with_label("Rechercher");
    gtk_box_pack_start(GTK_BOX(button_box2), btn_search, TRUE, TRUE, 0);
    g_signal_connect(btn_search, "clicked", G_CALLBACK(on_search_record_clicked), app);

    GtkWidget *button_box3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), button_box3, FALSE, FALSE, 0);

    GtkWidget *btn_compact = gtk_button_new_with_label("Compacter");
    gtk_box_pack_start(GTK_BOX(button_box3), btn_compact, TRUE, TRUE, 0);
    g_signal_connect(btn_compact, "clicked", G_CALLBACK(on_compact_clicked), app);

    GtkWidget *btn_defrag = gtk_button_new_with_label("Défragmenter");
    gtk_box_pack_start(GTK_BOX(button_box3), btn_defrag, TRUE, TRUE, 0);
    g_signal_connect(btn_defrag, "clicked", G_CALLBACK(on_defrag_clicked), app);

    // Liste des enregistrements
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, 
                                           G_TYPE_STRING, G_TYPE_BOOLEAN);
    app->file_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("ID", 
                                                                        renderer, 
                                                                        "text", 0, 
                                                                        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->file_list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Nom et Prénom", 
                                                     renderer, 
                                                     "text", 1, 
                                                     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->file_list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Filière", 
                                                     renderer, 
                                                     "text", 2, 
                                                     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->file_list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Section", 
                                                     renderer, 
                                                     "text", 3, 
                                                     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->file_list), column);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Supprimé", 
                                                     renderer, 
                                                     "active", 4, 
                                                     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->file_list), column);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    // Suite du create_gui()
    gtk_container_add(GTK_CONTAINER(scrolled_window), app->file_list);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled_window, TRUE, TRUE, 0);

    // Barre de statut
    app->status_bar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(main_box), app->status_bar, FALSE, FALSE, 0);

    g_signal_connect(app->window, "destroy", G_CALLBACK(on_quit_clicked), app);

    gtk_widget_show_all(app->window);
}

// Implémentation des fonctions de callback
static void on_create_file_clicked(GtkButton *button, AppData *app) {
    const char *filename = gtk_entry_get_text(GTK_ENTRY(app->entry_filename));
    int taille = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->spin_file_size));
    int mode = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_organization));
    int sort = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_sort));

    if (strlen(filename) == 0) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Veuillez entrer un nom de fichier");
        return;
    }

    if (app->current_file) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Fermez d'abord le fichier courant");
        return;
    }

    app->current_file = initialiserFichier(taille, app->ms, filename, mode, sort);
    if (app->current_file != NULL) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Fichier créé avec succès");
        update_file_list(app);
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Erreur lors de la création du fichier");
    }
}

static void on_open_file_clicked(GtkButton *button, AppData *app) {
    const char *filename = gtk_entry_get_text(GTK_ENTRY(app->entry_filename));
    
    if (strlen(filename) == 0) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Veuillez entrer un nom de fichier");
        return;
    }

    if (app->current_file) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Fermez d'abord le fichier courant");
        return;
    }

    app->current_file = ouvrirFichier(filename, app->ms);
    if (app->current_file != NULL) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Fichier ouvert avec succès");
        update_file_list(app);
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Erreur lors de l'ouverture du fichier");
    }
}

static void on_close_file_clicked(GtkButton *button, AppData *app) {
    if (!app->current_file) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Aucun fichier ouvert");
        return;
    }

    if (fermerFichier(app->current_file, app->ms)) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Fichier fermé avec succès");
        app->current_file = NULL;
        update_file_list(app);
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Erreur lors de la fermeture du fichier");
    }
}

static void on_rename_file_clicked(GtkButton *button, AppData *app) {
    const char *new_filename = gtk_entry_get_text(GTK_ENTRY(app->entry_filename));
    
    if (!app->current_file || strlen(new_filename) == 0) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Ouvrez un fichier et entrez le nouveau nom");
        return;
    }

    char old_name[30];
    strncpy(old_name, app->current_file->nomFichier, sizeof(old_name) - 1);
    RenameFichier(old_name, new_filename, app->ms);
    gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Fichier renommé avec succès");
}

static void on_add_record_clicked(GtkButton *button, AppData *app) {
    if (!app->current_file) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Ouvrez d'abord un fichier");
        return;
    }

    EnregistrementPhysique enr;
    enr.entete.suppprimer = false;
    enr.entete.id = app->current_file->entete.nextID++;

    strncpy(enr.data1, gtk_entry_get_text(GTK_ENTRY(app->entry_nom_prenom)), TAILLE_MAX_ENREGISTREMENT - 1);
    strncpy(enr.data2, gtk_entry_get_text(GTK_ENTRY(app->entry_filiere)), TAILLE_MAX_ENREGISTREMENT - 1);
    strncpy(enr.data3, gtk_entry_get_text(GTK_ENTRY(app->entry_section)), TAILLE_MAX_ENREGISTREMENT - 1);

    if (ajouterEnregistrement(app->ms, app->current_file, &enr, app->buffer)) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Enregistrement ajouté avec succès");
        update_file_list(app);
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Erreur lors de l'ajout de l'enregistrement");
    }
}

static void on_delete_record_clicked(GtkButton *button, AppData *app) {
    if (!app->current_file) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Ouvrez d'abord un fichier");
        return;
    }

    int id = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->spin_id));
    bool physical = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->check_physical_delete));

    if (supprimerEnregistrement(app->ms, app->current_file, id, app->buffer, physical)) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Enregistrement supprimé avec succès");
        update_file_list(app);
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Erreur lors de la suppression");
    }
}

static void on_search_record_clicked(GtkButton *button, AppData *app) {
    if (!app->current_file) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Ouvrez d'abord un fichier");
        return;
    }

    int id = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->spin_id));
    const char *nom = gtk_entry_get_text(GTK_ENTRY(app->entry_nom_prenom));
    const char *section = gtk_entry_get_text(GTK_ENTRY(app->entry_section));

    EnregistrementPhysique* enr = rechercherEnregistrement(app->current_file, id, nom, section);
    if (enr) {
        char message[256];
        snprintf(message, sizeof(message), "Trouvé: %s %s %s", enr->data1, enr->data2, enr->data3);
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, message);
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Enregistrement non trouvé");
    }
}

static void on_defrag_clicked(GtkButton *button, AppData *app) {
    if (!app->current_file) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Ouvrez d'abord un fichier");
        return;
    }

    if (Defragmentation(app->current_file)) {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Défragmentation réussie");
        update_file_list(app);
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(app->status_bar), 0, "Erreur lors de la défragmentation");
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppData *app = g_malloc0(sizeof(AppData));
    
    // Initialisation du disque virtuel avec une taille par défaut de 100 blocs
    app->ms = InitialiseMs(100);
    if (app->ms == NULL) {
        g_print("Erreur lors de l'initialisation du disque virtuel\n");
        g_free(app);
        return 1;
    }

    // Initialisation du buffer de transmission
    app->buffer = (BufferTransmission *)malloc(sizeof(BufferTransmission));
    if (app->buffer == NULL) {
        g_print("Erreur lors de l'allocation du buffer\n");
        VidezMS(app->ms);
        g_free(app);
        return 1;
    }
    app->buffer->taille = 0;
    app->buffer->B = NULL;

    app->current_file = NULL;

    create_gui(app);
    gtk_main();

    // Nettoyage
    if (app->current_file) {
        fermerFichier(app->current_file, app->ms);
    }
    VidezMS(app->ms);
    free(app->buffer);
    g_free(app);

    return 0;
}