
#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <limits>
#include <vector>
#include <fstream>
#include <functional>
#include <algorithm>

#include "TerminalColors.h"
#include "SourceFile.h"
// #include "llvm/Support/raw_ostream.h"


using std::string;
using std::exception;
using std::vector;

#define FATAL_ERROR(_string) throw ErrorReporter::reportInternal(std::string(_string) + " at " + __func__ + ":" + std::to_string(__LINE__) + " in " + __FILE__)

#define TODO FATAL_ERROR("TODO reached");
#define UNREACHABLE FATAL_ERROR("unreachable reached");
#define ASSERT(cond) if(!(cond)) FATAL_ERROR("assertion failed")

enum ErrorCode {
    ERR_INTERNAL,
    ERR_GENERAL  = 1000,
    WRN_GENERAL  = 2000,
    NOTE_GENERAL = 3000,
    HELP_GENERAL = 4000,
    ERR_UNKNOWN  = 5000,
};

namespace ErrorReporter 
{
    /* Basic information about the position of a token or node */
    class Position 
    {
    public:
        unsigned int line;
        unsigned int startPos;
        unsigned int endPos;
        SourceFile* file;
    };

    bool hasHelpArticle(ErrorCode errTy);

    class Error {
    public:
        Error(string message, ErrorCode type, Position position);
        Error(string message, string subMessage, ErrorCode type, Position position);
        void show();
        Error& withNote(string message, Position position);
        Error& withHelp(string message, Position position);
        Error& withNote(string message);
        Error& withHelp(string message);
    private:
        string msg;
        string subMsg;
        ErrorCode errTy;
        Position pos;
        vector<Error> secondaries;

        string tyToString();
        void sortSecondaries();
        string color(string str);
        void showSecondariesOnLine(string &line, size_t &i, unsigned int maxLine);
        void printIndent(unsigned int maxLine, bool showBar = true);
        void printIndentWithLineNum(unsigned int maxLine, bool showBar = true);
        void printPaddingLine(unsigned int maxLine, unsigned int line = 0, SourceFile *file = NULL);
    };

    /*
        Static class which contains a vector of Errors. 
        Any compile errors/warnings found will be reported here.
    */
    extern vector<Error> errors;
    string getLine(string fileName, int line);
    const Position POS_NONE = {0, 0, 0, NULL};

    /* Pretty-prints all errors reported so far */
    void showAll();

    /* Pretty-prints an Error. */
    void show(Error &err);

    /* 
        Create and save an error/warning.
        Returns the Error created.
    */
    Error& report(Error err);
    Error& report(string msg, ErrorCode errCode, Position pos);
    Error& report(string msg, string subMsg, ErrorCode errCode, Position pos);
    void reportAbort();

    /*
        Report an ERR_INTERNAL error - these should never be shown to a user of the compiler (in theory at least).
    */
    Error& reportInternal(string msg, ErrorCode errCode = ERR_INTERNAL, Position pos = POS_NONE);
}