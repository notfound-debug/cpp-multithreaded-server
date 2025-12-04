#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

// --- THE THREAD POOL CLASS ---
class ThreadPool {
public:
    ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            // Create workers that loop forever
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    // THE CRITICAL SECTION (Accessing the Queue)
                    {
                        // 1. Acquire lock
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        
                        // 2. Wait until there is a job OR we are stopping
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        
                        // 3. If stopping and empty, exit the thread
                        if (stop && tasks.empty()) return;
                        
                        // 4. Get the job
                        task = std::move(tasks.front());
                        tasks.pop();
                    } 
                    // Lock is released here automatically so others can grab jobs

                    // 5. Do the job
                    task();
                }
            });
        }
    }

    // Add a new job to the queue
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one(); // Wake up one worker!
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all(); // Wake everyone up to quit
        for (std::thread &worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};

// --- CLIENT HANDLING LOGIC ---
void handle_client(int client_socket) {
    char buffer[30000] = {0};
    read(client_socket, buffer, 30000);
    std::cout<<"Working on req...."<<buffer<<std::endl;
    // Simulate work
    //std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Faster sleep for testing
    std::string body = "<html><body><h1>Pooled!</h1></body></html>";
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: text/html\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;

    std::string response = oss.str();
    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
    std::cout<<"Req processed."<<std::endl;
}

// --- MAIN SERVER ---
int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) return 1;

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) return 1;
    if (listen(server_fd, 10) < 0) return 1;

    std::cout << "Thread Pool Server listening on port 8080..." << std::endl;

    // Create a pool of 4 workers
    // This server can now handle concurrent requests without exploding memory!
    ThreadPool pool(4);

    while (true) {
        int new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket < 0) continue;

        // Instead of launching a thread, we push the job to the pool
        pool.enqueue([new_socket] {
            handle_client(new_socket);
        });
    }

    return 0;
}
