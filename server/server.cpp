#include "server.h"
#include "clientSession.h"
#include <iostream>

Server::Server(short port, const string& cert_file, const string& key_file)
    : ssl_context(boost::asio::ssl::context::tlsv13_server),
      acceptor(iocontext, tcp::endpoint(tcp::v4(), port)),
      cert_file(cert_file), key_file(key_file) {
    load_certificates();
    do_accept();
}
void Server::load_certificates() {
    try {
        ssl_context.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3 |
            boost::asio::ssl::context::single_dh_use);
        ssl_context.use_certificate_chain_file(cert_file);
        ssl_context.use_private_key_file(key_file, boost::asio::ssl::context::pem);
        ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
        ssl_context.load_verify_file(cert_file); 
        cout << "SSL контекст успешно загружен" << endl;
    } catch (const exception& e) {
        cerr << "Ошибка загрузки SSL сертификатов: " << e.what() << endl;
        throw;
    }
}
void Server::run() {
    cout << "Защищенный сервер запущен на порту: " << acceptor.local_endpoint().port() << endl;
    iocontext.run();
} 
void Server::do_accept() {
    acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                cout << "Новый клиент подключен. Установка SSL соединения." << endl;
                try {
                    auto ssl_socket = make_shared<boost::asio::ssl::stream<tcp::socket>>(move(socket), ssl_context);
                    ssl_socket->async_handshake(
                        boost::asio::ssl::stream_base::server,
                        [this, ssl_socket](const boost::system::error_code& handshake_ec) {
                            if (!handshake_ec) {
                                cout << "SSL рукопожатие успешно." << endl;
                                make_shared<ClientSession>(move(*ssl_socket), userdbmanager,  quizdbmanager, resourcedbmanager, bfprotection)->start();
                            } else {
                                cerr << "Ошибка SSL рукопожатия: " << handshake_ec.message() << endl;
                            }
                        }
                    );
                } catch (const exception& e) {
                    cerr << "Ошибка создания SSL сокета: " << e.what() << endl;
                }
            }
            do_accept();
        }
    );
}
int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Использование: " << argv[0] << endl;
            return 1;
        }
        short port = stoi(argv[1]);
        string cert_file = "server.crt";
        string key_file = "server.key";
        Server server(port, cert_file, key_file);
        server.run();
    } catch (const exception& e) {
        cerr << "Ошибка запуска сервера: " << e.what() << endl;
        return 1;
    }
    return 0;
}
