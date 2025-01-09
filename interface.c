#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "mylib.h"




// Forward declarations of signal handlers
void on_button_create_clicked(GtkButton *button, gpointer user_data);
void on_button_add_clicked(GtkButton *button, gpointer user_data);
void on_button_delete_clicked(GtkButton *button, gpointer user_data);
void on_button_show_content_clicked(GtkButton *button, gpointer user_data);
void on_button_search_clicked(GtkButton *button, gpointer user_data);
void on_button_quit_clicked(GtkButton *button, gpointer user_data);
void on_button_confirm_clicked(GtkButton *button, gpointer user_data);
void on_button_confirm_create_clicked(GtkButton *button, gpointer user_data);
void on_button_confirm_delete_clicked(GtkButton *button, gpointer user_data);
void on_button_confirm_search_clicked(GtkButton *button, gpointer user_data);

HashTable *initialiserHashTable(int taille);

GtkApplication *global_app = NULL;

GtkWidget *button_add;
GtkWidget *button_delete;
GtkWidget *button_show_content;
GtkWidget *button_create;
GtkWidget *button_search;

Fichier fichier;
HashTable hashTable;
BufferTransmission buffer;
EnregistrementPhysique newEnregistrement;

// Activation callback
static void activate(GtkApplication *app, gpointer user_data)
{
    GtkBuilder *builder;
    GtkWidget *window;
     
     // Declare button variables
    GtkWidget *button_quit;
    GtkWidget *button_confirm;
    GtkWidget *button_confirm_create;
    GtkWidget *button_confirm_delete;
    GtkWidget *button_confirm_search;
    
    // Create a new builder from the Glade file
    builder = gtk_builder_new_from_file("design.glade");

    // Get the main window from the builder
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    if (!window) {
        g_printerr("Failed to fetch main window from the Glade file\n");
        return;
    }

    // Set the application for the window
    gtk_window_set_application(GTK_WINDOW(window), app);

    // Fetch buttons and other widgets
    button_create = GTK_WIDGET(gtk_builder_get_object(builder, "button_create"));
    button_add = GTK_WIDGET(gtk_builder_get_object(builder, "button_add"));
    button_delete = GTK_WIDGET(gtk_builder_get_object(builder, "button_delete"));
    button_show_content = GTK_WIDGET(gtk_builder_get_object(builder, "button_show_content"));
    button_search = GTK_WIDGET(gtk_builder_get_object(builder, "button_search"));
    button_quit = GTK_WIDGET(gtk_builder_get_object(builder, "button_quit"));
    button_confirm = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm"));
    button_confirm_create = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm_create"));
    button_confirm_delete = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm_delete"));
    button_confirm_search = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm_search"));
    GtkWidget *label_content = GTK_WIDGET(gtk_builder_get_object(builder, "label_content"));

    // Check if all buttons were fetched successfully
    if (!button_create || !button_add || !button_delete || !button_show_content || !button_quit || 
        !button_confirm || !button_confirm_create || !button_confirm_delete || !button_search || 
        !button_confirm_search) {
        g_printerr("Failed to fetch widgets from the Glade file\n");
        g_object_unref(builder);
        return;
    }

    // Connect signals
    g_signal_connect(G_OBJECT(button_create), "clicked", G_CALLBACK(on_button_create_clicked), builder);
    g_signal_connect(G_OBJECT(button_add), "clicked", G_CALLBACK(on_button_add_clicked), builder);
    g_signal_connect(G_OBJECT(button_delete), "clicked", G_CALLBACK(on_button_delete_clicked), builder);
    g_signal_connect(G_OBJECT(button_show_content), "clicked", G_CALLBACK(on_button_show_content_clicked), builder);
    g_signal_connect(G_OBJECT(button_search), "clicked", G_CALLBACK(on_button_search_clicked), builder);
    g_signal_connect(G_OBJECT(button_quit), "clicked", G_CALLBACK(on_button_quit_clicked), builder);
    g_signal_connect(G_OBJECT(button_confirm), "clicked", G_CALLBACK(on_button_confirm_clicked), builder);
    g_signal_connect(G_OBJECT(button_confirm_create), "clicked", G_CALLBACK(on_button_confirm_create_clicked), builder);
    g_signal_connect(G_OBJECT(button_confirm_delete), "clicked", G_CALLBACK(on_button_confirm_delete_clicked), builder);
    g_signal_connect(G_OBJECT(button_confirm_search), "clicked", G_CALLBACK(on_button_confirm_search_clicked), builder);
    // Present the main window
    gtk_window_present(GTK_WINDOW(window));
}
// Signal handler for the "Add" button
void on_button_create_clicked(GtkButton *button, gpointer user_data)
{
    printf("Create button was clicked\n");

    // Declare the modal and confirm button locally
    GtkBuilder *builder = GTK_BUILDER(user_data);
    GtkWidget *modal_window_inf = GTK_WIDGET(gtk_builder_get_object(builder, "information_modal"));
    GtkWidget *button_confirm_create = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm_create"));

    // Get the top-level parent window from the button widget using gtk_widget_get_ancestor
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW));

    // Corrected variable name: use modal_window_inf instead of information_modal
    gtk_window_set_transient_for(GTK_WINDOW(modal_window_inf), parent_window);

    // Set the modal window for the "Confirm" button
    g_signal_connect(button_confirm_create, "clicked", G_CALLBACK(on_button_confirm_create_clicked), builder);

    // Present the modal window using gtk_window_present
    gtk_window_present(GTK_WINDOW(modal_window_inf));
}

