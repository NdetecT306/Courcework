#include "MainWindow.h"
#include "NetworkClient.h"
#include <iostream>
#include <thread>

using namespace std;

class AuthApp : public Gtk::Application {
protected:
    void on_activate() override {
        cout << "Запуск системы тестирования знаний" << endl;
        // Создаем клиента  логированием
        g_network_client = new NetworkClient("localhost", 8080, "server.crt");
        cout << "Проверка доступности порта 8080..." << endl;
        try {
            tcp::socket test_socket(io_context);
            tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve("localhost", "8080");
            boost::asio::connect(test_socket, endpoints);
            cout << "Порт 8080 доступен" << endl;
            test_socket.close();
        } catch (const exception& e) {
            cout << "Порт 8080 недоступен: " << e.what() << endl;
        }        
        // Подключаемся к серверу
        cout << "\nПодключение к серверу" << endl;
        if (g_network_client->connect()) {
            cout << "Успешное подключение к серверу :)" << endl;
            cout << "\nТестирование соединения" << endl;
            json ping_response = g_network_client->check_connection();
            if (ping_response["status"] == "success") {
                cout << "Сервер отвечает корректно" << endl;
            } else {
                cout << "Сервер отвечает с ошибкой " << ping_response["message"] << endl;
            }
        } else {
            cout << "Не удалось подключиться к серверу :(" << endl;
            cout << "\nДля устранения неисправностей " << endl;
            cout << "1. Убедитесь, что сервер запущен:" << endl;
            cout << "   docker logs auth-server" << endl;
            cout << "2. Проверьте SSL соединение:" << endl;
            cout << "   openssl s_client -connect localhost:8080 -tls1_3" << endl;
            cout << "3. Проверьте порт:" << endl;
            cout << "   netstat -tln | grep :8080" << endl;
        }
        auto window = new MainWindow();
        window->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &AuthApp::on_window_hide), window));
        add_window(*window);
        window->show();
    }
private:
    void on_window_hide(Gtk::Window* window) {
        cout << "Закрытие приложения" << endl;
        if (g_network_client) {
            g_network_client->disconnect();
            delete g_network_client;
            g_network_client = nullptr;
        }
        delete window;
    }
    boost::asio::io_context io_context;
public:
    AuthApp() : Gtk::Application("org.testing.system.client") {}
};

int main(int argc, char* argv[]) {
    try {
        auto app = AuthApp();
        return app.run(argc, argv);
    } catch (const exception& e) {
        cerr << "КРИТИЧЕСКАЯ ОШИБКА: " << e.what() << endl;
        return 1;
    }
}
