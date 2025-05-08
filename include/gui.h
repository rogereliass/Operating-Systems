#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

// Global variables
extern GtkWidget *log_view;
extern GtkWidget *window;
extern GtkWidget *grid_view;
extern GtkWidget *resource_panel;
extern GtkWidget *memory_viewer;

// Function declarations
GtkWidget* init_gui(int argc, char *argv[]);
void run_gui();
void log_message(const char *message);
void update_grid_display();
void update_resource_panel(GtkWidget *panel);
void update_memory_viewer(GtkWidget *viewer);
void on_choose_scheduler(GtkButton *button, gpointer user_data);
void on_scheduler_combo_changed(GtkComboBox *combo, gpointer user_data);
void on_add_process(GtkButton *button, gpointer user_data);
void on_simulation_step(GtkButton *button, gpointer user_data);
void on_auto_run_toggle(GtkToggleButton *toggle, gpointer user_data);
void on_reset(GtkButton *button, gpointer user_data);
void on_exit_app(GtkButton *button, gpointer user_data);
gboolean auto_run_callback(gpointer data);
void cell_background_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer, 
                         GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data);

// Function to get program input through GUI
int get_program_input(int pid);

// Function to check if input has been received
int is_input_received();

// Function to check if the input was text
int is_program_text_input();

// Function to get the text input
const char* get_program_text_input();

// Function to get file content through GUI
char* get_file_content(const char* filename);

// Function to write content to a file through GUI
int write_file_content(const char* filename, const char* content);

#endif // GUI_H 