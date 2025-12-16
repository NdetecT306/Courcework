#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm.h>
#include "MainMenuScreen.h"
#include "LoginFormScreen.h"
#include "RegisterFormScreen.h"
#include "MainScreen.h"
using namespace std;

class MainWindow : public Gtk::Window {
private:
    Gtk::Stack* mainstack = nullptr;
    MainMenuScreen* mainmenu = nullptr;
    LoginFormScreen* loginform = nullptr;
    RegisterFormScreen* registerform = nullptr;
    MainScreen* mainscreen = nullptr;
    void setup_navigation();
    void load_main_css();
    void update_connection_status();
    bool check_connection();
    void on_login_success(const string& username);
    void on_logout();
public:
    MainWindow();
    ~MainWindow() override;
};

#endif