void on_button_add_clicked(GtkButton *button, gpointer user_data)
{
    printf("Add button was clicked\n");

    GtkBuilder *builder = gtk_builder_new_from_file("design.glade");
    GtkWidget *modal_window = GTK_WIDGET(gtk_builder_get_object(builder, "add_item_modal"));
    GtkWidget *confirm_button = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm"));

    // Set parent window for the modal
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW));
    gtk_window_set_transient_for(GTK_WINDOW(modal_window), parent_window);

    // Set the modal window for the "Confirm" button
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_button_confirm_clicked), builder);

    // Present the modal window using gtk_window_present
    gtk_window_present(GTK_WINDOW(modal_window));
}
void on_button_confirm_create_clicked(GtkButton *button, gpointer user_data)
{
    // Assuming user_data contains the main window's builder
    GtkBuilder *main_builder = GTK_BUILDER(user_data);

    GtkWidget *modal_window_inf = GTK_WIDGET(gtk_builder_get_object(main_builder, "information_modal"));
    GtkWidget *length_entry = GTK_WIDGET(gtk_builder_get_object(main_builder, "length_entry"));

    GtkWidget *length_error_label = GTK_WIDGET(gtk_builder_get_object(main_builder, "label_length_error"));

    const char *length_text = gtk_entry_get_text(GTK_ENTRY(length_entry));
    

    // Form validation
    if (strlen(length_text) == 0)
    {
        gtk_widget_set_visible(length_error_label, TRUE);
        return;
    }
    else
    {
        gtk_widget_set_visible(length_error_label, FALSE);

        // Additional validation: Check if the input is a number
        for (int i = 0; length_text[i] != '\0'; i++)
        {
            if (!isdigit(length_text[i]))
            {
                // If not a number, show error label and return
                gtk_widget_set_visible(length_error_label, TRUE);
                return;
            }
        }
    }

    // If it passes both checks, hide the error labels
    gtk_widget_set_visible(length_error_label, FALSE);
   int capaciteFichier = atoi(length_text);
     if (capaciteFichier <= 0) {
        // Handle invalid input (e.g., show an error message)
        return;
    }
    // initilaise the folder after confirming
    
     int num_blocks = capaciteFichier; 
     Virtualdisk *ms = InitialiseMs(num_blocks);
    char nom[256] = "monFichier"; 
    ModeOrganisationF sortMode = Contigue; 
    ModeOrganisationE mode = Trie; 
    fichier = *initialiserFichier(capaciteFichier, ms, nom, sortMode, mode); 

    HashTable *hashTable = initialiserHashTable(capaciteFichier);

    // Get the label widget where you want to display the new message
    GtkWidget *label_content = GTK_WIDGET(gtk_builder_get_object(main_builder, "label_content"));

    // Set the new message for the label
    const char *new_message = "Great, file has been created successfully!\n Now try to add some content to your file.";
    gtk_label_set_text(GTK_LABEL(label_content), new_message);

    // Get the buttons where you want to set visibility
    button_add = GTK_WIDGET(gtk_builder_get_object(main_builder, "button_add"));
    button_delete = GTK_WIDGET(gtk_builder_get_object(main_builder, "button_delete"));
    button_show_content = GTK_WIDGET(gtk_builder_get_object(main_builder, "button_show_content"));
    button_show_content = GTK_WIDGET(gtk_builder_get_object(main_builder, "button_show_content"));
    button_search = GTK_WIDGET(gtk_builder_get_object(main_builder, "button_search"));
    button_create = GTK_WIDGET(gtk_builder_get_object(main_builder, "button_create"));

    // Set the visibility of these buttons to TRUE
    gtk_widget_set_visible(button_add, TRUE);
    gtk_widget_set_visible(button_delete, TRUE);
    gtk_widget_set_visible(button_show_content, TRUE);
    gtk_widget_set_visible(button_search, TRUE);
    gtk_widget_set_visible(button_create, FALSE);
    
     // Free the allocated Virtualdisk memory
    VidezMS(ms); 
    free(ms);
    // Hide the modal window after confirming
    gtk_widget_set_visible(modal_window_inf, FALSE);
}

