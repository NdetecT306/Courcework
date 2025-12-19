#include "quizDatabase.h"
#include <iostream>

QuizDatabaseManager::QuizDatabaseManager() : connection(
    "host=172.17.0.1 "
    "port=5432 "
    "dbname=quiz "
    "user=admin306 "
    "password=ILoveMyCreation"
) {}
json QuizDatabaseManager::getCategories() {
    lock_guard<std::mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec("SELECT id, name FROM categories ORDER BY id");
        txn.commit();
        json categories = json::array();
        for (const auto& row : result) {
            json category;
            category["id"] = row["id"].as<int>();
            category["name"] = row["name"].as<string>();
            categories.push_back(category);
        }
        return categories;
    } catch (const exception& e) {
        cerr << "Ошибка при получении категорий: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
json QuizDatabaseManager::getQuestionsByCategory(int categoryid) {
    lock_guard<mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec_params(
            "SELECT id, question_text FROM questions WHERE category_id = $1 ORDER BY id",
            categoryid
        );
        txn.commit();
        json questions = json::array();
        for (const auto& row : result) {
            json question;
            question["id"] = row["id"].as<int>();
            question["text"] = row["question_text"].as<string>();
            auto answersresult = txn.exec_params(
                "SELECT id, answer_text, is_correct FROM answers WHERE question_id = $1",
                question["id"].get<int>()
            );
            json answers = json::array();
            for (const auto& answerrow : answersresult) {
                json answer;
                answer["id"] = answerrow["id"].as<int>();
                answer["text"] = answerrow["answer_text"].as<string>();
                answer["is_correct"] = answerrow["is_correct"].as<bool>();
                answers.push_back(answer);
            }
            question["answers"] = answers;
            questions.push_back(question);
        }
        return questions;
    } catch (const exception& e) {
        cerr << "Ошибка при получении вопросов: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
json QuizDatabaseManager::checkAnswers(const json& useranswers) {
    lock_guard<mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        int totalquestions = 0;
        int correctanswers = 0;
        json results = json::array();
        for (const auto& answer : useranswers) {
            int questionid = answer["question_id"];
            int answerid = answer["answer_id"];
            auto result = txn.exec_params(
                "SELECT is_correct FROM answers WHERE id = $1 AND question_id = $2",
                answerid, questionid
            );
            if (!result.empty()) {
                bool iscorrect = result[0]["is_correct"].as<bool>();
                totalquestions++;
                json questionresult;
                questionresult["question_id"] = questionid;
                questionresult["answer_id"] = answerid;
                questionresult["is_correct"] = iscorrect;
                if (iscorrect) {
                    correctanswers++;
                }
                auto correctresult = txn.exec_params(
                    "SELECT id, answer_text FROM answers WHERE question_id = $1 AND is_correct = true",
                    questionid
                );
                if (!correctresult.empty()) {
                    questionresult["correct_answer_id"] = correctresult[0]["id"].as<int>();
                    questionresult["correct_answer_text"] = correctresult[0]["answer_text"].as<std::string>();
                }
                results.push_back(questionresult);
            }
        }
        txn.commit();
        return {
            {"total_questions", totalquestions},
            {"correct_answers", correctanswers},
            {"score", (totalquestions > 0) ? (correctanswers * 100 / totalquestions) : 0},
            {"results", results}
        };
    } catch (const exception& e) {
        cerr << "Ошибка проверки ответов: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
json QuizDatabaseManager::getTestStatistics(const string& userlogin) {
    lock_guard<mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        txn.exec(
            "CREATE TABLE IF NOT EXISTS test_results ("
            "id SERIAL PRIMARY KEY,"
            "user_login VARCHAR(50) NOT NULL,"
            "category_id INTEGER NOT NULL,"
            "score INTEGER NOT NULL,"
            "total_questions INTEGER NOT NULL,"
            "correct_answers INTEGER NOT NULL,"
            "test_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ")"
        );
        txn.commit();
        pqxx::work txn2(connection);
        auto result = txn2.exec_params(
            "SELECT category_id, score, total_questions, correct_answers, test_date "
            "FROM test_results WHERE user_login = $1 ORDER BY test_date DESC",
            userlogin
        );
        txn2.commit();
        json statistics = json::array();
        for (const auto& row : result) {
            json stat;
            stat["category_id"] = row["category_id"].as<int>();
            stat["score"] = row["score"].as<int>();
            stat["total_questions"] = row["total_questions"].as<int>();
            stat["correct_answers"] = row["correct_answers"].as<int>();
            stat["test_date"] = row["test_date"].as<string>();
            statistics.push_back(stat);
        }
        return statistics;
    } catch (const exception& e) {
        cerr << "Ошибка в получении статистики: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
bool QuizDatabaseManager::saveTestResult(const string& userlogin, int categoryid, int score, int totalquestions, int correctanswers) {
    lock_guard<mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        txn.exec_params(
            "INSERT INTO test_results (user_login, category_id, score, total_questions, correct_answers) "
            "VALUES ($1, $2, $3, $4, $5)",
            userlogin, categoryid, score, totalquestions, correctanswers
        );
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Проблема с сохранением теста: " << e.what() << endl;
        return false;
    }
}
