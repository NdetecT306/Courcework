#include "ProfileSettingsWidget.h"
#include "NetworkClient.h"

extern NetworkClient* g_network_client;
ProfileSettingsWidget::ProfileSettingsWidget() : Gtk::Box(Gtk::Orientation::VERTICAL, 5) { //Создание
    set_margin(10);
    // Заголовок
    auto titlelabel = Gtk::make_managed<Gtk::Label>("Настройки профиля");
    titlelabel->add_css_class("title-2");
    titlelabel->set_halign(Gtk::Align::CENTER);
    append(*titlelabel);
    // Редактирование
    auto form_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 8);
    form_box->add_css_class("form-box");
    append(*form_box);
    // Никнейм
    auto nicknamebox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    nicknamebox->set_halign(Gtk::Align::FILL);
    form_box->append(*nicknamebox);
    auto nicknamelabel = Gtk::make_managed<Gtk::Label>("Никнейм:");
    nicknamelabel->set_size_request(120, -1);
    nicknamelabel->set_halign(Gtk::Align::START);
    nicknamebox->append(*nicknamelabel);
    nicknameentry = Gtk::make_managed<Gtk::Entry>();
    nicknameentry->set_hexpand(true);
    nicknamebox->append(*nicknameentry);
    // Статус
    auto statusbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    statusbox->set_halign(Gtk::Align::FILL);
    form_box->append(*statusbox);
    auto statuslabel = Gtk::make_managed<Gtk::Label>("Статус:");
    statuslabel->set_size_request(120, -1);
    statuslabel->set_halign(Gtk::Align::START);
    statusbox->append(*statuslabel);
    statusentry = Gtk::make_managed<Gtk::Entry>();
    statusentry->set_hexpand(true);
    statusbox->append(*statusentry);
    // Дата рождения
    auto birthdatebox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    birthdatebox->set_halign(Gtk::Align::FILL);
    form_box->append(*birthdatebox);
    auto birthdatelabel = Gtk::make_managed<Gtk::Label>("Дата рождения:");
    birthdatelabel->set_size_request(120, -1);
    birthdatelabel->set_halign(Gtk::Align::START);
    birthdatebox->append(*birthdatelabel);
    birthdateentry = Gtk::make_managed<Gtk::Entry>();
    birthdateentry->set_placeholder_text("ДД.ММ.ГГГГ");
    birthdateentry->set_hexpand(true);
    birthdatebox->append(*birthdateentry);
    // Редактирование/сохранение
    auto buttonbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    buttonbox->set_halign(Gtk::Align::CENTER);
    buttonbox->set_margin_top(15);
    append(*buttonbox);
    editbutton = Gtk::make_managed<Gtk::Button>("Редактировать");
    editbutton->add_css_class("edit-button");
    editbutton->signal_clicked().connect(sigc::mem_fun(*this, &ProfileSettingsWidget::on_edit_clicked));
    buttonbox->append(*editbutton);
    savebutton = Gtk::make_managed<Gtk::Button>("Сохранить");
    savebutton->add_css_class("save-button");
    savebutton->set_sensitive(false);
    savebutton->signal_clicked().connect(sigc::mem_fun(*this, &ProfileSettingsWidget::on_save_clicked));
    buttonbox->append(*savebutton);
    auto backbutton = Gtk::make_managed<Gtk::Button>("Назад");
    backbutton->add_css_class("back-button");
    backbutton->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ProfileSettingsWidget::on_back_clicked)));
    buttonbox->append(*backbutton);
    messagelabel = Gtk::make_managed<Gtk::Label>("");
    messagelabel->set_halign(Gtk::Align::CENTER);
    messagelabel->set_visible(false);
    append(*messagelabel);
    load_css();
}
void ProfileSettingsWidget::load_profile_data(const string& username) { //Информация о данных
    current_username = username;
    if (!g_network_client || !g_network_client->is_connected()) {
        show_message("Нет подключения к серверу", "error");
        return;
    }
    json response = g_network_client->get_profile(username);
    if (response["status"] == "success") {
        json profile = response["profile"];
        if (profile.contains("nickname") && !profile["nickname"].is_null()) {
            nicknameentry->set_text(profile["nickname"].get<std::string>());
        } else {
            nicknameentry->set_text(username);
        }
        if (profile.contains("status") && !profile["status"].is_null()) {
            statusentry->set_text(profile["status"].get<std::string>());
        } else {
            statusentry->set_text("");
        }
        if (profile.contains("birthdate") && !profile["birthdate"].is_null()) {
            birthdateentry->set_text(profile["birthdate"].get<std::string>());
        } else {
            birthdateentry->set_text("");
        }
    } else {
        nicknameentry->set_text(username);
        statusentry->set_text("");
        birthdateentry->set_text("");
    }
    set_fields_editable(false);
}
void ProfileSettingsWidget::load_css() {//Стили
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .title-2 {
            font-family: Sans;
            font-size: 18px;
            font-weight: bold;
            color: #344955;
            padding: 5px;
            margin-bottom: 5px;
        }
        .form-box {
            background-color: #f9f9f9;
            padding: 10px;
            border-radius: 5px;
            border: 1px solid #ddd;
            min-width: 400px;
        }
        .edit-button {
            background-color: white;
            color: #000000 !important;
            border: 1px solid #344955;
            padding: 5px 10px;
            border-radius: 3px;
            font-size: 12px;
            min-height: 25px;
        }
        .edit-button:hover {
            background-color: #e9ecef;
        }
        .save-button {
            background-color: white;
            color: #000000 !important;
            border: 1px solid #4CAF50;
            padding: 5px 10px;
            border-radius: 3px;
            font-size: 12px;
            min-height: 25px;
        }
        .save-button:hover {
            background-color: #e9ecef;
        }
        .save-button:disabled {
            background-color: #cccccc;
            border-color: #999999;
            color: #666666 !important;
        }
        .back-button {
            background-color: white;
            color: #000000 !important;
            border: 1px solid #344955;
            padding: 5px 10px;
            border-radius: 3px;
            font-size: 12px;
            min-height: 25px;
        }
        .back-button:hover {
            background-color: #e9ecef;
        }
        button {
            font-size: 12px;
            min-height: 25px;
            padding: 5px 15px;
        }
        label {
            font-size: 12px;
        }
        entry {
            font-size: 12px;
            min-height: 25px;
            padding: 3px;
        }
        .success-label {
            color: #155724;
            background-color: #d4edda;
            border: 1px solid #c3e6cb;
            padding: 5px;
            border-radius: 3px;
            text-align: center;
            font-size: 12px;
        }
        .error-label {
            color: #721c24;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 5px;
            border-radius: 3px;
            text-align: center;
            font-size: 12px;
        }
        .info-label {
            color: #0c5460;
            background-color: #d1ecf1;
            border: 1px solid #bee5eb;
            padding: 5px;
            border-radius: 3px;
            text-align: center;
            font-size: 12px;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(),css_provider,GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
void ProfileSettingsWidget::set_fields_editable(bool editable) { 
    nicknameentry->set_sensitive(editable);
    statusentry->set_sensitive(editable);
    birthdateentry->set_sensitive(editable);
    savebutton->set_sensitive(editable);
}
void ProfileSettingsWidget::on_edit_clicked() {
    set_fields_editable(true);
    editbutton->set_sensitive(false);
    show_message("Режим редактирования", "info");
}
void ProfileSettingsWidget::on_save_clicked() { //Сохранить
    if (!g_network_client || !g_network_client->is_connected()) {
        show_message("Нет подключения к серверу", "error");
        return;
    }
    try {
        json profile;
        profile["login"] = current_username;
        profile["nickname"] = nicknameentry->get_text();
        profile["status"] = statusentry->get_text();
        profile["birthdate"] = birthdateentry->get_text();
        profile["photo_path"] = ""; 
        json response = g_network_client->save_profile(profile);
        if (response["status"] == "success") {
            show_message("Профиль успешно сохранен", "success");
            set_fields_editable(false);
            editbutton->set_sensitive(true);
            signal_profile_updated.emit();
        } else {
            show_message("Ошибка сохранения профиля: " + string(response["message"]), "error");
        }
    } catch (const std::exception& e) {
        show_message("Ошибка: " + string(e.what()), "error");
    }
}
void ProfileSettingsWidget::on_back_clicked() { //Назад
    signal_back_clicked.emit();
}
void ProfileSettingsWidget::show_message(const string& text, const string& type) { //Сообщение
    messagelabel->set_text(text);
    messagelabel->set_visible(true);
    if (type == "success") {
        messagelabel->add_css_class("success-label");
        messagelabel->remove_css_class("error-label");
        messagelabel->remove_css_class("info-label");
    } else if (type == "error") {
        messagelabel->add_css_class("error-label");
        messagelabel->remove_css_class("success-label");
        messagelabel->remove_css_class("info-label");
    } else {
        messagelabel->add_css_class("info-label");
        messagelabel->remove_css_class("success-label");
        messagelabel->remove_css_class("error-label");
    }
    Glib::signal_timeout().connect_once([this]() {messagelabel->set_visible(false);}, 3000);
}
