#include "TestingScreen.h"
#include <iostream>
TestingScreen::TestingScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 10) { //Создание
    set_margin(15);
    titleLabel = Gtk::make_managed<Gtk::Label>("Тесты по категориям");
    titleLabel->set_halign(Gtk::Align::CENTER);
    titleLabel->add_css_class("categories-title");
    append(*titleLabel);
    categoriesGrid = Gtk::make_managed<Gtk::Grid>();
    categoriesGrid->set_row_spacing(15);
    categoriesGrid->set_column_spacing(15);
    categoriesGrid->set_halign(Gtk::Align::CENTER);
    categoriesGrid->set_valign(Gtk::Align::CENTER);
    categoriesGrid->set_vexpand(true);
    append(*categoriesGrid);
    createCategories();
    loadCss();
}
void TestingScreen::setUserStats(const std::map<int, int>& stats) {
    userStats = stats;
    updateCategoryScores();
}
void TestingScreen::loadCategoriesFromServer() { //Загрузка
    if (!g_network_client) return;
    if (!g_network_client->is_connected()) {
        if (!g_network_client->connect()) {
            return;
        }
    }
    json response = g_network_client->get_categories();
    if (response["status"] == "success" && response.contains("categories")) {
        categories.clear();
        for (const auto& el : response["categories"]) {
            Category cat;
            cat.id = el["id"];
            cat.name = el["name"];
            cat.button = nullptr;
            cat.scoreLabel = nullptr;
            categories.push_back(cat);
        }
        updateCategoriesDisplay();
    }
}
void TestingScreen::createCategories() {
    categories = {
        {1, "Термины", "Основные понятия информационной безопасности", nullptr, nullptr},
        {2, "Закон", "Законодательство в области ИБ", nullptr, nullptr},
        {3, "Криптография", "Шифрование и защита данных", nullptr, nullptr},
        {4, "Угрозы", "Виды угроз и атак", nullptr, nullptr}
    };
    updateCategoriesDisplay();
}
void TestingScreen::updateCategoriesDisplay() { //Обновить
    while (Gtk::Widget* child = categoriesGrid->get_first_child()) {
        categoriesGrid->remove(*child);
    }
    for (size_t i = 0; i < categories.size(); i++) {
        int row = i / 2;
        int col = i % 2;
        auto card = createCategoryCard(categories[i]);
        categories[i].button = card;
        categoriesGrid->attach(*card, col, row);
    }
}
Gtk::Button* TestingScreen::createCategoryCard(const Category& category) { //Создать карточку
    auto card = Gtk::make_managed<Gtk::Button>();
    card->set_size_request(250, 150);
    card->add_css_class("category-card");
    auto cardBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
    cardBox->set_margin(10);
    card->set_child(*cardBox);
    // Название категории
    auto nameLabel = Gtk::make_managed<Gtk::Label>(category.name);
    nameLabel->set_halign(Gtk::Align::START);
    nameLabel->set_valign(Gtk::Align::START);
    nameLabel->add_css_class("category-name");
    cardBox->append(*nameLabel);
    auto descLabel = Gtk::make_managed<Gtk::Label>(category.description);
    descLabel->set_halign(Gtk::Align::START);
    descLabel->set_valign(Gtk::Align::START);
    descLabel->add_css_class("category-desc");
    cardBox->append(*descLabel);
    // Контейнер для счета
    auto scoreContainer = Gtk::make_managed<Gtk::CenterBox>();
    scoreContainer->set_hexpand(true);
    scoreContainer->set_vexpand(true);
    cardBox->append(*scoreContainer);
    // Счет (в правом нижнем углу карты)
    auto scoreLabel = Gtk::make_managed<Gtk::Label>("0/10");
    scoreLabel->set_halign(Gtk::Align::END);
    scoreLabel->set_valign(Gtk::Align::END);
    scoreLabel->add_css_class("category-score");
    scoreContainer->set_end_widget(*scoreLabel);
    // Обновляем счет если есть статистика
    if (userStats.find(category.id) != userStats.end()) {
        int bestScore = userStats[category.id];
        scoreLabel->set_text(to_string(bestScore) + "/10");
    }
    size_t catInd = category.id - 1; 
    if (catInd < categories.size()) {
        categories[catInd].scoreLabel = scoreLabel;
    }
    card->signal_clicked().connect([this, category]() {onCategoryClicked(category.id, category.name);});
    return card;
}
void TestingScreen::updateCategoryScores() { //Обновить счет
    for (auto& el : categories) {
        if (el.scoreLabel && userStats.find(el.id) != userStats.end()) {
            int bestScore = userStats[el.id];
            el.scoreLabel->set_text(to_string(bestScore) + "/10");
        }
    }
}
void TestingScreen::onCategoryClicked(int categoryId, const string& categoryName) {
    signalStartTest.emit(categoryId, categoryName);
}
void TestingScreen::loadCss() {
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .categories-title {
            font-family: Sans;
            font-size: 24px;
            font-weight: bold;
            color: #344955;
            padding: 10px;
            margin-bottom: 20px;
            text-align: center;
        }
        .category-card {
            background-color: white;
            color: #000000 !important;
            border: 2px solid #344955;
            border-radius: 10px;
            padding: 15px;
            transition: all 0.3s ease-in-out;
        }
        .category-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 20px rgba(0,0,0,0.1);
            background-color: #f8f9fa;
            border-color: #c78d36;
        }
        .category-name {
            font-family: Sans;
            font-size: 20px;
            font-weight: bold;
            color: #000000 !important;
            margin-bottom: 5px;
        }
        .category-desc {
            font-family: Sans;
            font-size: 14px;
            color: #000000 !important;
            margin-top: 5px;
        }
        .category-score {
            font-family: Sans;
            font-size: 18px;
            font-weight: bold;
            color: #000000 !important;
            background-color: #d4edda;
            padding: 5px 10px;
            border-radius: 10px;
            border: 2px solid #c3e6cb;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}
