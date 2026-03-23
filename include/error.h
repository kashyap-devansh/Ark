#pragma once
#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <stdexcept>
#include <string>

#define DIVIDER "============================================================\n"

enum class SyntaxError {
    UNEXPECTED_TOKEN,
    UNRECOGNIZED_DATA_TYPE,
    UNRECOGNIZED_VALUE,
    INVALID_LIKE_PATTERN,
    EXPECTED_STRING_AFTER_LIKE,
    UNKNOWN_CREATE_KEYWORD,
    UNKNOWN_DROP_KEYWORD,
    UNKNOWN_SHOW_KEYWORD,
    EXPECTED_COMPARISON_OPERATOR,
    UNKNOWN_COMMAND,
};

enum class TypeError {
    LIMIT_NOT_INT,
    LIKE_REQUIRES_STRING,
    INVALID_NUMERIC_LITERAL,
    NEGATIVE_LIMIT,
};

enum class RuntimeError {
    COLUMN_NOT_FOUND,
    INSERT_TYPE_MISMATCH,
    TABLE_NOT_FOUND,
    NO_DATABASES,
    COLUMN_COUNT_MISMATCH,
    LIKE_PATTERN_TOO_SHORT,
};

enum class LogicError {
    MISSING_EXIT_AFTER_ERROR,
};

namespace detail {
    const char* toString(SyntaxError code);
    const char* toString(TypeError code);
    const char* toString(RuntimeError code);
}

class ArkException : public std::exception {
protected:
    int lineNumber;
    int columnNumber;
    std::string errorLexeme;
    std::string correctLexeme;

public:
    ArkException(const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme);
    virtual const char* what() const noexcept = 0;
};

class SyntaxException : public ArkException {
private:
    SyntaxError code;
    mutable std::string m_message;

public:
    SyntaxException(const SyntaxError code, const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme);
    const char* what() const noexcept override;
};

class TypeException : public ArkException {
private:
    TypeError code;
    mutable std::string m_message;

public:
    TypeException(const TypeError code, const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme);
    const char* what() const noexcept override;
};

class RuntimeException : public ArkException {
private:
    RuntimeError code;
    mutable std::string m_message;

public:
    RuntimeException(const RuntimeError code, const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme);
    const char* what() const noexcept override;
};

// class Logic : public ArkError {
// public :
//     const char* what() const noexcept override {

//     }
// };

#endif