#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// declare mutex lock "sell_tv"
// 2: declaire mutex lock "door"
// declare barrier

// declare sale price

void* bouncer(void* arg){

    // for n shoppers
        // unlock the door


}

void* shopper(void* arg){

    //1. wait at barrier
    //2. wait at the door

    // wait for tv sale (lock)
    // sale -- 
    // leave store (unlock)

}

int main(int argc, char** argv){

    // usage: gcc -o black_friday black_friday.c -lpthread

    // initialize "sell_tv"
    //2. initialize "door"

    // initialize barrier

    // initialize pthread_t array
    // initialize thread_nums -> optional

    // pthread_create n shopper threads

    //2. pthread create 1 bouncer thread

    // pthread_join n shopper threads

    // pthread_join 1 bouncer thread

    // destroy locks
    // destroy barriers

    // free arrays

}