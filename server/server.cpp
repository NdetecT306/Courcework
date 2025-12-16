#include "server.h"
#include <iostream>
Server::Server(short port)
    : acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
    do_accept();
}
void Server::run() {
    cout << "Сервер работает на порт: " << acceptor.local_endpoint().port() << endl;
    io_context.run();
} 
void Server::do_accept() {
    acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                cout << "Новый клиент подсоединен." << endl;
                make_shared<ClientSession>(move(socket), user_db_manager, quiz_db_manager, resource_db_manager, bf_protection)->start();
            }
            do_accept();
        }
    );
}
int main() {
    try {
        Server server(8080);
        server.run();
    } catch (const exception& e) {
        cerr << "Ошибка загрузки сервера: " << e.what() << endl;
        return 1;
    }
    return 0;
}
