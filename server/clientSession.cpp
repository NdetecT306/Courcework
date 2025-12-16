#include "clientSession.h"
#include <iostream>

ClientSession::ClientSession(tcp::socket sock, UserDatabaseManager& user_db_manager, 
                  QuizDatabaseManager& quiz_db_manager, ResourceDatabaseManager& resource_db_manager,BruteForceProtection& bf_protector)
    : socket(move(sock)), user_db(user_db_manager), quiz_db(quiz_db_manager), resource_db(resource_db_manager),bf_protection(bf_protector) {}
void ClientSession::start() {
    do_read();
}
void ClientSession::do_read() {
    auto self(shared_from_this());
    socket.async_read_some(boost::asio::buffer(buffer),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                string request_str(buffer.data(), length);
                process_request(request_str);
                do_read();
            }
        }
    );
}
void ClientSession::process_request(const std::string& request_str) {
    try {
        json request = json::parse(request_str);
        string command = request["command"];
        json response;
        if (command == "login") {
            string login = request["login"];
            string password = request["password"];
            if (!bf_protection.canAttemptLogin(login)) {
                int remaining_time = bf_protection.getRemainingBlockTime(login);
                response = {
                    {"status", "error"},
                    {"message", "Слишком много попыток входа. Попробуйте через " + to_string(remaining_time) + " секунд"},
                    {"blocked", true},
                    {"remaining_time", remaining_time}
                };
            } else {
                if (user_db.authenticateUser(login, password, bf_protection)) {
                    response = {
                        {"status", "success"},
                        {"message", "Authentication successful"},
                        {"user", login}
                    };
                } else {
                    int remaining_time = bf_protection.getRemainingBlockTime(login);
                    if (remaining_time > 0) {
                        response = {
                            {"status", "error"},
                            {"message", "Неверный логин или пароль. Слишком много попыток. Подождите " + to_string(remaining_time) + " секунд"},
                            {"blocked", true},
                            {"remaining_time", remaining_time}
                        };
                    } else {
                        response = {
                            {"status", "error"},
                            {"message", "Неверный логин или пароль"}
                        };
                    }
                }
            }
        }
        else if (command == "register") {
            string login = request["login"];
            string password = request["password"];
            if (user_db.userExists(login)) {
                response = {
                    {"status", "error"},
                    {"message", "User already exists"}
                };
            } else if (user_db.registerUser(login, password)) {
                response = {
                    {"status", "success"},
                    {"message", "Registration successful"}
                };
            } else {
                response = {
                    {"status", "error"},
                    {"message", "Registration failed"}
                };
            }
        }
        else if (command == "get_profile") {
            string login = request["login"];
            json profile = user_db.getUserProfile(login);
            response = {
                {"status", "success"},
                {"profile", profile}
            };
        }
        else if (command == "save_profile") {
            json profile = request["profile"];
            if (user_db.saveUserProfile(profile)) {
                response = {
                    {"status", "success"},
                    {"message", "Profile saved"}
                };
            } else {
                response = {
                    {"status", "error"},
                    {"message", "Failed to save profile"}
                };
            }
        }
        else if (command == "get_categories") {
            json categories = quiz_db.getCategories();
            response = {
                {"status", "success"},
                {"categories", categories}
            };
        }
        else if (command == "get_questions") {
            int category_id = request["category_id"];
            json questions = quiz_db.getQuestionsByCategory(category_id);
            response = {
                {"status", "success"},
                {"questions", questions}
            };
        }
        else if (command == "check_answers") {
            json user_answers = request["answers"];
            json result = quiz_db.checkAnswers(user_answers);
            if (request.contains("user") && request.contains("category_id")) {
                string user_login = request["user"];
                int category_id = request["category_id"];
                int total_questions = result["total_questions"];
                int correct_answers = result["correct_answers"];
                int score = result["score"];
                quiz_db.saveTestResult(user_login, category_id, score, total_questions, correct_answers);
            }
            response = {
                {"status", "success"},
                {"result", result}
            };
        }
        else if (command == "get_statistics") {
            string user_login = request["user"];
            json statistics = quiz_db.getTestStatistics(user_login);
            response = {
                {"status", "success"},
                {"statistics", statistics}
            };
        }
        else if (command == "get_resource_categories") {
            json categories = resource_db.getResourceCategories();
            response = {
                {"status", "success"},
                {"categories", categories}
            };
        }
        else if (command == "get_resources_by_category") {
            if (request.contains("category_id")) {
                int category_id = request["category_id"];
                json resources = resource_db.getResourcesByCategory(category_id);
                response = {
                    {"status", "success"},
                    {"resources", resources}
                };
            } else {
                response = {
                    {"status", "error"},
                    {"message", "Missing category_id parameter"}
                };
            }
        }
        else if (command == "get_all_resources") {
            json resources = resource_db.getAllResources();
            response = {
                {"status", "success"},
                {"resources", resources}
            };
        }
        else if (command == "get_resources_with_categories") {
            json resources = resource_db.getResourcesWithCategories();
            response = {
                {"status", "success"},
                {"data", resources}
            };
        }
        else if (command == "ping") {
            response = {
                {"status", "success"},
                {"message", "pong"}
            };
        }
        else {
            response = {
                {"status", "error"},
                {"message", "Unknown command"}
            };
        }
        send_response(response);
    } catch (const exception& e) {
        json error_response = {
            {"status", "error"},
            {"message", string("Server error: ") + e.what()}
        };
        send_response(error_response);
    }
}

void ClientSession::send_response(const json& response) {
    string response_str = response.dump() + "\n";
    auto self(shared_from_this());
    boost::asio::async_write(socket, boost::asio::buffer(response_str),
        [this, self](boost::system::error_code ec, size_t) {
            if (ec) {
                cerr << "Ошибка отправки: " << ec.message() << endl;
            }
        }
    );
}
