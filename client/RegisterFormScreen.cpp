#include "RegisterFormScreen.h"
#include "NetworkClient.h"

RegisterFormScreen::RegisterFormScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 5) { //Создание
    set_margin(15);
    // Заголовок
    title_label = Gtk::make_managed<Gtk::Label>("Регистрация нового пользователя");
    title_label->set_halign(Gtk::Align::CENTER);
    title_label->add_css_class("form-title");
    append(*title_label);
    // Регистрация
    form_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 8);
    form_box->add_css_class("form-box");
    form_box->set_margin(10);
    form_box->set_halign(Gtk::Align::CENTER);
    form_box->set_valign(Gtk::Align::CENTER);
    append(*form_box);
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
    // Пароль
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
    auto confirmbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    confirmbox->set_halign(Gtk::Align::FILL);
    form_box->append(*confirmbox);
    auto confirmlabel = Gtk::make_managed<Gtk::Label>("Подтверждение:");
    confirmlabel->set_size_request(100, -1);
    confirmbox->append(*confirmlabel);
    confirm_entry = Gtk::make_managed<Gtk::Entry>();
    confirm_entry->set_hexpand(true);
    confirm_entry->set_visibility(false);
    confirm_entry->set_input_purpose(Gtk::InputPurpose::PASSWORD);
    confirmbox->append(*confirm_entry);
    message_label = Gtk::make_managed<Gtk::Label>("");
    message_label->set_halign(Gtk::Align::CENTER);
    message_label->set_visible(false);
    form_box->append(*message_label);
    auto buttonsbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    buttonsbox->set_halign(Gtk::Align::CENTER);
    buttonsbox->set_margin_top(15);
    form_box->append(*buttonsbox);
    register_button = Gtk::make_managed<Gtk::Button>("Зарегистрироваться");
    register_button->add_css_class("form-button");
    register_button->set_size_request(200, 25);
    register_button->signal_clicked().connect(sigc::mem_fun(*this, &RegisterFormScreen::on_register_button_clicked));
    buttonsbox->append(*register_button);
    back_button = Gtk::make_managed<Gtk::Button>("Назад");
    back_button->add_css_class("back-button");
    back_button->set_size_request(150, 25);
    back_button->signal_clicked().connect(sigc::mem_fun(*this, &RegisterFormScreen::on_back_clicked));
    buttonsbox->append(*back_button);
    load_css();
}
void RegisterFormScreen::on_register_button_clicked() { //Регистрация
    string login = login_entry->get_text();
    string password = password_entry->get_text();
    string confirm = confirm_entry->get_text();
    if (login.empty() || password.empty() || confirm.empty()) {
        show_message("Все поля должны быть заполнены!", "error");
        return;
    }
    if (password != confirm) {
        show_message("Пароли не совпадают!", "error");
        return;
    }
    if (!g_network_client || !g_network_client->is_connected()) {
        show_message("Нет подключения к серверу!", "error");
        return;
    }
    json response = g_network_client->register_user(login, password);
    if (response["status"] == "success") {
        show_message("Регистрация успешна!", "success");
        Glib::signal_timeout().connect_once([this]() {signal_register_success.emit();}, 2000);
    } else {
        show_message("" + string(response["message"]), "error");
    }
}
void RegisterFormScreen::on_back_clicked() {
    login_entry->set_text("");
    password_entry->set_text("");
    confirm_entry->set_text("");
    message_label->set_visible(false);
    signal_back_to_menu.emit();
}
void RegisterFormScreen::show_message(const string& text, const string& type) {//Показать сообщение
    message_label->set_text(text);
    message_label->set_visible(true);
    if (type == "success") {
        message_label->add_css_class("success-label");
        message_label->remove_css_class("error-label");
    } else {
        message_label->add_css_class("error-label");
        message_label->remove_css_class("success-label");
    }
}
void RegisterFormScreen::load_css() { //Стиль
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
            min-width: 200px;
            min-height: 25px;
            transition: all 0.2s ease-in-out;
        }
        .form-button:hover {
            background-color: #e9ecef;
            transform: translateY(-1px);
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
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
