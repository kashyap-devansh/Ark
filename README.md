# Ark

Ark is a custom, lightweight database engine and SQL-like query language interpreter implemented in C++. 

> **Note:** 🚧 This project is currently **under development** and is actively being worked on. Features and syntax may be subject to change.

## Overview

Ark provides basic relational database capabilities, allowing users to parse and execute SQL-like statements. The project is built from scratch and includes its own custom tokenizer, expression parser, and core execution engine.

## Features (Current & Planned)

- **Databases**: Create and switch between databases (`CREATE DATABASE`, `USE`).
- **Tables**: Define tables with different data types (`CREATE TABLE`).
- **Data Insertion**: Insert rows into tables (`INSERT INTO`).
- **Querying**: Basic querying of data from tables (`SELECT`).
- *More features are actively being developed...*

## Architecture

The project consists of several core components:
- **Tokenizer (`tokenizer.cpp` / `tokenizer.h`)**: Lexical analysis of the `.ark` query files.
- **Parser (`parser.cpp` / `parser.h`)**: Parses tokens into an Abstract Syntax Tree (AST) representing database operations.
- **Core Engine (`core.cpp` / `core.h`)**: Executes the parsed database operations.
- **Database Manager (`databaseManager.cpp` / `databaseManager.h`)**: Handles storage and retrieval of database schema and table data.

## Example Usage

You can write Ark statements in a `.ark` file. For example (`ArkTest.ark`):

```sql
CREATE DATABASE testdb;
USE testdb;

CREATE TABLE users (id INT, name STRING, age INT);

INSERT INTO users VALUES (1, "Alice", 25);
INSERT INTO users VALUES (2, "Bob", 30);

SELECT * FROM users;
SELECT name, age FROM users;
```

## Getting Started

To compile the project, you need a C++ compiler (like g++).

```bash
# Example compilation command (assuming all src files are compiled together)
g++ src/main.cpp src/core.cpp src/databaseManager.cpp src/parser.cpp src/tokenizer.cpp -I include -o ark.exe
```

Run the executable by passing your script, or using it interactively (depending on implementation):

```bash
./ark.exe
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
