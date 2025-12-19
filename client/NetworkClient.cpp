#include "NetworkClient.h"
#include <iostream>
#include <thread>
#include <filesystem>
#include <algorithm>  
using namespace std;

NetworkClient::NetworkClient(const string& host, short port, const string& cert_file)
    : ssl_context(boost::asio::ssl::context::tlsv13_client),
      socket(io_context, ssl_context),
      serverhost(host),
      serverport(port),
      cert_file(cert_file),
      connected(false) {
    try {
        ssl_context.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3);
        if (filesystem::exists(cert_file)) {
            ssl_context.load_verify_file(cert_file);
            cout << "Загружен серверный сертификат: " << cert_file << endl;
        } else {
            cerr << "Серверный сертификат не найден: " << cert_file << endl;
        }
        ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
        ssl_context.set_verify_callback(
            [](bool preverified, boost::asio::ssl::verify_context& ctx) -> bool {
                cout << "[SSL Verify] preverified: " << preverified << endl;
                return preverified; 
            }
        );
    } catch (const exception& e) {
        cerr << "ОШИБКА НАСТРОЙКИ SSL: " << e.what() << endl;
        throw;
    }
    io_thread = thread([this]() {
        try {
            io_context.run();
        } catch (const exception& e) {
            cerr << "Ошибка в IO контексте: " << e.what() << endl;
        }
    });
}
NetworkClient::~NetworkClient() {
    disconnect();
    if (io_thread.joinable()) {
        io_context.stop();
        io_thread.join();
    }
}
bool NetworkClient::connect() {
    try {
        lock_guard<mutex> lock(socketmutex);       
        if (socket.lowest_layer().is_open()) {
            cout << "Сокет уже открыт" << endl;
            return true;
        }
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(serverhost, to_string(serverport));
        boost::asio::connect(socket.lowest_layer(), endpoints);
        cout << "TCP соединение установлено" << endl;
        SSL* ssl = socket.native_handle();
        if (ssl) {
            cout << "SSL handle получен" << endl;
            X509* cert = SSL_get_certificate(ssl);
            if (cert) {
                char subject[256];
                X509_NAME_oneline(X509_get_subject_name(cert), subject, 256);
                cout << "Клиентский сертификат готов." << endl;
            } else {
                cerr << "Клиентский сертификат НЕ загружен в SSL контекст!" << endl;
            }
        }
        // SSL рукопожатие
        cout << "Начало SSL рукопожатия..." << endl;
        socket.handshake(boost::asio::ssl::stream_base::client);
        cout << "SSL рукопожатие успешно!" << endl;
        if (ssl) {
            X509* server_cert = SSL_get_peer_certificate(ssl);
            if (server_cert) {
                char subject[256];
                X509_NAME_oneline(X509_get_subject_name(server_cert), subject, 256);
                cout << "Серверный сертификат." << endl;
                X509_free(server_cert);
            }
            X509* client_cert = SSL_get_certificate(ssl);
            if (client_cert) {
                char subject[256];
                X509_NAME_oneline(X509_get_subject_name(client_cert), subject, 256);
                cout << "Отправлен клиентский сертификат: " << subject << endl;
            } else {
                cerr << "Клиентский сертификат не был отправлен!" << endl;
            }
        }
        connected = true;
        return true;
    } catch (const boost::system::system_error& e) {
        cerr << "\nОШИБКА ПОДКЛЮЧЕНИЯ: " << e.what() << endl;
        cerr << "  Код: " << e.code() << endl;
        cerr << "  Категория: " << e.code().category().name() << endl;
        connected = false;
        return false;
    } catch (const exception& e) {
        cerr << "\nОШИБКА: " << e.what() << endl;
        connected = false;
        return false;
    }
}
void NetworkClient::disconnect() {
    lock_guard<mutex> lock(socketmutex);
    if (socket.lowest_layer().is_open()) {
        try {
            cout << "Закрытие SSL соединения" << endl;
            // Закрываем SSL соединение
            boost::system::error_code ec;
            socket.shutdown(ec);
            if (ec && ec != boost::asio::ssl::error::stream_truncated) {
                cerr << "Ошибка закрытия SSL: " << ec.message() << endl;
            }
            // Закрываем TCP соединение
            socket.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
            socket.lowest_layer().close(ec);
            cout << "Соединение закрыто" << endl;
        } catch (const exception& e) {
            cerr << "Ошибка при отключении: " << e.what() << endl;
        }
    }
    connected = false;
}
bool NetworkClient::is_connected() const {
    lock_guard<mutex> lock(socketmutex);
    return connected && socket.lowest_layer().is_open();
}
json NetworkClient::send_request(const json& request) {
    try {
        lock_guard<mutex> lock(socketmutex);
        if (!socket.lowest_layer().is_open() || !connected) {
            cout << "Соединение разорвано. Пытаемся переподключиться..." << endl;
            if (!connect()) {
                return {{"status", "error"}, {"message", "Нет подключения к серверу"}};
            }
        }
        string request_str = request.dump() + "\n";
        size_t bytes_written = boost::asio::write(socket, boost::asio::buffer(request_str));
        boost::asio::streambuf response_buf;
        size_t bytes_read = boost::asio::read_until(socket, response_buf, "\n");
        istream response_stream(&response_buf);
        string response_str;
        getline(response_stream, response_str);
        size_t substr_len = min(response_str.length(), static_cast<size_t>(100));
        return json::parse(response_str);
    } catch (const boost::system::system_error& e) {
        cerr << "Сетевая ошибка: " << e.what() << " (код: " << e.code() << ")" << endl;
        connected = false;
        if (e.code() == boost::asio::ssl::error::stream_truncated) {
            cout << "SSL соединение было разорвано. Пробуем переподключиться." << endl;
            if (connect()) {
                return send_request(request);
            }
        }
        return {{"status", "error"}, {"message", string("Сетевая ошибка: ") + e.what()}};
    } catch (const exception& e) {
        cerr << "Ошибка при отправке/получении: " << e.what() << endl;
        connected = false;
        return {{"status", "error"}, {"message", string("Ошибка: ") + e.what()}};
    }
}
json NetworkClient::login(const string& username, const string& password) {
    json request = {
        {"command", "login"},
        {"login", username},
        {"password", password}
    };
    return send_request(request);
}

