#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <thread> // NEW: For std::thread
#include <vector>

// the logic for handling a SINGLE client into its own function
void handle_client(int client_socket) {
    char buffer[30000] = {0};
    read(client_socket, buffer, 30000);
    
    // Simulate a "heavy" task to prove threading works (sleep for 5 seconds)
    // In real life, this would be reading a file from disk or querying a database.
    std::cout << "Working on a request..."<< buffer << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: text/html\r\n";
    oss << "Content-Length: 45\r\n"; // Hardcoded length for simplicity here
    oss << "\r\n";
    oss << "<html><body><h1>Request Processed!</h1></body></html>";

    std::string response = oss.str();
    send(client_socket, response.c_str(), response.size(), 0);
    
    close(client_socket);
    std::cout << "Request finished." << std::endl;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // ... (Socket setup is exactly the same as before) ...
    if (server_fd == -1) return 1;

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) return 1;
    if (listen(server_fd, 10) < 0) return 1;

    std::cout << "Threaded Server listening on port 8080..." << std::endl;

    while (true) {
        int new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket < 0) continue;

        std::cout << "New connection accepted!" << std::endl;

        // --- STEP 3 CHANGE: LAUNCH A THREAD ---
        // Instead of handling it here, we pass the work to a new thread.
        // std::thread(function_to_run, arguments...)
        // .detach() tells the OS: "Run this in the background, don't make main() wait for it."
        std::thread client_thread(handle_client, new_socket);
        client_thread.detach(); 
    }

    close(server_fd);
    return 0;
}
