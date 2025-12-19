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
    mutex dbmutex;
    
public:
    ResourceDatabaseManager();
    json getResourceCategories();
    json getResourcesByCategory(int categoryid);
    json getAllResources();
    json getResourcesWithCategories();
};
#endif
