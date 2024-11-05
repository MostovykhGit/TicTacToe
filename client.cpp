#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

void read_from_server(tcp::socket& socket) {
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, '\n');
    std::istream is(&buffer);
    std::string line;
    std::getline(is, line);
    std::cout << line << std::endl;
}

void write_to_server(tcp::socket& socket) {
    std::string message;
    std::getline(std::cin, message);
    message += "\n";
    boost::asio::write(socket, boost::asio::buffer(message));
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: client <host> <port>\n";
        return 1;
    }

    boost::asio::io_context io_context;
    tcp::socket socket(io_context);
    socket.connect(tcp::endpoint(boost::asio::ip::make_address(argv[1]), std::atoi(argv[2])));

    while (true) {
        read_from_server(socket);
        write_to_server(socket);
    }
}
