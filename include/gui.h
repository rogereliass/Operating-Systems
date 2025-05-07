#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

// Global variables
extern GtkWidget *log_view;
extern GtkWidget *window;

// Function declarations
GtkWidget* init_gui(int argc, char *argv[]);
void run_gui();
void log_message(const char *message);
void on_choose_scheduler(GtkButton *button, gpointer user_data);
void on_add_process(GtkButton *button, gpointer user_data);
void on_simulation_step(GtkButton *button, gpointer user_data);
void on_auto_run_toggle(GtkToggleButton *toggle, gpointer user_data);
void on_reset(GtkButton *button, gpointer user_data);
void on_exit_app(GtkButton *button, gpointer user_data);
gboolean auto_run_callback(gpointer data);

#endif // GUI_H 