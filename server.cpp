#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <array>
#include <memory>

using boost::asio::ip::tcp;

class TicTacToeServer {
public:
    TicTacToeServer(boost::asio::io_context& io_context, int port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }

private:
    void start_accept() {
        std::cout << "Waiting for players to connect..." << std::endl;
        auto new_session = std::make_shared<PlayerSession>(acceptor_.get_io_context(), *this);
        acceptor_.async_accept(new_session->socket(), 
                               [this, new_session](const boost::system::error_code& error) {
            if (!error) {
                if (players.size() < 2) {
                    players.push_back(new_session);
                    new_session->start(players.size() == 1 ? "Waiting for second player..." : "Game start! You are O");
                    
                    if (players.size() == 2) {
                        current_player = 0;
                        broadcast("Both players connected! X starts.\n" + board_display());
                        players[current_player]->do_write("Your move (enter cell 1-9): ");
                    }
                }
                start_accept();
            }
        });
    }

    std::string board_display() {
        std::string display;
        for (int i = 0; i < 9; ++i) {
            display += (board[i] == 0 ? std::to_string(i + 1) : (board[i] == 1 ? "X" : "O"));
            display += (i % 3 == 2) ? "\n" : "|";
        }
        return display;
    }

    bool check_win() {
        const std::array<std::array<int, 3>, 8> win_patterns = {{
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
            {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
            {0, 4, 8}, {2, 4, 6}
        }};
        for (const auto& pattern : win_patterns) {
            if (board[pattern[0]] == board[pattern[1]] && board[pattern[1]] == board[pattern[2]] && board[pattern[0]] != 0)
                return true;
        }
        return false;
    }

    bool check_draw() {
        for (int cell : board)
            if (cell == 0) return false;
        return true;
    }

    void broadcast(const std::string& message) {
        for (auto& player : players) {
            player->do_write(message);
        }
    }

    void make_move(int player_index, int cell) {
        if (cell < 1 || cell > 9 || board[cell - 1] != 0) {
            players[player_index]->do_write("Invalid move. Try again: ");
            return;
        }
        board[cell - 1] = (player_index == 0 ? 1 : 2);
        if (check_win()) {
            broadcast(board_display() + "\nPlayer " + std::string(player_index == 0 ? "X" : "O") + " wins!");
            reset_game();
        } else if (check_draw()) {
            broadcast(board_display() + "\nDraw!");
            reset_game();
        } else {
            current_player = 1 - current_player;
            broadcast(board_display());
            players[current_player]->do_write("Your move (enter cell 1-9): ");
        }
    }

    void reset_game() {
        board.fill(0);
        players.clear();
        start_accept();
    }

    class PlayerSession : public std::enable_shared_from_this<PlayerSession> {
    public:
        PlayerSession(boost::asio::io_context& io_context, TicTacToeServer& server)
            : socket_(io_context), server_(server) {}

        tcp::socket& socket() { return socket_; }

        void start(const std::string& message) {
            do_write(message);
            do_read();
        }

        void do_write(const std::string& message) {
            auto self(shared_from_this());
            boost::asio::async_write(socket_, boost::asio::buffer(message),
                                     [this, self](boost::system::error_code ec, std::size_t /*length*/) {});
        }

        void do_read() {
            auto self(shared_from_this());
            boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(input_), '\n',
                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        int move = std::stoi(input_);
                        input_.clear();
                        server_.make_move(server_.current_player, move);
                        do_read();
                    }
                });
        }

    private:
        tcp::socket socket_;
        TicTacToeServer& server_;
        std::string input_;
    };

    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<PlayerSession>> players;
    std::array<int, 9> board = {0};  // 0 - empty, 1 - X, 2 - O
    int current_player = 0;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: server <port>\n";
        return 1;
    }
    boost::asio::io_context io_context;
    TicTacToeServer server(io_context, std::atoi(argv[1]));
    io_context.run();
}
