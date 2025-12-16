#include "quizDatabase.h"
#include <iostream>

QuizDatabaseManager::QuizDatabaseManager() : connection(
    "host=localhost "
    "port=5432 "
    "dbname=quiz "
    "user=admin306 "
    "password=ILoveMyCreation"
) {}
json QuizDatabaseManager::getCategories() {
    lock_guard<std::mutex> lock(db_mutex);
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
json QuizDatabaseManager::getQuestionsByCategory(int category_id) {
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec_params(
            "SELECT id, question_text FROM questions WHERE category_id = $1 ORDER BY id",
            category_id
        );
        txn.commit();
        json questions = json::array();
        for (const auto& row : result) {
            json question;
            question["id"] = row["id"].as<int>();
            question["text"] = row["question_text"].as<string>();
            auto answers_result = txn.exec_params(
                "SELECT id, answer_text, is_correct FROM answers WHERE question_id = $1",
                question["id"].get<int>()
            );
            json answers = json::array();
            for (const auto& answer_row : answers_result) {
                json answer;
                answer["id"] = answer_row["id"].as<int>();
                answer["text"] = answer_row["answer_text"].as<string>();
                answer["is_correct"] = answer_row["is_correct"].as<bool>();
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
json QuizDatabaseManager::checkAnswers(const json& user_answers) {
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        int total_questions = 0;
        int correct_answers = 0;
        json results = json::array();
        for (const auto& answer : user_answers) {
            int question_id = answer["question_id"];
            int answer_id = answer["answer_id"];
            auto result = txn.exec_params(
                "SELECT is_correct FROM answers WHERE id = $1 AND question_id = $2",
                answer_id, question_id
            );
            if (!result.empty()) {
                bool is_correct = result[0]["is_correct"].as<bool>();
                total_questions++;
                json question_result;
                question_result["question_id"] = question_id;
                question_result["answer_id"] = answer_id;
                question_result["is_correct"] = is_correct;
                if (is_correct) {
                    correct_answers++;
                }
                auto correct_result = txn.exec_params(
                    "SELECT id, answer_text FROM answers WHERE question_id = $1 AND is_correct = true",
                    question_id
                );
                if (!correct_result.empty()) {
                    question_result["correct_answer_id"] = correct_result[0]["id"].as<int>();
                    question_result["correct_answer_text"] = correct_result[0]["answer_text"].as<std::string>();
                }
                results.push_back(question_result);
            }
        }
        txn.commit();
        return {
            {"total_questions", total_questions},
            {"correct_answers", correct_answers},
            {"score", (total_questions > 0) ? (correct_answers * 100 / total_questions) : 0},
            {"results", results}
        };
    } catch (const exception& e) {
        cerr << "Ошибка проверки ответов: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
json QuizDatabaseManager::getTestStatistics(const string& user_login) {
    lock_guard<mutex> lock(db_mutex);
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
            user_login
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
bool QuizDatabaseManager::saveTestResult(const string& user_login, int category_id, int score, int total_questions, int correct_answers) {
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        txn.exec_params(
            "INSERT INTO test_results (user_login, category_id, score, total_questions, correct_answers) "
            "VALUES ($1, $2, $3, $4, $5)",
            user_login, category_id, score, total_questions, correct_answers
        );
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Проблема с сохранением теста: " << e.what() << endl;
        return false;
    }
}
