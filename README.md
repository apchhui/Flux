# FluxDB CLI Documentation

## Overview
FluxDB is a command-line interface (CLI) for managing a database with various operations such as inserting, searching, deleting, and backing up data.

## Commands

### `create_db`
**Description:**
Creates a new database.

**Usage:**
```sh
create_db
```

---

### `write_str <value>`
**Description:**
Inserts a record as a string into the database.

**Usage:**
```sh
write_str "Hello, World!"
```

---

### `write_int <number>`
**Description:**
Inserts a record as an integer into the database.

**Usage:**
```sh
write_int 42
```

---

### `search <query>`
**Description:**
Searches for records containing the given query string.

**Usage:**
```sh
search "Hello"
```

---

### `search_exact <query>`
**Description:**
Searches for records that exactly match the given query string.

**Usage:**
```sh
search_exact "Hello, World!"
```

---

### `search_regex <regex>`
**Description:**
Searches for records using a regular expression pattern.

**Usage:**
```sh
search_regex "^H.*!$"
```

---

### `delete_record_by_id <id>`
**Description:**
Deletes a record by its unique ID.

**Usage:**
```sh
delete_record_by_id 123
```

---

### `delete_by_content <content>`
**Description:**
Deletes records that contain the given content.

**Usage:**
```sh
delete_by_content "Hello"
```

---

### `clear_db`
**Description:**
Flushes the database by removing all records.

**Usage:**
```sh
clear_db
```

---

### `backup_db`
**Description:**
Creates a backup of the database.

**Usage:**
```sh
backup_db
```

---

### `help`
**Description:**
Displays a list of available commands.

**Usage:**
```sh
help
```

---

### `exit`
**Description:**
Exits the FluxDB CLI.

**Usage:**
```sh
exit
```

---

## Additional Functions

### `load_config()`
Loads the configuration from `config.yaml`. If the file does not exist, it is created with default values.

### `checkDependencies(const Config &config)`
Checks if mandatory files such as `config.yaml`, `data.flux`, `indexes.flux`, and `logs/actions.log` exist. If any are missing, it prompts the user to create them.

### `print_banner()`
Displays the FluxDB banner and welcome message.

### `process_command(Database& db, const std::string& command)`
Processes user commands and executes the corresponding database operations.

### `main()`
Initializes the CLI, loads the configuration, checks dependencies, and starts an interactive loop for processing commands.

---

## Documentation
For more information, visit the official repository:
[Flux GitHub Repository](https://github.com/apchhui/Flux)
