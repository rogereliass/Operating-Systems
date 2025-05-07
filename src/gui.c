#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/os.h"
#include "../include/memory.h"
#include "../include/parser.h"
#include "../include/semaphore.h"
#include "../include/scheduler_interface.h"
#include "../include/fcfs_scheduler.h"
#include "../include/round_robin_scheduler.h"
#include "../include/mlfq_scheduler.h"

// External variables
extern Scheduler* scheduler;
extern pcb_t processes[3];
extern int num_processes;
extern int clock_tick;
extern int simulation_running;
extern int auto_mode;

// Function declarations
extern void choose_scheduler();
extern void load_program();
extern void simulation_step();
extern void add_process();
extern void sem_init_all();
extern void mem_init();

GtkWidget *log_view;
GtkWidget *window;
gboolean auto_run_callback(gpointer data);

void log_message(const char *message) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, message, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
}

void on_choose_scheduler(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Choose Scheduler",
        GTK_WINDOW(user_data), GTK_DIALOG_MODAL,
        "FCFS", 1, "Round Robin", 2, "MLFQ", 3, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Quantum (for RR only)");
    gtk_box_pack_start(GTK_BOX(content), entry, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    const gchar *quantum_text = gtk_entry_get_text(GTK_ENTRY(entry));
    int quantum = atoi(quantum_text);
    gtk_widget_destroy(dialog);

    switch (result) {
        case 1:
            scheduler = create_fcfs_scheduler();
            log_message("Scheduler: FCFS selected.");
            break;
        case 2:
            scheduler = create_rr_scheduler(quantum);
            log_message("Scheduler: Round Robin selected.");
            break;
        case 3:
            scheduler = create_mlfq_scheduler();
            log_message("Scheduler: MLFQ selected.");
            break;
        default:
            log_message("Scheduler selection canceled.");
            break;
    }
}

void on_add_process(GtkButton *button, gpointer user_data) {
    add_process();
    log_message("Programs loaded successfully.");
}

void on_simulation_step(GtkButton *button, gpointer user_data) {
    simulation_running = 1;
    load_program();
    simulation_step();
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Clock Tick: %d", clock_tick);
    log_message(buffer);
}

void on_auto_run_toggle(GtkToggleButton *toggle, gpointer user_data) {
    auto_mode = gtk_toggle_button_get_active(toggle);
    if (auto_mode) {
        // Check if we have processes and a scheduler
        if (num_processes == 0) {
            log_message("Error: No processes added. Please add processes first.");
            gtk_toggle_button_set_active(toggle, FALSE);
            auto_mode = 0;
            return;
        }
        if (scheduler == NULL) {
            log_message("Error: No scheduler selected. Please choose a scheduler first.");
            gtk_toggle_button_set_active(toggle, FALSE);
            auto_mode = 0;
            return;
        }
        
        // Reset simulation state
        simulation_running = 1;
        clock_tick = 0;
        log_message("Auto-run started.");
        g_timeout_add(1000, auto_run_callback, NULL);
    } else {
        log_message("Auto-run stopped.");
    }
}

gboolean auto_run_callback(gpointer data) {
    if (auto_mode && simulation_running) {
        load_program();
        simulation_step();
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Auto Clock Tick: %d", clock_tick);
        log_message(buffer);
        return TRUE;  // Keep the timeout active
    }
    return FALSE;  // Remove the timeout
}

void on_reset(GtkButton *button, gpointer user_data) {
    if (scheduler) scheduler->destroy(scheduler);
    mem_init();
    sem_init_all();
    clock_tick = 0;
    simulation_running = 0;
    auto_mode = 0;
    log_message("System reset.");
}

void on_exit_app(GtkButton *button, gpointer user_data) {
    if (scheduler) scheduler->destroy(scheduler);
    gtk_main_quit();
}

// Initialize GUI and return the window widget
GtkWidget* init_gui(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduler Simulation");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    GtkWidget *btn_scheduler = gtk_button_new_with_label("Choose Scheduler");
    GtkWidget *btn_load = gtk_button_new_with_label("Add Process");
    GtkWidget *btn_step = gtk_button_new_with_label("Step");
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_exit = gtk_button_new_with_label("Exit");
    GtkWidget *toggle_auto = gtk_toggle_button_new_with_label("Auto Run");

    gtk_box_pack_start(GTK_BOX(button_box), btn_scheduler, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_load, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_step, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), toggle_auto, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_reset, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_exit, TRUE, TRUE, 0);

    log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), log_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    g_signal_connect(btn_scheduler, "clicked", G_CALLBACK(on_choose_scheduler), window);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_add_process), NULL);
    g_signal_connect(btn_step, "clicked", G_CALLBACK(on_simulation_step), NULL);
    g_signal_connect(toggle_auto, "toggled", G_CALLBACK(on_auto_run_toggle), NULL);
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset), NULL);
    g_signal_connect(btn_exit, "clicked", G_CALLBACK(on_exit_app), NULL);

    gtk_widget_show_all(window);
    return window;
}

// Function to run the main GUI loop
void run_gui() {
    gtk_main();
}
