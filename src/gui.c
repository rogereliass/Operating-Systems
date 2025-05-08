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

#define MAX_SEMAPHORES 3  // Define the maximum number of semaphores
#define GRID_ROWS 20  // Increased rows to show more information
#define GRID_COLS 4
#define MAX_HISTORY 100  // Maximum number of history entries to keep
#define MEMORY_SIZE 60

// External declarations for memory
extern mem_word_t memory_pool[MAX_MEM_WORDS];

// Structure to store process state history
typedef struct {
    int time;
    int pid;
    char state[20];
    int priority;
    int pc;
} process_history_t;

// External variables
extern Scheduler* scheduler;
extern pcb_t processes[3];
extern int num_processes;
extern int clock_tick;
extern int simulation_running;
extern int auto_mode;

// Global variables for history tracking
static process_history_t process_history[MAX_HISTORY];
static int history_count = 0;

// Function declarations
extern void choose_scheduler();
extern void load_program();
extern void simulation_step();
extern void add_process();
extern void sem_init_all();
extern void mem_init();
extern void get_resource_status(resource_status_t* status_array, int* num_resources);
void update_grid_display();  // Added function declaration
void cell_background_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer, 
                         GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data);
void on_scheduler_combo_changed(GtkComboBox *combo, gpointer user_data);

// Global widgets
GtkWidget *window;
GtkWidget *overview_box;
GtkWidget *overview_label;  // Added for overview text
GtkWidget *process_list;
GtkWidget *process_tree_view;  // Added for tree view
GtkListStore *process_store;  // Changed from GtkWidget* to GtkListStore*
GtkWidget *queue_section;
GtkWidget *resource_panel;
GtkWidget *memory_viewer;
GtkWidget *log_view;
GtkWidget *btn_stop;
GtkWidget *btn_start;
GtkWidget *grid_view;  // New grid view widget
GtkWidget *grid_labels[GRID_ROWS][GRID_COLS];  // Array to store grid labels
gboolean auto_run_callback(gpointer data);

// Global variable to store the input value
static int program_input_value = 0;
static int input_received = 0;

// Global variable to store the text input value
static char program_text_input[256] = "";
static int is_text_input = 0;

void log_message(const char *message) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, message, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
}

void on_choose_scheduler(GtkButton *button, gpointer user_data) {
    // Create a dialog for scheduler selection
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Choose Scheduler",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    // Create a vertical box for the dialog content
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    // Add instructions label
    GtkWidget *label = gtk_label_new("Select scheduler type:");
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    // Create the dropdown menu
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "FCFS");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Round Robin");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "MLFQ");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0); // Default to FCFS
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 5);
    
    // Add quantum entry for Round Robin
    GtkWidget *quantum_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *quantum_label = gtk_label_new("Quantum (for RR):");
    GtkWidget *quantum_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(quantum_entry), "2"); // Default value
    
    gtk_box_pack_start(GTK_BOX(quantum_box), quantum_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quantum_box), quantum_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), quantum_box, FALSE, FALSE, 5);
    
    // Initially hide the quantum box since FCFS is selected by default
    gtk_widget_set_sensitive(quantum_box, FALSE);
    
    // Connect signal to show/hide quantum box based on selection
    g_signal_connect(combo, "changed", G_CALLBACK(on_scheduler_combo_changed), quantum_box);
    
    gtk_widget_show_all(dialog);

    // Run the dialog
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        // Get selected scheduler type
        int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
        const gchar *quantum_text = gtk_entry_get_text(GTK_ENTRY(quantum_entry));
        int quantum = atoi(quantum_text);
        
        // Create the appropriate scheduler
        switch (selected) {
            case 0: // FCFS
                scheduler = create_fcfs_scheduler();
                log_message("Scheduler: FCFS selected.");
                break;
            case 1: // Round Robin
                scheduler = create_rr_scheduler(quantum);
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "Scheduler: Round Robin selected with quantum %d.", quantum);
                log_message(buffer);
                break;
            case 2: // MLFQ
                scheduler = create_mlfq_scheduler();
                log_message("Scheduler: MLFQ selected.");
                break;
            default:
                log_message("No scheduler selected.");
                break;
        }
    } else {
        log_message("Scheduler selection canceled.");
    }
    
    gtk_widget_destroy(dialog);
}

// Handler for when scheduler combo box selection changes
void on_scheduler_combo_changed(GtkComboBox *combo, gpointer user_data) {
    GtkWidget *quantum_box = GTK_WIDGET(user_data);
    int selected = gtk_combo_box_get_active(combo);
    
    // Enable quantum input only for Round Robin (selection index 1)
    if (selected == 1) {
        gtk_widget_set_sensitive(quantum_box, TRUE);
    } else {
        gtk_widget_set_sensitive(quantum_box, FALSE);
    }
}

