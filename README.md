<div align="center">
  <h1>🐘 Ark SQL-like Database Engine</h1>
  <p>
    <strong>A high-performance, custom, lightweight database engine and SQL-like query language interpreter built entirely sequentially from scratch in C++.</strong>
  </p>
  <p>
    <img src="https://img.shields.io/badge/C++-17+-blue.svg?logo=c%2B%2B" alt="C++" />
    <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License" />
    <img src="https://img.shields.io/badge/Status-Under%20Development-orange.svg" alt="Status" />
  </p>
</div>

> **Note:** 🚧 This project is currently **under development** and is actively being worked on. Features, syntax, and architecture may be subject to change.

## 📖 Overview

**Ark** is an advanced, standalone database engine simulation and interpreter. 

### 🛠️ Built 100% From Scratch in C++
Unlike many projects that wrap existing engines or use parser generators, **Ark is built entirely from the ground up in C++**. It strictly eschews external database libraries (like SQLite) and parser generators (like Bison/Flex). 

Every single component of the pipeline is a custom, hand-written C++ implementation:
- **Zero External Dependencies:** No third-party database APIs or parsing libraries are used.
- **Composition over Inheritance:** In its Object-Oriented design, Ark rigorously utilizes **composition instead of inheritance**. This modern approach ensures loose coupling, superior modularity, and high-performance execution without the overhead of deep class hierarchies.
- **Custom Lexical Tokenizer & Parser:** Hand-rolled lexical analysis and Abstract Syntax Tree (AST) generation.
- **Native Execution Engine:** Custom in-memory table structures, data types, and row filtering algorithms.
- **Raw Disk Persistence:** Custom serialization for saving and loading databases directly to disk.

Ark provides robust relational database capabilities, allowing users to parse, validate, and execute a comprehensive subset of standard SQL-like statements. It handles its own custom data structure storage and memory management to simulate a complete functioning database environment.

---

## ⚙️ How Ark Works (Architecture)

Ark's architecture is meticulously separated into logical pipelines, taking a raw SQL query string and transforming it into executed data operations on disk:

1. **Lexical Analysis (`tokenizer.cpp`)**  
   The **Tokenizer** reads the raw `.ark` script file or query string character by character. It identifies boundaries and breaks the raw text into a manageable stream of **Tokens** (e.g., `KEYWORD_SELECT`, `IDENTIFIER`, `OPERATOR_EQUALS`, `STRING_LITERAL`).

2. **Abstract Parsing (`parser.cpp`)**  
   The **Parser** consumes the token stream to understand the structural grammar of the query. It validates the syntax against Ark's supported rules (e.g., ensuring `SELECT` is followed by columns and `FROM`). If valid, it translates the query into actionable operations.

3. **Core Execution Engine (`core.cpp`)**  
   This is the heavy lifter. The Core Engine contains the implementations for fundamental database structures like `Cell`, `Column`, `Row`, and `Table`. It executes the parsed instructions—filtering rows, checking conditions (like `WHERE`, `AND`, `OR`), matching patterns (`LIKE`), and manipulating the in-memory data tables.

4. **Database Management & Persistance (`databaseManager.cpp` & `databases/`)**  
   The Manager orchestrates multiple database environments, ensuring smooth switching (`USE database`) and table operations. Through the `SAVE` and `LOAD` commands, Ark serializes its in-memory tables and cell data, persisting them natively to disk within the `databases/` directory using its own custom binary/text representation.

---

## ✨ System Features

Ark continuously evolves, supporting a wide array of Database Management and Data Manipulation features:

### 🗄️ Database & Table Management
- **Database Control**: `CREATE DATABASE`, `DROP DATABASE`, `USE`, `SHOW DATABASES`.
- **Table Control**: `CREATE TABLE`, `DROP TABLE`, `SHOW TABLES`, `SHOW COLUMNS FROM <table>`.

### 📊 Supported Types
- `INT` | `DOUBLE` | `STRING` | `BOOL` | `NULL`

### 🛠️ Data Manipulation (DML)
- **Multi-row Insertions**: `INSERT INTO <table> VALUES (...), (...);`
- **Selective Retrieval**: `SELECT *` or `SELECT col1, col2 FROM <table>`.
- **Targeted Updates**: `UPDATE <table> SET col1 = val1 WHERE ...`
- **Deletions**: Delete all rows, or filter deletions with conditions and limits.

