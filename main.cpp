#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "Database.h"

namespace fs = std::filesystem;

struct Config {
    std::string filename;
    std::string backup_name;
};

Config load_config() {
    Config config;
    std::string configPath = "config.yaml";
    if (!fs::exists(configPath)) {
        std::ofstream ofs(configPath);
        if (!ofs) {
            std::cerr << "Error: cannot create config.yaml.\n";
            exit(1);
        }
        ofs << "filename: data.flux\nbackup_name: backup.flux\n";
        ofs.close();
        Database db;
        db.log("Succesfully created config.yaml");
    }
    std::ifstream ifs(configPath);
    if (!ifs) {
        std::cerr << "Ошибка: невозможно открыть файл config.yaml.\n";
        exit(1);
    }
    std::string line;
    while (std::getline(ifs, line)) {
        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            if (key == "filename") {
                config.filename = value;
            } else if (key == "backup_name") {
                config.backup_name = value;
            }
        }
    }
    return config;
}

void checkDependencies(const Config &config) {
    bool missingMandatory = false;
    if (!fs::exists(config.filename)) {
        std::cerr << "Mandatory file missing: " << config.filename << std::endl;
        missingMandatory = true;
    } else {
        std::cout << "Mandatory file found: " << config.filename << std::endl;
    }
    if (!fs::exists("indexes.flux")) {
        std::cerr << "Mandatory file missing: indexes.flux" << std::endl;
        missingMandatory = true;
    } else {
        std::cout << "Mandatory file found: indexes.flux" << std::endl;
    }
    if (!fs::exists("logs/actions.log")) {
        std::cerr << "Mandatory file missing: logs/actions.log" << std::endl;
        missingMandatory = true;
    } else {
        std::cout << "Mandatory file found: logs/actions.log" << std::endl;
    }
    if(!fs::exists("config.yaml")) {
        std::cerr << "Mandatory file missing: config.yaml" << std::endl;
        missingMandatory = true;
    } else {
        std::cout << "Mandatory file found: config.yaml" << std::endl;
    }
    if (missingMandatory) {
        std::cerr << "Mandatory dependencies are missing. Please create the missing files/folders.\nLook the docs" << std::endl;
    } else {
        std::cout << "All mandatory dependencies are present." << std::endl;
    }
    if (!fs::exists("backups")) {
        std::cout << "It is recommended to create backup[backup_db]" << std::endl;
    }
}

Config g_config;

void print_banner() {
    std::cout << R"(
  ______ _     
 |  ____| |   
 | |__  | |       
 |  __| | | | |  | |\ \/ /
 | |    | | | |__| | >  <
 |_|    |_|  \____/ /_/\_\
)" << std::endl;
    std::cout << "Welcome to FLUX Database CLI\n";
    std::cout << "Documentation: https://github.com/apchhui/FluxDB\n\n";
    std::cout << "Type 'help' for a list of commands.\n\n";
}

void process_command(Database& db, const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    if (cmd == "create_db") {
        if (db.create_db() == 0) {
            std::cout << "Database created successfully.\n";
        } else {
            std::cout << "Failed to create database.\n";
        }
    }
    else if (cmd == "write_str") {
        std::string value;
        std::getline(iss >> std::ws, value);
        if (value.empty()) {
            std::cout << "Invalid arguments for write_str.\n";
            return;
        }
        if (db.write_str(g_config.filename, value)) {
            std::cout << "Record inserted.\n";
        } else {
            std::cout << "Failed to insert record.\n";
        }
    }
    else if (cmd == "write_int") {
        int number;
        if (!(iss >> number)) {
            std::cout << "Invalid arguments for write_int. Expected an integer.\n";
            return;
        }
        if (db.write_int(g_config.filename, number)) {
            std::cout << "Record inserted.\n";
        } else {
            std::cout << "Failed to insert record.\n";
        }
    }
    else if (cmd == "search") {
        std::string query;
        std::getline(iss >> std::ws, query);
        if (query.empty()) {
            std::cout << "Invalid arguments for search.\n";
            return;
        }
        auto results = db.search(g_config.filename, query);
        for (const auto& res : results) {
            std::cout << res << "\n";
        }
    }
    else if (cmd == "search_exact") {
        std::string query;
        std::getline(iss >> std::ws, query);
        if (query.empty()) {
            std::cout << "Invalid arguments for search_exact.\n";
            return;
        }
        auto results = db.search_exact(g_config.filename, query);
        for (const auto& res : results) {
            std::cout << res << "\n";
        }
    }
    else if (cmd == "search_regex") {
        std::string regex_pattern;
        std::getline(iss >> std::ws, regex_pattern);
        if (regex_pattern.empty()) {
            std::cout << "Invalid arguments for search_regex.\n";
            return;
        }
        auto results = db.search_regex(g_config.filename, regex_pattern);
        for (const auto& res : results) {
            std::cout << res << "\n";
        }
    }
    else if (cmd == "delete_record_by_id") {
        uint32_t id;
        if (!(iss >> id)) {
            std::cout << "Invalid arguments for delete_record_by_id. Expected an integer ID.\n";
            return;
        }
        if (db.delete_record_by_id(g_config.filename, id)) {
            std::cout << "Record deleted.\n";
        } else {
            std::cout << "Record not found.\n";
        }
    }
    else if (cmd == "delete_by_content") {
        std::string content;
        iss >> content;
        if (content.empty()) {
            std::cout << "Invalid arguments for delete_by_content.\n";
            return;
        }
        if (db.delete_by_content(g_config.filename, content)) {
            std::cout << "Records deleted.\n";
        } else {
            std::cout << "Records not found.\n";
        }
    }
    else if (cmd == "clear_db") {
        if (db.clear_db(g_config.filename)) {
            std::cout << "Database flushed.\n";
        } else {
            std::cout << "Failed to flush database.\n";
        }
    }
    else if (cmd == "backup_db") {
        if (db.backup_db(g_config.filename, g_config.backup_name)) {
            std::cout << "Backup created successfully.\n";
        } else {
            std::cout << "Failed to create backup.\n";
        }
    }
    else if (cmd == "help") {
        std::cout << "Available commands:\n"
                  << "  create_db             - Create a new database\n"
                  << "  write_str <value>     - Insert a record (string)\n"
                  << "  write_int <number>    - Insert a record (integer)\n"
                  << "  search <query>        - Search for records\n"
                  << "  search_exact <query>  - Search for exact records\n"
                  << "  search_regex <regex>  - Search for records using regex\n"
                  << "  delete_record_by_id <id> - Delete a record by ID\n"
                  << "  delete_by_content <content> - Delete records by content\n"
                  << "  backup_db             - Create a backup\n"
                  << "  clear_db              - Flush the database\n"
                  << "  exit                  - Quit the CLI\n";
    }
    else if (cmd == "exit") {
        db.log("Disconnected from DB");
        std::cout << "Exiting...\n";
        exit(0);
    }
    else {
        std::cout << "Unknown command. Type 'help' for a list of commands.\n";
    }
}

int main() {
    g_config = load_config();
    checkDependencies(g_config);
    Database db;
    print_banner();
    db.log("Connected to DB");
    while (true) {
        std::cout << "Flux> ";
        std::string command;
        std::getline(std::cin, command);
        if (!command.empty()) {
            process_command(db, command);
        }
    }
    return 0;
}