void on_add_process(GtkButton *button, gpointer user_data) {
    // Create a dialog for program filename
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Process",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    // Create a vertical box for the dialog content
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    // Add program selection dropdown
    GtkWidget *program_label = gtk_label_new("Select program:");
    GtkWidget *program_combo = gtk_combo_box_text_new();
    
    // Add programs to the dropdown with programs/ prefix
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(program_combo), "programs/Program_1.txt");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(program_combo), "programs/Program_2.txt");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(program_combo), "programs/Program_3.txt");
    
    // Set the first program as default
    gtk_combo_box_set_active(GTK_COMBO_BOX(program_combo), 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), program_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), program_combo, TRUE, TRUE, 0);

    // Add arrival time entry
    GtkWidget *time_label = gtk_label_new("Enter arrival time (clock cycles):");
    GtkWidget *time_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), time_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), time_entry, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    // Run the dialog
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const gchar *filename = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(program_combo));
        const gchar *time_str = gtk_entry_get_text(GTK_ENTRY(time_entry));
        int arrival_time = atoi(time_str);

        // Create a temporary file to store the input
        FILE* temp_file = fopen("temp_input.txt", "w");
        if (temp_file) {
            fprintf(temp_file, "%s\n%d", filename, arrival_time);
            fclose(temp_file);
            
            // Call add_process with the temporary file
            add_process();
            log_message("Program loaded successfully.");
            
            // Remove the temporary file
            remove("temp_input.txt");
        } else {
            log_message("Error: Could not create temporary file.");
        }
    }

    gtk_widget_destroy(dialog);
}

void on_simulation_step(GtkButton *button, gpointer user_data) {
    simulation_running = 1;
    simulation_step();
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Clock Tick: %d", clock_tick);
    log_message(buffer);
    update_grid_display();  // Update grid after each step
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
        update_grid_display();  // Update grid during auto-run
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
    history_count = 0;  // Reset history
    log_message("System reset.");
    update_grid_display();  // Update grid after reset
}

void on_start(GtkButton *button, gpointer user_data) {
    if (!auto_mode && simulation_running == 0) {
        // Resume the simulation
        auto_mode = 1;
        simulation_running = 1;
        gtk_button_set_label(GTK_BUTTON(btn_stop), "Stop");
        log_message("Simulation resumed.");
        g_timeout_add(1000, auto_run_callback, NULL);
    }
}

void on_stop(GtkButton *button, gpointer user_data) {
    if (auto_mode) {
        // If auto mode is running, stop it
        auto_mode = 0;
        simulation_running = 0;
        gtk_button_set_label(GTK_BUTTON(button), "Exit");
        log_message("Simulation paused.");
    } else {
        // If already stopped, exit the application
        if (scheduler) scheduler->destroy(scheduler);
        gtk_main_quit();
    }
}

void update_resource_panel(GtkWidget* panel) {
    resource_status_t status[MAX_SEMAPHORES];
    int num_resources;
    
    // Get current resource status
    get_resource_status(status, &num_resources);
    
    // Clear existing widgets in the panel
    GList *children = gtk_container_get_children(GTK_CONTAINER(panel));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);
    
    // Create a vertical box to hold all resource information
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(panel), vbox);
    
    // Add a title label
    GtkWidget *title_label = gtk_label_new("Resource Management");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);
    
    // Display all resources (we should have exactly 3)
    for (int i = 0; i < num_resources; i++) {
        // Create frame for this resource
        GtkWidget *frame = gtk_frame_new(status[i].name);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 5);
        
        // Create a box inside the frame
        GtkWidget *resource_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_container_set_border_width(GTK_CONTAINER(resource_box), 5);
        gtk_container_add(GTK_CONTAINER(frame), resource_box);
        
        // Add status label
        char status_text[256];
        snprintf(status_text, sizeof(status_text), 
                "Status: %s", 
                status[i].value ? "Free" : "Locked");
        GtkWidget *status_label = gtk_label_new(status_text);
        gtk_label_set_xalign(GTK_LABEL(status_label), 0);
        gtk_box_pack_start(GTK_BOX(resource_box), status_label, FALSE, FALSE, 0);
        
        // Add holder label
        char holder_text[256];
        if (status[i].current_holder != -1) {
            snprintf(holder_text, sizeof(holder_text), "Current Holder: PID %d", status[i].current_holder);
        } else {
            snprintf(holder_text, sizeof(holder_text), "Current Holder: None");
        }
        GtkWidget *holder_label = gtk_label_new(holder_text);
        gtk_label_set_xalign(GTK_LABEL(holder_label), 0);
        gtk_box_pack_start(GTK_BOX(resource_box), holder_label, FALSE, FALSE, 0);
        
        // Add waiting queue label
        char queue_text[256] = "Waiting Queue: ";
        if (status[i].queue_size > 0) {
            char pid_str[16];
            for (int j = 0; j < status[i].queue_size; j++) {
                if (j > 0) strcat(queue_text, ", ");
                snprintf(pid_str, sizeof(pid_str), "PID %d", status[i].waiting_pids[j]);
                strcat(queue_text, pid_str);
            }
        } else {
            strcat(queue_text, "Empty");
        }
        GtkWidget *queue_label = gtk_label_new(queue_text);
        gtk_label_set_xalign(GTK_LABEL(queue_label), 0);
        gtk_box_pack_start(GTK_BOX(resource_box), queue_label, FALSE, FALSE, 0);
    }
    
    // Free allocated memory for waiting PIDs
    for (int i = 0; i < num_resources; i++) {
        if (status[i].waiting_pids) {
            free(status[i].waiting_pids);
        }
    }
    
    // Show all widgets
    gtk_widget_show_all(panel);
}

