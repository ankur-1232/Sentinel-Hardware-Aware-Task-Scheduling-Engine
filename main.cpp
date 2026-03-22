#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include <atomic>

#include <chrono>


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
    // 1. Setup
    unsigned int cores = std::thread::hardware_concurrency();
    Sentinel pool(cores); 
    std::atomic<int> packetCount{0};

    std::cout << "System detected " << cores << " cores. Starting benchmark..." << std::endl;

    // 2. Start Timer here (in the main thread)
    auto start = std::chrono::high_resolution_clock::now();

    // 3. The Producer (Sensor)
    std::thread sensor([&pool, &packetCount]() {
        for(int i = 0; i < 20; ++i) {
            int dataPoint = 30 + (i % 10);
            pool.addTask(dataPoint);
            packetCount++;
            // We sleep to simulate a real sensor, but the pool 
            // works in the background while we sleep!
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    sensor.join(); // Wait for sensor to finish sending

    /* Note: At this point, the sensor is done, but the workers might 
       still be finishing the last few Fibonacci numbers. 
       A true 'Performance' test would wait for the pool to be empty.
    */

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "--- RESULTS ---" << std::endl;
    std::cout << "Total packets injected: " << packetCount.load() << std::endl;
    std::cout << "Total Wall-Clock Time: " << diff.count() << " seconds." << std::endl;

    return 0;
}