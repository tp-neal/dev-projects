/*
=============================================================================
  PROJECT: Santa Clause Problem
=============================================================================
  NAME: Tyler Neal
  DATE: 1/19/2025
  FILE: thread-main.h
  PROGRAM PURPOSE:
    This program simulates Santa Claus's operations at the North Pole,
    coordinating between Santa, his reindeers, and elves using a Hoare
    type monitor for synchronization. It models Santa being awoken by
    either all reindeers returning from vacation or by elves in groups
    of three with problems, ensuring proper handling of Christmas
    preparations and toy-making issues without deadlock or starvation.
=============================================================================
*/

#ifndef THREAD_MAIN_H
#define THREAD_MAIN_H

#include <ThreadClass.h>
#include <string>
#include <chrono>
#include <thread>
#include <random>
#include <cstdarg>
#include "thread.h"

//=========================================================================
//                              Structures
//=========================================================================

/**
 * @brief Environment struct containing all necessary thread information
 *
 * @param total_elves Total number of elves
 * @param total_reindeer Total number of reindeer
 * @param required_deliveries Total number of toy deliveries required
 * @param print_mutex Mutex for printing
 * @param monitor Monitor for synchronization
 * @param santa Santa thread
 * @param elves Array of elf threads
 * @param reindeer Array of reindeer threads
 */
struct Environment {
    int total_elves;
    int total_reindeer;
    int required_deliveries;

    Mutex* print_mutex;
    MyMonitor* monitor;

    Santa* santa;
    Elf** elves;
    Reindeer** reindeer;

    Environment(int total_elves, int total_reindeer, int required_deliveries, 
                Mutex* print_mutex, MyMonitor* monitor, 
                Santa* santa, Elf** elves, Reindeer** reindeer) {   
        this->total_elves = total_elves;
        this->total_reindeer = total_reindeer;
        this->required_deliveries = required_deliveries;
        this->print_mutex = print_mutex;
        this->monitor = monitor;
        this->santa = santa;
        this->elves = elves;
        this->reindeer = reindeer;
    };

    ~Environment() {
        delete print_mutex;
        delete monitor;
        delete santa;
        for (int i = 0; i < total_elves; i++) delete elves[i];
        for (int i = 0; i < total_reindeer; i++) delete reindeer[i];
        delete[] elves;
        delete[] reindeer;
    }
};

//=========================================================================
//                         Helper Functions
//=========================================================================

/**
 * @brief Begin all threads in the environment
 * 
 * @param env Environment struct containing all necessary thread information
 */
void beginAllThreads(Environment* env);

/**
 * @brief Wait for all threads in the environment to finish
 * 
 * @param env Environment struct containing all necessary thread information
 */
void waitForAllThreads(Environment* env);

#endif // THREAD-MAIN_H