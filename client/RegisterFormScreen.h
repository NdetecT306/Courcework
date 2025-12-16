#ifndef REGISTERFORMSCREEN_H
#define REGISTERFORMSCREEN_H
#include <gtkmm.h>
#include <string>
#include <iostream>
#include "NetworkClient.h"
using namespace std;

class RegisterFormScreen : public Gtk::Box {
private:
    Gtk::Label* title_label;
    Gtk::Box* form_box;
    Gtk::Entry* login_entry;
    Gtk::Entry* password_entry;
    Gtk::Entry* confirm_entry;
    Gtk::Label* message_label;
    Gtk::Button* register_button;
    Gtk::Button* back_button;
    void on_register_button_clicked();
    void on_back_clicked();
    void show_message(const string& text, const string& type);
    void load_css();
public:
    RegisterFormScreen();
    sigc::signal<void()> signal_back_to_menu;
    sigc::signal<void()> signal_register_success;
};
#endif 
