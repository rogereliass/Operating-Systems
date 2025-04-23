#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_THREADS 3

typedef struct {
    pthread_t thread;
    int id;
    int burst_time;
    int remaining_time;
    int release_time;
    int start_time;
    int finish_time;
    int waiting_time;
    int response_time;
    int turnaround_time;
} ThreadInfo;

pthread_mutex_t lock;
ThreadInfo threads[NUM_THREADS];
int completed = 0;
int global_time = 0; // Tracks overall simulation time
int cpu_useful_work = 0;

// Function prototypes
void *bunnyId(void *arg);
void *print_chars(void *arg);
void *printInt(void *arg);
void *scheduler(void *arg);
int get_current_time();

// Get simulated current time
int get_current_time() {
    return global_time;
}

// Bunny function
void *bunnyId(void *arg) {
    ThreadInfo *t = (ThreadInfo *)arg;
    pthread_mutex_lock(&lock);
    t->start_time = get_current_time();
    if (t->response_time == -1)
        t->response_time = t->start_time - t->release_time;
    pthread_mutex_unlock(&lock);

    printf("\nThread %d (Bunny) started at time %d\n", t->id, t->start_time);
    printf("     /\\_/\\ \n");
    printf("    ( o.o ) \n");
    printf("    (  \"  ) \n");
    printf(" (id :%d) \n", t->id);
    printf("  /         \\ \n");
    printf(" / /       \\ \\ \n");
    printf("(_)         (_) \n");

    sleep(t->burst_time);

    pthread_mutex_lock(&lock);
    t->finish_time = get_current_time() + t->burst_time;
    cpu_useful_work += t->burst_time;
    pthread_mutex_unlock(&lock);

    return NULL;
}

// Print chars function
void *print_chars(void *arg) {
    ThreadInfo *t = (ThreadInfo *)arg;
    pthread_mutex_lock(&lock);
    t->start_time = get_current_time();
    if (t->response_time == -1)
        t->response_time = t->start_time - t->release_time;
    pthread_mutex_unlock(&lock);

    printf("\nThread %d (Print Chars) started at time %d\n", t->id, t->start_time);
   
    char ch1, ch2;
    printf("Enter The First Character: ");
    scanf(" %c", &ch1);
    printf("Enter The Second Character: ");
    scanf(" %c", &ch2);

    while (ch1 <= ch2) {
        printf("%c\n", ch1);
        ch1++;
    }

    sleep(t->burst_time);

    pthread_mutex_lock(&lock);
    t->finish_time = get_current_time() + t->burst_time;
    cpu_useful_work += t->burst_time;
    pthread_mutex_unlock(&lock);

    return NULL;
}

// Print integers function
void *printInt(void *arg) {
    ThreadInfo *t = (ThreadInfo *)arg;
    pthread_mutex_lock(&lock);
    t->start_time = get_current_time();
    if (t->response_time == -1)
        t->response_time = t->start_time - t->release_time;
    pthread_mutex_unlock(&lock);

    printf("\nThread %d (Print Int) started at time %d\n", t->id, t->start_time);
   
    int x, y, sum = 0, product = 1, count = 0;
    printf("Enter The First Integer: ");
    scanf("%d", &x);
    printf("Enter The Second Integer: ");
    scanf("%d", &y);

    int temp_x = x;
    while (temp_x <= y) {
        sum += temp_x;
        product *= temp_x;
        count++;
        temp_x++;
    }
   
    float avg = (float)sum / count;
    printf("Sum = %d\n", sum);
    printf("Product = %d\n", product);
    printf("Average = %.2f\n", avg);

    sleep(t->burst_time);

    pthread_mutex_lock(&lock);
    t->finish_time = get_current_time() + t->burst_time;
    cpu_useful_work += t->burst_time;
    pthread_mutex_unlock(&lock);

    return NULL;
}

// Shortest Remaining Time scheduler
void *scheduler(void *arg) {
    printf("\nScheduler started\n");

    while (completed < NUM_THREADS) {
        pthread_mutex_lock(&lock);
        global_time++;

        // Find thread with shortest remaining time
        int min_index = -1, min_time = __INT_MAX__;
        for (int i = 0; i < NUM_THREADS; i++) {
            if (threads[i].remaining_time > 0 && threads[i].remaining_time < min_time) {
                min_time = threads[i].remaining_time;
                min_index = i;
            }
        }

        if (min_index != -1) {
            printf("\nScheduler: Running thread %d (Remaining time: %d sec)\n",
                   threads[min_index].id, threads[min_index].remaining_time);
            sleep(threads[min_index].remaining_time);
            threads[min_index].remaining_time = 0;
            completed++;
        }

        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main() {
    pthread_t scheduler_thread;
    pthread_mutex_init(&lock, NULL);

    // Initialize thread info
    threads[0] = (ThreadInfo){.id = 1, .burst_time = 3, .remaining_time = 3, .release_time = 0, .response_time = -1};
    threads[1] = (ThreadInfo){.id = 2, .burst_time = 2, .remaining_time = 2, .release_time = 1, .response_time = -1};
    threads[2] = (ThreadInfo){.id = 3, .burst_time = 4, .remaining_time = 4, .release_time = 2, .response_time = -1};

    // Start scheduler
    pthread_create(&scheduler_thread, NULL, scheduler, NULL);

    // Create threads
    pthread_create(&threads[0].thread, NULL, print_chars, &threads[0]);
    pthread_create(&threads[1].thread, NULL, bunnyId, &threads[1]);
    pthread_create(&threads[2].thread, NULL, printInt, &threads[2]);

    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i].thread, NULL);
    }
    pthread_join(scheduler_thread, NULL);

    // Calculate performance metrics
    int total_time = global_time;
    float cpu_utilization = ((float)cpu_useful_work / total_time) * 100;

    printf("\nPerformance Metrics:\n");
    printf("--------------------\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].waiting_time = threads[i].start_time - threads[i].release_time;
        threads[i].turnaround_time = threads[i].finish_time - threads[i].release_time;

        printf("Thread %d:\n", threads[i].id);
        printf("  Release Time: %d sec\n", threads[i].release_time);
        printf("  Start Time: %d sec\n", threads[i].start_time);
        printf("  Finish Time: %d sec\n", threads[i].finish_time);
        printf("  Waiting Time: %d sec\n", threads[i].waiting_time);
        printf("  Response Time: %d sec\n", threads[i].response_time);
        printf("  Turnaround Time: %d sec\n", threads[i].turnaround_time);
    }

    printf("\nCPU Utilization: %.2f%%\n", cpu_utilization);
    printf("CPU Useful Work: %d sec\n", cpu_useful_work);
    printf("Total Time Elapsed: %d sec\n", total_time);
    printf("Memory Consumption: ~%lu bytes (approx)\n", sizeof(threads) + sizeof(pthread_t) * (NUM_THREADS + 1));

    pthread_mutex_destroy(&lock);
    return 0;
}