void on_button_confirm_clicked(GtkButton *button, gpointer user_data)
{
    GtkBuilder *builder = GTK_BUILDER(user_data);

    GtkWidget *modal_window = GTK_WIDGET(gtk_builder_get_object(builder, "add_item_modal"));
    GtkWidget *first_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "first_name_entry"));
    GtkWidget *second_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "second_name_entry"));
    GtkWidget *id_entry = GTK_WIDGET(gtk_builder_get_object(builder, "id_entry"));

    GtkWidget *first_name_error_label = GTK_WIDGET(gtk_builder_get_object(builder, "label_first_name_error"));
    GtkWidget *second_name_error_label = GTK_WIDGET(gtk_builder_get_object(builder, "label_second_name_error"));
    GtkWidget *id_error_label = GTK_WIDGET(gtk_builder_get_object(builder, "label_id_error"));

    const char *first_name = gtk_entry_get_text(GTK_ENTRY(first_name_entry));
    const char *second_name = gtk_entry_get_text(GTK_ENTRY(second_name_entry));
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(id_entry));

    // Form validation
    if (strlen(first_name) == 0)
    {
        gtk_widget_set_visible(first_name_error_label, TRUE);
        return;
    }
    else
    {
        gtk_widget_set_visible(first_name_error_label, FALSE);
    }

    if (strlen(second_name) == 0)
    {
        gtk_widget_set_visible(second_name_error_label, TRUE);
        return;
    }
    else
    {
        gtk_widget_set_visible(second_name_error_label, FALSE);
    }

    if (strlen(id_text) == 0)
    {
        gtk_widget_set_visible(id_error_label, TRUE);
        return;
    }
    else
    {
        gtk_widget_set_visible(id_error_label, FALSE);

        // Additional validation: Check if the input is a number
        for (int i = 0; id_text[i] != '\0'; i++)
        {
            if (!isdigit(id_text[i]))
            {
                // If not a number, show error label and return
                gtk_widget_set_visible(id_error_label, TRUE);
                return;
            }
        }

        // If it passes both checks, hide the error label
        gtk_widget_set_visible(id_error_label, FALSE);
    }

    strcpy(newEnregistrement.data1, first_name);
    strcpy(newEnregistrement.data2, second_name);
    strcpy(newEnregistrement.data3, id_text);
    
    int num_blocks = 10; // Replace with the desired number of blocks 
    Virtualdisk *ms = InitialiseMs(num_blocks); 
    if (ajouterEnregistrement(ms, &fichier, &newEnregistrement, &buffer))
    {
        printf("Enregistrement ajoute avec succes\n");
    }
    else
    {
        printf("Erreur lors de l'ajout de l'enregistrement\n");
    }

    // Hide the modal window after confirming
    gtk_widget_set_visible(modal_window, FALSE);
}

