#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <regex>
#include <sys/stat.h>
#include <unordered_map>
#include "Database.h"

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(a,b) _mkdir(a)
#endif

bool file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::vector<std::string>& Database::getFileCache(const std::string& filename) {
    if (fileCache.find(filename) == fileCache.end()) {
        std::vector<std::string> lines;
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        fileCache[filename] = std::move(lines);
    }
    return fileCache[filename];
}

void Database::flushCacheToFile(const std::string& filename) {
    auto& lines = getFileCache(filename);
    std::ofstream outfile(filename, std::ios::trunc);
    for (const auto& line : lines) {
        outfile << line << "\n";
    }
    outfile.close();
}

//
// Пример функции для получения следующего индекса записи через кэш
//
uint32_t Database::get_next_index(const std::string& filename) {
    auto& lines = getFileCache(filename);
    uint32_t next_index = 0;
    for (const auto& line : lines) {
        std::istringstream iss(line);
        uint32_t current_index;
        if (!(iss >> current_index)) continue;
        next_index = std::max(next_index, current_index);
    }
    return next_index + 1;
}

int Database::create_db() {
    try {
        std::string filename = "data.flux";
        if (file_exists(filename)) {
            log("Database already exists");
            return 2;
        }

        std::ofstream db(filename, std::ios::binary | std::ios::app);
        if (db.is_open()) {
            std::string time = getFormattedTime();

            std::ofstream indexes("indexes.flux", std::ios::binary | std::ios::app);
            if (indexes.is_open()) {
                std::string stime = getFormattedTime();
                log("Indexes file created");
            } else {
                log("Failed to create indexes file");
                return 1;
            }
            log("Database created successfully");
            fileCache[filename] = std::vector<std::string>(); // Инициализируем кэш для нового файла
            return 0;
        } else {
            log("Failed to create database file");
            return 1;
        }
    } catch (const std::exception& e) {
        log("Database creation failed with exception: " + std::string(e.what()));
        return 1;
    }
}

bool Database::write_record(const std::string& filename, const std::string& value) {
    auto& lines = getFileCache(filename);
    uint32_t next_index = get_next_index(filename);
    std::ostringstream record;
    record << next_index << " " << value;
    std::string record_str = record.str();

    // Добавляем запись в кэш
    lines.push_back(record_str);

    // Дописываем запись в файл
    std::ofstream file(filename, std::ios::app);
    if (!file) {
        log("Failed to write record to file: " + filename);
        return false;
    }
    file << record_str << std::endl;
    file.close();
    log("Record written to file: " + filename);
    return true;
}

bool Database::write_str(const std::string& filename, const std::string& value) {
    bool result = write_record(filename, value);
    log("String written to file: " + filename);
    return result;
}

bool Database::write_int(const std::string& filename, int number) {
    std::string numStr = std::to_string(number);
    bool result = write_record(filename, numStr);
    log("Integer written to file: " + filename);
    return result;
}

std::string to_lower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> Database::search(const std::string& filename, const std::string& query) {
    std::vector<std::string> results;
    auto& lines = getFileCache(filename);
    for (const auto& line : lines) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        std::istringstream iss(trimmed);
        uint32_t index;
        std::string value;
        if (!(iss >> index)) continue;
        std::getline(iss, value);
        if (!value.empty() && value[0] == ' ') {
            value = value.substr(1);
        }
        if (to_lower(value).find(to_lower(query)) != std::string::npos) {
            results.push_back(line);
        }
    }
    log("Search completed in file: " + filename);
    return results;
}

std::string Database::getFormattedTime() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* time_info = std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(time_info, "%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::vector<std::string> Database::search_exact(const std::string& filename, const std::string& query) {
    std::vector<std::string> results;
    auto& lines = getFileCache(filename);
    for (const auto& line : lines) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        std::istringstream iss(trimmed);
        uint32_t index;
        if (!(iss >> index)) continue;
        std::string value;
        std::getline(iss, value);
        value = trim(value);
        if (to_lower(value) == to_lower(query))
            results.push_back(line);
    }
    log("Exact search completed in file: " + filename);
    return results;
}

std::vector<std::string> Database::search_regex(const std::string& filename, const std::string& regex_pattern) {
    std::vector<std::string> results;
    auto& lines = getFileCache(filename);
    std::regex pattern(regex_pattern, std::regex_constants::icase);
    for (const auto& line : lines) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        std::istringstream iss(trimmed);
        uint32_t index;
        if (!(iss >> index)) continue;
        std::string value;
        std::getline(iss, value);
        value = trim(value);
        if (std::regex_search(value, pattern))
            results.push_back(line);
    }
    log("Regex search completed in file: " + filename);
    return results;
}

