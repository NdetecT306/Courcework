#ifndef MAINMENUSCREEN_H
#define MAINMENUSCREEN_H
#include <gtkmm.h>
#include <string>
class MainMenuScreen : public Gtk::Box {
private:
    Gtk::Label* title_label = nullptr;
    Gtk::Label* status_label = nullptr;
    Gtk::Box* buttons_box = nullptr;
    Gtk::Button* login_button = nullptr;
    Gtk::Button* register_button = nullptr;
    void on_login_clicked();
    void on_register_clicked();
    void load_css();
public:
    MainMenuScreen();
    void set_connection_status(bool connected);
    sigc::signal<void()> signal_show_login_form;
    sigc::signal<void()> signal_show_register_form;
};
#endif
