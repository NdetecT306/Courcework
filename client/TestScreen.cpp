#include "TestScreen.h"
#include "NetworkClient.h"
// Генератор случайных чисел для перемешивания вопросов
random_device rd;
mt19937 g(rd());
TestScreen::TestScreen() : Gtk::Box(Gtk::Orientation::VERTICAL, 10) { //Создание
    set_margin(20);
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    // Заголовок с номером вопроса
    questionHeader = Gtk::make_managed<Gtk::Label>("Вопрос 1 из 10");
    questionHeader->add_css_class("test-header");
    questionHeader->set_halign(Gtk::Align::CENTER);
    append(*questionHeader);
    // Вопрос
    questionLabel = Gtk::make_managed<Gtk::Label>();
    questionLabel->add_css_class("question-text");
    questionLabel->set_wrap(true);
    questionLabel->set_max_width_chars(80);
    questionLabel->set_width_chars(60);
    questionLabel->set_halign(Gtk::Align::CENTER);
    questionLabel->set_valign(Gtk::Align::START);
    append(*questionLabel);
    // Контейнер для ответов 
    answersGrid = Gtk::make_managed<Gtk::Grid>();
    answersGrid->set_row_spacing(15);
    answersGrid->set_column_spacing(15);
    answersGrid->set_halign(Gtk::Align::CENTER);
    answersGrid->set_valign(Gtk::Align::CENTER);
    answersGrid->set_margin_top(30);
    answersGrid->set_margin_bottom(30);
    append(*answersGrid);
    // 4 кнопки-ответа
    for (int i = 0; i < 4; i++) {
        answerButtons[i] = Gtk::make_managed<Gtk::Button>();
        answerButtons[i]->add_css_class("answer-button");
        answerButtons[i]->set_size_request(300, 100);
        answerButtons[i]->set_hexpand(true);
        answerButtons[i]->set_vexpand(true);
        int row = i / 2;
        int col = i % 2;
        answersGrid->attach(*answerButtons[i], col, row);
        auto text_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
        text_container->set_halign(Gtk::Align::CENTER);
        text_container->set_valign(Gtk::Align::CENTER);
        iconLabels[i] = Gtk::make_managed<Gtk::Label>("");
        iconLabels[i]->add_css_class("answer-icon");
        text_container->append(*iconLabels[i]);
        answerLabels[i] = Gtk::make_managed<Gtk::Label>();
        answerLabels[i]->add_css_class("answer-text");
        answerLabels[i]->set_wrap(true);
        answerLabels[i]->set_max_width_chars(35);
        answerLabels[i]->set_justify(Gtk::Justification::CENTER);
        text_container->append(*answerLabels[i]);
        answerButtons[i]->set_child(*text_container);
        answerButtons[i]->signal_clicked().connect([this, i]() { onAnswerSelected(i); });
    }
    // "Следующий вопрос"
    nextButton = Gtk::make_managed<Gtk::Button>("Следующий вопрос");
    nextButton->add_css_class("next-button");
    nextButton->set_size_request(200, 40);
    nextButton->set_sensitive(false);
    nextButton->set_halign(Gtk::Align::CENTER);
    nextButton->signal_clicked().connect(sigc::mem_fun(*this, &TestScreen::onNextQuestion));
    append(*nextButton);
    loadCss();
}
void TestScreen::startTest(const string& category_name, int category_id, const string& username) {//Подрубаемся к серверу
    currentUsername = username;
    currentSession = make_unique<TestSession>();
    currentSession->category_id = category_id;
    currentSession->category_name = category_name;
    if (!loadQuestionsFromServer(category_id)) {
        showErrorMessage("Не удалось загрузить вопросы. Проверьте подключение к серверу.");
        return;
    }
    if (currentSession->questions.empty()) {
        showErrorMessage("Нет вопросов в этой категории");
        return;
    }
    currentSession->current_question = 0;
    currentSession->score = 0;
    currentSession->user_answers.clear();
    currentSession->selected_answers.clear();
    showQuestion(0);
    resetAnswerStyles();
}
bool TestScreen::loadQuestionsFromServer(int category_id) { //Вопросы из категорий
    if (!g_network_client || !g_network_client->is_connected()) {
        if (!g_network_client->connect()) {
            return false;
        }
    }
    json response = g_network_client->get_questions(category_id);
    if (response["status"] == "success" && response.contains("questions")) {
        currentSession->questions.clear();
        for (const auto& question_data : response["questions"]) {
            Question question;
            question.id = question_data["id"];
            question.text = question_data["text"];
            // Загружаем ответы
            for (const auto& answer_data : question_data["answers"]) {
                int answer_id = answer_data["id"];
                string answer_text = answer_data["text"];
                bool is_correct = answer_data["is_correct"];
                question.answers.push_back({answer_id, answer_text});
                if (is_correct) {
                    question.correct_answer_id = answer_id;
                }
            }
            // Перемешиваем ответы
            shuffle(question.answers.begin(), question.answers.end(), g);
            currentSession->questions.push_back(question);
        }
        // Перемешиваем вопросы
        shuffle(currentSession->questions.begin(), currentSession->questions.end(), g);
        // Берем только 10 вопросов
        if (currentSession->questions.size() > 10) {
            currentSession->questions.resize(10);
        }
        currentSession->user_answers.resize(currentSession->questions.size(), false);
        currentSession->selected_answers.resize(currentSession->questions.size(), -1);
        return true;
    }
    return false;
}
void TestScreen::showQuestion(size_t question_index) { //Показать вопрос
    if (!currentSession || question_index >= currentSession->questions.size()) {
        return;
    }
    const Question& question = currentSession->questions[question_index];
    // Обновляем заголовок
    questionHeader->set_text("Вопрос " + to_string(question_index + 1) + " из " + to_string(currentSession->questions.size()));
    // Обновляем текст вопроса
    questionLabel->set_text(question.text);
    // Сбрасываем все стили и иконки
    for (int i = 0; i < 4; i++) {
        if (answerButtons[i]) {
            answerButtons[i]->remove_css_class("answer-correct");
            answerButtons[i]->remove_css_class("answer-wrong");
            answerButtons[i]->add_css_class("answer-button");
            answerButtons[i]->set_sensitive(true);
            iconLabels[i]->set_text(""); 
        }
    }
    // Обновляем ответы
    for (int i = 0; i < 4 && i < question.answers.size(); i++) {
        answerLabels[i]->set_text(question.answers[i].second);
        answerButtons[i]->set_visible(true);
        iconLabels[i]->set_text("");
    }
    // Если пользователь уже отвечал на этот вопрос
    if (currentSession->selected_answers[question_index] != -1) {
        int prev_selected_id = currentSession->selected_answers[question_index];
        bool prev_correct = currentSession->user_answers[question_index];
        int prev_index = -1;
        for (int i = 0; i < question.answers.size(); i++) {
            if (question.answers[i].first == prev_selected_id) {
                prev_index = i;
                break;
            }
        }
        if (prev_index != -1) {
            answerButtons[prev_index]->remove_css_class("answer-button");
            if (prev_correct) {
                answerButtons[prev_index]->add_css_class("answer-correct");
            } else {
                answerButtons[prev_index]->add_css_class("answer-wrong");
                for (int i = 0; i < question.answers.size(); i++) {
                    if (question.answers[i].first == question.correct_answer_id) {
                        answerButtons[i]->remove_css_class("answer-button");
                        answerButtons[i]->add_css_class("answer-correct");
                        break;
                    }
                }
            }
        }
        answerSubmitted = true;
        nextButton->set_sensitive(true);
        // Делаем все кнопки неактивными
        for (int i = 0; i < question.answers.size(); i++) {
            answerButtons[i]->set_sensitive(false);
        }
    } else {
        selectedAnswerIndex = -1;
        answerSubmitted = false;
        nextButton->set_sensitive(false);
    }
    if (question_index == currentSession->questions.size() - 1) {
        nextButton->set_label("Завершить тест");
    } else {
        nextButton->set_label("Следующий вопрос");
    }
}
void TestScreen::onAnswerSelected(int answer_index) { //Выбранный ответ
    if (answerSubmitted || !currentSession || 
        currentSession->current_question >= currentSession->questions.size()) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        if (answerButtons[i]) {
            answerButtons[i]->remove_css_class("answer-correct");
            answerButtons[i]->remove_css_class("answer-wrong");
            answerButtons[i]->add_css_class("answer-button");
            iconLabels[i]->set_text("");
        }
    }
    selectedAnswerIndex = answer_index;
    answerSubmitted = true;
    const Question& question = currentSession->questions[currentSession->current_question];
    int selected_answer_id = question.answers[answer_index].first;
    currentSession->selected_answers[currentSession->current_question] = selected_answer_id;
    bool is_correct = (selected_answer_id == question.correct_answer_id);
    currentSession->user_answers[currentSession->current_question] = is_correct;
    answerButtons[answer_index]->remove_css_class("answer-button");
    if (is_correct) {
        // Правильный ответ
        answerButtons[answer_index]->add_css_class("answer-correct");
        currentSession->score++;
    } else {
        // Неправильный ответ
        answerButtons[answer_index]->add_css_class("answer-wrong");
        for (int i = 0; i < question.answers.size(); i++) {
            if (question.answers[i].first == question.correct_answer_id) {
                answerButtons[i]->remove_css_class("answer-button");
                answerButtons[i]->add_css_class("answer-correct");
                break;
            }
        }
    }
    // Делаем все кнопки неактивными
    for (int i = 0; i < question.answers.size(); i++) {
        answerButtons[i]->set_sensitive(false);
    }
    nextButton->set_sensitive(true);
}
void TestScreen::onNextQuestion() { //Следующий вопрос
    if (!currentSession) return; 
    currentSession->current_question++;
    if (currentSession->current_question < currentSession->questions.size()) {
        showQuestion(currentSession->current_question);
    } else {
        onFinishTest();
    }
}
void TestScreen::onFinishTest() { //Закончить тест
    if (!currentSession) return;
    // Сохраняем результат на сервере
    saveTestResult();
    // Показываем результаты
    int total_questions = currentSession->questions.size();
    int correct_answers = currentSession->score;
    int percentage = (total_questions > 0) ? (correct_answers * 100) / total_questions : 0;
    auto dialog = new Gtk::MessageDialog("Результаты теста", false, Gtk::MessageType::INFO, Gtk::ButtonsType::OK, true);
    auto root = get_root();
    if (root) {
        auto window = dynamic_cast<Gtk::Window*>(root);
        if (window) {
            dialog->set_transient_for(*window);
        }
    }
    dialog->set_modal(true);
    dialog->set_secondary_text(
        "Категория: " + currentSession->category_name + "\n\n" +
        "Правильных ответов: " + to_string(correct_answers) + " из " + 
        to_string(total_questions) + "\n" +
        "Результат: " + to_string(percentage) + "%");
    dialog->signal_response().connect([this, dialog](int response_id) {dialog->close();delete dialog;
        Glib::signal_timeout().connect_once([this]() {signal_test_finished.emit();}, 100);
    });
    dialog->show();
}
void TestScreen::saveTestResult() { //Сохранить результат теста
    if (!currentSession || !g_network_client) return;
    if (!g_network_client->is_connected()) {
        if (!g_network_client->connect()) {
            return;
        }
    }
    if (currentSession->category_id == -1) {
        return; // Пропускаем сохранение для пользовательских тестов
    }
    // Подготавливаем ответы для отправки
    json user_answers = json::array();
    for (size_t i = 0; i < currentSession->questions.size(); i++) {
        if (currentSession->selected_answers[i] != -1) {
            json answer = {
                {"question_id", currentSession->questions[i].id},
                {"answer_id", currentSession->selected_answers[i]}
            };
            user_answers.push_back(answer);
        }
    }
    g_network_client->check_answers(
        currentUsername,
        currentSession->category_id,
        user_answers
    );
}
void TestScreen::resetAnswerStyles() { //Обновить экран
    for (int i = 0; i < 4; i++) {
        if (answerButtons[i]) {
            answerButtons[i]->remove_css_class("answer-correct");
            answerButtons[i]->remove_css_class("answer-wrong");
            answerButtons[i]->add_css_class("answer-button");
            answerButtons[i]->set_sensitive(true);
            auto context = answerButtons[i]->get_style_context();
            context->remove_class("answer-correct");
            context->remove_class("answer-wrong");
        }
    }
}
void TestScreen::showErrorMessage(const string& message) { //Сообщение об ошибке
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
    dialog->signal_response().connect([this, dialog](int response_id) {dialog->close();delete dialog;signal_back_to_categories.emit();});
    dialog->show();
}
void TestScreen::loadCss() { //Стиль
    auto css_provider = Gtk::CssProvider::create();
    const char* css_style = R"(
        .test-header {
            font-family: Sans;
            font-size: 24px;
            font-weight: bold;
            color: #344955;
            padding: 10px;
            margin-bottom: 10px;
        }
        .question-text {
            font-family: Sans;
            font-size: 18px;
            font-weight: bold;
            color: #2c3e50;
            padding: 20px;
            background-color: #f8f9fa;
            border-radius: 10px;
            border: 2px solid #dee2e6;
            min-width: 600px;
            max-width: 800px;
        }
        .answer-button {
            font-family: Sans;
            font-size: 16px;
            color: #000000;
            background-color: white;
            border: 2px solid #344955;
            border-radius: 8px;
            padding: 15px;
            transition: all 0.2s ease-in-out;
            min-width: 300px;
            min-height: 100px;
        }
        .answer-button:hover {
            background-color: #e9ecef;
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        .answer-correct {
            background-color: #d4edda;
            border: 3px solid #28a745;
        }
        .answer-correct label {
            color: #155724;
            font-weight: bold;
        }
        .answer-icon {
            font-size: 18px;
            font-weight: bold;
            margin-right: 5px;
        }
        .answer-wrong {
            background-color: #f8d7da;
            border: 3px solid #dc3545;
        }
        .answer-wrong label {
            color: #721c24;
            font-weight: bold;
        }
        .answer-text {
            font-size: 14px;
            font-weight: normal;
            color: #000000;
        }
        .next-button {
            font-family: Sans;
            font-size: 18px;
            font-weight: bold;
            color: #000000;
            background-color: #ffffff;
            border: 2px solid #28a745;
            border-radius: 8px;
            padding: 10px 30px;
            margin-top: 20px;
            transition: all 0.2s ease-in-out;
        }
        .next-button:hover {
            background-color: #e9ecef;
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(40, 167, 69, 0.3);
        }
        .next-button:disabled {
            background-color: #cccccc;
            border-color: #999999;
            color: #666666;
            transform: none;
            box-shadow: none;
        }
    )";
    css_provider->load_from_data(css_style);
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}
void TestScreen::startCustomTest(TestSession* session, const string& username) { //Пользовательский тест
    currentUsername = username;
    currentSession.reset(session); // Принимаем готовую сессию
    if (!currentSession || currentSession->questions.empty()) {
        showErrorMessage("Нет вопросов в тесте");
        return;
    }
    currentSession->current_question = 0;
    currentSession->score = 0;
    if (currentSession->user_answers.size() != currentSession->questions.size()) {
        currentSession->user_answers.resize(currentSession->questions.size(), false);
    }
    if (currentSession->selected_answers.size() != currentSession->questions.size()) {
        currentSession->selected_answers.resize(currentSession->questions.size(), -1);
    }
    showQuestion(0);
    resetAnswerStyles();
}