json NetworkClient::register_user(const string& username, const string& password) {
    json request = {
        {"command", "register"},
        {"login", username},
        {"password", password}
    };
    return send_request(request);
}

json NetworkClient::get_profile(const string& username) {
    json request = {
        {"command", "get_profile"},
        {"login", username}
    };
    return send_request(request);
}

json NetworkClient::save_profile(const json& profile) {
    json request = {
        {"command", "save_profile"},
        {"profile", profile}
    };
    return send_request(request);
}

json NetworkClient::check_connection() {
    json request = {{"command", "ping"}};
    return send_request(request);
}

json NetworkClient::get_categories() {
    json request = {{"command", "get_categories"}};
    return send_request(request);
}

json NetworkClient::get_questions(int categoryId) {
    json request = {
        {"command", "get_questions"},
        {"category_id", categoryId}
    };
    return send_request(request);
}

json NetworkClient::check_answers(const string& userLogin, int categoryId, const json& answers) {
    json request = {
        {"command", "check_answers"},
        {"user", userLogin},
        {"category_id", categoryId},
        {"answers", answers}
    };
    return send_request(request);
}

json NetworkClient::get_statistics(const string& userLogin) {
    json request = {
        {"command", "get_statistics"},
        {"user", userLogin}
    };
    return send_request(request);
}

json NetworkClient::get_resource_categories() {
    json request = {{"command", "get_resource_categories"}};
    return send_request(request);
}

json NetworkClient::get_resources_by_category(int categoryId) {
    json request = {
        {"command", "get_resources_by_category"},
        {"category_id", categoryId}
    };
    return send_request(request);
}

json NetworkClient::get_all_resources() {
    json request = {{"command", "get_all_resources"}};
    return send_request(request);
}

json NetworkClient::get_resources_with_categories() {
    json request = {{"command", "get_resources_with_categories"}};
    return send_request(request);
}

NetworkClient* g_network_client = nullptr;
