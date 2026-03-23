<div align="center">
  <h1>🐘 Ark — SQL-like Database Engine</h1>
  <p>
    <strong>A high-performance, lightweight relational database engine and SQL-like query language interpreter built entirely from scratch in C++.</strong>
  </p>
  <p>
    <img src="https://img.shields.io/badge/C++-17+-blue.svg?logo=c%2B%2B" alt="C++" />
    <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License" />
    <img src="https://img.shields.io/badge/Status-Active-brightgreen.svg" alt="Status" />
    <img src="https://img.shields.io/badge/Dependencies-None-orange.svg" alt="Zero Dependencies" />
  </p>
</div>

---

## 📖 Overview

**Ark** is a fully self-contained relational database engine and query language interpreter. It parses, validates, and executes a rich subset of SQL-like statements against a typed, in-memory data model — with full support for saving and loading databases to and from disk.

### 🛠️ Built 100% From Scratch in C++

Unlike projects that wrap existing engines or rely on parser generators, **every layer of Ark is a custom, hand-written C++ implementation**:

- **Zero External Dependencies** — No SQLite, no Bison, no Flex. Not a single third-party parsing or database library.
- **Hand-rolled Lexer & Parser** — A fully custom tokenizer and recursive-descent parser process every query from raw characters to execution.
- **Native Execution Engine** — Custom in-memory table structures with strict type validation, multi-condition filtering, pattern matching, and sorting.
- **Raw Disk Persistence** — Ark serializes and deserializes its own table structures and row data directly to disk using a custom format.
- **Structured Error Diagnostics** — A three-tier exception hierarchy (Syntax / Type / Runtime) reports the exact line, column, faulty lexeme, and a descriptive message for every error.
- **Composition over Inheritance** — Ark's object-oriented design rigorously favors composition, ensuring loose coupling, high modularity, and clean separation of concerns across all components.

---

## ⚙️ Architecture

Ark's pipeline is cleanly separated into discrete stages. A raw `.ark` script is transformed into executed data operations through the following chain:

```
 .ark Script File
       │
       ▼
  ┌─────────────┐
  │  main.cpp   │  Reads script, strips -- comments, splits on ';', drives the parser
  └──────┬──────┘
         │
         ▼
  ┌──────────────────┐
  │  Tokenizer       │  Lexes raw text into a typed Token stream
  │  tokenizer.cpp   │  (keywords, identifiers, operators, literals)
  └──────┬───────────┘
         │
         ▼
  ┌──────────────────┐
  │  Parser          │  Recursive-descent parsing; validates grammar;
  │  parser.cpp      │  dispatches to per-command sub-parsers
  └──────┬───────────┘
         │
         ├───────────────────────────────────────────────┐
         ▼                                               ▼
  ┌──────────────────┐                    ┌──────────────────────────┐
  │  Core Engine     │                    │  Database Manager         │
  │  core.cpp        │                    │  databaseManager.cpp      │
  │                  │                    │                           │
  │  Cell, Row,      │                    │  Manages multiple DBs;    │
  │  Column, Table,  │                    │  handles USE, CREATE,     │
  │  Database        │                    │  DROP, SAVE, LOAD         │
  └──────────────────┘                    └──────────────────────────┘
         │
         ▼
  ┌──────────────────┐
  │  Error System    │  SyntaxException, TypeException, RuntimeException
  │  error.cpp       │  with line / column / lexeme diagnostics
  └──────────────────┘
```

---

## ✨ Features

### 🗄️ Database & Table Management
- `CREATE DATABASE` / `DROP DATABASE` / `USE` / `SHOW DATABASES`
- `CREATE TABLE` with typed columns and duplicate-name detection
- `DROP TABLE` — removes both in-memory state and disk files immediately
- `SHOW TABLES` / `SHOW COLUMNS FROM <table>`

### 📊 Native Data Types

| Type | Literals | C++ Storage |
|---|---|---|
| `INT` | `42`, `-7` | `int` |
| `DOUBLE` | `3.14`, `-0.5` | `double` |
| `STRING` | `"hello world"` | `std::string` |
| `BOOL` | `TRUE`, `FALSE` | `bool` |
| `NULL` | `NULL` | null `Cell` |

### 🛠️ Data Manipulation (DML)
- **Multi-row insertions** with strict per-row type and column-count validation
- **Column-selective queries** — `SELECT col1, col2` or `SELECT *`
- **Multi-column updates** — `UPDATE ... SET col1 = v1, col2 = v2 ...`
- **Targeted or full-table deletions**

### 🔍 Advanced Querying & Filtering
- **Comparison operators**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical chaining**: `AND` / `OR` across multiple conditions in a single `WHERE` clause
- **Pattern matching**: `LIKE` on `STRING` columns — supports `%word%`, `A%`, `%z`
- **Sorting**: `ORDER BY <col> ASC | DESC` — fully composable with `WHERE`
- **Output control**: `LIMIT <n>` on `UPDATE` and `DELETE`

### 💾 File-backed Persistence
- `SAVE` — serializes all tables (structure + row data) to disk
- `LOAD` — restores a full database from disk back into memory
- On startup, `DatabaseManager` auto-discovers all previously created databases under `./databases/`

### 🚨 Structured Error Diagnostics

Every error includes an error code, a plain-English message, and the exact `LINE` / `COLUMN` of the offending token:

