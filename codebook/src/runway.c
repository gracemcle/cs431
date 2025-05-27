#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

char* get_timestamp() {
    static char timestamp[30];
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", timeinfo);
    return timestamp;
}

typedef struct plane_locks{
    pthread_mutex_t* takeoff;
    pthread_mutex_t* landing;
    int total_planes;
} plane_locks_t;

typedef struct shared_variables{
    int plane_num;
    plane_locks_t* plane_locks;
} shared_variables_t;

void* plane_routine(void* arg){
    shared_variables_t* vars = (shared_variables_t*)arg; // Changed to pointer
    int plane_num = vars->plane_num;
    int total_planes = vars->plane_locks->total_planes;
    
    pthread_mutex_lock(&vars->plane_locks->takeoff[plane_num]);
    printf("%s plane [%d] taking off \n", get_timestamp(), plane_num);
    pthread_mutex_unlock(&vars->plane_locks->takeoff[(plane_num + 1) % total_planes]);
    
    sleep(2); // plane flying

    pthread_mutex_lock(&vars->plane_locks->landing[plane_num]);
    printf("%s plane [%d] landing \n", get_timestamp(), plane_num);
    pthread_mutex_unlock(&vars->plane_locks->landing[(plane_num + 1) % total_planes]);
    
    free(vars);
    return NULL;
}

int main(int argc, char** argv){
    int num_planes = 3;
    pthread_t* plane_ids = (pthread_t*) malloc(sizeof(pthread_t) * num_planes);
    pthread_mutex_t* takeoff = malloc(sizeof(pthread_mutex_t) * num_planes);
    pthread_mutex_t* landing = malloc(sizeof(pthread_mutex_t) * num_planes);
    
    // Initialize all mutexes
    for(int i = 0; i < num_planes; i++){
        pthread_mutex_init(&takeoff[i], NULL); 
        pthread_mutex_init(&landing[i], NULL);
    }
    
    // Lock all mutexes initially except the first one for each type
    for(int i = 1; i < num_planes; i++){
        pthread_mutex_lock(&takeoff[i]); // Lock all except first
        pthread_mutex_lock(&landing[i]); // Lock all except first
    }
    
    plane_locks_t plane_locks = {
        .takeoff = takeoff, 
        .landing = landing, 
        .total_planes = num_planes
    };
    
    // Create threads
    for(int i = 0; i < num_planes; i++){
        shared_variables_t *vars = malloc(sizeof(shared_variables_t));
        vars->plane_num = i;
        vars->plane_locks = &plane_locks;
        pthread_create(&plane_ids[i], NULL, plane_routine, vars);
    }
    
    // Wait for all threads to complete
    for(int i = 0; i < num_planes; i++){
        pthread_join(plane_ids[i], NULL);
    }
    
    // Destroy mutexes
    for(int i = 0; i < num_planes; i++){
        pthread_mutex_destroy(&takeoff[i]); 
        pthread_mutex_destroy(&landing[i]); 
    }
    
    free(takeoff);
    free(landing);
    free(plane_ids);
    
    return 0;
}