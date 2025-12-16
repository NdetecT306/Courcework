#ifndef QUIZDATABASEMANAGER_H
#define QUIZDATABASEMANAGER_H
#include <string>
#include <mutex>
#include <pqxx/pqxx>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class QuizDatabaseManager {
private:
    pqxx::connection connection;
    mutex db_mutex;
    
public:
    QuizDatabaseManager();
    json getCategories();
    json getQuestionsByCategory(int category_id);
    json checkAnswers(const json& user_answers);
    json getTestStatistics(const string& user_login);
    bool saveTestResult(const string& user_login, int category_id, int score, int total_questions, int correct_answers);
};

#endif
