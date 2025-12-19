#include "LoginFormScreen.h"
#include "NetworkClient.h"
#include <chrono>
#include <thread>

extern NetworkClient* g_network_client;

LoginFormScreen::LoginFormScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 5) {
    set_margin(15);
    
    titleLabel = Gtk::make_managed<Gtk::Label>("Вход в систему");
    titleLabel->set_halign(Gtk::Align::CENTER);
    titleLabel->add_css_class("form-title");
    append(*titleLabel);
    
    // Форма входа
    formBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 8);
    formBox->add_css_class("form-box");
    formBox->set_margin(10);
    formBox->set_halign(Gtk::Align::CENTER);
    formBox->set_valign(Gtk::Align::CENTER);
    append(*formBox);
    
    // Логин
    auto loginbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    loginbox->set_halign(Gtk::Align::FILL);
    formBox->append(*loginbox);
    
    auto loginlabel = Gtk::make_managed<Gtk::Label>("Логин:");
    loginlabel->set_size_request(100, -1);
    loginbox->append(*loginlabel);
    
    loginEntry = Gtk::make_managed<Gtk::Entry>();
    loginEntry->set_hexpand(true);
    loginEntry->set_margin_bottom(5);
    loginbox->append(*loginEntry);
    
    // Поле пароля
    auto passwordbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    passwordbox->set_halign(Gtk::Align::FILL);
    formBox->append(*passwordbox);
    
    auto passwordlabel = Gtk::make_managed<Gtk::Label>("Пароль:");
    passwordlabel->set_size_request(100, -1);
    passwordbox->append(*passwordlabel);
    
    passwordEntry = Gtk::make_managed<Gtk::Entry>();
    passwordEntry->set_hexpand(true);
    passwordEntry->set_visibility(false);
    passwordEntry->set_input_purpose(Gtk::InputPurpose::PASSWORD);
    passwordbox->append(*passwordEntry);
    
    // Таймер блокировки
    timerLabel = Gtk::make_managed<Gtk::Label>("");
    timerLabel->set_halign(Gtk::Align::CENTER);
    timerLabel->set_visible(false);
    timerLabel->add_css_class("timer-label");
    formBox->append(*timerLabel);
    
    // Сообщение об ошибке
    messageLabel = Gtk::make_managed<Gtk::Label>("");
    messageLabel->set_halign(Gtk::Align::CENTER);
    messageLabel->set_visible(false);
    formBox->append(*messageLabel);
    
    // Кнопки
    auto buttonsbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    buttonsbox->set_halign(Gtk::Align::CENTER);
    buttonsbox->set_margin_top(15);
    formBox->append(*buttonsbox);
    
    loginButton = Gtk::make_managed<Gtk::Button>("Войти");
    loginButton->add_css_class("form-button");
    loginButton->set_size_request(150, 25);
    loginButton->signal_clicked().connect(sigc::mem_fun(*this, &LoginFormScreen::on_login_button_clicked));
    buttonsbox->append(*loginButton);
    
    backButton = Gtk::make_managed<Gtk::Button>("Назад");
    backButton->add_css_class("back-button");
    backButton->set_size_request(150, 25);
    backButton->signal_clicked().connect(sigc::mem_fun(*this, &LoginFormScreen::on_back_clicked));
    buttonsbox->append(*backButton);
    
    load_css();
}

void LoginFormScreen::reset_login_button() {
    if (loginButton) {
        loginButton->set_sensitive(true);
        loginButton->set_label("Войти");
    }
}

void LoginFormScreen::reset_form() {
    if (loginEntry) loginEntry->set_sensitive(true);
    if (passwordEntry) passwordEntry->set_sensitive(true);
    reset_login_button();
    if (messageLabel) messageLabel->set_visible(false);
    if (timerLabel) {
        timerLabel->set_visible(false);
        timerLabel->remove_css_class("error-label");
        timerLabel->remove_css_class("warning-label");
    }
}

void LoginFormScreen::show_success_message(const string& message) {
    show_message(message, "success");
}

