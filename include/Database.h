#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <unordered_map>

class Database {
public:

    int create_db();
    bool write_record(const std::string& filename, const std::string& value);
    bool write_str(const std::string& filename, const std::string& value);
    bool write_int(const std::string& filename, int number);
    std::vector<std::string> search(const std::string& filename, const std::string& query);
    std::vector<std::string> search_exact(const std::string& filename, const std::string& query);
    std::vector<std::string> search_regex(const std::string& filename, const std::string& regex_pattern);
    bool replace(const std::string& filename, uint32_t id, const std::string& newValue);
    bool replace_by_content(const std::string& filename, const std::string& content, const std::string& newValue);
    bool delete_record_by_id(const std::string& filename, uint32_t id);
    bool delete_by_content(const std::string& filename, const std::string& content);
    int get_records_count(const std::string& filename);
    bool clear_db(const std::string& filename);
    bool backup_db(const std::string& filename, const std::string& backup_filename);
    void log(const std::string& action);
    void get_total_weight(const std::string& filename);
    std::string getFormattedTime();

    std::vector<std::string>& getFileCache(const std::string& filename);
    void flushCacheToFile(const std::string& filename);
    uint32_t get_next_index(const std::string& filename);

private:

    std::unordered_map<std::string, std::vector<std::string>> fileCache;
};

#endif
