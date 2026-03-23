#include "error.h"

namespace detail {

    const char* toString(SyntaxError code) {
        switch(code) {
            case SyntaxError::UNEXPECTED_TOKEN : return "UNEXPECTED_TOKEN";
            case SyntaxError::UNRECOGNIZED_DATA_TYPE : return "UNRECOGNIZED_DATA_TYPE";
            case SyntaxError::UNRECOGNIZED_VALUE : return "UNRECOGNIZED_VALUE";
            case SyntaxError::INVALID_LIKE_PATTERN : return "INVALID_LIKE_PATTERN";
            case SyntaxError::EXPECTED_STRING_AFTER_LIKE : return "EXPECTED_STRING_AFTER_LIKE";
            case SyntaxError::UNKNOWN_CREATE_KEYWORD : return "UNKOWN_CREATE_KEYWORD";
            case SyntaxError::UNKNOWN_DROP_KEYWORD : return "UNKOWN_DROP_KEYWORD";
            case SyntaxError::UNKNOWN_SHOW_KEYWORD : return "UNKOWN_SHOW_KEYWORD";
            case SyntaxError::UNKNOWN_COMMAND : return "UNKOWN_COMMAND";
            default : return "UNKOWN";
        }
    }

    const char* toString(TypeError code) {
        switch(code) {
            case TypeError::LIMIT_NOT_INT : return "LIMIT_NOT_INT";
            case TypeError::LIKE_REQUIRES_STRING : return "LIKE_REQUIRES_STRING";
            case TypeError::INVALID_NUMERIC_LITERAL : return "INVALID_NUMERIC_LITERAL";
            default : return "UNKOWN";
        }
    }

    const char* toString(RuntimeError code) {
        switch(code) {
            case RuntimeError::COLUMN_NOT_FOUND : return "COLUMN_NOT_FOUND";
            case RuntimeError::TABLE_NOT_FOUND : return "TABLE_NOT_FOUND";
            case RuntimeError::NO_DATABASES : return "NO_DATABASES";
            case RuntimeError::COLUMN_COUNT_MISMATCH : return "COLUMN_COUNT_MISMATCH";
            case RuntimeError::LIKE_PATTERN_TOO_SHORT : return "LIKE_PATTERN_TOO_SHORT";
            default : return "UNKOWN";
        }
    }

}

ArkException::ArkException(const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme) : lineNumber(lineNumber), columnNumber(columnNumber), errorLexeme(errorLexeme), correctLexeme(correctLexeme) {}

SyntaxException::SyntaxException(const SyntaxError code, const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme) : ArkException(lineNumber, columnNumber, errorLexeme, correctLexeme), code(code) {}

const char* SyntaxException::what() const noexcept {
    std::string location = "LINE: " + std::to_string(lineNumber) + ", COLUMN: " + std::to_string(columnNumber) + "\n";

    switch(code) {
        case SyntaxError::UNEXPECTED_TOKEN : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unexpected token\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Unexpected token '" + errorLexeme + "', expected '" + correctLexeme + "'.\n";
            m_message += location;
            m_message += DIVIDER;
            break; 
        }
        case SyntaxError::UNRECOGNIZED_DATA_TYPE : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unrecognized data type\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: '" + errorLexeme + "' is not a recognized data type. Expected INT, STRING, FLOAT or BOOL.\n";
            m_message += location;
            m_message += DIVIDER;
            break; 
        }
        case SyntaxError::UNRECOGNIZED_VALUE : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unrecognized value token\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: '" + errorLexeme + "' is not a valid value token.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case SyntaxError::INVALID_LIKE_PATTERN : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Invalid LIKE pattern\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Invalid LIKE pattern '" + errorLexeme + "'. Pattern must be a string literal.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case SyntaxError::EXPECTED_STRING_AFTER_LIKE :
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Expected string after LIKE\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Expected a string literal after LIKE, got '" + errorLexeme + "'.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        case SyntaxError::UNKNOWN_CREATE_KEYWORD : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unknown keyword after CREATE\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Unexpected token '" + errorLexeme + "' after CREATE. Expected TABLE or DATABASE.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case SyntaxError::UNKNOWN_DROP_KEYWORD : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unknown keyword after DROP\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Unexpected token '" + errorLexeme + "' after DROP. Expected TABLE or DATABASE.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case SyntaxError::UNKNOWN_SHOW_KEYWORD : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unknown keyword after SHOW\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Unexpected token '" + errorLexeme + "' after SHOW. Expected TABLES, DATABASES or COLUMNS.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case SyntaxError::EXPECTED_COMPARISON_OPERATOR : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Expected comparison operator\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Expected a comparison operator (==, !=, >=, <=, >, <) but got '" + errorLexeme + "'.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case SyntaxError::UNKNOWN_COMMAND : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unknown command\n";
            m_message += "CODE: E-SYNTAX-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: '" + errorLexeme + "' is not a recognized command.\n";
            m_message += location;
            m_message += DIVIDER;
            break; 
        }
        default: {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "SYNTAX ERROR: Unknown\n";
            m_message += "CODE: E-SYNTAX-UNKNOWN\n";
            m_message += "MESSAGE: An unknown syntax error occurred.\n";
            m_message += location;
            m_message += DIVIDER;
            break; 
        }
    }
    return m_message.c_str();
}

