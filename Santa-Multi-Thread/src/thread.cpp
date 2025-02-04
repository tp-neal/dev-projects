/*
=============================================================================
  PROJECT: Santa Clause Problem
=============================================================================
  NAME: Tyler Neal
  DATE: 1/19/2025
  FILE: thread.cpp
  PROGRAM PURPOSE:
    This program simulates Santa Claus's operations at the North Pole,
    coordinating between Santa, his reindeers, and elves using a Hoare
    type monitor for synchronization. It models Santa being awoken by
    either all reindeers returning from vacation or by elves in groups
    of three with problems, ensuring proper handling of Christmas
    preparations and toy-making issues without deadlock or starvation.
=============================================================================
*/

#include "thread.h"

//===========================================================================//
//                               MONITOR                                     //
//===========================================================================//

// Monitor Constructor
MyMonitor::MyMonitor(int total_elves, int total_reindeer, int required_deliveries, Mutex* mtx) 
    : Monitor("monitor", HOARE), 

    // Conditions & Mutex
    print_mutex(mtx),
    attention_required("attention_required"),
    santa_ready("santa_ready"),
    elves_ready("elves_ready"),
    reindeers_ready("reindeers_ready"),
    sleigh_ready("sleigh_ready"),
    vacation_ready("vacation_ready"),

    // Integers
    total_elves(total_elves),
    total_reindeer(total_reindeer),
    required_deliveries(required_deliveries),
    question_counter(0),
    reindeer_home(0),
    delivery_count(0),

    // Booleans
    elves_have_question(false),
    all_deer_back(false),
    retired(false),
    santa_sleeping(false),
    santas_home(true),

    // Etc.
    elf_queue(new int[3]),  // Queue for elves with questions
    elf_queue_iter(0)       // Iterator for elf queue
{}

// Monitor Methods
void MyMonitor::Sleep() {
    MonitorBegin();

    // Before sleeping, check if still needed
    if (!elves_have_question && !all_deer_back) {
        // Sleep until attention is required
        printout(SANTA, print_mutex, "Santa takes a nap zZz");
        santa_sleeping = true;
        attention_required.Wait();

        printout(SANTA, print_mutex, "Santa wakes up!");
        santa_sleeping = false;
    }

    MonitorEnd();
}

void MyMonitor::AskQuestion(int id) {
    MonitorBegin();

    // Ask question if santa's home and not already with a group
    if (!elves_have_question && santas_home) {

        // Add elf question to queue
        printout(ELF, print_mutex, "Elf %d has a problem", id);
        question_counter++;
        elf_queue[elf_queue_iter++] = id;

        // Wait until group of three to wake santa
        if (question_counter < 3) {
            elves_ready.Wait(); 
        } else {
            printout(ELF, print_mutex, "Elves %d, %d, %d, wake up Santa", elf_queue[0], elf_queue[1], elf_queue[2]);
            elves_have_question = true;
            attention_required.Signal();
        }
    }

    MonitorEnd();
}

void MyMonitor::AnswerQuestion() {
    MonitorBegin();

    // Santa answers elves questions
    printout(SANTA, print_mutex, "Santa answers the question posted by elves %d, %d, %d", elf_queue[0], elf_queue[1], elf_queue[2]);
    random_wait(MAX_ANSWER_QUESTION_TIME); // take time to answer question

    // Reset elf group related variables
    printout(ELF, print_mutex, "Elves %d, %d, %d, return to work", elf_queue[0], elf_queue[1], elf_queue[2]);
    elf_queue_iter = 0;
    for (int i = 0; i < question_counter; i++) elf_queue[i] = 0;
    for (int i = 1; i < question_counter; i++) elves_ready.Signal();
    question_counter = 0;
    elves_have_question = false;

    MonitorEnd();
}

void MyMonitor::ReindeerBack(int id) {
    MonitorBegin();

    printout(REINDEER, print_mutex, "Reindeer %d returns", id);
    reindeer_home++;

    // If returning reindeer is the last home, wake up santa
    if (reindeer_home >= total_reindeer) {
        printout(REINDEER, print_mutex, "The last reindeer %d wakes up Santa", id);
        all_deer_back = true;
        santas_home = false; // Prevent elves from asking santa a question
        attention_required.Signal();
    }
    
    MonitorEnd();
}

void MyMonitor::WaitOthers(int id) {
    MonitorBegin();

    // Wait for all reindeer to return
    if(all_deer_back == false) {
        reindeers_ready.Wait();
    }

    MonitorEnd();
}

void MyMonitor::GatherDeer() {
    MonitorBegin();

    // Round up reindeer to wait at the sleigh
    for (int i = 0; i < total_reindeer - 1; i++) {
        reindeers_ready.Signal();
    }

    MonitorEnd();
}

void MyMonitor::WaitSleigh(int id) {
    MonitorBegin();

    // Wait for santa to finish preparing the sleigh
    sleigh_ready.Wait();

    MonitorEnd();
}

void MyMonitor::PrepareSleigh() {
    MonitorBegin();

    // Santa preps the sleigh, and lets reindeer know when he's done
    printout(SANTA, print_mutex, "Santa is preparing the sleigh");
    random_wait(MAX_PREP_SLEIGH_TIME); // take time to prep the sleigh
    for (int i = 0; i < total_reindeer; i++) {
        sleigh_ready.Signal();
    }

    MonitorEnd();
}

