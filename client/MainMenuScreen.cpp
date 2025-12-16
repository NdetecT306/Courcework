#include "MainMenuScreen.h"
#include "NetworkClient.h"
MainMenuScreen::MainMenuScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 5) { //Создание 
    set_margin(15);
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    // Заголовок
    title_label = Gtk::make_managed<Gtk::Label>("Система тестирования знаний по информационной безопасности");
    title_label->set_halign(Gtk::Align::CENTER);
    title_label->add_css_class("main-title");
    append(*title_label);
    // Статус подключения
    status_label = Gtk::make_managed<Gtk::Label>();
    status_label->set_halign(Gtk::Align::CENTER);
    status_label->add_css_class("status-label");
    append(*status_label);
    // Кнопки
    buttons_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
    buttons_box->set_margin_top(25);
    buttons_box->set_margin_bottom(25);
    buttons_box->set_halign(Gtk::Align::CENTER);
    buttons_box->set_valign(Gtk::Align::CENTER);
    append(*buttons_box);
    // "Войти"
    login_button = Gtk::make_managed<Gtk::Button>("Войти");
    login_button->add_css_class("main-menu-button");
    login_button->set_size_request(300, 40);
    login_button->signal_clicked().connect(sigc::mem_fun(*this, &MainMenuScreen::on_login_clicked));
    buttons_box->append(*login_button);
    // "Зарегистрироваться"
    register_button = Gtk::make_managed<Gtk::Button>("Зарегистрироваться");
    register_button->add_css_class("main-menu-button");
    register_button->set_size_request(300, 40);
    register_button->signal_clicked().connect(sigc::mem_fun(*this, &MainMenuScreen::on_register_clicked));
    buttons_box->append(*register_button);
    load_css();
}
void MainMenuScreen::set_connection_status(bool connected) { //Подключено или нет, вот в чем вопрос
    if (connected) {
        status_label->set_text("Подключено к серверу");
        status_label->add_css_class("status-success");
        status_label->remove_css_class("status-error");
        login_button->set_sensitive(true);
        register_button->set_sensitive(true);
    } else {
        status_label->set_text("Не удалось подключиться к серверу");
        status_label->add_css_class("status-error");
        status_label->remove_css_class("status-success");
        login_button->set_sensitive(false);
        register_button->set_sensitive(false);
    }
}
void MainMenuScreen::on_login_clicked() { //Вход
    signal_show_login_form.emit();
}
void MainMenuScreen::on_register_clicked() { //Регистрация
    signal_show_register_form.emit();
}
void MainMenuScreen::load_css() { //Стили
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .main-title {
            font-family: Sans;
            font-size: 24px;
            font-weight: bold;
            color: #c78d36;
            background-color: #344955;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            min-width: 600px;
        }
        .main-menu-button {
            font-size: 18px;
            font-weight: bold;
            color: #000000;
            background-color: white;
            padding: 10px;
            border-radius: 8px;
            border: 2px solid #c78d36;
            transition: all 0.3s ease-in-out;
            min-width: 300px;
            min-height: 40px;
        }
        .main-menu-button:hover {
            background-color: #e9ecef;
            transform: translateY(-3px);
            box-shadow: 0 5px 10px rgba(199, 141, 54, 0.3);
        }
        .main-menu-button:disabled {
            opacity: 0.5;
            transform: none;
            box-shadow: none;
        }
        .status-label {
            font-size: 14px;
            padding: 8px;
            margin: 10px;
            border-radius: 4px;
            background-color: #f0f0f0;
            min-height: 30px;
        }
        .status-success {
            background-color: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .status-error {
            background-color: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), css_provider,GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
