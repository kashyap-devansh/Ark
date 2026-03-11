# 🐘 Ark

> **Note:** 🚧 This project is currently **under development** and is actively being worked on. Features and syntax may be subject to change.

## 📖 Overview

**Ark** is a custom, lightweight database engine and SQL-like query language interpreter implemented in C++. 

It provides relational database capabilities, allowing users to parse and execute basic SQL-like statements. The project is built from scratch and includes its own custom tokenizer, expression parser, and core execution engine. It handles custom data storage and memory management to simulate a functioning database environment.

## ✨ Current Features

- **Database Management**: Create, drop, and switch between databases (`CREATE DATABASE`, `DROP DATABASE`, `USE`).
- **Table Operations**: Define and drop tables (`CREATE TABLE`, `DROP TABLE`).
- **Data Types**: Full support for `INT`, `DOUBLE`, `STRING`, and `BOOL` data types.
- **Advanced Data Manipulation (DML)**: 
  - Insert single or multiple records at once (`INSERT INTO`).
  - Retrieve data with column selection (`SELECT`).
  - Update specific records (`UPDATE`).
  - Delete records (`DELETE`).
  - **Query Filtering**: Powerful conditional filtering using `WHERE` with multiple operators (`==`, `!=`, `<`, `>`, `<=`, `>=`).
  - **Pattern Matching**: String matching using `LIKE '<c>%'`.
  - **Limiting Results**: Restrict modifications or deletions using `LIMIT <n>`.
- **Navigation & Inspection**: Introspect databases, tables, and column structures (`SHOW DATABASES`, `SHOW TABLES`, `SHOW COLUMNS`).
- **Persistence**: Save and load databases entirely to and from disk (`SAVE`, `LOAD`).

*Note: Graph systems and compound conditions (AND/OR) are currently planned for future updates.*

## 🗂️ Supported SQL Syntax

Ark supports a custom subset of standard SQL. Here is a quick reference guide:

```sql
-- Database Commands
CREATE DATABASE <name>;
DROP DATABASE <name>;
USE <database>;
SHOW DATABASES;

-- Table Commands
CREATE TABLE <name> (<col> <type>, ...);
DROP TABLE <name>;
SHOW TABLES;
SHOW COLUMNS FROM <table>;

-- Insert Data
INSERT INTO <table> VALUES (<val>, ...);
INSERT INTO <table> VALUES (<val>, ...), (<val>, ...);

-- Query Data
SELECT * FROM <table>;
SELECT <col1>, <col2> FROM <table> WHERE <col> == <val>;
SELECT * FROM <table> WHERE <col> LIKE '<c>%';

-- Update Data
UPDATE <table> SET <col> = <val> WHERE <col> == <val> LIMIT <n>;

-- Delete Data
DELETE FROM <table> WHERE <col> > <val>;

-- Persistence
SAVE;
LOAD;
```

## 🏗️ Project Structure

The project code is organized logically to separate lexical analysis, parsing, and execution:

```text
Ark/
├── databases/              # Directory where database files (.tbl and .dat) are saved
├── include/                # Header files for all system components
│   ├── core.h              # Engine definitions (Cell, Column, Row, Table, Database)
│   ├── databaseManager.h   # Orchestrates system databases
│   ├── parser.h            # AST and query parsing structs
│   └── tokenizer.h         # Lexical analysis definitions
├── src/                    # Implementation source files
│   ├── core.cpp            # Core engine execution logic
│   ├── databaseManager.cpp # Database manager functionality
│   ├── main.cpp            # Entry point for the interpreter
│   ├── parser.cpp          # Parsing tokens into operations
│   └── tokenizer.cpp       # Tokenizing raw queries
├── ArkTest.ark             # Example SQL script to test the engine
└── README.md               # Project documentation
```

## 🚀 Example Usage

You can write Ark statements in a `.ark` script file. For example (`ArkTest.ark`):

```sql
CREATE DATABASE testdb;
USE testdb;

CREATE TABLE users (id INT, name STRING, age INT);

INSERT INTO users VALUES (1, "Alice", 25);
INSERT INTO users VALUES (2, "Bob", 30);
INSERT INTO users VALUES (3, "Charlie", 35);

UPDATE users SET age = 18 WHERE id >= 1 LIMIT 3;

SELECT * FROM users;
SAVE;
```

## 🛠️ Getting Started

### Prerequisites
To compile the project, you need a C++ compiler (like `g++`).

### Compilation
Compile the project by passing all source files and including the header directory:

```bash
g++ -Iinclude src/*.cpp -o program
```

### Execution
Run the executable by passing your script file containing Ark commands:

```bash
./program.exe ArkTest.ark
```

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
