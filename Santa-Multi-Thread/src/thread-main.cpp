/*
=============================================================================
  PROJECT: Santa Clause Problem
=============================================================================
  NAME: Tyler Neal
  DATE: 1/19/2025
  FILE: thread-main.cpp
  PROGRAM PURPOSE:
    This program simulates Santa Claus's operations at the North Pole,
    coordinating between Santa, his reindeers, and elves using a Hoare
    type monitor for synchronization. It models Santa being awoken by
    either all reindeers returning from vacation or by elves in groups
    of three with problems, ensuring proper handling of Christmas
    preparations and toy-making issues without deadlock or starvation.
=============================================================================
*/

//===========================================================================
//                               Main
//===========================================================================

#include "thread.h"
#include "thread-main.h"

int main (int argc, char* argv[]) {
    // Check Argument Count
    if (argc < 4 || argc > 4) {
        printf("Usage: ./prog5 num_elves num_reindeer num_toy_deliveries");
        return EXIT_FAILURE;
    }

    // Parse Command Line Arguments
    int num_elves = (atoi(argv[1]) > 0) ? atoi(argv[1]) : 7;    // default values if necessary
    int num_reindeer = (atoi(argv[2]) > 0) ? atoi(argv[2]) : 9;
    int num_toy_deliveries = (atoi(argv[3]) > 0) ? atoi(argv[3]) : 3;

    // Monitor and Print Mutex Creation
    Mutex* print_mutex = new Mutex();
    MyMonitor* monitor = new MyMonitor(num_elves, num_reindeer, num_toy_deliveries, print_mutex);

    // Allocate Space for Threads
    Santa* santa = new Santa(monitor, print_mutex);
    Elf** elves = (Elf**)malloc((num_elves) * sizeof(Elf*));
    Reindeer** reindeer = (Reindeer**)malloc((num_reindeer) * sizeof(Reindeer*));

    // Initialize Environment
    Environment env_info(num_elves, num_reindeer, num_toy_deliveries, 
                         print_mutex, monitor, santa, elves, reindeer);

    // Begin Thread
    beginAllThreads(&env_info);

    // Wait for All Threads to Finish
    waitForAllThreads(&env_info);

    // Return Success
    printf("\nExit Success!\n");
    return EXIT_SUCCESS;
}

/**
 * @brief Begin all threads in the environment
 * 
 * @param env Environment struct containing all necessary thread information
 */
void beginAllThreads(Environment* env) {
    Santa* santa = env->santa;
    Elf** elves = env->elves;
    Reindeer** reindeer = env->reindeer;
    MyMonitor* monitor = env->monitor;
    Mutex* print_mutex = env->print_mutex;

    // Create and Start All Threads
    env->santa->Begin();                             
    for (int i = 0; i < env->total_reindeer; i++) { // initialize reindeer
        env->reindeer[i] = new Reindeer(i+1, monitor, print_mutex);
        reindeer[i]->Begin();
    }
    for (int i = 0; i < env->total_elves; i++) {    // initialize elves
        elves[i] = new Elf(i+1, monitor, print_mutex);
        elves[i]->Begin();
    }
}

/**
 * @brief Wait for all threads in the environment to finish
 * 
 * @param env Environment struct containing all necessary thread information
 */
void waitForAllThreads(Environment* env) {
    Santa* santa = env->santa;
    Elf** elves = env->elves;
    Reindeer** reindeer = env->reindeer;

    // Wait for All Threads to Finish
    santa->Join();
    for (int i = 0; i < env->total_reindeer; i++) {
        reindeer[i]->Join();
    }
    for (int i = 0; i < env->total_elves; i++) {
        elves[i]->Join();
    }
}