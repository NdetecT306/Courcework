#ifndef TESTSCREEN_H
#define TESTSCREEN_H
#include <gtkmm.h>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;
using json = nlohmann::json;

struct Question {
    int id;
    string text;
    vector<pair<int, string>> answers;
    int correctAnswerId;
};
struct TestSession {
    int categoryId;
    string categoryName;
    vector<Question> questions;
    size_t currentQuestion;
    int score;
    vector<bool> userAnswers;
    vector<int> selectedAnswers;
};
class TestScreen : public Gtk::Box {
protected:
    void showQuestion(size_t questionIndex);
    void onAnswerSelected(int answerIndex);
    void onNextQuestion();
    void onFinishTest();
    void resetAnswerStyles();
private:
    Gtk::Label* questionHeader = nullptr;
    Gtk::Label* questionLabel = nullptr;
    Gtk::Grid* answersGrid = nullptr;
    Gtk::Button* answerButtons[4] = {nullptr, nullptr, nullptr, nullptr};
    Gtk::Label* answerLabels[4] = {nullptr, nullptr, nullptr, nullptr};
    Gtk::Label* iconLabels[4] = {nullptr, nullptr, nullptr, nullptr};
    Gtk::Button* nextButton = nullptr;
    unique_ptr<TestSession> currentSession;
    string currentUsername;
    int selectedAnswerIndex = -1;
    bool answerSubmitted = false;
    bool loadQuestionsFromServer(int category_id);
    void saveTestResult();
    void showErrorMessage(const std::string& message);
    void loadCss();
public:
    TestScreen();
    void startTest(const string& category_name, int category_id, const string& username);
    void startCustomTest(TestSession* session, const string& username);
    // Сигналы
    sigc::signal<void()> signal_test_finished;
    sigc::signal<void()> signal_back_to_categories;
};
#endif 