// Function to get program input through GUI
int get_program_input(int pid) {
    input_received = 0;
    program_input_value = 0;
    is_text_input = 0;  // Reset text input flag
    memset(program_text_input, 0, sizeof(program_text_input));  // Clear text input buffer
    
    // Create a dialog for program input
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Program Input",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    // Create a vertical box for the dialog content
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    // Add input label and entry
    char label_text[100];
    snprintf(label_text, sizeof(label_text), "Enter value for Process %d:", pid);
    GtkWidget *input_label = gtk_label_new(label_text);
    GtkWidget *input_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), input_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), input_entry, TRUE, TRUE, 0);
    
    // Add information about text input
    GtkWidget *info_label = gtk_label_new("Note: Enter a number or text (filenames, etc.)");
    gtk_widget_set_margin_top(info_label, 5);
    gtk_box_pack_start(GTK_BOX(vbox), info_label, FALSE, FALSE, 0);

    gtk_widget_show_all(dialog);

    // Run the dialog
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const gchar *input_text = gtk_entry_get_text(GTK_ENTRY(input_entry));
        
        // Check if the input is a number or text
        char *endptr;
        long value = strtol(input_text, &endptr, 10);
        
        if (*endptr == '\0' && endptr != input_text) {
            // It's a valid number
            program_input_value = (int)value;
            is_text_input = 0;
        } else {
            // It's text
            strncpy(program_text_input, input_text, sizeof(program_text_input) - 1);
            program_text_input[sizeof(program_text_input) - 1] = '\0';
            is_text_input = 1;
            program_input_value = 0;  // Reset numeric value
        }
        
        input_received = 1;
    }

    gtk_widget_destroy(dialog);
    return program_input_value;
}

// Function to check if the input was text
int is_program_text_input() {
    return is_text_input;
}

// Function to get the text input
const char* get_program_text_input() {
    return program_text_input;
}

// Function to check if input has been received
int is_input_received() {
    return input_received;
}

// Function to add current state to history
void add_to_history() {
    if (history_count >= MAX_HISTORY) {
        // Shift all entries down by one
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            process_history[i] = process_history[i + 1];
        }
        history_count--;
    }
    
    // Add current state for each process
    for (int i = 0; i < num_processes; i++) {
        process_history[history_count].time = clock_tick;
        process_history[history_count].pid = processes[i].pid;
        strncpy(process_history[history_count].state, 
                state_type_to_string(processes[i].state), 
                sizeof(process_history[history_count].state) - 1);
        process_history[history_count].priority = processes[i].priority;
        process_history[history_count].pc = processes[i].pc;
        history_count++;
    }
}

// Function to get scheduler name
const char* get_scheduler_name() {
    if (!scheduler) return "None";
    
    switch (scheduler->type) {
        case SCHEDULER_FCFS:
            return "FCFS";
        case SCHEDULER_RR:
            return "Round Robin";
        case SCHEDULER_MLFQ:
            return "MLFQ";
        default:
            return "Unknown";
    }
}

// Function to create the overview section
GtkWidget* create_overview_section() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *label = gtk_label_new("System Overview");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    
    // Create overview info label
    overview_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(box), overview_label, FALSE, FALSE, 0);
    
    return box;
}

// Function to create the process list section
GtkWidget* create_process_list() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *label = gtk_label_new("Process List");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    
    // Create a tree view for processes
    process_tree_view = gtk_tree_view_new();
    process_store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, 
                                     G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, 
                                     G_TYPE_INT);
    gtk_tree_view_set_model(GTK_TREE_VIEW(process_tree_view), GTK_TREE_MODEL(process_store));
    
    // Add columns
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_tree_view), 
                               gtk_tree_view_column_new_with_attributes("PID", 
                               gtk_cell_renderer_text_new(), "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_tree_view), 
                               gtk_tree_view_column_new_with_attributes("State", 
                               gtk_cell_renderer_text_new(), "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_tree_view), 
                               gtk_tree_view_column_new_with_attributes("Priority", 
                               gtk_cell_renderer_text_new(), "text", 2, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_tree_view), 
                               gtk_tree_view_column_new_with_attributes("Mem Low", 
                               gtk_cell_renderer_text_new(), "text", 3, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_tree_view), 
                               gtk_tree_view_column_new_with_attributes("Mem High", 
                               gtk_cell_renderer_text_new(), "text", 4, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_tree_view), 
                               gtk_tree_view_column_new_with_attributes("PC", 
                               gtk_cell_renderer_text_new(), "text", 5, NULL));
    
    gtk_box_pack_start(GTK_BOX(box), process_tree_view, TRUE, TRUE, 0);
    return box;
}