### 🔍 Advanced Querying & Filtering (`WHERE`)
Ark boasts a powerful conditional filtering engine:
- **Relational Operators**: `==`, `!=`, `<`, `>`, `<=`, `>=`.
- **Logical Chaining**: Combine multiple conditions natively using `AND` & `OR`.
- **Pattern Matching**: Find string patterns using `LIKE '<char>%'`.
- **Sorting Results**: Sort your queried data using `ORDER BY <col> [ASC | DESC]`.
- **Output Control**: Restrict output or mutation counts using `LIMIT <n>`.
- **Pretty Print**: Built-in formatted border tables for `SELECT` and `SHOW` commands, adjusting dynamically to content size.

> *Note: 🚨 An advanced, robust **Error Handling and Logging system** is currently beginning to be actively integrated into Ark! This highly-requested feature will provide clear, actionable query diagnostics and syntax error reporting. Graph-based database systems are also planned for future updates.*

---

## 🗂️ Supported SQL Syntax Guide

Here is a quick reference guide to Ark's custom dialect of SQL:

```sql
-- ════════ DATABASE COMMANDS ════════
CREATE DATABASE <name>;
DROP DATABASE <name>;
USE <database>;
SHOW DATABASES;

-- ════════ TABLE COMMANDS ═══════════
CREATE TABLE <name> (<col> <type>, ...);
DROP TABLE <name>;
SHOW TABLES;
SHOW COLUMNS FROM <table>;

-- ════════ DATA INSERTION ═══════════
INSERT INTO <table> VALUES (<val>, ...);
INSERT INTO <table> VALUES (<val>, ...), (<val>, ...);
INSERT INTO <table> (<col>, ...) VALUES (<val>, ...);

-- ════════ DATA QUERIES ═════════════
SELECT * FROM <table>;
SELECT <col1>, <col2> FROM <table> WHERE <col> == <val>;
SELECT * FROM <table> WHERE <col> == <val> AND <col2> != <val2>;
SELECT * FROM <table> WHERE <col> LIKE '<c>%';
SELECT * FROM <table> ORDER BY <col> ASC;
SELECT * FROM <table> WHERE <col> > <val> ORDER BY <col2> DESC LIMIT <n>;

-- ════════ DATA UPDATES ═════════════
UPDATE <table> SET <col> = <val> [, ...] WHERE <col> == <val> LIMIT <n>;
UPDATE <table> SET <col> = <val> WHERE <col> LIKE '<c>%';

-- ════════ DATA DELETION ════════════
DELETE FROM <table>;
DELETE FROM <table> LIMIT <n>;
DELETE FROM <table> WHERE <col> > <val> OR <col2> <= <val2>;

-- ════════ PERSISTENCE ══════════════
SAVE;
LOAD;
```

---

## 🏗️ Project Structure

```text
Ark/
├── databases/              # Saved database persistence files (.tbl and .dat)
├── include/                # Header files for all system components
│   ├── core.h              # Engine structures (Cell, Column, Row, Table)
│   ├── databaseManager.h   # Orchestrates system databases & persistence
│   ├── parser.h            # AST and query parsing structures
│   └── tokenizer.h         # Lexical analysis definitions
├── src/                    # Implementation source files
│   ├── core.cpp            # Core engine query execution logic
│   ├── databaseManager.cpp # DB state operations
│   ├── main.cpp            # Entry point / Script runner
│   ├── parser.cpp          # Parses tokens into systemic operations
│   └── tokenizer.cpp       # Tokenizes raw queries into lexical units
├── ArkTest.ark             # Example SQL script to test the engine
└── README.md               # Project documentation
```

---

## 🚀 Getting Started

### 1. Prerequisites
You need a working standard **C++ compiler** (e.g., `g++` or `clang++`).

### 2. Compilation
Compile the project by linking all source files from the `src/` directory and including the header files:

```bash
g++ -Iinclude src/*.cpp -o program.exe
```

### 3. Execution
Write your targeted Ark statements in a script file (e.g., `ArkTest.ark`). Run the compiled executable, passing the script file as an argument:

```bash
./program.exe ArkTest.ark
```

*Example Script (`ArkTest.ark`):*
```sql
CREATE DATABASE testdb;
USE testdb;

CREATE TABLE users (id INT, name STRING, age INT);

INSERT INTO users VALUES (1, "Alice", 25), (2, "Bob", 30), (3, "Charlie", 35);
DELETE FROM users WHERE age >= 30 AND id == 2;

SELECT * FROM users;
SAVE;
```

---

## 📄 License
This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.
