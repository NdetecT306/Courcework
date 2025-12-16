#ifndef MAINSCREEN_H
#define MAINSCREEN_H
#include "TestScreen.h"
#include "TestingScreen.h"
#include "CustomTestScreen.h"
#include "ProfileSettingsWidget.h"
#include "ResourcesScreen.h"
#include <gtkmm.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;
class MainScreen : public Gtk::Box {
protected:
    void on_resources_clicked();
    void on_testing_clicked();
    void on_custom_test_clicked();
    void on_statistics_clicked();
    void on_settings_clicked();
    void on_username_clicked();
    void on_start_test(int category_id, const string& category_name);
    void on_start_custom_test(int total_questions, const string& test_name, const set<int>& category_ids, int dummy);
    void on_test_finished();
    void on_back_to_categories();
    void on_back_from_settings();
    void on_back_from_custom_test();
    void on_back_from_resources();
private:
    Gtk::CenterBox* header_box = nullptr;
    Gtk::Label* title_label = nullptr;
    Gtk::Button* username_button = nullptr;
    Gtk::Stack* content_stack = nullptr;
    Gtk::Box* bottom_buttons_box = nullptr;
    ResourcesScreen* resources_screen = nullptr;
    TestingScreen* testing_screen = nullptr;
    TestScreen* test_screen = nullptr;
    CustomTestScreen* custom_test_screen = nullptr;
    ProfileSettingsWidget* settings_widget = nullptr;
    string current_username;
    void load_user_statistics();
    void load_statistics_display();
    void show_error_dialog(const string& message);
    void load_css();
public:
    MainScreen();
    void set_username(const string& username);
    sigc::signal<void()> signal_logout;
};
#endif // MAINSCREEN_H
