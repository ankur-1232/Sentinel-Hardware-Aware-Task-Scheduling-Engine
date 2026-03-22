#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

// 1. The Work (The "Fuel")
long long fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

class Sentinel {
private:
    std::queue<int> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stop = false; 
    std::vector<std::thread> m_workers;

public:
    Sentinel(int numThreads) {
        for (int i = 0; i < numThreads; ++i) {
            // 2. Creating the "Worker" using a Lambda
            m_workers.emplace_back([this, i] {
                while (true) {
                    int task;
                    {
                        // 3. The "Waiting Room" Logic
                        std::unique_lock<std::mutex> lock(this->m_mutex);
                        
                        // Wait until there's a task or we are told to stop
                        this->m_cv.wait(lock, [this] { 
                            return this->m_stop || !this->m_tasks.empty(); 
                        });

                        if (this->m_stop && this->m_tasks.empty()) return;

                        task = m_tasks.front();
                        m_tasks.pop();
                    }

                    // 4. Doing the actual work
                    std::cout << "Thread " << i << " calculating Fib(" << task << ")..." << std::endl;
                    long long result = fibonacci(task);
                    std::cout << "Thread " << i << " finished! Result: " << result << std::endl;
                }
            });
        }
    }

    void addTask(int n) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.push(n);
        }
        m_cv.notify_one(); // Wake up one sleeping worker
    }

    ~Sentinel() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all(); // Wake everyone up to tell them to exit
        for (std::thread &worker : m_workers) {
            worker.join(); // Wait for threads to finish safely
        }
    }
};

int main() {
    Sentinel pool(4); // Create a pool with 4 worker threads

    // Add some heavy tasks
    pool.addTask(35);
    pool.addTask(40);
    pool.addTask(42);
    pool.addTask(38);

    // Give it a moment to process before the program ends
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    return 0;
}