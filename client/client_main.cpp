#include "MainWindow.h"
#include "NetworkClient.h"
#include <iostream>
#include <thread>
using namespace std;
class AuthApp : public Gtk::Application {
protected:
    void on_activate() override {
        g_network_client = new NetworkClient();
        cout << "Запуск системы тестирования знаний..." << endl;
        if (g_network_client->connect()) {
            cout << "Подключено к серверу" << endl;
        } else {
            cout << "Не удалось подключиться к серверу" << endl;
        }
        auto window = new MainWindow();
        window->signal_hide().connect(sigc::bind(sigc::mem_fun(*this,&AuthApp::on_window_hide), window));
        add_window(*window);
        window->show();
    }
private:
    void on_window_hide(Gtk::Window* window) {
        delete window;
    }
public:
    AuthApp() : Gtk::Application("org.testing.system.client") {}
};
int main(int argc, char* argv[]) {
    auto app = AuthApp();
    return app.run(argc, argv);
}
