#include "header.h"

/**
 * Pushes a ProcessInfo item onto the queue in a thread-safe manner
 * Uses move semantics to avoid copying
 * 
 * @param item The ProcessInfo item to push onto the queue
 */
void ProcessInfoQueue::push(ProcessInfo&& item) {
    // Lock the mutex to ensure thread safety
    std::unique_lock<std::mutex> lock(mutex);
    // Move the item onto the queue
    queue.push(std::move(item));
    // Unlock before notification to reduce contention
    lock.unlock();
    // Notify one waiting thread that data is available
    cond.notify_one();
}

/**
 * Attempts to pop an item from the queue in a thread-safe manner
 * Returns immediately if queue is empty
 * 
 * @param item Reference to store the popped ProcessInfo
 * @return true if an item was popped, false if queue was empty
 */
bool ProcessInfoQueue::try_pop(ProcessInfo& item) {
    // Lock the mutex to ensure thread safety
    std::unique_lock<std::mutex> lock(mutex);
    // Check if queue is empty
    if (queue.empty()) {
        return false;
    }
    // Move front item to output parameter
    item = std::move(queue.front());
    // Remove the item from queue
    queue.pop();
    return true;
}

// Define the global queue for storing completed process information
ProcessInfoQueue g_completedProcesses;