# High-Performance HTTP Server in C++

A multi-threaded HTTP server built from scratch in C++ using POSIX sockets. Designed to handle high concurrency efficiently without relying on external networking libraries.

## 🚀 Key Features

- **Custom Thread Pool:** Implemented a fixed-size worker pool (4 threads) to prevent context-switching overhead and memory exhaustion under load.
- **Raw POSIX Sockets:** Direct interaction with the Linux kernel networking stack (`sys/socket.h`, `netinet/in.h`).
- **High Concurrency:** Capable of handling 10,000+ requests per second (benchmarked with `wrk`).
- **Zero Dependencies:** Built using only the C++ Standard Library and POSIX headers.

## 🛠️ Architecture

The server follows a **Leader-Follower** thread pool model:

1.  **Main Thread:** Listens on Port 8080 and accepts incoming TCP connections.
2.  **Task Queue:** Accepted socket descriptors are pushed into a thread-safe queue protected by a mutex.
3.  **Worker Threads:** A pool of workers waits on a `condition_variable`. When a connection arrives, a worker wakes up, processes the HTTP request, sends a response, and returns to sleep.

## 💻 How to Run

### Prerequisites
- Linux environment (Ubuntu/WSL)
- g++ compiler

### Build & Run

```bash
# Compile the server
g++ -o server final_server.cpp -pthread

# Run the server
./server
```
### Stress Test

Using wrk (HTTP benchmarking tool):

### Simulate 500 concurrent users for 10 seconds
wrk -t8 -c500 -d10s http://localhost:8080/


## 📈 Performance

Benchmark Results (WSL/Ubuntu):

Throughput: ~10,600 Requests/sec

Latency: ~17ms avg under heavy load (500 concurrent connections)

Transfer: ~1.26MB/sec