void MyMonitor::FlyOff(int id) {
    MonitorBegin();

    // Reindeer take off to deliver toys, and anticipate their vacation
    vacation_ready.Wait();

    MonitorEnd();
}

void MyMonitor::DeliverToys() {
    MonitorBegin();

    // The team takes off to deliver toys
    printout(SANTA, print_mutex, "The team flies off! (%d)", ++delivery_count);
    printout(SANTA, print_mutex, "..........");
    random_wait(MAX_DELIVERY_TIME); // Spend time delivering toys

    // If there's more deliveries to be done reset variables, otherwise retire
    if (delivery_count < required_deliveries) {
        all_deer_back = false;
        reindeer_home = 0;
        santas_home = true;
    } else {
        printout(SANTA, print_mutex, "\nAfter (%d) deliveries, Santa retires and is on vacation!\n", delivery_count);
        retired = true;
        // Resume waiting elves
        for (int i = 0; i < total_elves; i++) {
            elves_ready.Signal();
        }
    }

    // Send reindeer off on vacation
    for (int i = 0; i < total_reindeer; i++) {
        vacation_ready.Signal();
    }

    MonitorEnd();
}

//===========================================================================//
//                                SANTA                                      //
//===========================================================================//

// Santa Constructor
Santa::Santa(MyMonitor* monitor, Mutex* mtx) : monitor(monitor), print_mutex(mtx) {
    printout(SANTA, print_mutex, "Santa thread starts");
    printout(SANTA, print_mutex, "..........");
}

// Santa Behavior
void Santa::ThreadFunc() {
    Thread::ThreadFunc();
    wait(1); // Allows for clean initial thread creation print (not necessary)

    // Work until all deliveries are completed
    while(!monitor->retired) {
        monitor->Sleep(); // Take a nap

        if (monitor->all_deer_back) { // If deer are back, prep toy delivery
            monitor->GatherDeer();
            monitor->PrepareSleigh();
            monitor->DeliverToys();
        }

        if (monitor->elves_have_question) { // If elves have question, answer them
            monitor->AnswerQuestion();
        }
    }

    Exit();
}

//===========================================================================//
//                                 ELF                                       //
//===========================================================================//

// Elf Constructor
Elf::Elf(int id, MyMonitor* monitor, Mutex* mtx) : id(id), monitor(monitor), print_mutex(mtx) {
    printout(ELF, print_mutex, "Elf %d starts", id);
    printout(ELF, print_mutex, "..........");
}

// Elf Behavior
void Elf::ThreadFunc() {
    Thread::ThreadFunc();
    wait(1); // Allows for clean initial thread creation print (not necessary)

    // Work until santa has retired
    while(true) {
        if (monitor->retired) break;  
        random_wait(MAX_DELAY_SECONDS);  // Make toys
        if (monitor->retired) break;
        monitor->AskQuestion(id);        // Encountered a problem
        if (monitor->retired) break;
        random_wait(MAX_DELAY_SECONDS);  // Problem solved, take a rest
    }

    // Terminate
    printout(ELF, print_mutex, "Elf %d terminates", id);
    Exit();
}

//===========================================================================//
//                               REINDEER                                    //
//===========================================================================//

// Reindeer Constructor
Reindeer::Reindeer(int id, MyMonitor* monitor, Mutex* mtx) : id(id), monitor(monitor), print_mutex(mtx) {
    printout(REINDEER, print_mutex, "Reindeer %d starts", id);
    printout(REINDEER, print_mutex, "..........");
}

// Reindeer Behavior
void Reindeer::ThreadFunc() {
    Thread::ThreadFunc();
    wait(1); // Allows for clean initial thread creation print (not necessary)

    // Work until santa has retired
    while(true) {
        if (monitor->retired) break;
        random_wait(MAX_DELAY_SECONDS);     // Tan on the beach
        monitor->ReindeerBack(id);          // Report back to santa
        if(!monitor->all_deer_back) {       // If deer still gone, wait for them to arrive
            monitor->WaitOthers(id);
        }
        monitor->WaitSleigh(id);            // Wait for santa to attach sleigh
        monitor->FlyOff(id);                // Go deliver toys, then take a vacation
        if (monitor->retired) break;
        random_wait(MAX_DELAY_SECONDS);     // Prepare for vacation
    }

    // Terminate
    printout(REINDEER, print_mutex, "Reindeer %d terminates", id);
    Exit();
}

//===========================================================================//
//                               HELPERS                                     //
//===========================================================================//

/**
 * @brief Wait for a specific number of seconds
 * 
 * @param seconds Number of seconds to wait
 */
void wait(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

/**
 * @brief Wait for a random number of seconds between 1 and a specified maximum
 * 
 * @param max_wait Maximum number of seconds to wait
 */
void random_wait(int max_wait) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, max_wait);
    wait(distrib(gen));
}

/**
 * @brief Create a string of spaces
 * 
 * @param num_spaces Number of spaces to create
 * @return std::string String of spaces
 */
std::string spaces(int num_spaces) {
    return std::string(num_spaces, ' ');
}

/**
 * @brief Print a formatted string with a specified number of spaces
 * 
 * @param space_count Number of spaces to print before the string
 * @param print_mutex Mutex for printing
 * @param format Format string to print
 */
void printout(Print_Style style, Mutex* print_mutex, const char* format, ...) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);

    print_mutex->Lock();
    std::cout << spaces(style) << buffer << std::endl;
    print_mutex->Unlock();
}