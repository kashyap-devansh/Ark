<div align="center">

```
     █████╗ ██████╗ ██╗  ██╗
    ██╔══██╗██╔══██╗██║ ██╔╝
    ███████║██████╔╝█████╔╝ 
    ██╔══██║██╔══██╗██╔═██╗ 
    ██║  ██║██║  ██║██║  ██╗
    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
```

### A SQL-Like Relational Database Engine, Built From Scratch in C++

[![C++17](https://img.shields.io/badge/C++-17-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-22c55e?style=flat-square)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Active-22c55e?style=flat-square)]()
[![Dependencies](https://img.shields.io/badge/Dependencies-Zero-f97316?style=flat-square)]()

**Ark** is a fully self-contained relational database engine and query language interpreter.  
It parses, validates, and executes a rich SQL-like query language against a typed, in-memory data model —  
with support for multi-database management, multi-table joins, aggregate functions, pattern matching, and custom disk persistence.

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

Ark is built **100% from scratch in C++** with zero external dependencies — no third-party parsing libraries, no embedded engines, no code generators. Every layer is a custom, hand-written implementation:

| Layer | Description |
|-------|-------------|
| **Lexer** | Hand-rolled character-level tokenizer producing a typed `Token` stream from raw `.ark` text |
| **Parser** | Recursive-descent parser that validates grammar and dispatches to per-command sub-parsers |
| **Engine** | Custom in-memory table structures with strict type validation, multi-condition filtering, sorting, aggregate computation, and multi-table joins |
| **Persistence** | Raw disk serialization using a custom quote-safe CSV format with auto-discovery on startup |
| **Diagnostics** | Three-tier exception hierarchy (`Syntax / Type / Runtime`) with exact line, column, and lexeme reporting |

---

## Architecture

A raw `.ark` script flows through a clean, linear pipeline from text to executed operations:

```
 .ark Script
      │
      ▼
 ┌──────────────┐
 │   main.cpp   │  Reads script · strips -- comments · splits on ';' · drives pipeline
 └──────┬───────┘
        │
        ▼
 ┌──────────────────┐
 │   Tokenizer      │  Raw text  ──►  typed Token stream
 │   tokenizer.cpp  │  (keywords, identifiers, operators, literals)
 └──────┬───────────┘
        │
        ▼
 ┌──────────────────┐
 │   Parser         │  Recursive-descent · validates grammar
 │   parser.cpp     │  dispatches to per-command sub-parsers
 └──────┬───────────┘
        │
        ├─────────────────────────────────────────┐
        ▼                                         ▼
 ┌──────────────────┐               ┌─────────────────────────┐
 │   Core Engine    │               │   Database Manager      │
 │   core.cpp       │               │   databaseManager.cpp   │
 │                  │               │                         │
 │  Cell · Row      │               │  Multi-DB lifecycle     │
 │  Column · Table  │               │  USE · CREATE · DROP    │
 │  Database        │               │  SAVE · LOAD            │
 └──────────────────┘               └─────────────────────────┘
        │
        ▼
 ┌──────────────────┐
 │   Error System   │  SyntaxException · TypeException · RuntimeException
 │   error.cpp      │  line / column / lexeme diagnostics
 └──────────────────┘
```

---

## Features

### 🗄️ Database Management

| Command | Description |
|---------|-------------|
| `CREATE DATABASE` | Creates a new database and registers it on disk |
| `DROP DATABASE` | Deletes the database directory and all its files |
| `USE` | Selects a database and loads it into memory |
| `SHOW DATABASES` | Lists all known databases |

### 🗃️ Table Management

| Command | Description |
|---------|-------------|
| `CREATE TABLE` | Creates a typed table; detects duplicate column names |
| `DROP TABLE` | Removes the table from memory and deletes its disk files |
| `TRUNCATE TABLE` | Clears all rows; preserves the schema |
| `RENAME TABLE ... TO` | Renames a table and its disk files atomically |
| `RENAME COLUMN ... TO ... FROM` | Renames a column and persists the updated schema |
| `SHOW TABLES` | Lists all tables in the active database |
| `SHOW COLUMNS FROM` | Displays column names and types in a formatted table |

### ✏️ Data Manipulation

- **Multi-row `INSERT`** — insert multiple rows in one statement; every row is validated for type and column count
- **`SELECT *` or column-selective** — choose specific columns or all at once
- **Column aliasing** — `SELECT col AS alias` renames headers in the output
- **`DISTINCT`** — deduplicates result rows across all selected columns
- **Multi-column `UPDATE`** — `SET col1 = v1, col2 = v2` in one statement; supports `WHERE` and `LIMIT`
- **`DELETE`** — targeted via `WHERE` or full-table; supports `LIMIT`
- **`NULL` values** — insertable, storable, and comparable across all types

### 🔗 Joins

All four join types are supported. The `ON` condition uses `table.column = table.column` dot-notation.

| Join Type | Behaviour |
|-----------|-----------|
| `INNER JOIN` | Only rows with a match in both tables |
| `LEFT JOIN` | All left rows; NULLs for unmatched right rows |
| `RIGHT JOIN` | All right rows; NULLs for unmatched left rows |
| `FULL JOIN` / `FULL OUTER JOIN` | All rows from both tables; NULLs where no match |

> **Note:** Joins support `SELECT *` and column-selective queries. `WHERE`, `ORDER BY`, and `LIMIT` are not composable with joins.

### 📊 Aggregate Functions

All aggregates support an optional `WHERE` clause for filtered computation.

| Function | Description |
|----------|-------------|
| `COUNT(*)` | Total number of rows |
| `COUNT(col)` | Count of non-NULL values in a column |
| `SUM(col)` | Sum of numeric values |
| `AVG(col)` | Average of numeric values |
| `MIN(col)` | Minimum value in a column |
| `MAX(col)` | Maximum value in a column |

### 🔍 Querying & Filtering

| Feature | Details |
|---------|---------|
| Comparison operators | `==`  `!=`  `<`  `>`  `<=`  `>=` |
| Logical chaining | `AND` / `OR` across multiple conditions in one `WHERE` clause |
| Pattern matching | `LIKE` on `STRING` columns — `%word%`, `A%`, `%z` |
| Sorting | `ORDER BY <col> ASC \| DESC` — composable with `WHERE` and `DISTINCT` |
| Row limiting | `LIMIT <n>` on `SELECT`, `UPDATE`, and `DELETE` |

### 💾 Persistence

- **`SAVE`** — serializes all tables (schema + data) to disk using a custom quote-safe CSV format
- **`LOAD`** — restores the full database from disk into memory
- **Auto-discovery** — on startup `DatabaseManager` scans `./databases/` and registers all previously created databases

---

## Data Types

| Type | Example Literals | C++ Storage |
|------|-----------------|-------------|
| `INT` | `42`, `-7` | `int` |
| `DOUBLE` | `3.14`, `-0.5` | `double` |
| `STRING` | `"hello world"` | `std::string` |
| `BOOL` | `TRUE`, `FALSE` | `bool` |
| `NULL` | `NULL` | null `Cell` |

---

## Error Diagnostics

Every error pinpoints the exact location and cause:

```
-----------------------------------------------------------
SYNTAX ERROR: Unexpected token
CODE: E-SYNTAX-UNEXPECTED_TOKEN
MESSAGE: Unexpected token 'FORM', expected 'FROM'.
LINE: 3, COLUMN: 15
-----------------------------------------------------------

-----------------------------------------------------------
RUNTIME ERROR: Column not found
CODE: E-RUNTIME-COLUMN_NOT_FOUND
MESSAGE: Column 'scroe' does not exist in Table 'users'.
LINE: 4, COLUMN: 22
-----------------------------------------------------------
```

**Full error code reference:**

<details>
<summary><strong>Syntax Errors</strong> — <code>E-SYNTAX-*</code></summary>

| Code | Trigger |
|------|---------|
| `UNEXPECTED_TOKEN` | Got token X, expected token Y |
| `UNRECOGNIZED_DATA_TYPE` | Unknown type keyword in `CREATE TABLE` |
| `UNRECOGNIZED_VALUE` | Invalid literal in a value position |
| `INVALID_LIKE_PATTERN` | `LIKE` pattern does not use `%` correctly |
| `EXPECTED_STRING_AFTER_LIKE` | Non-string token after `LIKE` |
| `EXPECTED_COMPARISON_OPERATOR` | Missing `==`, `!=`, etc. in `WHERE` |
| `UNKNOWN_CREATE_KEYWORD` | Unknown keyword after `CREATE` |
| `UNKNOWN_DROP_KEYWORD` | Unknown keyword after `DROP` |
| `UNKNOWN_SHOW_KEYWORD` | Unknown keyword after `SHOW` |
| `UNKNOWN_COMMAND` | Unrecognized statement start token |

</details>

<details>
<summary><strong>Type Errors</strong> — <code>E-TYPE-*</code></summary>

| Code | Trigger |
|------|---------|
| `LIMIT_NOT_INT` | Non-integer value after `LIMIT` |
| `NEGATIVE_LIMIT` | Negative integer after `LIMIT` |
| `LIKE_REQUIRES_STRING` | `LIKE` used on a non-`STRING` column |
| `INVALID_NUMERIC_LITERAL` | Malformed number token |

</details>

<details>
<summary><strong>Runtime Errors</strong> — <code>E-RUNTIME-*</code></summary>

| Code | Trigger |
|------|---------|
| `COLUMN_NOT_FOUND` | Referenced column does not exist in the table |
| `TABLE_NOT_FOUND` | Referenced table does not exist in the database |
| `TABLE_ALREADY_EXISTS` | `CREATE TABLE` on an existing name |
| `DUPLICATE_COLUMN_NAME` | Two columns with the same name in `CREATE TABLE` |
| `INSERT_TYPE_MISMATCH` | Value type does not match column type |
| `COLUMN_COUNT_MISMATCH` | Wrong number of values in `INSERT` |
| `NO_DATABASE_SELECTED` | DML command issued with no active `USE` |
| `NO_DATABASES` | `SHOW DATABASES` when none exist |
| `LIKE_PATTERN_TOO_SHORT` | `LIKE` pattern is fewer than 2 characters |
| `FILE_NOT_PROVIDED` | No `.ark` file passed as argument |
| `FILE_NOT_FOUND` | Specified `.ark` file does not exist |
| `INVALID_FILE_EXTENSION` | File argument does not end in `.ark` |

</details>

---

## Syntax Reference

All statements end with `;`. Line comments start with `--`.

```sql
-- ══════════════════════════════════════
--  DATABASE
-- ══════════════════════════════════════

CREATE DATABASE <n>;
DROP DATABASE <n>;
USE <n>;
SHOW DATABASES;


-- ══════════════════════════════════════
--  TABLES
-- ══════════════════════════════════════

CREATE TABLE <n> (<col> <type>, ...);
DROP TABLE <n>;
TRUNCATE TABLE <n>;
RENAME TABLE <n> TO <new_name>;
RENAME COLUMN <col> TO <new_col> FROM <table>;
SHOW TABLES;
SHOW COLUMNS FROM <table>;


-- ══════════════════════════════════════
--  INSERT
-- ══════════════════════════════════════

INSERT INTO <table> VALUES (<val>, ...);
INSERT INTO <table> VALUES (<val>, ...), (<val>, ...);   -- multi-row


-- ══════════════════════════════════════
--  SELECT
-- ══════════════════════════════════════

SELECT * FROM <table>;
SELECT <col1>, <col2> FROM <table>;
SELECT <col> AS <alias> FROM <table>;
SELECT DISTINCT <col> FROM <table>;

-- with WHERE
SELECT * FROM <table> WHERE <col> == <val>;
SELECT * FROM <table> WHERE <col> > <val> AND <col2> != <val2>;
SELECT * FROM <table> WHERE <col> LIKE "%word%";

-- with ORDER BY
SELECT * FROM <table> ORDER BY <col> ASC;
SELECT * FROM <table> ORDER BY <col> DESC;

-- combined
SELECT DISTINCT <col> AS <alias>
    FROM <table>
    WHERE <col2> > <val>
    ORDER BY <col> ASC
    LIMIT <n>;


-- ══════════════════════════════════════
--  JOINS
-- ══════════════════════════════════════

-- Supports SELECT * and column-selective queries.
-- WHERE / ORDER BY / LIMIT are not supported after a JOIN.

SELECT * FROM <t1> INNER JOIN      <t2> ON <t1>.<col> = <t2>.<col>;
SELECT * FROM <t1> LEFT JOIN       <t2> ON <t1>.<col> = <t2>.<col>;
SELECT * FROM <t1> RIGHT JOIN      <t2> ON <t1>.<col> = <t2>.<col>;
SELECT * FROM <t1> FULL JOIN       <t2> ON <t1>.<col> = <t2>.<col>;
SELECT * FROM <t1> FULL OUTER JOIN <t2> ON <t1>.<col> = <t2>.<col>;

SELECT <col1>, <col2> FROM <t1> INNER JOIN <t2> ON <t1>.<col> = <t2>.<col>;


-- ══════════════════════════════════════
--  AGGREGATES
-- ══════════════════════════════════════

COUNT(*)     FROM <table>;
COUNT(<col>) FROM <table>;
SUM(<col>)   FROM <table>;
AVG(<col>)   FROM <table>;
MIN(<col>)   FROM <table>;
MAX(<col>)   FROM <table>;

-- all aggregates accept an optional WHERE clause:
COUNT(*) FROM <table> WHERE <col> == <val>;
AVG(<col>) FROM <table> WHERE <col2> > <val>;


-- ══════════════════════════════════════
--  UPDATE
-- ══════════════════════════════════════

UPDATE <table> SET <col> = <val>;
UPDATE <table> SET <col> = <val>, <col2> = <val2>;
UPDATE <table> SET <col> = <val> WHERE <col2> == <val2>;
UPDATE <table> SET <col> = <val> WHERE <col2> > <val2> LIMIT <n>;


-- ══════════════════════════════════════
--  DELETE
-- ══════════════════════════════════════

DELETE FROM <table>;
DELETE FROM <table> WHERE <col> == <val>;
DELETE FROM <table> WHERE <col> > <val> OR <col2> <= <val2>;
DELETE FROM <table> WHERE <col> == <val> LIMIT <n>;


-- ══════════════════════════════════════
--  PERSISTENCE
-- ══════════════════════════════════════

SAVE;
LOAD;
```

---

## Project Structure

```
Ark/
├── databases/                    # Auto-generated at runtime
│   └── <dbname>/
│       ├── tables/               # .tbl — column name/type schema per table
│       ├── data/                 # .dat — quote-safe CSV row data per table
│       └── tables.meta           # Registry of table names for this database
│
├── include/
│   ├── core.h                    # Cell, Column, Row, Table, Database
│   ├── databaseManager.h         # DatabaseManager — multi-database orchestration
│   ├── error.h                   # ArkException hierarchy (Syntax / Type / Runtime)
│   ├── helper.h                  # Condition evaluation, column lookup, formatted output
│   ├── parser.h                  # Parser class and Condition struct
│   └── tokenizer.h               # Token, Tokenizer, TokenType, keyword table
│
├── src/
│   ├── core.cpp                  # Cell, Row, Column, Table, Database implementations
│   ├── databaseManager.cpp       # DB lifecycle, persistence, auto-discovery
│   ├── error.cpp                 # Exception formatting with diagnostics
│   ├── helper.cpp                # Utilities: evaluation, lookup, table printing
│   ├── main.cpp                  # Entry point — script reader and dispatcher
│   ├── parser.cpp                # Full recursive-descent parser
│   └── tokenizer.cpp             # Character-level lexer
│
├── ArkTest.ark                   # Example script
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

### Run

```bash
./ark ArkTest.ark
```

### Example Script

```sql
-- ArkTest.ark

CREATE DATABASE school;
USE school;

CREATE TABLE students (id INT, name STRING, grade DOUBLE, enrolled BOOL);
CREATE TABLE clubs    (student_id INT, club STRING);

INSERT INTO students VALUES
    (1, "Alice",   95.5, TRUE),
    (2, "Bob",     74.0, TRUE),
    (3, "Charlie", 58.2, FALSE),
    (4, "Diana",   88.7, TRUE);

INSERT INTO clubs VALUES
    (1, "Chess"), (2, "Chess"), (4, "Drama");

-- Sort all students by grade
SELECT * FROM students ORDER BY grade DESC;

-- Remove failing students
DELETE FROM students WHERE grade < 60.0;

-- Aliased output with filtering
SELECT name AS student_name, grade
    FROM students
    WHERE enrolled == TRUE
    ORDER BY grade DESC;

-- Join students with their clubs
SELECT * FROM students INNER JOIN clubs ON students.id = clubs.student_id;

-- Aggregates
COUNT(*) FROM students;
AVG(grade) FROM students;

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

+----+-------+----------+----------+------------+-------+
| id | name  | grade    | enrolled | student_id | club  |
+----+-------+----------+----------+------------+-------+
| 1  | Alice | 95.5     | true     | 1          | Chess |
| 4  | Diana | 88.7     | true     | 4          | Drama |
+----+-------+----------+----------+------------+-------+

+----------+
| COUNT(*) |
+----------+
| 3        |
+----------+

+------------+
| AVG(grade) |
+------------+
| 86.066667  |
+------------+

Database saved.
```

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.