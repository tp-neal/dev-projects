# Project: Santa's Multithreaded Workshop

## Overview:

    This program simulates Santa's workship, coordinating communication between Santa, his elves, and reindeer. Synchornization tools such as a Hoar-style monitor, semaphores, and mutexes are used to allow Santa and his reindeer deliver toys, while the elves build them.

## Problem Description:

    Santa needs to be able to get his rest each year, but he is constantly
    needed around his workshop. The elves have questions when making toys,
    and the reindeer must request Santa to sleigh around the world delivering
    presents. This multi-threaded implementation allows Santa to sleep when
    he isn't needed, and be signaled when he is.

    Santa: 
        Can be awakened by all reindeer returning, or three elves
        needing help.

    Reindeer: 
        Must wait for Santa after they return from their vacation.

    Elves: 
        Can ask Santa for help with toy making in groups of three.

## Constraints:

    + Santa can only help one group of three elves at a time.
    + Santa cannot help elves with questions if he is delivering presents.
    + All reindeer must be back before Santa can deliver toys.
    + Santa prioritizes helping reindeer over helping elves.

## Features:

    + Hoare-style monitor
    + Configurable number of elves, reindeer, and deliveries before
      retirement.
    + Semi-randomized wait times to simultate variousu scenarios.
    + Clean organized console output.

## Required:

    1. C++ compiler

## Usage:

    Simple:
    
        1. Navigate to "/src/"
        2. Run the file "run.sh" 
            - This will run the program with the default values
            
    Custom:

        1. Navigate to "/src/"
        2. Make clean
        3. Make all
        4. Run the program with optional parameters
            usage: ./prog5 [num_elves] [num_reindeer] [num_toy_deliveries]