// Function to create the queue section
GtkWidget* create_queue_section() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    
    // Add title
    GtkWidget *label = gtk_label_new("Queue Status");
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_widget_set_margin_bottom(label, 10);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    
    // Create sections for Ready, Running, and Blocked queues
    GtkWidget *ready_frame = gtk_frame_new("Ready Queue");
    GtkWidget *running_frame = gtk_frame_new("Running Process");
    GtkWidget *blocked_frame = gtk_frame_new("Blocked Queue");
    
    // Set shadow type for frames
    gtk_frame_set_shadow_type(GTK_FRAME(ready_frame), GTK_SHADOW_ETCHED_IN);
    gtk_frame_set_shadow_type(GTK_FRAME(running_frame), GTK_SHADOW_ETCHED_IN);
    gtk_frame_set_shadow_type(GTK_FRAME(blocked_frame), GTK_SHADOW_ETCHED_IN);
    
    // Create boxes inside frames
    GtkWidget *ready_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *running_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *blocked_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    gtk_container_set_border_width(GTK_CONTAINER(ready_box), 5);
    gtk_container_set_border_width(GTK_CONTAINER(running_box), 5);
    gtk_container_set_border_width(GTK_CONTAINER(blocked_box), 5);
    
    gtk_container_add(GTK_CONTAINER(ready_frame), ready_box);
    gtk_container_add(GTK_CONTAINER(running_frame), running_box);
    gtk_container_add(GTK_CONTAINER(blocked_frame), blocked_box);
    
    // Add labels for each queue
    GtkWidget *ready_label = gtk_label_new("Empty");
    GtkWidget *running_label = gtk_label_new("Empty");
    GtkWidget *blocked_label = gtk_label_new("Empty");
    
    gtk_label_set_xalign(GTK_LABEL(ready_label), 0);
    gtk_label_set_xalign(GTK_LABEL(running_label), 0);
    gtk_label_set_xalign(GTK_LABEL(blocked_label), 0);
    
    gtk_box_pack_start(GTK_BOX(ready_box), ready_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(running_box), running_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(blocked_box), blocked_label, FALSE, FALSE, 0);
    
    // Pack frames into main box
    gtk_box_pack_start(GTK_BOX(box), ready_frame, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), running_frame, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), blocked_frame, FALSE, FALSE, 5);
    
    // Store labels for later updates
    g_object_set_data(G_OBJECT(box), "ready_label", ready_label);
    g_object_set_data(G_OBJECT(box), "running_label", running_label);
    g_object_set_data(G_OBJECT(box), "blocked_label", blocked_label);
    
    return box;
}

// Add this function to update the queue display
void update_queue_display(GtkWidget* queue_section) {
    GtkWidget *ready_label = g_object_get_data(G_OBJECT(queue_section), "ready_label");
    GtkWidget *running_label = g_object_get_data(G_OBJECT(queue_section), "running_label");
    GtkWidget *blocked_label = g_object_get_data(G_OBJECT(queue_section), "blocked_label");
    
    char ready_text[256] = "";
    char running_text[256] = "";
    char blocked_text[256] = "";
    
    // Get processes from scheduler
    if (scheduler) {
        // Get ready queue
        pcb_t* ready_pcb = scheduler->queue(scheduler);
        if (ready_pcb) {
            snprintf(ready_text, sizeof(ready_text), "PID %d", ready_pcb->pid);
        }
        
        // Get running process
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].state == RUNNING) {
                snprintf(running_text, sizeof(running_text), "PID %d", processes[i].pid);
                break;
            }
        }
        
        // Get blocked queue
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].state == BLOCKED) {
                if (strlen(blocked_text) > 0) strcat(blocked_text, ", ");
                char pid_str[16];
                snprintf(pid_str, sizeof(pid_str), "PID %d", processes[i].pid);
                strcat(blocked_text, pid_str);
            }
        }
    }
    
    // Update labels
    gtk_label_set_text(GTK_LABEL(ready_label), strlen(ready_text) > 0 ? ready_text : "Empty");
    gtk_label_set_text(GTK_LABEL(running_label), strlen(running_text) > 0 ? running_text : "Empty");
    gtk_label_set_text(GTK_LABEL(blocked_label), strlen(blocked_text) > 0 ? blocked_text : "Empty");
}

