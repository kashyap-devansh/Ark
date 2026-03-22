<div align="center">
  <h1>🐘 Ark SQL-like Database Engine</h1>
  <p>
    <strong>A high-performance, custom, lightweight database engine and SQL-like query language interpreter built entirely sequentially from scratch in C++.</strong>
  </p>
  <p>
    <img src="https://img.shields.io/badge/C++-17+-blue.svg?logo=c%2B%2B" alt="C++" />
    <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License" />
    <img src="https://img.shields.io/badge/Status-Active-brightgreen.svg" alt="Status" />
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
   The **Tokenizer** reads the raw `.ark` script file or query string character by character. It identifies boundaries and breaks the raw text into a manageable stream of **Tokens** (e.g., `SELECT`, `IDENTIFIER`, `==`, `STRING_LITERAL`).

2. **Abstract Parsing (`parser.cpp`)**  
   The **Parser** consumes the token stream to understand the structural grammar of the query. It validates the syntax against Ark's supported rules. If valid, it translates the query into actionable operations.

3. **Core Execution Engine (`core.cpp`)**  
   The Core Engine contains the implementations for fundamental database structures like `Cell`, `Column`, `Row`, and `Table`. It executes the parsed instructions—filtering rows, checking conditions (like `WHERE`, `AND`, `OR`), matching patterns (`LIKE`), and manipulating the in-memory data tables.

4. **Database Management & Persistence (`databaseManager.cpp` & `databases/`)**  
   The Manager orchestrates multiple database environments, ensuring smooth switching (`USE`) and table operations. Through the `SAVE` and `LOAD` commands, Ark serializes its in-memory tables and cell data, persisting them natively to disk within the `databases/` directory using its own custom binary/text representation.

5. **Diagnostic Error System (`error.cpp`)**  
   Catches syntactical, type-based, and runtime exceptions. Extracts precise query locations (`LINE`, `COLUMN`), faulty lexemes, and recommended fixes, ensuring users receive actionable feedback instantly.

---

## ✨ System Features

Ark currently supports a robust array of Database Management and Data Manipulation features tightly integrated into its core:

### 🗄️ Database & Table Management
- **Database Control**: `CREATE DATABASE`, `DROP DATABASE`, `USE`, `SHOW DATABASES`.
- **Table Control**: `CREATE TABLE`, `DROP TABLE`, `SHOW TABLES`, `SHOW COLUMNS FROM <table>`.

### 📊 Supported Types
Ark natively validates and stores the following primitives:
- `INT`
- `DOUBLE`
- `STRING`
- `BOOL` (supports `TRUE`, `FALSE`)
- `NULL`

### 🛠️ Data Manipulation (DML)
- **Multi-row Insertions**: `INSERT INTO <table> VALUES (...), (...);`
- **Selective Retrieval**: `SELECT * FROM <table>` or `SELECT col1, col2 FROM <table>`.
- **Targeted Updates**: `UPDATE <table> SET col = val WHERE ...`
- **Deletions**: Delete all rows, or filter deletions with conditions and limits.

### 🔍 Advanced Querying & Filtering (`WHERE`)
Ark boasts a powerful conditional filtering engine:
- **Relational Operators**: `==`, `!=`, `<`, `>`, `<=`, `>=`.
- **Logical Chaining**: Combine multiple conditions natively using `AND`, `OR`, & `NOT`.
- **Pattern Matching**: Find string patterns using `LIKE '<char>%'`.
- **Sorting Results**: Sort queried data using `ORDER BY <col> ASC` or `DESC`.
- **Output Control**: Restrict output or mutation counts using `LIMIT <n>`.

### 🚨 Advanced Error Handling & Diagnostics
Ark includes a comprehensive, built-in exception engine providing descriptive, actionable feedback:
- **Syntax Diagnostics**: Real-time checking for unexpected tokens or unknown commands. (e.g., `E-SYNTAX-UNEXPECTED_TOKEN`)
- **Type Safety**: Strictly validates type mismatches, such as string constraints on `LIKE` operators or preventing negative `LIMIT` arguments. (e.g., `E-TYPE-LIKE_REQUIRES_STRING`)
- **Runtime Defenses**: Prevents operations on missing tables or columns, and validates `INSERT` column-count parity entirely during query execution. (e.g., `E-RUNTIME-COLUMN_NOT_FOUND`)

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
├── databases/              # Saved database persistence files
├── include/                # Header files for all system components
│   ├── core.h              # Engine structures (Cell, Column, Row, Table)
│   ├── databaseManager.h   # Orchestrates system databases & persistence
│   ├── error.h             # Exception definitions (Syntax/Type/Runtime)
│   ├── helper.h            # Utility functions for parsing
│   ├── parser.h            # AST and query parsing structures
│   └── tokenizer.h         # Lexical analysis definitions
├── src/                    # Implementation source files
│   ├── core.cpp            # Core engine query execution logic
│   ├── databaseManager.cpp # DB state operations
│   ├── error.cpp           # Detailed error reporting implementations
│   ├── helper.cpp          # System utilities
│   ├── main.cpp            # Entry point / Script runner
│   ├── parser.cpp          # Parses tokens into systemic operations
│   └── tokenizer.cpp       # Tokenizes raw queries into lexical units
├── ArkTest.ark             # Example SQL script to test the engine
└── README.md               # Project documentation
```

---

## 🚀 Getting Started

### 1. Prerequisites
You need a working standard **C++17 compiler** (e.g., `g++` or `clang++`).

### 2. Compilation
Compile the project by linking all source files from the `src/` directory and including the header files:

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o program.exe
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

CREATE TABLE users (id INT, name STRING, score DOUBLE, active BOOL);

INSERT INTO users VALUES 
    (1, "Alice", 95.5, true), 
    (2, "Bob", 80.0, false), 
    (3, "Charlie", 72.3, true);

DELETE FROM users WHERE score <= 80.0 AND id == 2;

SELECT * FROM users ORDER BY id DESC;
SAVE;
```

---

## 📄 License
This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.
