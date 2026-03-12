#include "helper.h"

bool evaluateCondition(const Cell& left, TokenType op, const Cell& right) {
    switch(op) {
        case TokenType::TOK_EQUAL_EQUAL:
            return left == right;

        case TokenType::TOK_NOT_EQUAL:
            return left != right;

        case TokenType::TOK_GREATER:
            return left > right;

        case TokenType::TOK_LESS:
            return left < right;

        case TokenType::TOK_GREATER_EQUAL:
            return left >= right;

        case TokenType::TOK_LESS_EQUAL:
            return left <= right;

        default:
            return false;
    }
}