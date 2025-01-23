# Five hungry men
Project in C++ based on dining philosophers problem.
Resolved using priority based system with sleep time.

## Prerequisites
* g++ in version 5.0 or higher

## How to start
1. Compile with g++
```sh
   g++ five_hungry_men_easy.cpp
``` 
2. Run the program
```sh
./a.out {numLoops} {maxConsecutiveMeals}
```
`numLoops` - int, default 3 <br /> First parameter that gives number of loops in simulation <br />`maxConsecutiveMeals` - int, default 2 <br />Second parameter that gives number of max eaten meals in a row by one hungry man
