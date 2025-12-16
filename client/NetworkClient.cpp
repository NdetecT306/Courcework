#include "NetworkClient.h"
#include <iostream>
#include <thread>
using namespace std;
//Конструктор и деструктор
NetworkClient::NetworkClient(const string& host, short port):  
    socket(io_context), serverhost(host), serverport(port), connected(false) {
    thread([this]() { io_context.run(); }).detach();
}
NetworkClient::~NetworkClient() {
    disconnect();
}
bool NetworkClient::connect() {//Связь
    try {
        lock_guard<std::mutex> lock(socketmutex);
        if (socket.is_open()) {
            socket.close();
        }
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(serverhost, to_string(serverport));
        boost::asio::connect(socket, endpoints);
        connected = true;
        cout << "Подключено к серверу " << serverhost << ":" << serverport << endl;
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка подключения к серверу: " << e.what() << endl;
        connected = false;
        return false;
    }
}
void NetworkClient::disconnect() {//ОтсСвязь
    lock_guard<std::mutex> lock(socketmutex);
    if (socket.is_open()) {
        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
        socket.close(ec);
    }
    connected = false;
}
bool NetworkClient::is_connected() const {//Соединены
    lock_guard<mutex> lock(socketmutex);
    return connected && socket.is_open();
}
json NetworkClient::send_request(const json& request) { //Отправить запрос
    try {
        lock_guard<mutex> lock(socketmutex);
        if (!socket.is_open()) {
            if (!connect()) {
                return {{"status", "error"}, {"message", "Нет подключения к серверу"}};
            }
        }
        string request_str = request.dump() + "\n";
        boost::asio::write(socket, boost::asio::buffer(request_str));
        boost::asio::streambuf response_buf;
        boost::asio::read_until(socket, response_buf, "\n");
        istream response_stream(&response_buf);
        string response_str;
        getline(response_stream, response_str);
        return json::parse(response_str);
    } catch (const exception& e) {
        cerr << "Ошибка сети: " << e.what() << endl;
        connected = false;
        return {{"status", "error"}, {"message", string("Сетевая ошибка: ") + e.what()}};
    }
}
json NetworkClient::login(const string& username, const string& password) { //Вход и регистрация
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
json NetworkClient::get_profile(const string& username) { //Получить и сохранить профиль
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
    json request = {
        {"command", "ping"}
    };
    return send_request(request);
}
json NetworkClient::get_categories() {//Получить вопросы, категории, ответы, статистику, ресурсы
    json request = {
        {"command", "get_categories"}
    };
    return send_request(request);
}
json NetworkClient::get_questions(int category_id) {
    json request = {
        {"command", "get_questions"},
        {"category_id", category_id}
    };
    return send_request(request);
}
json NetworkClient::check_answers(const string& user_login, int category_id, const json& answers) {
    json request = {
        {"command", "check_answers"},
        {"user", user_login},
        {"category_id", category_id},
        {"answers", answers}
    };
    return send_request(request);
}
json NetworkClient::get_statistics(const string& user_login) {
    json request = {
        {"command", "get_statistics"},
        {"user", user_login}
    };
    return send_request(request);
}
json NetworkClient::get_resource_categories() {
    json request = {
        {"command", "get_resource_categories"}
    };
    return send_request(request);
}
json NetworkClient::get_resources_by_category(int category_id) {
    json request = {
        {"command", "get_resources_by_category"},
        {"category_id", category_id}
    };
    return send_request(request);
}
json NetworkClient::get_all_resources() {
    json request = {
        {"command", "get_all_resources"}
    };
    return send_request(request);
}
json NetworkClient::get_resources_with_categories() {
    json request = {
        {"command", "get_resources_with_categories"}
    };
    return send_request(request);
}
// Глобальная переменная пользователя
NetworkClient* g_network_client = nullptr;
