#pragma once
#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <stdexcept>

class ArkError : public std::exception {
private :
    int lineNumber;
    int columNumber;
    std::string errorLexeme;
    std::string correctLexeme;

public :
    ArkError(const int lineNumber, const int columNumber, const std::string errorLexeme, const std::string correctLexme) : lineNumber(lineNumber), columNumber(columNumber), errorLexeme(errorLexeme), correctLexeme(correctLexeme) {};

    virtual const char* what() const noexcept = 0;
};

class SyntaxError : public ArkError {
public :
    SyntaxError(const int lineNumber, const int columNumber, const std::string errorLexeme, const std::string correctLexme) : ArkError(lineNumber, columNumber, errorLexeme, correctLexme) {}

    const char* what() const noexcept override {
        
    }
};

class TypeError : public ArkError {
public :
    const char* what() const noexcept override {

    }
};

class ValueError : public ArkError {
public :
    const char* what() const noexcept override {

    }
};

class RunTimeError : public ArkError {
public :
    const char* what() const noexcept override {

    }
};

class LogicError : public ArkError {
public :
    const char* what() const noexcept override {

    }
};

#endif