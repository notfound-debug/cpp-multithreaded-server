#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream> // Added for string stream

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    while (true) {
        int new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        char buffer[30000] = {0};
        read(new_socket, buffer, 30000);
        
        // We aren't parsing the request yet, just acknowledging we got it.
        std::cout << "Received request"<< buffer << std::endl;

        // --- STEP 2 CHANGE: CONSTRUCT A VALID HTTP RESPONSE ---
        
        // 1. The Body (The HTML content)
        std::string body = "<html><body><h1>Hello from C++!</h1><p>This is a real HTTP server.</p></body></html>";

        // 2. The Header (The Metadata)
        // HTTP/1.1 200 OK  -> Protocol version and Status Code (200 means Success)
        // Content-Type     -> Tells browser this is HTML, not just text
        // Content-Length   -> Size of the body (crucial for the browser to know when to stop reading)
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        oss << "Content-Type: text/html\r\n";
        oss << "Content-Length: " << body.size() << "\r\n";
        oss << "\r\n"; // The Blank Line (End of Headers)
        oss << body;

        std::string response = oss.str();
        
        // 3. Send it
        send(new_socket, response.c_str(), response.size(), 0);
        
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
