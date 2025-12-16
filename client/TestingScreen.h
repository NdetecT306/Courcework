#ifndef TESTINGSCREEN_H
#define TESTINGSCREEN_H

#include <gtkmm.h>
#include <string>
#include <vector>
#include <map>
#include "NetworkClient.h"
using namespace std;

class TestingScreen : public Gtk::Box {
private:
    struct Category {
        int id;
        string name;
        string description;
        Gtk::Button* button = nullptr;
        Gtk::Label* scoreLabel = nullptr;
    };
    Gtk::Label* titleLabel = nullptr;
    Gtk::Grid* categoriesGrid = nullptr;
    std::vector<Category> categories;
    std::map<int, int> userStats;
    void createCategories();
    void updateCategoriesDisplay();
    Gtk::Button* createCategoryCard(const Category& category);
    void updateCategoryScores();
    void onCategoryClicked(int categoryId, const string& categoryName);
    void loadCss();
public:
    TestingScreen();
    void setUserStats(const std::map<int, int>& stats);
    void loadCategoriesFromServer();
    sigc::signal<void(int, const std::string&)> signalStartTest;
};

#endif
