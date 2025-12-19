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
        int questionCount;
        Gtk::CheckButton* checkButton = nullptr;
    };
    Gtk::Label* titleLabel = nullptr;
    Gtk::Box* categoriesBox = nullptr;
    Gtk::SpinButton* questionsSpin = nullptr;
    Gtk::Button* createTestButton = nullptr;
    Gtk::Button* backButton = nullptr;
    Gtk::Label* statusLabel = nullptr;
    vector<CategoryInfo> categories;
    string currentUsername;
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
