#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void *bunnyId(void *arg) {
    pthread_t id = *(pthread_t *)arg;
    printf("\n--- Bunny Thread Start ---\n");
    printf("     /\\_/\\ \n");
    printf("    ( o.o ) \n");
    printf("    (  \"  ) \n");
    printf(" (id :%ld) \n", id);
    printf("  /         \\ \n");
    printf(" / /       \\ \\ \n");
    printf("(_)         (_)\n");
    fflush(stdout); // Ensure all output is flushed
    sleep(2); // Simulate execution time
    return NULL;
}

void *print_chars(void *arg) {
    char ch1, ch2;
    printf("\n--- Print Chars Thread Start ---\n");
    printf("Enter The First Character: ");
    scanf(" %c", &ch1);
    printf("Enter The Second Character: ");
    scanf(" %c", &ch2);

    while (ch1 <= ch2) {
        printf("%c\n", ch1);
        ch1++;
        sleep(1); // Simulate work
    }
    return NULL;
}

void *printInt(void *arg) {
    int x, y;
    int sum = 0, product = 1, count = 0;
    float avg;

    printf("\n--- Print Int Thread Start ---\n");
    printf("Enter The First Integer: ");
    scanf("%d", &x);
    printf("Enter The Second Integer: ");
    scanf("%d", &y);

    for (int i = x; i <= y; i++) {
        sum += i;
        product *= i;
        count++;
        sleep(1); // Simulate work
    }

    avg = (float)sum / count;
    printf("Sum = %d\n", sum);
    printf("Product = %d\n", product);
    printf("Average = %.2f\n", avg);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    long base_time = get_time_ms();

    // Metrics arrays
    long arrival[3] = {0, 1000, 2000}; // Simulated arrival/release times
    long start[3], finish[3], exec[3], wait[3], response[3], turnaround[3];
    long cpu_work = 0;

    // Thread 1 - print_chars
    while (get_time_ms() - base_time < arrival[0]);
    start[0] = get_time_ms() - base_time;
    pthread_create(&t1, NULL, print_chars, NULL);
    pthread_join(t1, NULL);
    finish[0] = get_time_ms() - base_time;
    exec[0] = finish[0] - start[0];
    cpu_work += exec[0];
    wait[0] = start[0] - arrival[0];
    response[0] = wait[0];
    turnaround[0] = finish[0] - arrival[0];

    // Thread 2 - bunnyId
    while (get_time_ms() - base_time < arrival[1]);
    start[1] = get_time_ms() - base_time;
    pthread_create(&t2, NULL, bunnyId, (void *)&t2);
    pthread_join(t2, NULL);
    finish[1] = get_time_ms() - base_time;
    exec[1] = finish[1] - start[1];
    cpu_work += exec[1];
    wait[1] = start[1] - arrival[1];
    response[1] = wait[1];
    turnaround[1] = finish[1] - arrival[1];

    // Thread 3 - printInt
    while (get_time_ms() - base_time < arrival[2]);
    start[2] = get_time_ms() - base_time;
    pthread_create(&t3, NULL, printInt, NULL);
    pthread_join(t3, NULL);
    finish[2] = get_time_ms() - base_time;
    exec[2] = finish[2] - start[2];
    cpu_work += exec[2];
    wait[2] = start[2] - arrival[2];
    response[2] = wait[2];
    turnaround[2] = finish[2] - arrival[2];

    long total_time = finish[2];
    float cpu_util = (float)cpu_work / total_time * 100;
    size_t mem_usage = sizeof(pthread_t) * 3 + sizeof(long) * 21;

    // Report
    printf("\n---- FCFS Scheduling Performance Metrics ----\n");
    printf("Thread\tArrival\tStart\tFinish\tExec\tWait\tResp\tTurnaround\n");
    for (int i = 0; i < 3; i++) {
        printf("T%d\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\n", i + 1,
               arrival[i], start[i], finish[i], exec[i], wait[i], response[i], turnaround[i]);
    }

    printf("\nCPU Useful Work Time: %ld ms", cpu_work);
    printf("\nTotal Time (Schedule): %ld ms", total_time);
    printf("\nCPU Utilization: %.2f%%", cpu_util);
    printf("\nApproximate Memory Consumption: %zu bytes\n", mem_usage);

    return 0;
}