// Function to create the resource management panel
GtkWidget* create_resource_panel() {
    // Create a scrolled window to contain the resource panel
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    
    // Create the main box for the resource panel with padding
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    gtk_container_add(GTK_CONTAINER(scrolled), box);
    
    // Create frames for each resource
    const char *resource_names[] = {
        RESOURCE_USER_INPUT,
        RESOURCE_USER_OUTPUT,
        RESOURCE_FILE
    };
    
    // Create frames for all resources
    for (int i = 0; i < 3; i++) {
        // Create frame with border and padding
        GtkWidget *frame = gtk_frame_new(resource_names[i]);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        gtk_widget_set_margin_bottom(frame, 5);
        gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE, 0);
        
        // Create a box inside the frame with padding
        GtkWidget *resource_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_container_set_border_width(GTK_CONTAINER(resource_box), 5);
        gtk_container_add(GTK_CONTAINER(frame), resource_box);
        
        // Add status label
        GtkWidget *status_label = gtk_label_new("Status: Not Initialized");
        gtk_label_set_xalign(GTK_LABEL(status_label), 0);
        gtk_widget_set_margin_bottom(status_label, 2);
        gtk_box_pack_start(GTK_BOX(resource_box), status_label, FALSE, FALSE, 0);
        
        // Add holder label
        GtkWidget *holder_label = gtk_label_new("Current Holder: None");
        gtk_label_set_xalign(GTK_LABEL(holder_label), 0);
        gtk_widget_set_margin_bottom(holder_label, 2);
        gtk_box_pack_start(GTK_BOX(resource_box), holder_label, FALSE, FALSE, 0);
        
        // Add waiting queue label
        GtkWidget *queue_label = gtk_label_new("Waiting Queue: Empty");
        gtk_label_set_xalign(GTK_LABEL(queue_label), 0);
        gtk_widget_set_margin_bottom(queue_label, 2);
        gtk_box_pack_start(GTK_BOX(resource_box), queue_label, FALSE, FALSE, 0);
    }
    
    // Show all widgets
    gtk_widget_show_all(scrolled);
    
    return scrolled;
}

// Function to create the memory viewer
GtkWidget* create_memory_viewer() {
    // Create a scrolled window to contain the memory viewer
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    
    // Create a box for the memory viewer
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    gtk_container_add(GTK_CONTAINER(scrolled), box);
    
    // Create a list store for memory content
    // Columns: Index, Name, Value, Status (Used/Free)
    GtkListStore *store = gtk_list_store_new(4, 
                                          G_TYPE_INT,       // Index
                                          G_TYPE_STRING,    // Name
                                          G_TYPE_STRING,    // Value
                                          G_TYPE_BOOLEAN);  // Used status
    
    // Create tree view
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Create text renderer for all columns
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "height", 30, "xpad", 10, "ypad", 6, NULL);  // Increased size
    
    // Create cell renderer for the "used" status (colored background)
    GtkCellRenderer *renderer_used = gtk_cell_renderer_text_new();
    g_object_set(renderer_used, "height", 30, "xpad", 10, "ypad", 6, NULL);  // Increased size
    
    // Add columns to the tree view
    GtkTreeViewColumn *index_column = gtk_tree_view_column_new_with_attributes(
        "Index", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), index_column);
    
    GtkTreeViewColumn *name_column = gtk_tree_view_column_new_with_attributes(
        "Name", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_min_width(name_column, 150);  // Reduced from 200
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), name_column);
    
    GtkTreeViewColumn *value_column = gtk_tree_view_column_new_with_attributes(
        "Value", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_min_width(value_column, 150);  // Reduced from 200
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), value_column);
    
    GtkTreeViewColumn *status_column = gtk_tree_view_column_new_with_attributes(
        "Status", renderer_used, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), status_column);
    
    // Add cell data function to set background color based on usage status
    gtk_tree_view_column_set_cell_data_func(name_column, renderer,
        (GtkTreeCellDataFunc) cell_background_func, GINT_TO_POINTER(3), NULL);
    gtk_tree_view_column_set_cell_data_func(value_column, renderer,
        (GtkTreeCellDataFunc) cell_background_func, GINT_TO_POINTER(3), NULL);
    gtk_tree_view_column_set_cell_data_func(index_column, renderer,
        (GtkTreeCellDataFunc) cell_background_func, GINT_TO_POINTER(3), NULL);
    gtk_tree_view_column_set_cell_data_func(status_column, renderer_used,
        (GtkTreeCellDataFunc) cell_background_func, GINT_TO_POINTER(3), NULL);
    
    // Initialize tree view with empty data
    for (int i = 0; i < MAX_MEM_WORDS; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, i,                // Index
                          1, "",               // Name (empty)
                          2, "",               // Value (empty)
                          3, FALSE,            // Not used
                          -1);
    }
    
    // Store the list store for later updates
    g_object_set_data(G_OBJECT(scrolled), "memory_store", store);
    
    gtk_box_pack_start(GTK_BOX(box), tree_view, TRUE, TRUE, 0);
    
    // Show all widgets
    gtk_widget_show_all(scrolled);
    
    return scrolled;
}

// Cell data function for setting background color based on usage status
void cell_background_func(GtkTreeViewColumn *col,
                         GtkCellRenderer *renderer,
                         GtkTreeModel *model,
                         GtkTreeIter *iter,
                         gpointer user_data) {
    gboolean used;
    gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &used, -1);
    
    // Set background color based on used status
    if (used) {
        g_object_set(renderer, "background", "#e6f2ff", NULL);  // Light blue for used cells
    } else {
        g_object_set(renderer, "background", "white", NULL);    // White for free cells
    }
    
    // If this is for the status column, set the text
    if (col == gtk_tree_view_get_column(GTK_TREE_VIEW(gtk_tree_view_column_get_tree_view(col)), 3)) {
        g_object_set(renderer, "text", used ? "Used" : "Free", NULL);
    }
}

