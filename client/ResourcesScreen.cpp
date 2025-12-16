#include "ResourcesScreen.h"
#include "NetworkClient.h"
#include <iostream>
#include <cstdlib>
extern NetworkClient* g_network_client;
ResourcesScreen::ResourcesScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 10) { //Создание
    set_margin(10);
    scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow>();
    scrolled_window->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrolled_window->set_hexpand(true);
    scrolled_window->set_vexpand(true);
    Gtk::Box::append(*scrolled_window); 
    content_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
    content_box->set_margin(10);
    scrolled_window->set_child(*content_box);
    load_css();
}
void ResourcesScreen::loadResources() { //Загрузка
    clearResources();
    if (!g_network_client || !g_network_client->is_connected()) {
        show_error_message("Нет подключения к серверу");
        return;
    }
    json response = g_network_client->get_all_resources();
    if (response["status"] == "success" && response.contains("resources")) {
        for (const auto& resource_data : response["resources"]) {
            ResourceItem item;
            item.id = resource_data["id"];
            item.name = resource_data["name"];
            item.author = resource_data["author"];
            item.link = resource_data["link"];
            item.category = resource_data["category"];
            resources.push_back(item);
            create_resource_widget(item);
        }
        if (resources.empty()) {
            auto emptylabel = Gtk::make_managed<Gtk::Label>("Нет доступных ресурсов");
            emptylabel->add_css_class("empty-label");
            content_box->append(*emptylabel);
        }
    } else {
        show_error_message("Не удалось загрузить ресурсы");
    }
}
void ResourcesScreen::clearResources() {
    while (Gtk::Widget* child = content_box->get_first_child()) {
        content_box->remove(*child);
    }
    resources.clear();
}
void ResourcesScreen::clear_resources() {
    clearResources();
}
void ResourcesScreen::create_resource_widget(const ResourceItem& resource) { //Кнопки ресурсов
    auto rescontainer = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
    rescontainer->add_css_class("resource-container");
    rescontainer->set_margin_bottom(10);
    // Категория
    auto catlabel = Gtk::make_managed<Gtk::Label>("[" + resource.category + "]");
    catlabel->add_css_class("resource-category");
    catlabel->set_halign(Gtk::Align::START);
    rescontainer->append(*catlabel);
    // Название ресурса
    auto namelabel = Gtk::make_managed<Gtk::Label>(resource.name);
    namelabel->add_css_class("resource-name");
    namelabel->set_wrap(true);
    namelabel->set_halign(Gtk::Align::START);
    rescontainer->append(*namelabel);
    // Автор (если есть)
    if (!resource.author.empty()) {
        auto authorlabel = Gtk::make_managed<Gtk::Label>("Автор: " + resource.author);
        authorlabel->add_css_class("resource-author");
        authorlabel->set_halign(Gtk::Align::START);
        rescontainer->append(*authorlabel);
    }
    auto linkbutton = Gtk::make_managed<Gtk::Button>("Открыть ссылку");
    linkbutton->add_css_class("resource-link-button");
    linkbutton->set_halign(Gtk::Align::START);
    linkbutton->signal_clicked().connect([this, resource]() {
        on_link_clicked(resource.link);
    });
    rescontainer->append(*linkbutton);
    content_box->append(*rescontainer);
}
void ResourcesScreen::on_link_clicked(const string& url) { //Открыть
    string command = "xdg-open \"" + url + "\"";
    int result = system(command.c_str());
    (void)result;
}
void ResourcesScreen::show_error_message(const string& message) {
    auto errorlabel = Gtk::make_managed<Gtk::Label>(message);
    errorlabel->add_css_class("error-label");
    content_box->append(*errorlabel);
}
void ResourcesScreen::load_css() { //Стили
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .resource-container {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 8px;
            padding: 15px;
            min-width: 600px;
            max-width: 800px;
        }
        .resource-category {
            font-family: Sans;
            font-size: 14px;
            font-weight: bold;
            color: #6c757d;
            margin-bottom: 5px;
        }
        .resource-name {
            font-family: Sans;
            font-size: 16px;
            font-weight: bold;
            color: #212529;
            margin-bottom: 5px;
        }
        .resource-author {
            font-family: Sans;
            font-size: 14px;
            color: #495057;
            margin-bottom: 10px;
            font-style: italic;
        }
        .resource-link-button {
            font-family: Sans;
            font-size: 14px;
            color: #000000 !important;
            background-color: white;
            border: 1px solid #28a745;
            border-radius: 5px;
            padding: 8px 15px;
            transition: all 0.2s ease-in-out;
        }
        .resource-link-button:hover {
            background-color: #e9ecef;
            transform: translateY(-1px);
            box-shadow: 0 2px 4px rgba(40, 167, 69, 0.2);
        }
        .empty-label {
            font-family: Sans;
            font-size: 16px;
            color: #6c757d;
            padding: 20px;
            text-align: center;
        }
        .error-label {
            font-family: Sans;
            font-size: 14px;
            color: #721c24;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 10px;
            border-radius: 5px;
            text-align: center;
        }
        scrolledwindow {
            background-color: #ADD8E6;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}
