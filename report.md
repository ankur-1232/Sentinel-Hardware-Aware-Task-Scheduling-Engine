# Sentinel: High-Performance Multi-threaded Task Scheduler
**Author:** [Your Name]  
**Category:** Systems Programming / Concurrent Computing  
**Date:** March 2026

## 1. Project Overview
**Sentinel** is a lightweight, hardware-aware Thread Pool designed to handle computationally expensive tasks (simulated via Fibonacci sequence generation) without blocking the main execution flow. It serves as a foundation for real-time telemetry processing, where "Sensor" data must be ingested and processed across multiple CPU cores simultaneously.

## 2. Project Objectives
* **Minimize Latency:** Offload heavy math from the producer thread to background workers.
* **Resource Efficiency:** Use a fixed worker pool to prevent "Thread Explosion" (spawning too many threads).
* **Thread Safety:** Ensure zero race conditions during task handoffs using Mutexes and Condition Variables.
* **Hardware Adaptability:** Automatically scale the worker count based on the host machine’s physical CPU cores.

---

## 3. System Architecture & File Tree
```text
multithread-programming/
├── main.cpp          # Core implementation (Sentinel class, Sensor, and Main)
├── sentinel          # Compiled Binary (Linux/ELF)
└── Report.md         # Technical Documentation
```

---

## 4. Technical Specifications

### Header Dependencies
| Header | Purpose |
| :--- | :--- |
| `<thread>` | Managing the lifecycle of worker and sensor threads. |
| `<mutex>` | Ensuring exclusive access to the shared Task Queue. |
| `<condition_variable>` | Facilitating efficient thread "sleeping" and "waking." |
| `<atomic>` | Thread-safe counters for packet tracking without mutex overhead. |
| `<chrono>` | High-resolution timing for performance benchmarking. |

### Class Structure: `Sentinel`
| Member Name | Scope | Type | Purpose |
| :--- | :--- | :--- | :--- |
| `m_tasks` | Private | `std::queue<int>` | Shared buffer for pending Fibonacci indices. |
| `m_mutex` | Private | `std::mutex` | Synchronizes access to `m_tasks`. |
| `m_cv` | Private | `std::condition_variable` | Signals workers when new tasks are available. |
| `m_workers` | Private | `std::vector<std::thread>` | Container managing the lifetime of all worker threads. |
| `m_stop` | Private | `bool` | Atomic-like flag to signal threads to shut down safely. |

---

## 5. Methods & Functionality

| Method | Access | Parameters | Purpose |
| :--- | :--- | :--- | :--- |
| `Sentinel(int)` | Public | `int numThreads` | Constructor: Spawns the worker threads and starts their loops. |
| `addTask(int)` | Public | `int n` | Pushes a task to the queue and notifies a waiting worker. |
| `~Sentinel()` | Public | N/A | Graceful Shutdown: Signals `m_stop`, wakes all threads, and `join()`s them. |
| `fibonacci(int)`| Global | `int n` | The "Work Payload": Recursive calculation of the Nth Fib number. |

---

## 6. Benchmarking & Results (The "Exploratory" Outcome)

### Test Case: 20-Packet Telemetry Stream
* **Hardware:** [e.g., 4-Core Intel/AMD Processor]
* **Input:** 20 tasks (varying from $Fib(30)$ to $Fib(39)$).
* **Methodology:** A "Sensor" thread injects one task every 200ms.

| Metric | Observation |
| :--- | :--- |
| **Total Tasks Injected** | 20 |
| **Active Worker Threads** | 4 (Adaptive to CPU Cores) |
| **Avg. Sensor Latency** | ~4.02 seconds (Total duration of the injection) |
| **System Behavior** | Workers processed $Fib(30)$ instantly while the sensor was sleeping for the next packet. |

**Observation:** By the time the Sensor finished injecting the 20th packet, nearly all previous packets were already processed. The "Wait Time" for the user was reduced to near-zero because the processing happened **in the gaps** between data arrivals.

---

## 7. Future Extensions
* **Task Prioritization:** Implementing a `std::priority_queue` to handle critical sensor alerts before standard logs.
* **Work Stealing:** Balancing the load if one thread gets stuck on a very large $Fib(n)$ calculation.
* **Lock-Free Queue:** Moving from `std::mutex` to an atomic-based queue for extreme high-performance scenarios.

---