// Function to update the memory viewer with the current memory_pool data
void update_memory_viewer(GtkWidget *viewer) {
    // Get the memory store
    GtkListStore *store = g_object_get_data(G_OBJECT(viewer), "memory_store");
    if (!store) return;
    
    // Update each row with memory_pool data
    for (int i = 0; i < MAX_MEM_WORDS; i++) {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, i)) {
            // Check if memory cell has data
            gboolean used = (memory_pool[i].name[0] != '\0');
            
            // Update the row
            gtk_list_store_set(store, &iter,
                              1, used ? memory_pool[i].name : "",    // Name
                              2, used ? memory_pool[i].value : "",   // Value
                              3, used,                               // Used status
                              -1);
        }
    }
}

// Function to update the display
void update_grid_display() {
    char buffer[256];
    
    // Update Overview Section
    snprintf(buffer, sizeof(buffer), 
             "Total Processes: %d\n"
             "Current Clock: %d\n"
             "Scheduler: %s",
             num_processes,
             clock_tick,
             get_scheduler_name());
    gtk_label_set_text(GTK_LABEL(overview_label), buffer);
    
    // Update Process List
    gtk_list_store_clear(GTK_LIST_STORE(process_store));
    
    for (int i = 0; i < num_processes; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(GTK_LIST_STORE(process_store), &iter);
        gtk_list_store_set(GTK_LIST_STORE(process_store), &iter,
                          0, processes[i].pid,
                          1, state_type_to_string(processes[i].state),
                          2, processes[i].priority,
                          3, processes[i].mem_low,
                          4, processes[i].mem_high,
                          5, processes[i].pc,
                          -1);
    }
    
    // Update Queue Section
    update_queue_display(queue_section);
    
    // Update Resource Panel
    update_resource_panel(resource_panel);
    
    // Update Memory Viewer
    update_memory_viewer(memory_viewer);
}

