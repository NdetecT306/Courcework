#ifndef RESOURCEDATABASEMANAGER_H
#define RESOURCEDATABASEMANAGER_H
#include <string>
#include <mutex>
#include <pqxx/pqxx>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class ResourceDatabaseManager {
private:
    pqxx::connection connection;
    mutex db_mutex;
    
public:
    ResourceDatabaseManager();
    json getResourceCategories();
    json getResourcesByCategory(int category_id);
    json getAllResources();
    json getResourcesWithCategories();
};
#endif
