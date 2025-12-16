#include "resourceDatabase.h"
#include <iostream>
ResourceDatabaseManager::ResourceDatabaseManager() : connection(
    "host=localhost "
    "port=5432 "
    "dbname=resource "
    "user=admin306 "
    "password=ILoveMyCreation"
) {}
json ResourceDatabaseManager::getResourceCategories() {
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec("SELECT category_id, category_name FROM resource_categories ORDER BY category_id");
        txn.commit();
        json categories = json::array();
        for (const auto& row : result) {
            json category;
            category["id"] = row["category_id"].as<int>();
            category["name"] = row["category_name"].as<std::string>();
            categories.push_back(category);
        }
        return categories;
    } catch (const exception& e) {
        cerr << "Проблема с категориями ресурсов: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
json ResourceDatabaseManager::getResourcesByCategory(int category_id) {
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec_params(
            "SELECT resource_id, resource_name, author, resource_link "
            "FROM resources WHERE category_id = $1 ORDER BY resource_id",
            category_id
        );
        txn.commit();
        json resources = json::array();
        for (const auto& row : result) {
            json resource;
            resource["id"] = row["resource_id"].as<int>();
            resource["name"] = row["resource_name"].as<string>();
            resource["author"] = row["author"].is_null() ? "" : row["author"].as<string>();
            resource["link"] = row["resource_link"].as<string>();
            resources.push_back(resource);
        }
        return resources;
    } catch (const exception& e) {
        cerr << "Проблема с ресурсами: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
json ResourceDatabaseManager::getAllResources() {
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec(
            "SELECT r.resource_id, r.resource_name, r.author, r.resource_link, "
            "rc.category_name "
            "FROM resources r "
            "JOIN resource_categories rc ON r.category_id = rc.category_id "
            "ORDER BY rc.category_name, r.resource_name"
        );
        txn.commit();
        json resources = json::array();
        for (const auto& row : result) {
            json resource;
            resource["id"] = row["resource_id"].as<int>();
            resource["name"] = row["resource_name"].as<string>();
            resource["author"] = row["author"].is_null() ? "" : row["author"].as<string>();
            resource["link"] = row["resource_link"].as<string>();
            resource["category"] = row["category_name"].as<string>();
            resources.push_back(resource);
        }
        return resources;
    } catch (const std::exception& e) {
        cerr << "Проблема с ресурсами: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
json ResourceDatabaseManager::getResourcesWithCategories() {
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        auto categories_result = txn.exec(
            "SELECT category_id, category_name FROM resource_categories ORDER BY category_id"
        );
        json result = json::object();
        json categories_array = json::array();
        for (const auto& cat_row : categories_result) {
            json category;
            int cat_id = cat_row["category_id"].as<int>();
            string cat_name = cat_row["category_name"].as<string>();
            category["id"] = cat_id;
            category["name"] = cat_name;
            auto resources_result = txn.exec_params(
                "SELECT resource_id, resource_name, author, resource_link "
                "FROM resources WHERE category_id = $1 ORDER BY resource_name",
                cat_id
            );
            json resources = json::array();
            for (const auto& res_row : resources_result) {
                json resource;
                resource["id"] = res_row["resource_id"].as<int>();
                resource["name"] = res_row["resource_name"].as<string>();
                resource["author"] = res_row["author"].is_null() ? "" : res_row["author"].as<string>();
                resource["link"] = res_row["resource_link"].as<string>();
                resources.push_back(resource);
            }
            category["resources"] = resources;
            categories_array.push_back(category);
        }
        txn.commit();
        result["categories"] = categories_array;
        return result;
    } catch (const exception& e) {
        cerr << "Выдача ресурсов пользователю пошла под откос: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
