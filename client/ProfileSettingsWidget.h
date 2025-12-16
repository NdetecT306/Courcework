#ifndef PROFILESETTINGSWIDGET_H
#define PROFILESETTINGSWIDGET_H
#include <gtkmm.h>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
class ProfileSettingsWidget : public Gtk::Box {
protected:
    void on_edit_clicked();
    void on_save_clicked();
    void on_back_clicked();
private:
    Gtk::Entry* nicknameentry = nullptr;
    Gtk::Entry* statusentry = nullptr;
    Gtk::Entry* birthdateentry = nullptr;
    Gtk::Button* editbutton = nullptr;
    Gtk::Button* savebutton = nullptr;
    Gtk::Label* messagelabel = nullptr;
    string current_username;
    void set_fields_editable(bool editable);
    void show_message(const string& text, const string& type);
    void load_css();
public:
    ProfileSettingsWidget();
    void load_profile_data(const string& username);
    sigc::signal<void()> signal_back_clicked;
    sigc::signal<void()> signal_profile_updated;
};
#endif 
