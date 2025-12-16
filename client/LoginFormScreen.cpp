#include "LoginFormScreen.h"
#include "NetworkClient.h"

LoginFormScreen::LoginFormScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 5) { //Создание
    set_margin(15);
    // Заголовок
    title_label = Gtk::make_managed<Gtk::Label>("Вход в систему");
    title_label->set_halign(Gtk::Align::CENTER);
    title_label->add_css_class("form-title");
    append(*title_label);
    // Форма входа
    form_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 8);
    form_box->add_css_class("form-box");
    form_box->set_margin(10);
    form_box->set_halign(Gtk::Align::CENTER);
    form_box->set_valign(Gtk::Align::CENTER);
    append(*form_box);
    // Поле логина
    auto loginbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    loginbox->set_halign(Gtk::Align::FILL);
    form_box->append(*loginbox);
    auto loginlabel = Gtk::make_managed<Gtk::Label>("Логин:");
    loginlabel->set_size_request(100, -1);
    loginbox->append(*loginlabel);
    login_entry = Gtk::make_managed<Gtk::Entry>();
    login_entry->set_hexpand(true);
    login_entry->set_margin_bottom(5);
    loginbox->append(*login_entry);
    // Поле пароля
    auto passwordbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    passwordbox->set_halign(Gtk::Align::FILL);
    form_box->append(*passwordbox);
    auto passwordlabel = Gtk::make_managed<Gtk::Label>("Пароль:");
    passwordlabel->set_size_request(100, -1);
    passwordbox->append(*passwordlabel);
    password_entry = Gtk::make_managed<Gtk::Entry>();
    password_entry->set_hexpand(true);
    password_entry->set_visibility(false);
    password_entry->set_input_purpose(Gtk::InputPurpose::PASSWORD);
    passwordbox->append(*password_entry);
    // Таймер блокировки
    timer_label = Gtk::make_managed<Gtk::Label>("");
    timer_label->set_halign(Gtk::Align::CENTER);
    timer_label->set_visible(false);
    timer_label->add_css_class("timer-label");
    form_box->append(*timer_label);
    // Сообщение об ошибке
    message_label = Gtk::make_managed<Gtk::Label>("");
    message_label->set_halign(Gtk::Align::CENTER);
    message_label->set_visible(false);
    form_box->append(*message_label);
    // Кнопки
    auto buttonsbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    buttonsbox->set_halign(Gtk::Align::CENTER);
    buttonsbox->set_margin_top(15);
    form_box->append(*buttonsbox);
    login_button = Gtk::make_managed<Gtk::Button>("Войти");
    login_button->add_css_class("form-button");
    login_button->set_size_request(150, 25);
    login_button->signal_clicked().connect(sigc::mem_fun(*this, &LoginFormScreen::on_login_button_clicked));
    buttonsbox->append(*login_button);
    back_button = Gtk::make_managed<Gtk::Button>("Назад");
    back_button->add_css_class("back-button");
    back_button->set_size_request(150, 25);
    back_button->signal_clicked().connect(sigc::mem_fun(*this, &LoginFormScreen::on_back_clicked));
    buttonsbox->append(*back_button);
    load_css();
}
void LoginFormScreen::show_success_message(const string& message) { //Показть сообщение
    show_message(message, "success");
}
void LoginFormScreen::on_login_button_clicked() { //Попытка войти
    string login = login_entry->get_text();
    string password = password_entry->get_text();
    if (login.empty() || password.empty()) {
        show_message("Логин и пароль не могут быть пустыми!", "error");
        return;
    }
    if (!g_network_client || !g_network_client->is_connected()) {
        show_message("Нет подключения к серверу!", "error");
        return;
    }
    login_button->set_sensitive(false);
    login_button->set_label("Проверка...");
    // Отправляем запрос на сервер
    json response = g_network_client->login(login, password);
    // Проверяем, заблоирован ли пользователь
    if (response.contains("blocked") && response["blocked"] == true) {
        int remaining_time = response["remaining_time"];
        show_blocked_message(remaining_time);
        login_button->set_sensitive(true);
        login_button->set_label("Войти");
        return;
    }
    if (response["status"] == "success") {
        show_message("Успешный вход!", "success");
        Glib::signal_timeout().connect_once([this, login]() {signal_login_success.emit(login);}, 1000);
    } else {
        show_message("" + string(response["message"]), "error");
        login_button->set_sensitive(true);
        login_button->set_label("Войти");
    }
}
void LoginFormScreen::show_blocked_message(int seconds) { //Блокировка 
    timer_label->set_visible(true);
    timer_label->set_text("Подождите " + to_string(seconds) + " секунд...");
    timer_label->add_css_class("error-label");
    // Блокируем форму
    login_entry->set_sensitive(false);
    password_entry->set_sensitive(false);
    login_button->set_sensitive(false);
    // Запускаем обратный отсчет
    auto countdown = [this, seconds]() {
        for (int i = seconds; i > 0; --i) {
            Glib::signal_idle().connect_once([this, i]() {
                timer_label->set_text("Подождите " + to_string(i) + " секунд...");
                if (i <= 3) {
                    timer_label->add_css_class("warning-label");
                }
            });
            this_thread::sleep_for(chrono::seconds(1));
        }
        // По окончании таймера разблокируем форму
        Glib::signal_idle().connect_once([this]() {
            timer_label->set_visible(false);
            timer_label->remove_css_class("error-label");
            timer_label->remove_css_class("warning-label");
            login_entry->set_sensitive(true);
            password_entry->set_sensitive(true);
            login_button->set_sensitive(true);
            login_button->set_label("Войти");
            show_message("Можно попробовать снова", "info");
        });
    };
    // Запускаем таймер в отдельном потоке
    thread(countdown).detach();
}
void LoginFormScreen::on_back_clicked() { //Назад
    login_entry->set_text("");
    password_entry->set_text("");
    message_label->set_visible(false);
    timer_label->set_visible(false);
    signal_back_to_menu.emit();
}
void LoginFormScreen::show_message(const string& text, const string& type) { //Сообщение
    message_label->set_text(text);
    message_label->set_visible(true);
    if (type == "success") {
        message_label->add_css_class("success-label");
        message_label->remove_css_class("error-label");
        message_label->remove_css_class("info-label");
    } else if (type == "error") {
        message_label->add_css_class("error-label");
        message_label->remove_css_class("success-label");
        message_label->remove_css_class("info-label");
    } else if (type == "info") {
        message_label->add_css_class("info-label");
        message_label->remove_css_class("success-label");
        message_label->remove_css_class("error-label");
    }
    // Скрываем сообщение через 3 секунды
    Glib::signal_timeout().connect_once([this]() {
        message_label->set_visible(false);
    }, 3000);
}
void LoginFormScreen::load_css() { //Стили
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .form-title {
            font-family: Sans;
            font-size: 21px;
            font-weight: bold;
            color: #344955;
            padding: 10px;
            margin-bottom: 5px;
        }
        .form-box {
            background-color: #f9f9f9;
            padding: 15px;
            border-radius: 5px;
            border: 2px solid #344955;
            min-width: 400px;
        }
        .form-button {
            font-size: 14px;
            font-weight: bold;
            color: #000000;
            background-color: white;
            border: 1px solid #4CAF50;
            padding: 5px;
            border-radius: 5px;
            min-width: 150px;
            min-height: 25px;
            transition: all 0.2s ease-in-out;
        }
        .form-button:hover {
            background-color: #e9ecef;
            transform: translateY(-1px);
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }
        .form-button:disabled {
            background-color: #cccccc;
            border-color: #999999;
            color: #666666;
            transform: none;
            box-shadow: none;
        }
        .back-button {
            font-size: 14px;
            font-weight: bold;
            color: #000000;
            background-color: white;
            border: 1px solid #344955;
            padding: 5px;
            border-radius: 5px;
            min-width: 150px;
            min-height: 25px;
            transition: all 0.2s ease-in-out;
        }
        .back-button:hover {
            background-color: #e9ecef;
            transform: translateY(-1px);
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }
        entry {
            font-size: 12px;
            padding: 4px;
            border: 1px solid #ddd;
            border-radius: 3px;
            min-height: 25px;
        }
        entry:disabled {
            background-color: #f5f5f5;
            color: #999999;
        }
        .success-label {
            color: #155724;
            background-color: #d4edda;
            border: 1px solid #c3e6cb;
            padding: 5px;
            border-radius: 3px;
            font-size: 12px;
            min-height: 20px;
        }
        .error-label {
            color: #721c24;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 5px;
            border-radius: 3px;
            font-size: 12px;
            min-height: 20px;
        }
        .info-label {
            color: #0c5460;
            background-color: #d1ecf1;
            border: 1px solid #bee5eb;
            padding: 5px;
            border-radius: 3px;
            font-size: 12px;
            min-height: 20px;
        }
        .timer-label {
            font-size: 14px;
            font-weight: bold;
            color: #856404;
            background-color: #fff3cd;
            border: 2px solid #ffeaa7;
            padding: 8px;
            border-radius: 5px;
            margin: 10px 0;
            min-height: 30px;
        }
        .warning-label {
            color: #721c24;
            background-color: #f8d7da;
            border-color: #f5c6cb;
            animation: pulse 1s infinite;
        }
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.7; }
            100% { opacity: 1; }
        }
        label {
            font-size: 12px;
            min-height: 25px;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}
