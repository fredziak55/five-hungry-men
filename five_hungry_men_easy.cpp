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

// Atomic counters for the total amount each man has eaten -> variables that can be read and modified by multiple threads concurrently without using locks
std::atomic<int> totalEaten[N];

// Function for each hungry man thread
void hungryMan(int id) {
    // Random number generator setup
    std::random_device rd;       // Random seed
    std::mt19937 gen(rd());      // Mersenne Twister engine
    std::uniform_int_distribution<int> dist(1, 5); // Range of meal weight -> wagi posiłków

    while (true) {
        // Get how much this man has eaten so far
        int myEaten = totalEaten[id].load(); //.load() -> read the value of the atomic variable

        // Priority based on how much was eaten (less eaten => higher priority)
        int priority = myEaten;

        // Identify left and right forks by index
        int left = std::min(id, (id + 1) % N); //modulo N -> to make sure it is the circular table and range within 0 to N-1
        int right = std::max(id, (id + 1) % N); // there are 5 forks, so the right fork is the one with the higher index

        // Wait based on priority (lower eaten => shorter wait)
        std::this_thread::sleep_for(std::chrono::milliseconds(priority * 50 + 50)); // sleep time based on priority, + 50 to avoid 0 sleep time

        // Lock forks in the correct order to avoid deadlock
        forks[left].lock();
        forks[right].lock();

        // Generate random meal size and update total
        int meal = dist(gen); //generate random number from generator
        totalEaten[id].fetch_add(meal); // .fetch_add() -> add a value to the atomic variable

        // Print info about this meal
        std::cout << "Hungry man " << id << " ate " << meal  << " (total: " << totalEaten[id].load() << ")\n";

        // Simulate eating time
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Unlock forks
        forks[right].unlock();
        forks[left].unlock();
    }
}

int main() {
    // Initialize total eaten
    for(int i = 0; i < N; i++) {
        totalEaten[i] = 0;
    }

    // Create vector to hold our five threads
    std::vector<std::thread> men;

    // Launch a thread for each hungry man
    for(int i = 0; i < N; i++) {
        men.emplace_back(hungryMan, i); // new element at the last element of the vector
    }

    // Join threads (will run indefinitely) główny wątek czeka na wątki głodomorów
    for(auto &m : men) {
        m.join();
    }

    return 0;
}