bool Database::replace(const std::string& filename, uint32_t id, const std::string& newValue) {
    auto& lines = getFileCache(filename);
    bool found = false;
    for (auto& line : lines) {
        std::istringstream iss(line);
        uint32_t currentId;
        if (!(iss >> currentId)) continue;
        if (currentId == id) {
            std::ostringstream oss;
            oss << id << " " << newValue;
            line = oss.str();
            found = true;
            break;
        }
    }
    if (found) {
        flushCacheToFile(filename);
        log("Replacement completed in file: " + filename);
    }
    return found;
}

bool Database::replace_by_content(const std::string& filename, const std::string& content, const std::string& newValue) {
    std::vector<std::string> matches = search_exact(filename, content);
    if (matches.empty()) {
        log("No matches found for replacement by content in file: " + filename);
        return false;
    }
    std::istringstream iss(matches[0]);
    uint32_t id;
    if (!(iss >> id)) {
        log("Failed to parse ID for replacement by content in file: " + filename);
        return false;
    }
    bool result = replace(filename, id, newValue);
    log("Content replaced in file: " + filename);
    return result;
}

bool Database::delete_record_by_id(const std::string& filename, uint32_t id) {
    auto& lines = getFileCache(filename);
    auto originalSize = lines.size();
    lines.erase(std::remove_if(lines.begin(), lines.end(),
                 [id](const std::string& line) {
                     std::istringstream iss(line);
                     uint32_t currentId;
                     if (!(iss >> currentId)) return false;
                     return currentId == id;
                 }), lines.end());
    if (lines.size() == originalSize) {
        log("Record not found for deletion by ID in file: " + filename);
        return false;
    }
    flushCacheToFile(filename);
    log("Record deleted by ID in file: " + filename);
    return true;
}

bool Database::delete_by_content(const std::string& filename, const std::string& content) {
    std::vector<std::string> matches = search_exact(filename, content);
    if (matches.empty()) {
        log("No matches found for deletion by content in file: " + filename);
        return false;
    }
    std::istringstream iss(matches[0]);
    uint32_t id;
    if (!(iss >> id)) {
        log("Failed to parse ID for deletion by content in file: " + filename);
        return false;
    }
    bool result = delete_record_by_id(filename, id);
    log("Content deleted in file: " + filename);
    return result;
}

int Database::get_records_count(const std::string& filename) {
    auto& lines = getFileCache(filename);
    int count = lines.size();
    log("Records counted in file: " + filename);
    return count;
}

bool Database::clear_db(const std::string& filename) {
    fileCache[filename].clear();
    std::ofstream file(filename, std::ios::trunc);
    if (!file) {
        log("Failed to clear database file: " + filename);
        return false;
    }
    file.close();
    log("Database cleared: " + filename);
    return true;
}

bool create_directory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
#ifdef _WIN32
        return mkdir(path.c_str(), 0777) == 0;
#else
        return mkdir(path.c_str(), 0777) == 0;
#endif
    }
    return (info.st_mode & S_IFDIR) != 0;
}

std::string get_current_time() {
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
    return std::string(buffer);
}

bool Database::backup_db(const std::string& filename, const std::string& backup_filename) {
    std::string backup_dir = "backups";
    if (!create_directory(backup_dir)) {
        log("Failed to create backup directory");
        return false;
    }
    std::string backup_path = backup_dir + "/" + backup_filename;
    std::ifstream src(filename, std::ios::binary);
    if (!src) {
        log("Failed to open source file for backup: " + filename);
        return false;
    }
    std::ofstream dst(backup_path, std::ios::binary);
    if (!dst) {
        log("Failed to create backup file: " + backup_path);
        return false;
    }
    dst << src.rdbuf();
    log("Backup created: " + backup_path);
    return true;
}

void Database::log(const std::string& action) {
    std::string log_dir = "logs";
    if (!create_directory(log_dir)) {
        std::cerr << "Error creating log folder" << std::endl;
        return;
    }
    std::ofstream log_file(log_dir + "/actions.log", std::ios::app);
    if (!log_file) {
        std::cerr << "Error opening log file" << std::endl;
        return;
    }
    log_file << "[" << get_current_time() << " - " << action << "]" << std::endl;
}

void Database::get_total_weight(const std::string& filename) {
    auto& lines = getFileCache(filename);
    int total_weight = 0;
    for (const auto &line : lines) {
        total_weight += line.length();
    }
    log("Total weight counted in file: " + filename);
    std::cout << "Total weight: " << total_weight << " bytes" << std::endl;
}
