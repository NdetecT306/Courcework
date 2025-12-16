#include "CustomTestScreen.h"
#include <iostream>
#include <sstream>
CustomTestScreen::CustomTestScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 10) { //Создание
    set_margin(20);
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    // Заголовок
    title_label = Gtk::make_managed<Gtk::Label>("Создание пользовательского теста");
    title_label->add_css_class("custom-test-title");
    title_label->set_halign(Gtk::Align::CENTER);
    append(*title_label);
    // Основной контейнер
    auto maincontainer = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 20);
    maincontainer->set_halign(Gtk::Align::CENTER);
    maincontainer->set_valign(Gtk::Align::CENTER);
    append(*maincontainer);
    // Категории
    auto categories_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
    categories_container->set_halign(Gtk::Align::START);
    maincontainer->append(*categories_container);
    // Заголовок категорий
    auto categoriesheader = Gtk::make_managed<Gtk::Label>("Категории:");
    categoriesheader->add_css_class("categories-header");
    categoriesheader->set_halign(Gtk::Align::START);
    categories_container->append(*categoriesheader);
    // Контейнер для категорий
    categories_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
    categories_box->add_css_class("categories-box");
    categories_container->append(*categories_box);
    // Настройки для теста
    auto settingscontainer = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 15);
    settingscontainer->set_halign(Gtk::Align::START);
    settingscontainer->set_margin_start(20);
    maincontainer->append(*settingscontainer);
    auto settingsheader = Gtk::make_managed<Gtk::Label>("Настройки:");
    settingsheader->add_css_class("settings-header");
    settingsheader->set_halign(Gtk::Align::START);
    settingscontainer->append(*settingsheader);
    // Количество вопросов
    auto questionsrow = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    questionsrow->set_halign(Gtk::Align::START);
    questionsrow->add_css_class("questions-row");
    settingscontainer->append(*questionsrow);
    auto questionslabel = Gtk::make_managed<Gtk::Label>("Вопросов:");
    questionslabel->add_css_class("questions-label");
    questionslabel->set_size_request(80, -1);
    questionsrow->append(*questionslabel);
    questions_spin = Gtk::make_managed<Gtk::SpinButton>();
    questions_spin->set_range(1, 100);
    questions_spin->set_value(10);
    questions_spin->set_increments(1, 5);
    questions_spin->add_css_class("questions-spin");
    questions_spin->set_size_request(60, -1);
    questions_spin->set_sensitive(false);
    questionsrow->append(*questions_spin);
    // Статусное сообщение
    status_label = Gtk::make_managed<Gtk::Label>("Выберите категории слева");
    status_label->add_css_class("status-label");
    status_label->set_halign(Gtk::Align::START);
    settingscontainer->append(*status_label);
    // Кнопки
    auto buttonsbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    buttonsbox->set_halign(Gtk::Align::CENTER);
    buttonsbox->set_margin_top(20);
    settingscontainer->append(*buttonsbox);
    // Кнопка создания теста
    create_test_button = Gtk::make_managed<Gtk::Button>("Создать тест");
    create_test_button->add_css_class("create-test-button");
    create_test_button->set_size_request(120, 35);
    create_test_button->set_sensitive(false);
    create_test_button->signal_clicked().connect(sigc::mem_fun(*this, &CustomTestScreen::create_test));
    buttonsbox->append(*create_test_button);
    // Кнопка назад
    back_button = Gtk::make_managed<Gtk::Button>("Назад");
    back_button->add_css_class("back-button");
    back_button->set_size_request(120, 35);
    back_button->signal_clicked().connect([this]() {signal_back_to_main.emit();});
    buttonsbox->append(*back_button);
    load_css();
}
void CustomTestScreen::load_categories(const string& username) { //Загрузка нашего чуда
    current_username = username;
    // Очищаем предыдущие категории
    categories.clear();
    while (Gtk::Widget* child = categories_box->get_first_child()) {
        categories_box->remove(*child);
    }
    if (!g_network_client || !g_network_client->is_connected()) {
        status_label->set_text("Нет подключения к серверу");
        status_label->add_css_class("error-label");
        return;
    }
    json response = g_network_client->get_categories();
    if (response["status"] == "success" && response.contains("categories")) {
        for (const auto& cat_data : response["categories"]) {
            CategoryInfo cat;
            cat.id = cat_data["id"];
            cat.name = cat_data["name"];
            cat.question_count = 0;
            json questions_response = g_network_client->get_questions(cat.id);
            if (questions_response["status"] == "success" && 
                questions_response.contains("questions")) {
                cat.question_count = questions_response["questions"].size();
            }
            categories.push_back(cat);
        }
        for (auto& cat : categories) {
            auto checkbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);
            checkbox->set_halign(Gtk::Align::START);
            checkbox->add_css_class("category-checkbox");
            cat.check_button = Gtk::make_managed<Gtk::CheckButton>();
            cat.check_button->add_css_class("category-check");
            cat.check_button->signal_toggled().connect(sigc::mem_fun(*this, &CustomTestScreen::on_category_toggled));
            checkbox->append(*cat.check_button);
            // Название категории
            string labeltext = cat.name + " (" + to_string(cat.question_count) + ")";
            auto label = Gtk::make_managed<Gtk::Label>(labeltext);
            label->add_css_class("category-label");
            label->set_halign(Gtk::Align::START);
            checkbox->append(*label);
            categories_box->append(*checkbox);
        }
        if (categories.empty()) {
            status_label->set_text("Нет доступных категорий");
            status_label->add_css_class("warning-label");
        } else {
            status_label->set_text("Выберите категории слева");
            status_label->remove_css_class("error-label");
            status_label->remove_css_class("warning-label");
        }
    } else {
        status_label->set_text("Не удалось загрузить категории");
        status_label->add_css_class("error-label");
    }
}
void CustomTestScreen::create_test() { //Создание своего теста
    set<int> selected_categories;
    int total_questions = 0;
    int max_questions = 0;
    // Выбранные категории
    for (const auto& cat : categories) {
        if (cat.check_button && cat.check_button->get_active()) {
            selected_categories.insert(cat.id);
            max_questions += min(cat.question_count, 10); 
        }
    }
    if (selected_categories.empty()) {
        status_label->set_text("Выберите хотя бы одну категорию");
        status_label->add_css_class("error-label");
        return;
    }
    int requested_questions = questions_spin->get_value_as_int();
    if (requested_questions > max_questions) {
        string error_msg = "Максимум: " + to_string(max_questions);
        status_label->set_text(error_msg);
        status_label->add_css_class("error-label");
        return;
    }
    if (requested_questions < 1) {
        status_label->set_text("Минимум 1 вопрос");
        status_label->add_css_class("error-label");
        return;
    }
    signal_start_custom_test.emit(requested_questions, "Пользовательский тест", selected_categories, -1);
}
void CustomTestScreen::on_category_toggled() {//Условие для создания
    int selected_count = 0;
    int max_questions = 0;
    for (const auto& cat : categories) {
        if (cat.check_button && cat.check_button->get_active()) {
            selected_count++;
            max_questions += std::min(cat.question_count, 10); 
        }
    }
    if (selected_count > 0) {
        questions_spin->set_sensitive(true);
        questions_spin->set_range(1, max_questions);
        questions_spin->set_value(std::min(10, max_questions));
        create_test_button->set_sensitive(true);
        string status_text = "Выбрано: " + to_string(selected_count) + "\nМаксимум: " + to_string(max_questions);
        status_label->set_text(status_text);
        status_label->remove_css_class("error-label");
    } else {
        questions_spin->set_sensitive(false);
        create_test_button->set_sensitive(false);
        status_label->set_text("Выберите категории слева");
        status_label->add_css_class("error-label");
    }
}
void CustomTestScreen::load_css() { //Стили
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .custom-test-title {
            font-family: Sans;
            font-size: 24px;
            font-weight: bold;
            color: #344955;
            padding: 10px;
            margin-bottom: 15px;
            text-align: center;
        }
        .categories-header {
            font-family: Sans;
            font-size: 16px;
            font-weight: bold;
            color: #344955;
            margin-bottom: 8px;
            text-align: left;
        }
        .settings-header {
            font-family: Sans;
            font-size: 16px;
            font-weight: bold;
            color: #344955;
            margin-bottom: 12px;
            text-align: left;
        }
        .categories-box {
            background-color: white;
            border: 2px solid #344955;
            border-radius: 8px;
            padding: 15px;
            min-width: 250px;
            max-height: 300px;
        }
        .category-checkbox {
            padding: 6px;
            margin: 3px 0;
            border-bottom: 1px solid #eee;
        }
        .category-checkbox:last-child {
            border-bottom: none;
        }
        .category-check {
            min-width: 18px;
            min-height: 18px;
        }
        .category-label {
            font-family: Sans;
            font-size: 14px;
            color: #000000 !important;
            margin-left: 8px;
        }
        .questions-row {
            margin-bottom: 15px;
        }
        .questions-label {
            font-family: Sans;
            font-size: 14px;
            color: #344955;
            text-align: right;
        }
        .questions-spin {
            font-family: Sans;
            font-size: 14px;
            padding: 5px;
            border: 1px solid #344955;
            border-radius: 4px;
            background-color: white;
            text-align: center;
        }
        .status-label {
            font-family: Sans;
            font-size: 13px;
            padding: 10px;
            margin: 10px 0;
            border-radius: 6px;
            text-align: left;
            min-height: 50px;
            background-color: #f0f0f0;
            border: 1px solid #ddd;
        }
        .error-label {
            background-color: #f8d7da !important;
            color: #721c24 !important;
            border-color: #f5c6cb !important;
        }
        .warning-label {
            background-color: #fff3cd !important;
            color: #856404 !important;
            border-color: #ffeaa7 !important;
        }
        .create-test-button {
            font-family: Sans;
            font-size: 14px;
            font-weight: bold;
            color: #000000 !important;
            background-color: white;
            border: 2px solid #28a745;
            border-radius: 6px;
            padding: 8px 20px;
            transition: all 0.2s ease-in-out;
        }
        .create-test-button:hover {
            background-color: #e9ecef;
            transform: translateY(-2px);
            box-shadow: 0 3px 6px rgba(40, 167, 69, 0.2);
        }
        .create-test-button:disabled {
            background-color: #cccccc;
            border-color: #999999;
            color: #666666 !important;
            transform: none;
            box-shadow: none;
        }
        .back-button {
            font-family: Sans;
            font-size: 14px;
            font-weight: bold;
            color: #000000 !important;
            background-color: white;
            border: 2px solid #344955;
            border-radius: 6px;
            padding: 8px 20px;
            transition: all 0.2s ease-in-out;
        }
        .back-button:hover {
            background-color: #e9ecef;
            transform: translateY(-2px);
            box-shadow: 0 3px 6px rgba(0,0,0,0.1);
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(),css_provider,GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
