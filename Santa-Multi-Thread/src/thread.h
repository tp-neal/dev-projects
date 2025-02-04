/*
=============================================================================
  PROJECT: Santa Clause Problem
=============================================================================
  NAME: Tyler Neal
  DATE: 1/19/2025
  FILE: thread.h
  PROGRAM PURPOSE:
    This program simulates Santa Claus's operations at the North Pole,
    coordinating between Santa, his reindeers, and elves using a Hoare
    type monitor for synchronization. It models Santa being awoken by
    either all reindeers returning from vacation or by elves in groups
    of three with problems, ensuring proper handling of Christmas
    preparations and toy-making issues without deadlock or starvation.
=============================================================================
*/

#ifndef THREAD_H
#define THREAD_H

#include <ThreadClass.h>
#include <string>
#include <chrono>
#include <thread>
#include <random>
#include <cstdarg>

//=========================================================================
//                             Definitions
//=========================================================================
/*  The following definition values are arbitrary and may be adjusted as 
    needed. To better simulate different scenarios, actual waiting times 
    will be randomized within a range of 'x' to the defined maximum value, 
    where 'x' is the minimum value fed to the randomizer function. 
*/
#define MAX_DELAY_SECONDS 5         // Max time delay used for waiting reindeer and elves
#define MAX_ANSWER_QUESTION_TIME 3  // Max time to answer questions
#define MAX_PREP_SLEIGH_TIME 3      // Max time to prepare sleigh
#define MAX_DELIVERY_TIME 5         // Max time to deliver toys

//=========================================================================
//                             Enumerations
//=========================================================================

enum Print_Style {  // Represent space count for printout prfix's
    SANTA = 0,
    ELF = 9,
    REINDEER = 4
};

//=========================================================================
//                    Class an Function Declarations
//=========================================================================

// Monitor class for managing interactions
class MyMonitor : public Monitor {
public:
    MyMonitor(int total_elves, int total_reindeer, int required_deliveries, Mutex* mtx);

    void Sleep();
    void AskQuestion(int id);
    void AnswerQuestion();
    void ReindeerBack(int id);
    void WaitOthers(int id);
    void GatherDeer();
    void WaitSleigh(int id);
    void PrepareSleigh();
    void FlyOff(int id);
    void DeliverToys();
    
    bool elves_have_question;
    bool all_deer_back;
    bool retired;

private:
    Mutex* print_mutex;
    Condition attention_required;
    Condition santa_ready;
    Condition elves_ready;
    Condition reindeers_ready;
    Condition sleigh_ready;
    Condition vacation_ready;

    int total_elves;
    int total_reindeer;
    int required_deliveries;
    int reindeer_home;
    int question_counter;
    int delivery_count;

    bool santa_sleeping;
    bool santas_home;

    int* elf_queue;
    int elf_queue_iter;
};

// Santa class representing Santa's thread and behaviors
class Santa : public Thread {
public:
    Santa(MyMonitor* monitor, Mutex* mtx);

private:
    void ThreadFunc() override;
    MyMonitor* monitor;
    Mutex* print_mutex;
};

// Elf class representing each elf's thread and behaviors
class Elf : public Thread {
public:
    Elf(int id, MyMonitor* monitor, Mutex* mtx);

private:
    void ThreadFunc() override;
    int id;
    MyMonitor* monitor;
    Mutex* print_mutex;
};

// Reindeer class representing each reindeer's thread and behaviors
class Reindeer : public Thread {
public:
    Reindeer(int id, MyMonitor* monitor, Mutex* mtx);

private:
    void ThreadFunc() override;
    int id;
    MyMonitor* monitor;
    Mutex* print_mutex;
};

//=========================================================================
//                         Helper Functions
//=========================================================================

/**
 * @brief Wait for a specified number of seconds
 * 
 * @param seconds Number of seconds to wait
 */
void wait(int seconds);

/**
 * @brief Wait for a random number of seconds between 1 and the specified maximum
 * 
 * @param max_wait Maximum number of seconds to wait
 */
void random_wait(int max_wait);

/**
 * @brief Generate a string of spaces for formatting
 * 
 * @param num_spaces Number of spaces to generate
 * @return std::string String of spaces
 */
std::string spaces(int num_spaces);

/**
 * @brief Print formatted output to the console
 * 
 * @param space_count Number of spaces to indent
 */
void printout(Print_Style style, Mutex* print_mutex, const char* format, ...);

#endif // THREAD_H