TypeException::TypeException(const TypeError code, const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme) : ArkException(lineNumber, columnNumber, errorLexeme, correctLexeme), code(code) {}

const char* TypeException::what() const noexcept {
    std::string location = "LINE: " + std::to_string(lineNumber) + ", COLUMN: " + std::to_string(columnNumber) + "\n";

    switch(code) {
        case TypeError::LIMIT_NOT_INT : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "TYPE ERROR: Invalid LIMIT value\n";
            m_message += "CODE: E-TYPE-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: LIMIT value '" + errorLexeme + "' is not valid. LIMIT requires an INT.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case TypeError::NEGATIVE_LIMIT : {
            m_message = "\n";
            m_message += DIVIDER;
            m_message += "TYPE ERROR : NEGATIVE LIMIT\n";
            m_message += "CODE: E-TYPE-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: LIMIT value '" + errorLexeme + "' should not be negative.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case TypeError::LIKE_REQUIRES_STRING : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "TYPE ERROR: LIKE requires a STRING column\n";
            m_message += "CODE: E-TYPE-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Column '" + errorLexeme + "' is not of type STRING. LIKE can only be used on STRING columns.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case TypeError::INVALID_NUMERIC_LITERAL : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "TYPE ERROR: Invalid numeric literal\n";
            m_message += "CODE: E-TYPE-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: '" + errorLexeme + "' is not a valid numeric literal. Expected a valid INT or FLOAT value.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        default: {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "TYPE ERROR: Unknown\n";
            m_message += "CODE: E-TYPE-UNKNOWN\n";
            m_message += "MESSAGE: An unknown type error occurred.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
    }
    return m_message.c_str();
}

RuntimeException::RuntimeException(const RuntimeError code, const int lineNumber, const int columnNumber, const std::string errorLexeme, const std::string correctLexeme) : ArkException(lineNumber, columnNumber, errorLexeme, correctLexeme), code(code) {}

const char* RuntimeException::what() const noexcept {
    std::string location = "LINE: " + std::to_string(lineNumber) + ", COLUMN: " + std::to_string(columnNumber) + "\n";

    switch(code) {
        case RuntimeError::COLUMN_NOT_FOUND : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "RUNTIME ERROR: Column not found\n";
            m_message += "CODE: E-RUNTIME-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Column '" + errorLexeme + "' does not exist in Table '" + correctLexeme + "'. \n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case RuntimeError::TABLE_NOT_FOUND : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "RUNTIME ERROR: Table not found\n";
            m_message += "CODE: E-RUNTIME-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: Table '" + errorLexeme + "' does not exist in Database '" + correctLexeme + "'. \n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case RuntimeError::NO_DATABASES : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "RUNTIME ERROR: No databases found\n";
            m_message += "CODE: E-RUNTIME-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: No databases exist. Create a database first using CREATE DATABASE.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case RuntimeError::COLUMN_COUNT_MISMATCH : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "RUNTIME ERROR: Column count mismatch\n";
            m_message += "CODE: E-RUNTIME-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: INSERT value count does not match column count. Got '" + errorLexeme + "', expected '" + correctLexeme + "'.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case RuntimeError::INSERT_TYPE_MISMATCH : {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "RUNTIME ERROR: Type mismatch on insert\n";
            m_message += "CODE: E-RUNTIME-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: One or more values do not match the column types of Table '" + errorLexeme + "'.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
        case RuntimeError::LIKE_PATTERN_TOO_SHORT : {
            m_message = "\n";
            m_message += DIVIDER;
            m_message += "RUNTIME ERROR: LIKE pattern too short\n";
            m_message += "CODE: E-RUNTIME-" + std::string(detail::toString(code)) + "\n";
            m_message += "MESSAGE: The LIKE pattern '" + errorLexeme + "' is too short. Pattern must be at least 2 characters long (e.g. 'A%').\n";
            m_message += DIVIDER;
            break;
        }
        default: {
            m_message  = "\n";
            m_message += DIVIDER;
            m_message += "RUNTIME ERROR: Unknown\n";
            m_message += "CODE: E-RUNTIME-UNKNOWN\n";
            m_message += "MESSAGE: An unknown runtime error occurred.\n";
            m_message += location;
            m_message += DIVIDER;
            break;
        }
    }
    return m_message.c_str();
}