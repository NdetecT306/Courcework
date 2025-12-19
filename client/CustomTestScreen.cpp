#include "CustomTestScreen.h"

CustomTestScreen::CustomTestScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 10) { //Создание
    set_margin(20);
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    // Заголовок
    titleLabel = Gtk::make_managed<Gtk::Label>("Создание пользовательского теста");
    titleLabel->add_css_class("custom-test-title");
    titleLabel->set_halign(Gtk::Align::CENTER);
    append(*titleLabel);
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
    categoriesBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
    categoriesBox->add_css_class("categories-box");
    categories_container->append(*categoriesBox);
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
    questionsSpin = Gtk::make_managed<Gtk::SpinButton>();
    questionsSpin->set_range(1, 100);
    questionsSpin->set_value(10);
    questionsSpin->set_increments(1, 5);
    questionsSpin->add_css_class("questions-spin");
    questionsSpin->set_size_request(60, -1);
    questionsSpin->set_sensitive(false);
    questionsrow->append(*questionsSpin);
    // Статусное сообщение
    statusLabel = Gtk::make_managed<Gtk::Label>("Выберите категории слева");
    statusLabel->add_css_class("status-label");
    statusLabel->set_halign(Gtk::Align::START);
    settingscontainer->append(*statusLabel);
    // Кнопки
    auto buttonsbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    buttonsbox->set_halign(Gtk::Align::CENTER);
    buttonsbox->set_margin_top(20);
    settingscontainer->append(*buttonsbox);
    // Кнопка создания теста
    createTestButton = Gtk::make_managed<Gtk::Button>("Создать тест");
    createTestButton->add_css_class("create-test-button");
    createTestButton->set_size_request(120, 35);
    createTestButton->set_sensitive(false);
    createTestButton->signal_clicked().connect(sigc::mem_fun(*this, &CustomTestScreen::create_test));
    buttonsbox->append(*createTestButton);
    // Кнопка назад
    backButton = Gtk::make_managed<Gtk::Button>("Назад");
    backButton->add_css_class("back-button");
    backButton->set_size_request(120, 35);
    backButton->signal_clicked().connect([this]() {signal_back_to_main.emit();});
    buttonsbox->append(*backButton);
    load_css();
}
void CustomTestScreen::load_categories(const string& username) { //Загрузка нашего чуда
    currentUsername = username;
    // Очищаем предыдущие категории
    categories.clear();
    while (Gtk::Widget* child = categoriesBox->get_first_child()) {
        categoriesBox->remove(*child);
    }
    if (!g_network_client || !g_network_client->is_connected()) {
        statusLabel->set_text("Нет подключения к серверу");
        statusLabel->add_css_class("error-label");
        return;
    }
    json response = g_network_client->get_categories();
    if (response["status"] == "success" && response.contains("categories")) {
        for (const auto& cat_data : response["categories"]) {
            CategoryInfo cat;
            cat.id = cat_data["id"];
            cat.name = cat_data["name"];
            cat.questionCount = 0;
            json questions_response = g_network_client->get_questions(cat.id);
            if (questions_response["status"] == "success" && 
                questions_response.contains("questions")) {
                cat.questionCount = questions_response["questions"].size();
            }
            categories.push_back(cat);
        }
        for (auto& cat : categories) {
            auto checkbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);
            checkbox->set_halign(Gtk::Align::START);
            checkbox->add_css_class("category-checkbox");
            cat.checkButton = Gtk::make_managed<Gtk::CheckButton>();
            cat.checkButton->add_css_class("category-check");
            cat.checkButton->signal_toggled().connect(sigc::mem_fun(*this, &CustomTestScreen::on_category_toggled));
            checkbox->append(*cat.checkButton);
            // Название категории
            string labeltext = cat.name + " (" + to_string(cat.questionCount) + ")";
            auto label = Gtk::make_managed<Gtk::Label>(labeltext);
            label->add_css_class("category-label");
            label->set_halign(Gtk::Align::START);
            checkbox->append(*label);
            categoriesBox->append(*checkbox);
        }
        if (categories.empty()) {
            statusLabel->set_text("Нет доступных категорий");
            statusLabel->add_css_class("warning-label");
        } else {
            statusLabel->set_text("Выберите категории слева");
            statusLabel->remove_css_class("error-label");
            statusLabel->remove_css_class("warning-label");
        }
    } else {
        statusLabel->set_text("Не удалось загрузить категории");
        statusLabel->add_css_class("error-label");
    }
}
void CustomTestScreen::create_test() { //Создание своего теста
    set<int> selectedcategories;
    int maxquestions = 0;
    // Выбранные категории
    for (const auto& cat : categories) {
        if (cat.checkButton && cat.checkButton->get_active()) {
            selectedcategories.insert(cat.id);
            maxquestions += min(cat.questionCount, 10); 
        }
    }
    if (selectedcategories.empty()) {
        statusLabel->set_text("Выберите хотя бы одну категорию");
        statusLabel->add_css_class("error-label");
        return;
    }
    int requestedquestions = questionsSpin->get_value_as_int();
    if (requestedquestions > maxquestions) {
        string error_msg = "Максимум: " + to_string(maxquestions);
        statusLabel->set_text(error_msg);
        statusLabel->add_css_class("error-label");
        return;
    }
    if (requestedquestions < 1) {
        statusLabel->set_text("Минимум 1 вопрос");
        statusLabel->add_css_class("error-label");
        return;
    }
    signal_start_custom_test.emit(requestedquestions, "Пользовательский тест", selectedcategories, -1);
}
void CustomTestScreen::on_category_toggled() {//Условие для создания
    int selectedCount = 0;
    int maxQuestions = 0;
    for (const auto& cat : categories) {
        if (cat.checkButton && cat.checkButton->get_active()) {
            selectedCount++;
            maxQuestions += std::min(cat.questionCount, 10); 
        }
    }
    if (selectedCount > 0) {
        questionsSpin->set_sensitive(true);
        questionsSpin->set_range(1, maxQuestions);
        questionsSpin->set_value(std::min(10, maxQuestions));
        createTestButton->set_sensitive(true);
        string status_text = "Выбрано: " + to_string(selectedCount) + "\nМаксимум: " + to_string(maxQuestions);
        statusLabel->set_text(status_text);
        statusLabel->remove_css_class("error-label");
    } else {
        questionsSpin->set_sensitive(false);
        createTestButton->set_sensitive(false);
        statusLabel->set_text("Выберите категории слева");
        statusLabel->add_css_class("error-label");
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
