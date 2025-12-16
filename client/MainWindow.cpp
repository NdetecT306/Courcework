#include "MainWindow.h"
#include "NetworkClient.h"
#include <iostream>
MainWindow::MainWindow() { //Создание
    set_title("Система тестирования знаний по информационной безопасности");
    set_default_size(1200, 300);
    mainstack = Gtk::make_managed<Gtk::Stack>();
    mainstack->set_transition_type(Gtk::StackTransitionType::SLIDE_LEFT_RIGHT);
    mainstack->set_transition_duration(300);
    set_child(*mainstack);
    mainmenu = Gtk::make_managed<MainMenuScreen>();
    mainstack->add(*mainmenu, "menu", "Главное меню");
    loginform = Gtk::make_managed<LoginFormScreen>();
    mainstack->add(*loginform, "login", "Форма входа");
    registerform = Gtk::make_managed<RegisterFormScreen>();
    mainstack->add(*registerform, "register", "Форма регистрации");
    mainscreen = Gtk::make_managed<MainScreen>();
    mainstack->add(*mainscreen, "main", "Главный экран");
    setup_navigation();
    load_main_css();
    mainstack->set_visible_child(*mainmenu);
    update_connection_status();
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::check_connection), 3000);
}
MainWindow::~MainWindow() {//Уничтожить
    if (g_network_client) {
        g_network_client->disconnect();
        delete g_network_client;
        g_network_client = nullptr;
    }
}
void MainWindow::setup_navigation() { //Регистрация
    mainmenu->signal_show_login_form.connect([this]() {mainstack->set_visible_child(*loginform);});
    mainmenu->signal_show_register_form.connect([this]() {mainstack->set_visible_child(*registerform);});
    loginform->signal_back_to_menu.connect([this]() { mainstack->set_visible_child(*mainmenu);});
    registerform->signal_back_to_menu.connect([this]() {mainstack->set_visible_child(*mainmenu);});
    loginform->signal_login_success.connect(sigc::mem_fun(*this, &MainWindow::on_login_success));
    registerform->signal_register_success.connect([this]() {
        mainstack->set_visible_child(*loginform);
        loginform->show_success_message("Регистрация успешна! Войдите в систему.");
    });
    mainscreen->signal_logout.connect(sigc::mem_fun(*this, &MainWindow::on_logout));
}
void MainWindow::load_main_css() { //Стили
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        window {
            background-color: #ADD8E6;
        }
        .entry {
            font-size: 14px;
            padding: 5px;
            margin: 5px;
        }
        .dialog-label {
            font-size: 15px;
            margin: 5px;
        }
        button {
            background-color: white;
        }
        button label {
            color: #000000;
            font-family: Sans;
            font-size: 14px;
            font-weight: normal;
        }
        .main-menu-button,
        .form-button,
        .next-button,
        .finish-button,
        .save-button,
        .photo-button,
        .edit-button,
        .back-button,
        .username-button,
        .bottom-button {
            background-color: white;
        }
        .category-card {
            background-color: white;
        }
        .category-card label {
            color: #000000;
        }
        .answer-button {
            background-color: white;
        }
        .answer-button label {
            color: #000000;
        }
        .answer-correct {
            background-color: #d4edda;
        }
        .answer-correct label {
            color: #000000;
        }
        .answer-wrong {
            background-color: #f8d7da;
        }
        .answer-wrong label {
            color: #000000;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}
void MainWindow::update_connection_status() { //Статус связи
    if (mainmenu) {
        mainmenu->set_connection_status(g_network_client && g_network_client->is_connected());
    }
}
bool MainWindow::check_connection() { //Проверка связи
    if (g_network_client) {
        bool was_connected = g_network_client->is_connected();
        json response = g_network_client->check_connection();
        bool is_connected = (response["status"] == "success" || response["status"] == "error");
        if (was_connected != is_connected) {
            update_connection_status();
        }
    }
    return true;
}
void MainWindow::on_login_success(const string& username) { //Загрузка и выгрузка
    mainscreen->set_username(username);
    mainstack->set_visible_child(*mainscreen);
}
void MainWindow::on_logout() {
    mainstack->set_visible_child(*mainmenu);
    update_connection_status();
}
