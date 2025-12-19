#include "clientSession.h"

ClientSession::ClientSession(boost::asio::ssl::stream<tcp::socket> sock, 
                  UserDatabaseManager& userdbmanager, 
                  QuizDatabaseManager& quizdbmanager, 
                  ResourceDatabaseManager& resourcedbmanager, 
                  BruteForceProtection& bfprotector)
    : socket(move(sock)), 
      userdb(userdbmanager), 
      quizdb(quizdbmanager), 
      resourcedb(resourcedbmanager), 
      bfprotection(bfprotector) {}
void ClientSession::start() {
    auto self(shared_from_this());
    socket.async_handshake(
        boost::asio::ssl::stream_base::server,
        [this, self](const boost::system::error_code& error) {
            if (!error) {
                do_read();
            } else {
                cerr << "Ошибка SSL рукопожатия: " << error.message() << endl;
            }
        }
    );
}

void ClientSession::do_read() {
    auto self(shared_from_this());
    socket.async_read_some(boost::asio::buffer(buffer, max_length),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                string requeststr(buffer, length);
                process_request(requeststr);
                do_read();
            } else if (ec == boost::asio::ssl::error::stream_truncated) {
                // Нормальное завершение TLS соединения
                cout << "Клиент отключился (TLS stream truncated)" << endl;
            } else {
                cerr << "Ошибка чтения: " << ec.message() << endl;
            }
        }
    );
}

void ClientSession::process_request(const std::string& requeststr) {
    try {
        json request = json::parse(requeststr);
        string command = request["command"];
        json response;
        
        if (command == "login") {
            string login = request["login"];
            string password = request["password"];
            
            if (!bfprotection.canAttemptLogin(login)) {
                int remainingtime = bfprotection.getRemainingBlockTime(login);
                response = {
                    {"status", "error"},
                    {"message", "Слишком много попыток входа. Попробуйте через " + to_string(remainingtime) + " секунд"},
                    {"blocked", true},
                    {"remaining_time", remainingtime}
                };
            } else {
                if (userdb.authenticateUser(login, password, bfprotection)) {
                    response = {
                        {"status", "success"},
                        {"message", "Аутентификация успешна"},
                        {"user", login}
                    };
                } else {
                    int remainingtime = bfprotection.getRemainingBlockTime(login);
                    if (remainingtime > 0) {
                        response = {
                            {"status", "error"},
                            {"message", "Неверный логин или пароль. Слишком много попыток. Подождите " + to_string(remainingtime) + " секунд"},
                            {"blocked", true},
                            {"remaining_time", remainingtime}
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
            
            if (userdb.userExists(login)) {
                response = {
                    {"status", "error"},
                    {"message", "Пользователь уже существует"}
                };
            } else if (userdb.registerUser(login, password)) {
                response = {
                    {"status", "success"},
                    {"message", "Регистрация успешна"}
                };
            } else {
                response = {
                    {"status", "error"},
                    {"message", "Ошибка регистрации"}
                };
            }
        }
        else if (command == "get_profile") {
            string login = request["login"];
            json profile = userdb.getUserProfile(login);
            response = {
                {"status", "success"},
                {"profile", profile}
            };
        }
        else if (command == "save_profile") {
            json profile = request["profile"];
            if (userdb.saveUserProfile(profile)) {
                response = {
                    {"status", "success"},
                    {"message", "Профиль сохранен"}
                };
            } else {
                response = {
                    {"status", "error"},
                    {"message", "Ошибка сохранения профиля"}
                };
            }
        }
        else if (command == "get_categories") {
            json categories = quizdb.getCategories();
            response = {
                {"status", "success"},
                {"categories", categories}
            };
        }
        else if (command == "get_questions") {
            int categoryid = request["category_id"];
            json questions = quizdb.getQuestionsByCategory(categoryid);
            response = {
                {"status", "success"},
                {"questions", questions}
            };
        }
        else if (command == "check_answers") {
            json useranswers = request["answers"];
            json result = quizdb.checkAnswers(useranswers);
            
            if (request.contains("user") && request.contains("category_id")) {
                string userlogin = request["user"];
                int categoryid = request["category_id"];
                int totalquestions = result["total_questions"];
                int correctanswers = result["correct_answers"];
                int score = result["score"];
                
                quizdb.saveTestResult(userlogin, categoryid, score, totalquestions, correctanswers);
            }
            
            response = {
                {"status", "success"},
                {"result", result}
            };
        }
        else if (command == "get_statistics") {
            string userlogin = request["user"];
            json statistics = quizdb.getTestStatistics(userlogin);
            response = {
                {"status", "success"},
                {"statistics", statistics}
            };
        }
        else if (command == "get_resource_categories") {
            json categories = resourcedb.getResourceCategories();
            response = {
                {"status", "success"},
                {"categories", categories}
            };
        }
        else if (command == "get_resources_by_category") {
            if (request.contains("category_id")) {
                int categoryid = request["category_id"];
                json resources = resourcedb.getResourcesByCategory(categoryid);
                response = {
                    {"status", "success"},
                    {"resources", resources}
                };
            } else {
                response = {
                    {"status", "error"},
                    {"message", "Не указан ID категории"}
                };
            }
        }
        else if (command == "get_all_resources") {
            json resources = resourcedb.getAllResources();
            response = {
                {"status", "success"},
                {"resources", resources}
            };
        }
        else if (command == "get_resources_with_categories") {
            json resources = resourcedb.getResourcesWithCategories();
            response = {
                {"status", "success"},
                {"data", resources}
            };
        }
        else if (command == "ping") {
            response = {
                {"status", "success"},
                {"message", "pong"},
                {"encrypted", true}
            };
        }
        else {
            response = {
                {"status", "error"},
                {"message", "Неизвестная команда"}
            };
        }
        
        send_response(response);
    } catch (const exception& e) {
        json error_response = {
            {"status", "error"},
            {"message", string("Ошибка сервера: ") + e.what()}
        };
        send_response(error_response);
    }
}

void ClientSession::send_response(const json& response) {
    string responsestr = response.dump() + "\n";
    auto self(shared_from_this());
    
    boost::asio::async_write(socket, boost::asio::buffer(responsestr),
        [this, self](boost::system::error_code ec, size_t) {
            if (ec) {
                cerr << "Ошибка отправки ответа: " << ec.message() << endl;
            }
        }
    );
}
