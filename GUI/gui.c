#include <gtk/gtk.h>

// === Callback Prototypes ===
static void on_quit_clicked(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // === Embedded CSS Styling ===
    const gchar *style_data =
        "* {"
        "font-family: 'Segoe UI', sans-serif;"
        "font-size: 14px;"
        "color:rgb(0, 0, 0);"
        "}"
        "window {"
        "background-color: #282a36;"
        "}"
        "frame > label {"
        "font-weight: bold;"
        "font-size: 16px;"
        "color: #bd93f9;"
        "padding: 8px;"
        "}"
        "button {"
        "background-color: #6272a4;"
        "color: #f8f8f2;"
        "border-radius: 8px;"
        "padding: 8px 16px;"
        "}"
        "button:hover {"
        "background-color: #44475a;"
        "}"
        "entry, combobox {"
        "background-color: #44475a;"
        "border-radius: 6px;"
        "padding: 6px;"
        "color: #f8f8f2;"
        "}"
        "label {"
        "color:rgb(0, 0, 0);"
        "}";

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, style_data, -1, NULL);
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduler Simulation");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // === Overview Panel ===
    GtkWidget *overview_frame = gtk_frame_new("System Overview");
    GtkWidget *overview_label = gtk_label_new("Processes: 0 | Clock: 0 | Algorithm: FCFS");
    gtk_container_add(GTK_CONTAINER(overview_frame), overview_label);
    gtk_box_pack_start(GTK_BOX(main_box), overview_frame, FALSE, FALSE, 0);

    // === Scheduler Control Panel ===
    GtkWidget *control_frame = gtk_frame_new("Scheduler Controls");
    GtkWidget *control_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *algo_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algo_combo), NULL, "FCFS");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algo_combo), NULL, "Round Robin");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algo_combo), NULL, "MLFQ");
    GtkWidget *quantum_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(quantum_entry), "Quantum");
    GtkWidget *start_btn = gtk_button_new_with_label("Start");
    GtkWidget *stop_btn = gtk_button_new_with_label("Stop");
    GtkWidget *reset_btn = gtk_button_new_with_label("Reset");
    gtk_box_pack_start(GTK_BOX(control_box), algo_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(control_box), quantum_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(control_box), start_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(control_box), stop_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(control_box), reset_btn, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(control_frame), control_box);
    gtk_box_pack_start(GTK_BOX(main_box), control_frame, FALSE, FALSE, 0);

    // === Process List ===
    GtkWidget *process_frame = gtk_frame_new("Process List");
    GtkWidget *process_list = gtk_label_new("PID | State | Priority | Memory | PC");
    gtk_container_add(GTK_CONTAINER(process_frame), process_list);
    gtk_box_pack_start(GTK_BOX(main_box), process_frame, FALSE, FALSE, 0);

    // === Queue Section ===
    GtkWidget *queue_frame = gtk_frame_new("Queue View");
    GtkWidget *queue_label = gtk_label_new("Ready Queue | Blocked Queue | Running Process");
    gtk_container_add(GTK_CONTAINER(queue_frame), queue_label);
    gtk_box_pack_start(GTK_BOX(main_box), queue_frame, FALSE, FALSE, 0);

    // === Resource Management ===
    GtkWidget *resource_frame = gtk_frame_new("Resource Management");
    GtkWidget *resource_label = gtk_label_new("Mutexes: userInput, userOutput, file | Holders & Waiters");
    gtk_container_add(GTK_CONTAINER(resource_frame), resource_label);
    gtk_box_pack_start(GTK_BOX(main_box), resource_frame, FALSE, FALSE, 0);

    // === Memory Viewer ===
    GtkWidget *memory_frame = gtk_frame_new("Memory Viewer");
    GtkWidget *memory_label = gtk_label_new("Memory Visualization (60 words)");
    gtk_container_add(GTK_CONTAINER(memory_frame), memory_label);
    gtk_box_pack_start(GTK_BOX(main_box), memory_frame, FALSE, FALSE, 0);

    // === Log & Console ===
    GtkWidget *log_frame = gtk_frame_new("Execution Log");
    GtkWidget *log_label = gtk_label_new("Instruction Log and Event Messages");
    gtk_container_add(GTK_CONTAINER(log_frame), log_label);
    gtk_box_pack_start(GTK_BOX(main_box), log_frame, FALSE, FALSE, 0);

    // === Process Creation Panel ===
    GtkWidget *creation_frame = gtk_frame_new("Process Creation");
    GtkWidget *creation_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *load_button = gtk_button_new_with_label("Add Process");
    GtkWidget *arrival_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(arrival_entry), "Arrival Time");
    gtk_box_pack_start(GTK_BOX(creation_box), load_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(creation_box), arrival_entry, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(creation_frame), creation_box);
    gtk_box_pack_start(GTK_BOX(main_box), creation_frame, FALSE, FALSE, 0);

    // === Execution Buttons ===
    GtkWidget *execution_frame = gtk_frame_new("Execution Mode");
    GtkWidget *execution_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *step_button = gtk_button_new_with_label("Step");
    GtkWidget *auto_button = gtk_button_new_with_label("Auto");
    gtk_box_pack_start(GTK_BOX(execution_box), step_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(execution_box), auto_button, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(execution_frame), execution_box);
    gtk_box_pack_start(GTK_BOX(main_box), execution_frame, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
