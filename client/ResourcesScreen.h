#ifndef RESOURCESSCREEN_H
#define RESOURCESSCREEN_H
#include <gtkmm.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
struct ResourceItem {
    int id;
    string name;
    string author;
    string link;
    string category;
};
class ResourcesScreen : public Gtk::Box {
protected:
    void clear_resources();
    void create_resource_widget(const ResourceItem& resource);
    void on_link_clicked(const std::string& url);
    void show_error_message(const std::string& message);
private:
    Gtk::ScrolledWindow* scrolled_window = nullptr;
    Gtk::Box* content_box = nullptr;
    vector<ResourceItem> resources;
    void load_css();
public:
    ResourcesScreen();
    void loadResources();
    void clearResources();
    void append(Gtk::Widget& widget) {
        content_box->append(widget);
    }
    sigc::signal<void()> signal_back_to_main;
};

#endif 
