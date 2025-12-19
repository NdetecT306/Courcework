#include "MainScreen.h"
#include "NetworkClient.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <utility>
#include <map>
#include <filesystem>
using namespace std;

random_device rd_custom;
mt19937 g_custom(rd_custom());

MainScreen::MainScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 5) {
    set_margin(15);
    headerBox = Gtk::make_managed<Gtk::CenterBox>();
    headerBox->add_css_class("header-box");
    headerBox->set_margin_bottom(10);
    append(*headerBox);
    
    titleLabel = Gtk::make_managed<Gtk::Label>("Система тестирования знаний по информационной безопасности");
    titleLabel->add_css_class("title");
    titleLabel->set_halign(Gtk::Align::START);
    headerBox->set_start_widget(*titleLabel);
    
    usernameButton = Gtk::make_managed<Gtk::Button>();
    usernameButton->add_css_class("username-button");
    usernameButton->signal_clicked().connect(sigc::mem_fun(*this, &MainScreen::on_username_clicked));
    headerBox->set_end_widget(*usernameButton);
    
    // Центральный контент
    contentStack = Gtk::make_managed<Gtk::Stack>();
    contentStack->set_transition_type(Gtk::StackTransitionType::SLIDE_LEFT_RIGHT);
    contentStack->set_transition_duration(300);
    contentStack->set_vexpand(true);
    append(*contentStack);
    
    //1 кнопка
    resourcesScreen = Gtk::make_managed<ResourcesScreen>();
    resourcesScreen->signal_back_to_main.connect(sigc::mem_fun(*this, &MainScreen::on_back_from_resources));
    contentStack->add(*resourcesScreen, "resources", "Ресурсы");
    
    // 2 кнопка 
    testingScreen = Gtk::make_managed<TestingScreen>();
    testingScreen->signalStartTest.connect(sigc::mem_fun(*this, &MainScreen::on_start_test));
    contentStack->add(*testingScreen, "testing", "Тестирование");
    
    // 3 кнопка
    testScreen = Gtk::make_managed<TestScreen>();
    testScreen->signal_test_finished.connect(sigc::mem_fun(*this, &MainScreen::on_test_finished));
    testScreen->signal_back_to_categories.connect(sigc::mem_fun(*this, &MainScreen::on_back_to_categories));
    contentStack->add(*testScreen, "test", "Активный тест");
    
    // 4 кнопка
    customTestScreen = Gtk::make_managed<CustomTestScreen>();
    customTestScreen->signal_back_to_main.connect(sigc::mem_fun(*this, &MainScreen::on_back_from_custom_test));
    customTestScreen->signal_start_custom_test.connect(sigc::mem_fun(*this, &MainScreen::on_start_custom_test));
    contentStack->add(*customTestScreen, "custom_test", "Пользовательский тест");
    
    // 5 кнопка
    settingsWidget = Gtk::make_managed<ProfileSettingsWidget>();
    settingsWidget->signal_back_clicked.connect(sigc::mem_fun(*this, &MainScreen::on_back_from_settings));
    contentStack->add(*settingsWidget, "settings", "Настройки профиля");
    
    // Контейнер для нижних кнопок
    auto bottomcontainer = Gtk::make_managed<Gtk::CenterBox>();
    bottomcontainer->set_margin_top(10);
    bottomcontainer->set_margin_bottom(5);
    append(*bottomcontainer);
    
    // Контейнер для кнопок
    bottomButtonsBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);
    bottomButtonsBox->set_halign(Gtk::Align::CENTER);
    bottomcontainer->set_center_widget(*bottomButtonsBox);
    
    // Создаем 5 кнопок с изображениями
    vector<string> button_images = {
        "infresources.jpg",    // Первая кнопка - ресурсы
        "tests.jpg",           // Вторая кнопка - мои тесты
        "makeyourtest.jpg",    // Третья кнопка - создание пользовательского теста
        "statistics.jpg",      // Четвертая кнопка - статистика
        "settings.jpg"         // Пятая кнопка - настройки
    };
    
    vector<string> button_tooltips = {
        "Ресурсы",
        "Мои тесты",
        "Создать свой тест",
        "Статистика",
        "Настройки"
    };
    
    // Создаем 5 кнопок с изображениями
    for (size_t i = 0; i < button_images.size(); i++) {
        auto btn = Gtk::make_managed<Gtk::Button>();
        // Пытаемся загрузить изображение
        bool imageloaded = false;
        string imagepath = "/home/nastasyapingvi-306/kukur/client/" + button_images[i];
        // Проверяем существование файла
        if (filesystem::exists(imagepath)) {
            try {
                auto picture = Gtk::make_managed<Gtk::Picture>();
                auto file = Gio::File::create_for_path(imagepath);
                picture->set_file(file);
                picture->set_can_shrink(true);
                picture->set_content_fit(Gtk::ContentFit::SCALE_DOWN);
                picture->set_size_request(24, 24);
                picture->set_margin(5);
                btn->set_child(*picture);
                imageloaded = true;
            } catch (const std::exception& e) {
                cerr << "Ошибка загрузки изображения " << imagepath << ": " << e.what() << std::endl;
            }
        } else {
            cerr << "Файл изображения не найден: " << imagepath << std::endl;
        }
        
        // Если изображение не загрузилось, используем текст
        if (!imageloaded) {
            auto label = Gtk::make_managed<Gtk::Label>(button_tooltips[i]);
            label->set_margin(8);
            label->add_css_class("button-text");
            btn->set_child(*label);
        }
        
        btn->set_tooltip_text(button_tooltips[i]);
        btn->add_css_class("bottom-button");
        btn->set_size_request(120, 40); 
        
        // Подключаем обработчик клика
        if (i == 0) { 
            btn->signal_clicked().connect([this]() {
                on_resources_clicked();
            });
        } else if (i == 1) {
            btn->signal_clicked().connect([this]() {
                on_testing_clicked();
            });
        } else if (i == 2) {
            btn->signal_clicked().connect([this]() {
                on_custom_test_clicked();
            });
        } else if (i == 3) { 
            btn->signal_clicked().connect([this]() {
                on_statistics_clicked();
            });
        } else if (i == 4) { 
            btn->signal_clicked().connect([this]() {
                on_settings_clicked();
            });
        }
        
        bottomButtonsBox->append(*btn);
    }
    
    load_css();
}

