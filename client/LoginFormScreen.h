#ifndef LOGINFORMSCREEN_H
#define LOGINFORMSCREEN_H
#include <gtkmm.h>
#include <string>
#include "json.hpp"
#include <iostream>
#include <thread>
#include <chrono>
using json = nlohmann::json;
using namespace std;

class LoginFormScreen : public Gtk::Box {
public:
    LoginFormScreen();
    void show_success_message(const std::string& message);
    sigc::signal<void(const string&)> signal_login_success;
    sigc::signal<void()> signal_back_to_menu;
private:
    void on_login_button_clicked();
    void on_back_clicked();
    void show_message(const string& text, const string& type = "info");
    void show_blocked_message(int seconds);
    void load_css();
    Gtk::Label* title_label = nullptr;
    Gtk::Box* form_box = nullptr;
    Gtk::Entry* login_entry = nullptr;
    Gtk::Entry* password_entry = nullptr;
    Gtk::Label* message_label = nullptr;
    Gtk::Label* timer_label = nullptr; 
    Gtk::Button* login_button = nullptr;
    Gtk::Button* back_button = nullptr;
};

#endif 