void on_button_quit_clicked(GtkButton *button, gpointer user_data)
{
    printf("Quit button was clicked\n");

    // Free any allocated resources here
    libererFichier(&fichier);

    // Use the global application reference to quit the application
    if (global_app != NULL)
    {
        g_application_quit(G_APPLICATION(global_app));
    }
}

void on_button_show_content_clicked(GtkButton *button, gpointer user_data)
{
    // Assuming user_data contains the main window's builder
    GtkBuilder *main_builder = GTK_BUILDER(user_data);

    // // Get the label widget where you want to display the content
    GtkWidget *label_content = GTK_WIDGET(gtk_builder_get_object(main_builder, "label_content"));

    // Open the TOV file for reading
    const char *nomFichier = "monFichierTOV.tov";
    FILE *fichierPhysique = fopen(nomFichier, "r");

    // Check if the file was opened successfully
    if (fichierPhysique == NULL)
    {
        printf("Error opening the TOV file for reading.\n");
        return;
    }

    // Read and display the content line by line
    char ligne[1024];
    gchar *content_text = g_strdup(""); // Initialize an empty string

    while (fgets(ligne, sizeof(ligne), fichierPhysique) != NULL)
    {
        // Append the current line to the content text
        content_text = g_strconcat(content_text, ligne, NULL);
        printf(ligne);
    }

    // Close the file
    fclose(fichierPhysique);

    // Set the content text to the label
    gtk_label_set_text(GTK_LABEL(label_content), content_text);

    // Free the allocated memory for content_text
    g_free(content_text);
}

void on_button_search_clicked(GtkButton *button, gpointer user_data)
{
    printf("seatch button created");
    // Declare the modal and confirm button locally
    GtkBuilder *builder = gtk_builder_new_from_file("design.glade");
    GtkWidget *modal_window_search = GTK_WIDGET(gtk_builder_get_object(builder, "search_modal"));
    GtkWidget *button_confirm_search = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm_search"));

    // Set parent window for the modal
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW));
    gtk_window_set_transient_for(GTK_WINDOW(modal_window_search), parent_window);

    // Set the modal window for the "Confirm" button
    g_signal_connect(button_confirm_search, "clicked", G_CALLBACK(on_button_confirm_search_clicked), builder);

    // Present the modal window using gtk_window_present
    gtk_window_present(GTK_WINDOW(modal_window_search));
}

void on_button_delete_clicked(GtkButton *button, gpointer user_data)
{
    printf("Delete button was clicked\n");

    // Declare the modal and confirm button locally
    GtkBuilder *builder = gtk_builder_new_from_file("design.glade");
    GtkWidget *modal_window_delete = GTK_WIDGET(gtk_builder_get_object(builder, "delete_modal"));
    GtkWidget *confirm_button_delete = GTK_WIDGET(gtk_builder_get_object(builder, "button_confirm_delete"));

    // Set parent window for the modal
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW));
    gtk_window_set_transient_for(GTK_WINDOW(modal_window_delete), parent_window);

    // Set the modal window for the "Confirm" button
    g_signal_connect(confirm_button_delete, "clicked", G_CALLBACK(on_button_confirm_delete_clicked), builder);

    // Present the modal window using gtk_window_present
    gtk_window_present(GTK_WINDOW(modal_window_delete));
}

