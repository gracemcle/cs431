#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// Define missing constants if not available
#ifndef SCHED_BATCH
#define SCHED_BATCH 3
#endif

#ifndef SCHED_IDLE
#define SCHED_IDLE 5
#endif

volatile int count = 0;

pthread_mutex_t count_lock;

pthread_barrier_t barrier;

void print_thread_priority_info() {
    // Get Linux thread ID
    pid_t tid = syscall(SYS_gettid);
    pid_t pid = getpid();
    
    printf("=== Thread Priority Information ===\n");
    printf("Process ID: %d\n", pid);
    printf("Thread ID:  %d\n", tid);
    
    // Get scheduling policy
    int policy = sched_getscheduler(0);
    if (policy == -1) {
        perror("sched_getscheduler");
        return;
    }
    
    const char* policy_name;
    switch (policy) {
        case SCHED_OTHER: policy_name = "SCHED_OTHER"; break;
        case SCHED_FIFO:  policy_name = "SCHED_FIFO"; break;
        case SCHED_RR:    policy_name = "SCHED_RR"; break;
#ifdef SCHED_BATCH
        case SCHED_BATCH: policy_name = "SCHED_BATCH"; break;
#endif
#ifdef SCHED_IDLE
        case SCHED_IDLE:  policy_name = "SCHED_IDLE"; break;
#endif
        default:          
            policy_name = "UNKNOWN"; 
            break;
    }
    printf("Scheduling Policy: %s (%d)\n", policy_name, policy);
    
    // Get scheduling parameters
    struct sched_param param;
    if (sched_getparam(0, &param) == 0) {
        printf("RT Priority: %d\n", param.sched_priority);
    }
    
    // Get nice value
    errno = 0;
    int nice_val = getpriority(PRIO_PROCESS, tid);
    if (errno == 0) {
        printf("Nice value: %d\n", nice_val);
    } else {
        perror("getpriority");
    }
    
    if (policy == SCHED_OTHER 
#ifdef SCHED_BATCH
        || policy == SCHED_BATCH 
#endif
#ifdef SCHED_IDLE
        || policy == SCHED_IDLE
#endif
        ) {
        printf("Display Priority (PR): %d\n", 20 + nice_val);
    } else {
        printf("Display Priority (PR): rt\n");
    }
}

void print_scheduler_details(int pid) {
    pid_t tid = syscall(SYS_gettid);
    char path[256];
    FILE *file;
        
    snprintf(path, sizeof(path), "/proc/%d/sched", tid);
    file = fopen(path, "r");
    if (file) {
        char line[512];
        int found_relevant = 0;
        
        while (fgets(line, sizeof(line), file)) {
            // Look for interesting scheduler fields
            if (strstr(line, "se.vruntime") ||
                strstr(line, "se.sum_exec_runtime") ||
                strstr(line, "policy") ||
                strstr(line, "prio") ||
                strstr(line, "nice")) {
                printf("[%d] %s", pid, line);
                found_relevant = 1;
            }
        }
        
        fclose(file);
    }
}

void* worker_thread(void* arg) {
    int thread_num = *(int*)arg;

    sleep(1);

    pthread_barrier_wait(&barrier);
    
    print_scheduler_details(thread_num);

    pthread_barrier_wait(&barrier);
    pthread_mutex_lock(&count_lock);

    printf("\n--- Thread %d In the Lock ---\n", thread_num);
    print_thread_priority_info();
    print_scheduler_details(thread_num);
    
    // Do some work to accumulate runtime

    volatile long counter = 0;
    for (int i = 0; i < 100000000; i++) {
        counter++;
    }


    print_scheduler_details(thread_num);

    printf("\n--- Thread %d Released Lock ---\n", thread_num);

    pthread_mutex_unlock(&count_lock);
    
    
    return NULL;
}

int main() {
    printf("Main thread information:\n");
    print_thread_priority_info();
    print_scheduler_details(-1);

    // Initialize count lock
    pthread_mutex_init(&count_lock, NULL);
    pthread_barrier_init(&barrier, NULL, 3);
    
    // Create a few worker threads
    pthread_t threads[3];
    int thread_nums[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        // Spawn 3 threads with thread nums. Each thread will execute the worker_thread routine. 
        pthread_create(&threads[i], NULL, worker_thread, &thread_nums[i]);
    }
    
    // Wait for threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}