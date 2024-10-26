#include "header.h"

void ProcessInfoQueue::push(ProcessInfo&& item) {
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(std::move(item));
    lock.unlock();
    cond.notify_one();
}

bool ProcessInfoQueue::try_pop(ProcessInfo& item) {
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.empty()) {
        return false;
    }
    item = std::move(queue.front());
    queue.pop();
    return true;
}

// Define the global queue
ProcessInfoQueue g_completedProcesses;