void on_button_confirm_delete_clicked(GtkButton *button, gpointer user_data)
{
    // Assuming user_data contains the main window's builder
    GtkBuilder *main_builder = GTK_BUILDER(user_data);

    GtkWidget *modal_window_delete = GTK_WIDGET(gtk_builder_get_object(main_builder, "delete_modal"));
    GtkWidget *id_entry_delete = GTK_WIDGET(gtk_builder_get_object(main_builder, "id_delete_entry"));
    GtkWidget *id_error_label_delete = GTK_WIDGET(gtk_builder_get_object(main_builder, "label_id_delete_error"));

    const char *id_text_delete = gtk_entry_get_text(GTK_ENTRY(id_entry_delete));

    // Additional validation: Check if the input is a non-empty numeric string
    for (int i = 0; id_text_delete[i] != '\0'; i++)
    {
        if (!isdigit(id_text_delete[i]))
        {
            // If not a number, show error label and return
            gtk_widget_set_visible(id_error_label_delete, TRUE);
            return;
        }
    }

    // Form validation
    if (strlen(id_text_delete) == 0)
    {
        gtk_widget_set_visible(id_error_label_delete, TRUE);
        return;
    }
    else
    {
        gtk_widget_set_visible(id_error_label_delete, FALSE);
    }

    int id = atoi(id_text_delete);
    bool suppression_physique = true;
    supprimerEnregistrement(&fichier, id, &buffer, suppression_physique);

    // If it passes both checks, hide the error labels
    gtk_widget_set_visible(id_error_label_delete, FALSE);

    // Perform delete operation here

    // Hide the modal window after confirming
    gtk_widget_set_visible(modal_window_delete, FALSE);
}

void on_button_confirm_search_clicked(GtkButton *button, gpointer user_data)
{
    GtkBuilder *builder = GTK_BUILDER(user_data);
    GtkWidget *modal_window_search = GTK_WIDGET(gtk_builder_get_object(builder, "search_modal"));
    GtkWidget *id_search_entry = GTK_WIDGET(gtk_builder_get_object(builder, "id_search_entry_search"));
    GtkWidget *search_label = GTK_WIDGET(gtk_builder_get_object(builder, "search_label"));

    if (!search_label)
    {
        return;
    }
    

    // Check if search entry widget is valid
    if (!id_search_entry || !GTK_IS_EDITABLE(id_search_entry))
    {
        g_printerr("Search entry widget is not valid\n");
        return;
    }

    // Check if search label widget is valid
    if (!search_label || !GTK_IS_LABEL(search_label))
    {
        g_printerr("Search label widget is not valid\n");
        return;
    }

    const char *id_text_search = gtk_entry_get_text(GTK_ENTRY(id_search_entry));
    printf("Input ID for search: %s\n", id_text_search); // Debugging output

    int id = atoi(id_text_search);
    const char *name = "some name";  
const char *sec = "some sec"; 
    EnregistrementPhysique *foundRecord = rechercherEnregistrement(&fichier, id, name, sec);

    if (foundRecord != NULL)
    {
        // Debugging output to check the found record
        printf("Found Record: ID: %s, First name: %s, Second name: %s\n",
               foundRecord->data3,
               foundRecord->data1,
               foundRecord->data2);

        char label_text[1024];
        snprintf(label_text, sizeof(label_text), "Record Found:\nID: %s, FirstName: %s, SecondName: %s \n",
                 foundRecord->data3,
                 foundRecord->data1,
                 foundRecord->data2);
        printf("%s", label_text);
        gtk_label_set_text(GTK_LABEL(search_label), label_text);
        gtk_widget_set_visible(search_label, true);

    }
    else
    {
        gtk_label_set_text(GTK_LABEL(search_label), "ID Not Found");
    }

    gtk_widget_set_visible(modal_window_search, FALSE);
}
 int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;

    // Initialize global variables
    buffer.taille = 0; // Initialize buffer size
    // Initialize other global variables as needed

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    global_app = app;

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

