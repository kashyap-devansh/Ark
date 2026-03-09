# Ark

Ark is a custom, lightweight database engine and SQL-like query language interpreter implemented in C++. 

> **Note:** 🚧 This project is currently **under development** and is actively being worked on. Features and syntax may be subject to change.

## Overview

Ark provides relational database capabilities, allowing users to parse and execute basic SQL-like statements. The project is built from scratch and includes its own custom tokenizer, expression parser, and core execution engine. It handles custom data storage and memory management to simulate a functioning database environment.

## Current Features

The following features have been implemented and are currently supported:

- **Database Management**: Create, drop, and switch between databases (`CREATE DATABASE`, `DROP DATABASE`, `USE`).
- **Table Operations**: Define and drop tables (`CREATE TABLE`, `DROP TABLE`).
- **Data Types**: Support for `INT`, `DOUBLE`, `STRING`, and `BOOL` data types.
- **Data Manipulation (DML)**: 
  - Insert records (`INSERT INTO`).
  - Retrieve basic data (`SELECT * FROM`, `SELECT col1, col2 FROM`).
  - Update specific records (`UPDATE`).
  - Delete records (`DELETE FROM`).
- **Navigation & Inspection**: See available databases, tables, and column structures (`SHOW DATABASES`, `SHOW TABLES`, `SHOW COLUMNS`).
- **Persistence**: Save and load databases entirely to and from disk (`SAVE`, `LOAD`).

*More advanced query filtering (e.g., `WHERE`, `LIKE`, `LIMIT`, `ORDER BY`) and graph systems are currently planned for future updates.*

## Project Structure

The project code is organized into the following structure:

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
├── .gitignore              # Ignored files for version control
├── ArkTest.ark             # Example SQL script to test the engine
└── README.md               # Project documentation
```

## Example Usage

You can write Ark statements in a `.ark` file. For example (`ArkTest.ark`):

```sql
CREATE DATABASE school;
USE school;

CREATE TABLE students (id INT, name STRING, gpa DOUBLE, enrolled BOOL);

INSERT INTO students VALUES (1, "Alice", 3.9, true);
INSERT INTO students VALUES (2, "Bob", 3.5, false);

SELECT * FROM students;
SELECT name, gpa FROM students;

SAVE;
```

## Getting Started

To compile the project, you need a C++ compiler (like `g++`).

```bash
# Example compilation command compiling all source files together
g++ -Iinclude src/*.cpp -o program
```

Run the executable by passing your script, or using it interactively (depending on implementation):

```bash
./program.exe <fileName.ark>
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
