<div align="center">

<pre>
     █████╗ ██████╗ ██╗  ██╗
    ██╔══██╗██╔══██╗██║ ██╔╝
    ███████║██████╔╝█████╔╝ 
    ██╔══██║██╔══██╗██╔═██╗ 
    ██║  ██║██║  ██║██║  ██╗
    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
</pre>
<h3>A SQL-Like Relational Database Engine, Built From Scratch in C++</h3>

<p>
  <img src="https://img.shields.io/badge/C++-17-00599C?style=flat-square&logo=c%2B%2B&logoColor=white" alt="C++17" />
  <img src="https://img.shields.io/badge/License-MIT-22c55e?style=flat-square" alt="MIT License" />
  <img src="https://img.shields.io/badge/Status-Active-22c55e?style=flat-square" alt="Active" />
  <img src="https://img.shields.io/badge/Dependencies-Zero-f97316?style=flat-square" alt="Zero Dependencies" />
</p>

<p>
  <strong>Ark</strong> is a fully self-contained relational database engine and query language interpreter.<br/>
  It parses, validates, and executes a rich SQL-like query language against a typed, in-memory data model —<br/>
  with support for multi-database management, aggregate functions, pattern matching, and custom disk persistence.
</p>

</div>

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Features](#features)
- [Data Types](#data-types)
- [Error Diagnostics](#error-diagnostics)
- [Syntax Reference](#syntax-reference)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [License](#license)

---

## Overview

Ark is built **100% from scratch in C++** — no third-party parsing libraries, no embedded engines, no code generators. Every layer of the system is a custom, hand-written implementation:

- **Hand-rolled Lexer & Parser** — A fully custom tokenizer and recursive-descent parser process every query from raw characters to execution-ready form.
- **Native Execution Engine** — Custom in-memory table structures with strict type validation, multi-condition filtering, pattern matching, sorting, and aggregate computation.
- **Raw Disk Persistence** — Ark serializes and deserializes its own table structures and row data directly to disk using a custom file format with quote-safe CSV encoding.
- **Structured Error Diagnostics** — A three-tier exception hierarchy (`Syntax / Type / Runtime`) reports the exact line, column, faulty lexeme, and a descriptive message for every error.
- **Composition over Inheritance** — Ark's design rigorously favors composition, ensuring loose coupling, high modularity, and clean separation of concerns across all components.
- **Zero External Dependencies** — No SQLite, no Bison, no Flex. Not a single third-party parsing or database library.

---

## Architecture

A raw `.ark` script is transformed into executed data operations through a clean, linear pipeline:

```
 .ark Script
      │
      ▼
 ┌──────────────┐
 │   main.cpp   │  Reads script, strips comments, splits on ';', drives pipeline
 └──────┬───────┘
        │
        ▼
 ┌──────────────────┐
 │   Tokenizer      │  Lexes raw text → typed Token stream
 │   tokenizer.cpp  │  (keywords, identifiers, operators, literals)
 └──────┬───────────┘
        │
        ▼
 ┌──────────────────┐
 │   Parser         │  Recursive-descent parsing; validates grammar;
 │   parser.cpp     │  dispatches to per-command sub-parsers
 └──────┬───────────┘
        │
        ├─────────────────────────────────────────┐
        ▼                                         ▼
 ┌──────────────────┐               ┌─────────────────────────┐
 │   Core Engine    │               │   Database Manager      │
 │   core.cpp       │               │   databaseManager.cpp   │
 │                  │               │                         │
 │  Cell, Row,      │               │  Manages multiple DBs;  │
 │  Column, Table,  │               │  handles USE, CREATE,   │
 │  Database        │               │  DROP, SAVE, LOAD       │
 └──────────────────┘               └─────────────────────────┘
        │
        ▼
 ┌──────────────────┐
 │   Error System   │  SyntaxException, TypeException, RuntimeException
 │   error.cpp      │  with line / column / lexeme diagnostics
 └──────────────────┘
```

---

## Features

### Database & Table Management
- `CREATE DATABASE` / `DROP DATABASE` / `USE` / `SHOW DATABASES`
- `CREATE TABLE` with typed columns and duplicate-name detection
- `DROP TABLE` — removes both in-memory state and disk files immediately
- `TRUNCATE TABLE` — clears all rows while keeping the table structure intact
- `RENAME TABLE` — renames a table and its underlying files atomically
- `RENAME COLUMN ... TO ... FROM` — renames a column and persists the updated structure
- `SHOW TABLES` / `SHOW COLUMNS FROM <table>`

### Data Manipulation (DML)
- **Multi-row insertions** with strict per-row type and column-count validation
- **Column-selective queries** — `SELECT col1, col2` or `SELECT *`
- **Column aliasing** — `SELECT col AS alias` renames output headers inline
- **Distinct filtering** — `SELECT DISTINCT` deduplicates result rows
- **Multi-column updates** — `UPDATE ... SET col1 = v1, col2 = v2 ...`
- **Targeted or full-table deletions**

### Aggregate Functions
- `COUNT(*)` / `COUNT(col)` — total rows or non-NULL values in a column
- `SUM(col)` — sum of numeric values
- `AVG(col)` — average of numeric values
- `MIN(col)` / `MAX(col)` — minimum and maximum value in a column
- All aggregates support an optional `WHERE` clause for filtered computation

### Advanced Querying & Filtering
- **Comparison operators** — `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical chaining** — `AND` / `OR` across multiple conditions in a single `WHERE` clause
- **Pattern matching** — `LIKE` on `STRING` columns with `%word%`, `A%`, `%z` patterns
- **Sorting** — `ORDER BY <col> ASC | DESC`, fully composable with `WHERE` and `DISTINCT`
- **Output control** — `LIMIT <n>` on `SELECT`, `UPDATE`, and `DELETE`

### File-Backed Persistence
- `SAVE` — serializes all tables (structure + row data) to disk
- `LOAD` — restores a full database from disk back into memory
- String values are quote-wrapped on save and stripped on load — comma-safe for all string content
- On startup, `DatabaseManager` auto-discovers all previously created databases under `./databases/`

---

## Data Types

| Type | Literals | C++ Storage |
|------|----------|-------------|
| `INT` | `42`, `-7` | `int` |
| `DOUBLE` | `3.14`, `-0.5` | `double` |
| `STRING` | `"hello world"` | `std::string` |
| `BOOL` | `TRUE`, `FALSE` | `bool` |
| `NULL` | `NULL` | null `Cell` |

---

## Error Diagnostics

Every error includes an error code, a plain-English description, and the exact `LINE` / `COLUMN` of the offending token:

```
-----------------------------------------------------------
RUNTIME ERROR: Column not found
CODE: E-RUNTIME-COLUMN_NOT_FOUND
MESSAGE: Column 'scroe' does not exist in Table 'users'.
LINE: 4, COLUMN: 22
-----------------------------------------------------------
```

**Error categories:**

| Category | Prefix | Error Codes |
|----------|--------|-------------|
| Syntax | `E-SYNTAX-` | `UNEXPECTED_TOKEN`, `UNRECOGNIZED_DATA_TYPE`, `UNRECOGNIZED_VALUE`, `INVALID_LIKE_PATTERN`, `EXPECTED_STRING_AFTER_LIKE`, `EXPECTED_COMPARISON_OPERATOR`, `UNKNOWN_CREATE_KEYWORD`, `UNKNOWN_DROP_KEYWORD`, `UNKNOWN_SHOW_KEYWORD`, `UNKNOWN_COMMAND` |
| Type | `E-TYPE-` | `LIMIT_NOT_INT`, `NEGATIVE_LIMIT`, `LIKE_REQUIRES_STRING`, `INVALID_NUMERIC_LITERAL` |
| Runtime | `E-RUNTIME-` | `COLUMN_NOT_FOUND`, `TABLE_NOT_FOUND`, `TABLE_ALREADY_EXISTS`, `DUPLICATE_COLUMN_NAME`, `INSERT_TYPE_MISMATCH`, `COLUMN_COUNT_MISMATCH`, `NO_DATABASE_SELECTED`, `NO_DATABASES`, `LIKE_PATTERN_TOO_SHORT`, `FILE_NOT_PROVIDED`, `FILE_NOT_FOUND` |

---

## Syntax Reference

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
TRUNCATE TABLE <name>;
RENAME TABLE <name> TO <new_name>;
RENAME COLUMN <col> TO <new_col> FROM <table>;
SHOW TABLES;
SHOW COLUMNS FROM <table>;

-- ════════ DATA INSERTION ═══════════
INSERT INTO <table> VALUES (<val>, ...);
INSERT INTO <table> VALUES (<val>, ...), (<val>, ...);

-- ════════ DATA QUERIES ═════════════
SELECT * FROM <table>;
SELECT <col1>, <col2> FROM <table>;
SELECT <col> AS <alias> FROM <table>;
SELECT DISTINCT <col> FROM <table>;
SELECT * FROM <table> WHERE <col> == <val>;
SELECT * FROM <table> WHERE <col> > <val> AND <col2> != <val2>;
SELECT * FROM <table> WHERE <col> LIKE "%word%";
SELECT * FROM <table> ORDER BY <col> ASC;
SELECT * FROM <table> WHERE <col> > <val> ORDER BY <col2> DESC;
SELECT DISTINCT <col> FROM <table> ORDER BY <col> ASC;

-- ════════ AGGREGATE FUNCTIONS ══════
COUNT(*) FROM <table>;
COUNT(<col>) FROM <table> WHERE <col2> == <val>;
SUM(<col>) FROM <table>;
AVG(<col>) FROM <table> WHERE <col2> > <val>;
MIN(<col>) FROM <table>;
MAX(<col>) FROM <table>;

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

## Project Structure

```
Ark/
├── databases/                   # Auto-generated directory for persisted database files
│   └── <dbname>/
│       ├── tables/              # .tbl files — column name/type definitions per table
│       ├── data/                # .dat files — quote-safe CSV row data per table
│       └── tables.meta          # Registry of table names for this database
│
├── include/                     # Header files
│   ├── core.h                   # Cell, Column, Row, Table, Database — core data model
│   ├── databaseManager.h        # DatabaseManager — multi-database orchestration
│   ├── error.h                  # ArkException hierarchy (Syntax / Type / Runtime)
│   ├── helper.h                 # Utilities: condition evaluation, table printing
│   ├── parser.h                 # Parser class and Condition struct
│   └── tokenizer.h              # Token, Tokenizer, TokenType, Keyword definitions
│
├── src/                         # Implementation files
│   ├── core.cpp                 # Core data model: Cell, Row, Column, Table, Database
│   ├── databaseManager.cpp      # Database lifecycle, persistence, and auto-discovery
│   ├── error.cpp                # Exception formatting with line/column diagnostics
│   ├── helper.cpp               # Condition evaluation, column lookup, formatted output
│   ├── main.cpp                 # Entry point — script reader and statement dispatcher
│   ├── parser.cpp               # Full recursive-descent parser for all Ark statements
│   └── tokenizer.cpp            # Character-level lexer with full keyword table
│
├── ArkTest.ark                  # Example script for testing and demonstration
└── README.md
```

---

## Getting Started

### Prerequisites

A C++17-compatible compiler: `g++` (GCC 8+) or `clang++` (Clang 7+).

### Compile

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o ark
```

### Run a Script

Write your Ark statements in a `.ark` file and pass it as an argument:

```bash
./ark ArkTest.ark
```

### Example

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

SELECT * FROM students ORDER BY grade DESC;

DELETE FROM students WHERE grade < 60.0;

SELECT name AS student_name, grade FROM students WHERE enrolled == TRUE ORDER BY grade DESC;

COUNT(*) FROM students;

SAVE;
```

**Output:**

```
+----+---------+----------+----------+
| id | name    | grade    | enrolled |
+----+---------+----------+----------+
| 1  | Alice   | 95.5     | true     |
| 4  | Diana   | 88.7     | true     |
| 2  | Bob     | 74.0     | true     |
| 3  | Charlie | 58.2     | false    |
+----+---------+----------+----------+

+--------------+----------+
| student_name | grade    |
+--------------+----------+
| Alice        | 95.5     |
| Diana        | 88.7     |
| Bob          | 74.0     |
+--------------+----------+

+----------+
| COUNT(*) |
+----------+
| 3        |
+----------+

Database saved.
```

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.