// Initialize GUI and return the window widget
GtkWidget* init_gui(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduler Simulation");
    gtk_window_set_default_size(GTK_WINDOW(window), 1600, 1100);  // Increase window size
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Add CSS styling for memory cells and general GUI elements
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        ".memory-used { background-color: #e6f2ff; }"  // Light blue background for used cells
        ".memory-header { font-weight: bold; }"
        "frame { margin: 5px; }"
        "label { margin: 2px; }", -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Create main container with spacing
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);  // Increased spacing
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 15);  // Increased border
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Create a paned container for top and bottom sections
    GtkWidget *main_paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(main_box), main_paned, TRUE, TRUE, 0);
    
    // Create a box for the top section (Overview, Process List, Queue, Resources)
    GtkWidget *top_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);  // Increased spacing
    gtk_paned_add1(GTK_PANED(main_paned), top_section);
    
    // Create the first row (Overview and Process List)
    GtkWidget *top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);  // Increased spacing
    gtk_box_pack_start(GTK_BOX(top_section), top_row, FALSE, FALSE, 0);
    
    // Create overview section with a frame
    GtkWidget *overview_frame = gtk_frame_new("System Overview");
    gtk_frame_set_shadow_type(GTK_FRAME(overview_frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_margin_start(overview_frame, 5);  // Added margin
    gtk_widget_set_margin_end(overview_frame, 5);  // Added margin
    gtk_box_pack_start(GTK_BOX(top_row), overview_frame, FALSE, FALSE, 0);
    
    overview_box = create_overview_section();
    gtk_container_add(GTK_CONTAINER(overview_frame), overview_box);
    
    // Create process list with a frame
    GtkWidget *process_frame = gtk_frame_new("Process List");
    gtk_frame_set_shadow_type(GTK_FRAME(process_frame), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(top_row), process_frame, TRUE, TRUE, 0);
    
    process_list = create_process_list();
    gtk_container_add(GTK_CONTAINER(process_frame), process_list);

    // Create the second row (Queue and Resource panels)
    GtkWidget *middle_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);  // Increased spacing
    gtk_box_pack_start(GTK_BOX(top_section), middle_row, TRUE, TRUE, 0);
    
    // Create queue section with a frame
    GtkWidget *queue_frame = gtk_frame_new("Queue Status");
    gtk_frame_set_shadow_type(GTK_FRAME(queue_frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_margin_start(queue_frame, 5);  // Added margin
    gtk_widget_set_margin_end(queue_frame, 5);  // Added margin
    gtk_box_pack_start(GTK_BOX(middle_row), queue_frame, TRUE, TRUE, 0);
    
    queue_section = create_queue_section();
    gtk_container_add(GTK_CONTAINER(queue_frame), queue_section);
    
    // Create resource panel with a frame
    GtkWidget *resource_frame = gtk_frame_new("Resource Management");
    gtk_frame_set_shadow_type(GTK_FRAME(resource_frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_margin_start(resource_frame, 5);  // Added margin
    gtk_widget_set_margin_end(resource_frame, 5);  // Added margin
    gtk_box_pack_start(GTK_BOX(middle_row), resource_frame, TRUE, TRUE, 0);
    
    resource_panel = create_resource_panel();
    gtk_container_add(GTK_CONTAINER(resource_frame), resource_panel);

    // Create a horizontal paned container for the bottom section
    GtkWidget *bottom_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_add2(GTK_PANED(main_paned), bottom_paned);
    
    // Create memory viewer with a frame
    GtkWidget *memory_frame = gtk_frame_new("Memory Viewer");
    gtk_frame_set_shadow_type(GTK_FRAME(memory_frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_margin_start(memory_frame, 5);  // Added margin
    gtk_widget_set_margin_end(memory_frame, 5);  // Added margin
    gtk_paned_add1(GTK_PANED(bottom_paned), memory_frame);
    
    memory_viewer = create_memory_viewer();
    gtk_container_add(GTK_CONTAINER(memory_frame), memory_viewer);
    
    // Create log view with a frame
    GtkWidget *log_frame = gtk_frame_new("System Log");
    gtk_frame_set_shadow_type(GTK_FRAME(log_frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_margin_start(log_frame, 5);  // Added margin
    gtk_widget_set_margin_end(log_frame, 5);  // Added margin
    gtk_paned_add2(GTK_PANED(bottom_paned), log_frame);
    
    // Create scrolled window for log
    GtkWidget *log_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(log_scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(log_frame), log_scroll);
    
    // Create log view
    log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    gtk_container_add(GTK_CONTAINER(log_scroll), log_view);
    
    // Set initial position for the main paned divider (30% of height to top section)
    gtk_paned_set_position(GTK_PANED(main_paned), 400);
    
    // Set position for bottom paned divider (60% to memory viewer, 40% to log)
    gtk_paned_set_position(GTK_PANED(bottom_paned), 960);

    // Create control buttons in a frame
    GtkWidget *button_frame = gtk_frame_new("Controls");
    gtk_frame_set_shadow_type(GTK_FRAME(button_frame), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(main_box), button_frame, FALSE, FALSE, 0);
    
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(button_box), 5);
    gtk_container_add(GTK_CONTAINER(button_frame), button_box);

    GtkWidget *btn_scheduler = gtk_button_new_with_label("Choose Scheduler");
    GtkWidget *btn_load = gtk_button_new_with_label("Add Process");
    GtkWidget *btn_step = gtk_button_new_with_label("Step");
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    btn_stop = gtk_button_new_with_label("Stop");
    btn_start = gtk_button_new_with_label("Start");
    GtkWidget *toggle_auto = gtk_toggle_button_new_with_label("Auto Run");

    // Add some padding to buttons
    gtk_widget_set_margin_start(btn_scheduler, 5);
    gtk_widget_set_margin_end(btn_scheduler, 5);
    gtk_widget_set_margin_start(btn_load, 5);
    gtk_widget_set_margin_end(btn_load, 5);
    gtk_widget_set_margin_start(btn_step, 5);
    gtk_widget_set_margin_end(btn_step, 5);
    gtk_widget_set_margin_start(toggle_auto, 5);
    gtk_widget_set_margin_end(toggle_auto, 5);
    gtk_widget_set_margin_start(btn_start, 5);
    gtk_widget_set_margin_end(btn_start, 5);
    gtk_widget_set_margin_start(btn_stop, 5);
    gtk_widget_set_margin_end(btn_stop, 5);
    gtk_widget_set_margin_start(btn_reset, 5);
    gtk_widget_set_margin_end(btn_reset, 5);

    gtk_box_pack_start(GTK_BOX(button_box), btn_scheduler, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_load, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_step, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), toggle_auto, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_start, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_stop, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_reset, TRUE, TRUE, 0);

    // Connect signals
    g_signal_connect(btn_scheduler, "clicked", G_CALLBACK(on_choose_scheduler), NULL);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_add_process), NULL);
    g_signal_connect(btn_step, "clicked", G_CALLBACK(on_simulation_step), NULL);
    g_signal_connect(toggle_auto, "toggled", G_CALLBACK(on_auto_run_toggle), NULL);
    g_signal_connect(btn_start, "clicked", G_CALLBACK(on_start), NULL);
    g_signal_connect(btn_stop, "clicked", G_CALLBACK(on_stop), NULL);
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset), NULL);

    gtk_widget_show_all(window);
    return window;
}

// Function to run the main GUI loop
void run_gui() {
    gtk_main();
}

// Function to get file content through GUI
char* get_file_content(const char* filename) {
    static char file_buffer[1024];
    file_buffer[0] = '\0'; // Initialize to empty string
    
    // Create a dialog for file selection
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Read File",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    // Create a vertical box for the dialog content
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    // Add file path label
    GtkWidget *path_label = gtk_label_new("Reading file:");
    gtk_label_set_xalign(GTK_LABEL(path_label), 0);
    gtk_box_pack_start(GTK_BOX(vbox), path_label, FALSE, FALSE, 0);
    
    // Display the filename
    GtkWidget *filename_label = gtk_label_new(filename);
    gtk_label_set_xalign(GTK_LABEL(filename_label), 0);
    gtk_widget_set_margin_start(filename_label, 10);
    gtk_box_pack_start(GTK_BOX(vbox), filename_label, FALSE, FALSE, 5);
    
    // First try to open the file to check if it exists
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        // File doesn't exist, show an error message
        GtkWidget *error_label = gtk_label_new("Error: File does not exist or cannot be opened.");
        gtk_label_set_xalign(GTK_LABEL(error_label), 0);
        gtk_widget_set_margin_top(error_label, 10);
        gtk_box_pack_start(GTK_BOX(vbox), error_label, FALSE, FALSE, 0);
        
        // Log the error
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Error: Could not open file '%s'", filename);
        log_message(log_msg);
    } else {
        // File exists, read the content
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);
        
        if (size > 0) {
            // Limit the size to prevent buffer overflow
            size_t read_size = (size < sizeof(file_buffer) - 1) ? size : sizeof(file_buffer) - 1;
            
            // Read file content
            fread(file_buffer, 1, read_size, file);
            file_buffer[read_size] = '\0'; // Null-terminate
            
            // Display file content preview
            GtkWidget *content_label = gtk_label_new("File content:");
            gtk_label_set_xalign(GTK_LABEL(content_label), 0);
            gtk_widget_set_margin_top(content_label, 10);
            gtk_box_pack_start(GTK_BOX(vbox), content_label, FALSE, FALSE, 0);
            
            GtkWidget *text_view = gtk_text_view_new();
            gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(buffer, file_buffer, -1);
            
            GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            gtk_container_add(GTK_CONTAINER(scroll), text_view);
            gtk_widget_set_size_request(scroll, 400, 200);
            gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 5);
            
            // Log success
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Read %zu bytes from file '%s'", read_size, filename);
            log_message(log_msg);
        } else {
            // Empty file
            GtkWidget *empty_label = gtk_label_new("File is empty.");
            gtk_label_set_xalign(GTK_LABEL(empty_label), 0);
            gtk_widget_set_margin_top(empty_label, 10);
            gtk_box_pack_start(GTK_BOX(vbox), empty_label, FALSE, FALSE, 0);
            
            // Log that the file is empty
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "File '%s' is empty", filename);
            log_message(log_msg);
        }
        
        fclose(file);
    }
    
    gtk_widget_show_all(dialog);
    
    // Run the dialog
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    return file_buffer;
}