```
-----------------------------------------------------------
RUNTIME ERROR: Column not found
CODE: E-RUNTIME-COLUMN_NOT_FOUND
MESSAGE: Column 'scroe' does not exist in Table 'users'.
LINE: 4, COLUMN: 22
-----------------------------------------------------------
```

| Category | Prefix | Error Codes |
|---|---|---|
| Syntax | `E-SYNTAX-` | `UNEXPECTED_TOKEN`, `UNRECOGNIZED_DATA_TYPE`, `INVALID_LIKE_PATTERN`, `EXPECTED_STRING_AFTER_LIKE`, `EXPECTED_COMPARISON_OPERATOR`, `UNKNOWN_COMMAND` |
| Type | `E-TYPE-` | `LIMIT_NOT_INT`, `NEGATIVE_LIMIT`, `LIKE_REQUIRES_STRING`, `INVALID_NUMERIC_LITERAL` |
| Runtime | `E-RUNTIME-` | `COLUMN_NOT_FOUND`, `TABLE_NOT_FOUND`, `INSERT_TYPE_MISMATCH`, `COLUMN_COUNT_MISMATCH`, `TABLE_ALREADY_EXISTS`, `DUPLICATE_COLUMN_NAME`, `NO_DATABASE_SELECTED`, `NO_DATABASES` |

---

## 🗂️ Syntax Reference

All statements must end with `;`. Line comments use `--`.

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

-- ════════ DATA QUERIES ═════════════
SELECT * FROM <table>;
SELECT <col1>, <col2> FROM <table>;
SELECT * FROM <table> WHERE <col> == <val>;
SELECT * FROM <table> WHERE <col> > <val> AND <col2> != <val2>;
SELECT * FROM <table> WHERE <col> LIKE "%word%";
SELECT * FROM <table> ORDER BY <col> ASC;
SELECT * FROM <table> WHERE <col> > <val> ORDER BY <col2> DESC;

-- ════════ DATA UPDATES ═════════════
UPDATE <table> SET <col> = <val>;
UPDATE <table> SET <col> = <val>, <col2> = <val2> WHERE <col> == <val>;
UPDATE <table> SET <col> = <val> WHERE <col2> > <val2> LIMIT <n>;

-- ════════ DATA DELETION ════════════
DELETE FROM <table>;
DELETE FROM <table> WHERE <col> == <val>;
DELETE FROM <table> WHERE <col> > <val> OR <col2> <= <val2> LIMIT <n>;

-- ════════ PERSISTENCE ══════════════
SAVE;
LOAD;
```

---

## 🏗️ Project Structure

```text
Ark/
├── databases/                  # Auto-generated directory for persisted database files
│   └── <dbname>/
│       ├── tables/             # .tbl files — column name/type definitions per table
│       ├── data/               # .dat files — CSV row data per table
│       └── tables.meta         # Registry of table names for this database
│
├── include/                    # Header files for all system components
│   ├── core.h                  # Cell, Column, Row, Table, Database — core data model
│   ├── databaseManager.h       # DatabaseManager — multi-database orchestration
│   ├── error.h                 # ArkException hierarchy (Syntax / Type / Runtime)
│   ├── helper.h                # Utility functions: condition evaluation, table printing
│   ├── parser.h                # Parser class and Condition struct
│   └── tokenizer.h             # Token, Tokenizer, TokenType, Keyword definitions
│
├── src/                        # Implementation source files
│   ├── core.cpp                # Core data model: Cell, Row, Column, Table, Database
│   ├── databaseManager.cpp     # Database lifecycle, persistence, and auto-discovery
│   ├── error.cpp               # Exception formatting with line/column diagnostics
│   ├── helper.cpp              # Condition evaluation, column lookup, formatted output
│   ├── main.cpp                # Entry point — script reader and statement dispatcher
│   ├── parser.cpp              # Full recursive-descent parser for all Ark statements
│   └── tokenizer.cpp           # Character-level lexer with full keyword table
│
├── ArkTest.ark                 # Example Ark script for testing and demonstration
└── README.md                   # Project documentation
```

---

## 🚀 Getting Started

### Prerequisites
A C++17-compatible compiler: `g++` (GCC 8+) or `clang++` (Clang 7+).

### Compilation

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o ark
```

### Running a Script

Write your Ark statements in a `.ark` file and pass it as an argument:

```bash
./ark ArkTest.ark
```

### Example Script

```sql
-- ArkTest.ark

CREATE DATABASE school;
USE school;

CREATE TABLE students (id INT, name STRING, grade DOUBLE, enrolled BOOL);

INSERT INTO students VALUES
    (1, "Alice",   95.5, TRUE),
    (2, "Bob",     74.0, TRUE),
    (3, "Charlie", 58.2, FALSE),
    (4, "Diana",   88.7, TRUE);

-- Show all students sorted by grade descending
SELECT * FROM students ORDER BY grade DESC;

-- Remove failing students
DELETE FROM students WHERE grade < 60.0;

-- Top enrolled students
SELECT name, grade FROM students WHERE enrolled == TRUE ORDER BY grade DESC;

SAVE;
```

**Output (example):**
```
+----+---------+--------+----------+
| id | name    | grade  | enrolled |
+----+---------+--------+----------+
| 1  | Alice   | 95.5   | true     |
| 4  | Diana   | 88.7   | true     |
| 2  | Bob     | 74.0   | true     |
| 3  | Charlie | 58.2   | false    |
+----+---------+--------+----------+
Database saved.
```

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.