void MainScreen::set_username(const string& username) { 
    currentUsername = username;
    usernameButton->set_label(username);
    usernameButton->set_tooltip_text("Нажмите для выхода");
    load_user_statistics();
    testingScreen->loadCategoriesFromServer();
}

void MainScreen::load_user_statistics() {
    if (!g_network_client || !g_network_client->is_connected()) {
        return;
    }
    
    json response = g_network_client->get_statistics(currentUsername);
    if (response["status"] == "success" && response.contains("statistics")) {
        std::map<int, int> test_stats;
        for (const auto& el : response["statistics"]) {
            int categoryId = el["category_id"];
            int score = el["score"];
            // Сохраняем лучший результат
            if (test_stats.find(categoryId) == test_stats.end() || 
                score > test_stats[categoryId]) {
                test_stats[categoryId] = score;
            }
        }
        testingScreen->setUserStats(test_stats);
    }
}

void MainScreen::on_start_test(int categoryId, const string& categoryName) {
    contentStack->set_visible_child(*testScreen);
    testScreen->startTest(categoryName, categoryId, currentUsername);
}

void MainScreen::on_start_custom_test(int totalQuestions, const string& testName, const set<int>& categoryIds, int dummy) {
    vector<Question> allquestions;
    if (!g_network_client || !g_network_client->is_connected()) {
        if (!g_network_client->connect()) {
            show_error_dialog("Нет подключения к серверу");
            return;
        }
    }
    
    bool hasquestions = false;
    for (int category_id : categoryIds) {
        json response = g_network_client->get_questions(category_id);
        if (response["status"] == "success" && response.contains("questions")) {
            hasquestions = true;
            for (const auto& question_data : response["questions"]) {
                Question question;
                question.id = question_data["id"];
                question.text = question_data["text"];
                for (const auto& answer_data : question_data["answers"]) {
                    int answer_id = answer_data["id"];
                    string answer_text = answer_data["text"];
                    bool is_correct = answer_data["is_correct"];
                    question.answers.push_back({answer_id, answer_text});
                    if (is_correct) {
                        question.correctAnswerId = answer_id;
                    }
                }
                allquestions.push_back(question);
            }
        } else {
            cout << "Не удалось загрузить вопросы для категории " << category_id << endl;
        }
    }
    
    if (!hasquestions || allquestions.empty()) {
        show_error_dialog("Не удалось загрузить вопросы из выбранных категорий");
        return;
    }
    
    shuffle(allquestions.begin(), allquestions.end(), g_custom);
    if (static_cast<int>(allquestions.size()) > totalQuestions) {
        allquestions.resize(totalQuestions);
    }
    
    for (auto& question : allquestions) {
        shuffle(question.answers.begin(), question.answers.end(), g_custom);
    }
    
    auto session = new TestSession();
    session->categoryId = -1;
    session->categoryName = testName;
    session->questions = allquestions;
    session->currentQuestion = 0;
    session->score = 0;
    session->userAnswers.resize(allquestions.size(), false);
    session->selectedAnswers.resize(allquestions.size(), -1);
    
    testScreen->startCustomTest(session, currentUsername);
    contentStack->set_visible_child(*testScreen);
}
void MainScreen::on_test_finished() {
    contentStack->set_visible_child(*testingScreen);
    load_user_statistics();
    queue_draw();
}
void MainScreen::on_back_to_categories() {
    contentStack->set_visible_child(*testingScreen);
    queue_draw();
}
void MainScreen::on_resources_clicked() {
    contentStack->set_visible_child(*resourcesScreen);
    resourcesScreen->clearResources();
    resourcesScreen->loadResources();
}
void MainScreen::on_testing_clicked() {
    contentStack->set_visible_child(*testingScreen);
}
void MainScreen::on_custom_test_clicked() { 
    contentStack->set_visible_child(*customTestScreen);
    customTestScreen->load_categories(currentUsername);
}
void MainScreen::on_statistics_clicked() {
    contentStack->set_visible_child(*resourcesScreen);
    resourcesScreen->clearResources();
    load_statistics_display();
}
void MainScreen::on_settings_clicked() {
    contentStack->set_visible_child(*settingsWidget);
    settingsWidget->load_profile_data(currentUsername);
}
void MainScreen::on_back_from_settings() {
    contentStack->set_visible_child(*testingScreen);
}
void MainScreen::on_back_from_custom_test() {
    contentStack->set_visible_child(*testingScreen);
}
void MainScreen::on_back_from_resources() {
    contentStack->set_visible_child(*testingScreen);
}
void MainScreen::load_statistics_display() {
    if (!g_network_client || !g_network_client->is_connected()) {
        auto error_label = Gtk::make_managed<Gtk::Label>("Нет подключения к серверу");
        error_label->add_css_class("error-label");
        resourcesScreen->append(*error_label);
        return;
    }
    json response = g_network_client->get_statistics(currentUsername);
    if (response["status"] == "success" && response.contains("statistics")) {
        json stats = response["statistics"];
        if (stats.empty()) {
            auto emptylabel = Gtk::make_managed<Gtk::Label>("Вы еще не проходили тесты");
            emptylabel->add_css_class("empty-label");
            resourcesScreen->append(*emptylabel);
            return;
        }
        // Подсчитываем общую статистику
        int total_tests = 0;
        int total_correct = 0;
        int total_questions = 0;
        std::map<int, pair<int, int>> category_stats;
        for (const auto& stat : stats) {
            total_tests++;
            total_correct += stat["correct_answers"].get<int>();
            total_questions += stat["total_questions"].get<int>();
            int category_id = stat["category_id"].get<int>();
            
            if (category_stats.find(category_id) == category_stats.end()) {
                category_stats[category_id] = std::make_pair(0, 0);
            }
            category_stats[category_id].first += stat["total_questions"].get<int>();
            category_stats[category_id].second += stat["correct_answers"].get<int>();
        }
        // Общая статистика
        auto summary_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
        summary_box->add_css_class("summary-box");
        summary_box->set_size_request(600, -1);
        auto total_label = Gtk::make_managed<Gtk::Label>(
            "Всего тестов: " + std::to_string(total_tests) + 
            "\nВсего вопросов: " + std::to_string(total_questions) +
            "\nПравильных ответов: " + std::to_string(total_correct) +
            "\nОбщий процент: " + std::to_string((total_questions > 0) ? (total_correct * 100 / total_questions) : 0) + "%"
        );
        total_label->add_css_class("total-stats");
        total_label->set_wrap(true);
        summary_box->append(*total_label);
        resourcesScreen->append(*summary_box);
        // Статистика по категориям
        if (!category_stats.empty()) {
            auto categories_title = Gtk::make_managed<Gtk::Label>("По категориям:");
            categories_title->add_css_class("categories-title");
            categories_title->set_size_request(600, -1);
            resourcesScreen->append(*categories_title);
            for (const auto& entry : category_stats) {
                int catId = entry.first;
                int cattotal = entry.second.first;
                int catcorrect = entry.second.second;
                int percentage = (cattotal > 0) ? (catcorrect * 100 / cattotal) : 0;
                string catname = "Категория " + to_string(catId);
                if (g_network_client->is_connected()) {
                    json cat_response = g_network_client->get_categories();
                    if (cat_response["status"] == "success" && cat_response.contains("categories")) {
                        for (const auto& cat : cat_response["categories"]) {
                            if (cat["id"].get<int>() == catId) {
                                catname = cat["name"].get<std::string>();
                                break;
                            }
                        }
                    }
                }
                auto catbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
                catbox->add_css_class("category-stats-box");
                catbox->set_size_request(600, -1);
                auto catlabel = Gtk::make_managed<Gtk::Label>(catname + ": ");
                catlabel->add_css_class("category-name");
                catbox->append(*catlabel);
                auto statslabel = Gtk::make_managed<Gtk::Label>(to_string(catcorrect) + "/" + to_string(cattotal) + " (" + to_string(percentage) + "%)");
                statslabel->add_css_class("category-stats");
                catbox->append(*statslabel);
                resourcesScreen->append(*catbox);
            }
        }
    } else {
        auto errorlabel = Gtk::make_managed<Gtk::Label>("Не удалось загрузить статистику");
        errorlabel->add_css_class("error-label");
        errorlabel->set_size_request(600, -1);
        resourcesScreen->append(*errorlabel);
    }
}
void MainScreen::on_username_clicked() {
    auto root = get_root();
    if (!root) {
        signal_logout.emit();
        return;
    }
    auto window = dynamic_cast<Gtk::Window*>(root);
    if (!window) {
        signal_logout.emit();
        return;
    }
    auto dialog = new Gtk::MessageDialog(*window,"Выход из системы", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO, true);
    dialog->set_modal(true);
    dialog->set_secondary_text("Вы уверены, что хотите выйти?");
    dialog->signal_response().connect([this, dialog](int response_id) {
        if (response_id == Gtk::ResponseType::YES) {
            signal_logout.emit();
        }
        delete dialog;
    });
    dialog->show();
}
void MainScreen::show_error_dialog(const string& message) {
    auto dialog = new Gtk::MessageDialog("Ошибка", false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK, true);
    auto root = get_root();
    if (root) {
        auto window = dynamic_cast<Gtk::Window*>(root);
        if (window) {
            dialog->set_transient_for(*window);
        }
    }
    dialog->set_modal(true);
    dialog->set_secondary_text(message);
    dialog->signal_response().connect([dialog](int response_id) {
        dialog->close();
        delete dialog;
    });
    dialog->show();
}
void MainScreen::load_css() {
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .title {
            font-family: Sans;
            font-size: 24px;
            font-weight: bold;
            color: #c78d36;
            background-color: #344955;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            min-width: 400px;
        }
        .username-button {
            background-color: white;
            color: #000000;
            border-radius: 10px;
            min-width: 100px;
            min-height: 25px;
            border: 1px solid #c78d36;
            font-size: 14px;
            font-weight: bold;
            padding: 5px 10px;
            margin: 0;
            transition: all 0.2s ease-in-out;
        }
        .username-button:hover {
            background-color: #e9ecef;
            transform: translateY(-1px);
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }
        .dialog-label {
            font-family: Sans;
            font-size: 15px;
            margin: 5px;
        }
        .bottom-button {
            border-radius: 8px;
            background-color: white;
            color: #000000;
            border: 2px solid #c78d36;
            transition: all 0.2s ease-in-out;
            box-shadow: 0 1px 3px rgba(0,0,0,0.2);
            min-width: 120px;
            min-height: 40px;
            padding: 5px;
            margin: 0 5px;
        }
        .bottom-button:hover {
            transform: translateY(-3px);
            background-color: #f8f9fa;
            box-shadow: 0 3px 8px rgba(199, 141, 54, 0.4);
            border-color: #a67c2b;
        }
        .bottom-button:active {
            transform: translateY(-1px);
            box-shadow: 0 1px 4px rgba(199, 141, 54, 0.4);
        }
        .bottom-button:disabled {
            opacity: 0.7;
            transform: none;
            box-shadow: none;
        }
        .bottom-button picture {
            margin: 0;
            padding: 0;
        }
        .bottom-button label {
            font-family: Sans;
            font-size: 12px;
            font-weight: bold;
            color: #344955;
        }
        .button-text {
            font-family: Sans;
            font-size: 12px;
            font-weight: bold;
            color: #344955;
        }
        .header-box {
            margin: 10px;
            padding: 5px;
        }
        .summary-box {
            background-color: #f8f9fa;
            border: 2px solid #344955;
            border-radius: 10px;
            padding: 20px;
            margin: 10px;
        }
        .total-stats {
            font-family: Sans;
            font-size: 18px;
            font-weight: bold;
            color: #212529;
            line-height: 1.5;
        }
        .categories-title {
            font-family: Sans;
            font-size: 18px;
            font-weight: bold;
            color: #344955;
            margin-top: 20px;
            margin-bottom: 10px;
        }
        .category-stats-box {
            background-color: #e9ecef;
            border: 1px solid #dee2e6;
            border-radius: 8px;
            padding: 10px;
            margin: 5px 0;
        }
        .category-name {
            font-family: Sans;
            font-size: 16px;
            font-weight: bold;
            color: #495057;
            min-width: 150px;
        }
        .category-stats {
            font-family: Sans;
            font-size: 16px;
            color: #28a745;
            font-weight: bold;
        }
        .empty-label {
            font-family: Sans;
            font-size: 16px;
            color: #6c757d;
            padding: 20px;
        }
        .error-label {
            font-family: Sans;
            font-size: 14px;
            color: #721c24;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 10px;
            border-radius: 5px;
            margin: 10px;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