// Function to write content to a file through GUI
int write_file_content(const char* filename, const char* content) {
    int success = 0;
    
    // Create a dialog for file writing
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Write to File",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "Write", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    // Create a vertical box for the dialog content
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    // Add file path label
    GtkWidget *path_label = gtk_label_new("Writing to file:");
    gtk_label_set_xalign(GTK_LABEL(path_label), 0);
    gtk_box_pack_start(GTK_BOX(vbox), path_label, FALSE, FALSE, 0);
    
    // Display the filename
    GtkWidget *filename_label = gtk_label_new(filename);
    gtk_label_set_xalign(GTK_LABEL(filename_label), 0);
    gtk_widget_set_margin_start(filename_label, 10);
    gtk_box_pack_start(GTK_BOX(vbox), filename_label, FALSE, FALSE, 5);
    
    // Display content label
    GtkWidget *content_label = gtk_label_new("Content to write:");
    gtk_label_set_xalign(GTK_LABEL(content_label), 0);
    gtk_widget_set_margin_top(content_label, 10);
    gtk_box_pack_start(GTK_BOX(vbox), content_label, FALSE, FALSE, 0);
    
    // Display current content in editable text view
    GtkWidget *text_view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, content, -1);
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_widget_set_size_request(scroll, 400, 200);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 5);
    
    // Add warning about overwriting
    GtkWidget *warning_label = gtk_label_new("Warning: This will overwrite any existing file content.");
    gtk_label_set_xalign(GTK_LABEL(warning_label), 0);
    gtk_widget_set_margin_top(warning_label, 10);
    gtk_box_pack_start(GTK_BOX(vbox), warning_label, FALSE, FALSE, 0);
    
    gtk_widget_show_all(dialog);
    
    // Run the dialog
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        // Get modified content from text view
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);
        char *modified_content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        
        // Write to file
        FILE *file = fopen(filename, "w");
        if (file) {
            fputs(modified_content, file);
            fclose(file);
            success = 1;
            
            // Log success
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Successfully wrote to file '%s'", filename);
            log_message(log_msg);
        } else {
            // Log error
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Error: Could not open file '%s' for writing", filename);
            log_message(log_msg);
        }
        
        g_free(modified_content);
    } else {
        // Log canceled
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "File writing to '%s' canceled", filename);
        log_message(log_msg);
    }
    
    gtk_widget_destroy(dialog);
    
    return success;
}
