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
    mutex dbmutex;
    
public:
    QuizDatabaseManager();
    json getCategories();
    json getQuestionsByCategory(int categoryid);
    json checkAnswers(const json& useranswers);
    json getTestStatistics(const string& userlogin);
    bool saveTestResult(const string& userlogin, int categoryid, int score, int totalquestions, int correctanswers);
};

#endif
