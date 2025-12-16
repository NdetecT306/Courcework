#ifndef CUSTOMTESTSCREEN_H
#define CUSTOMTESTSCREEN_H

#include <gtkmm.h>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include "NetworkClient.h"
using namespace std;
class CustomTestScreen : public Gtk::Box {
private:
    struct CategoryInfo {
        int id;
        string name;
        int question_count;
        Gtk::CheckButton* check_button = nullptr;
    };
    Gtk::Label* title_label = nullptr;
    Gtk::Box* categories_box = nullptr;
    Gtk::SpinButton* questions_spin = nullptr;
    Gtk::Button* create_test_button = nullptr;
    Gtk::Button* back_button = nullptr;
    Gtk::Label* status_label = nullptr;
    vector<CategoryInfo> categories;
    string current_username;
    void load_css();
    void create_test();
    void on_category_toggled();
public:
    CustomTestScreen();
    void load_categories(const std::string& username);
    sigc::signal<void()> signal_back_to_main;
    sigc::signal<void(int, const string&, const set<int>&, int)> signal_start_custom_test;
};
#endif