void LoginFormScreen::on_login_button_clicked() {
    string login = loginEntry->get_text();
    string password = passwordEntry->get_text();
    
    if (login.empty() || password.empty()) {
        show_message("Логин и пароль не могут быть пустыми!", "error");
        return;
    }
    
    if (!g_network_client || !g_network_client->is_connected()) {
        show_message("Нет подключения к серверу!", "error");
        return;
    }
    
    loginButton->set_sensitive(false);
    loginButton->set_label("Проверка...");
    
    // Отправляем запрос на сервер
    json response = g_network_client->login(login, password);
    
    // Проверяем, заблокирован ли пользователь
    if (response.contains("blocked") && response["blocked"] == true) {
        int remaining_time = response["remaining_time"];
        show_blocked_message(remaining_time);
        return;
    }
    
    if (response["status"] == "success") {
        show_message("Успешный вход!", "success");
        reset_login_button();
        Glib::signal_timeout().connect_once([this, login]() {
            signal_login_success.emit(login);
        }, 1000);
    } else {
        show_message("" + string(response["message"]), "error");
        reset_login_button();
    }
}

void LoginFormScreen::show_blocked_message(int seconds) {
    timerLabel->set_visible(true);
    timerLabel->set_text("Подождите " + to_string(seconds) + " секунд...");
    timerLabel->add_css_class("error-label");
    
    // Блокируем форму
    loginEntry->set_sensitive(false);
    passwordEntry->set_sensitive(false);
    loginButton->set_sensitive(false);
    
    // Запускаем обратный отсчет в отдельном потоке
    auto countdown = [this, seconds]() {
        for (int i = seconds; i > 0; --i) {
            Glib::signal_idle().connect_once([this, i]() {
                if (timerLabel) {
                    timerLabel->set_text("Подождите " + to_string(i) + " секунд...");
                    if (i <= 3) {
                        timerLabel->add_css_class("warning-label");
                    }
                }
            });
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        Glib::signal_idle().connect_once([this]() {
            if (timerLabel) {
                timerLabel->set_visible(false);
                timerLabel->remove_css_class("error-label");
                timerLabel->remove_css_class("warning-label");
            }
            if (loginEntry) loginEntry->set_sensitive(true);
            if (passwordEntry) passwordEntry->set_sensitive(true);
            reset_login_button();
            if (messageLabel) {
                messageLabel->set_text("Можно попробовать снова");
                messageLabel->add_css_class("info-label");
                messageLabel->set_visible(true);
                Glib::signal_timeout().connect_once([this]() {
                    if (messageLabel) messageLabel->set_visible(false);
                }, 3000);
            }
        });
    };
    
    std::thread(countdown).detach();
}

void LoginFormScreen::on_back_clicked() {
    if (loginEntry) loginEntry->set_text("");
    if (passwordEntry) passwordEntry->set_text("");
    if (messageLabel) messageLabel->set_visible(false);
    if (timerLabel) timerLabel->set_visible(false);
    reset_login_button();
    signal_back_to_menu.emit();
}

void LoginFormScreen::show_message(const string& text, const string& type) {
    if (!messageLabel) return;
    
    messageLabel->set_text(text);
    messageLabel->set_visible(true);
    
    if (type == "success") {
        messageLabel->add_css_class("success-label");
        messageLabel->remove_css_class("error-label");
        messageLabel->remove_css_class("info-label");
    } else if (type == "error") {
        messageLabel->add_css_class("error-label");
        messageLabel->remove_css_class("success-label");
        messageLabel->remove_css_class("info-label");
    } else if (type == "info") {
        messageLabel->add_css_class("info-label");
        messageLabel->remove_css_class("success-label");
        messageLabel->remove_css_class("error-label");
    }
    
    // Скрываем сообщение через 3 секунды
    Glib::signal_timeout().connect_once([this]() {
        if (messageLabel) {
            messageLabel->set_visible(false);
        }
    }, 3000);
}

void LoginFormScreen::load_css() {
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
