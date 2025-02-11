#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>

// Number of "hungry men"
static const int N = 5; 

// An array of mutexes (forks) -> object thath lets only one thread access a resource at a time
std::mutex forks[N];

// Mutex for synchronizing output
std::mutex outputMutex;

// Atomic counters for the total amount each man has eaten -> variables that can be read and modified by multiple threads concurrently without using locks
std::atomic<int> totalEaten[N];

bool hasHighestPriority(int id) {
    // current priority is totalEaten[id]
    int myEaten = totalEaten[id].load();
    for(int i = 0; i < N; i++) {
        if(i != id && totalEaten[i].load() < myEaten) {
            return false;
        }
    }
    return true;
}

// Function for each hungry man thread
void hungryMan(int id, int numLoops, int maxConsecutiveMeals) {
    // Random number generator setup
    std::random_device rd;       // Random seed
    std::mt19937 gen(rd());      // Mersenne Twister engine
    std::uniform_int_distribution<int> dist(1, 50); // Range of meal weight -> wagi posiłków

    for(int i = 0; i < numLoops; ++i) { // 
        // Get how much this man has eaten so far
        int myEaten = totalEaten[id].load(); //.load() -> read the value of the atomic variable

        // Priority based on how much was eaten (less eaten => higher priority)
        int priority = myEaten;

        // Identify left and right forks by index
        int left = std::min(id, (id + 1) % N); //modulo N -> to make sure it is the circular table and range within 0 to N-1
        int right = std::max(id, (id + 1) % N); // there are 5 forks, so the right fork is the one with the higher index

        // Wait based on priority (lower eaten => shorter wait)
        std::this_thread::sleep_for(std::chrono::microseconds(priority * 50 + 50)); // sleep time based on priority, + 50 to avoid 0 sleep time

        // Lock forks in the correct order to avoid deadlock
        std::scoped_lock lock(forks[left], forks[right]); // scoped_lock is a C++17 feature that can lock multiple mutexes at once. 
        //this can be done using locks, but scoped_lock is more convenient

        int mealsEatenNow = 0;
        // keep eating while top priority and not exceeding maxConsecutiveMeals
        while(hasHighestPriority(id) && mealsEatenNow < maxConsecutiveMeals) {
            // Generate random meal size and update total
            int meal = dist(gen); //generate random number from generator
            totalEaten[id].fetch_add(meal); // .fetch_add() -> add a value to the atomic variable

            // Print info about this meal
            {
                std::lock_guard<std::mutex> guard(outputMutex); // lock_guard - wrapper that locks when it is created and automatically unlocks it when it goes out of scope.
                std::cout << "Hungry man " << id << " ate " << meal  << " (total: " << totalEaten[id].load() << ")\n";
            }
            // Simulate eating time
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            mealsEatenNow++;
        }

        // Unlock forks -> automatically unlocked when lock goes out of scope
        // forks[right].unlock();
        // forks[left].unlock();
    }
}

int main(int argc, char* argv[]) {
    // Initialize total eaten
    for(int i = 0; i < N; i++) {
        totalEaten[i] = 0;
    }

    // Create vector to hold our five threads
    std::vector<std::thread> men;

    int numLoops = 3; // how many times men attempt to eat, tracking eating cycles per thread (default 3)
    if(argc > 1) {
        numLoops = std::stoi(argv[1]);
    }

    int maxConsecutiveMeals = 2; 
    if(argc > 2) {
        maxConsecutiveMeals = std::stoi(argv[2]);
    }

    // Launch a thread for each hungry man
    for(int i = 0; i < N; i++) {
        men.emplace_back(hungryMan, i, numLoops, maxConsecutiveMeals); // new element at the last element of the vector
    }

    // Join threads (will run indefinitely) główny wątek czeka na wątki głodomorów
    for(auto &m : men) {
        m.join();
    }

    return 0;
}