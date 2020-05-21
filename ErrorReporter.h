
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
    ERR_GENERAL = 1000,
    ERR_WARNING = 2000,
    ERR_NOTE    = 3000,
    ERR_UNKNOWN = 4000,
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

    /* Represents dino compile time errors */
    class Error {
    public:
        string msg;
        string subMsg;
        ErrorCode errTy;
        Position pos;
        vector<Error> notes;

        Error(string message, ErrorCode type, Position position) : msg(message), subMsg(), errTy(type), pos(position) {};
        Error(string message, string subMessage, ErrorCode type, Position position) : msg(message), subMsg(subMessage), errTy(type), pos(position) {};
        void addNote(Error note) { notes.push_back(note); }
        void show(unsigned int maxLine = 0);
        void showBasic(unsigned int maxLine = 0);
        string tyToString();
        void sortSecondaries();
        string color(string str);
        void printIndent(unsigned int maxLine, bool showLine = false);
        void printPaddingLine(unsigned int maxLine, unsigned int line = 0, SourceFile *file = NULL);

        Error& withSecondary(Error err) { notes.push_back(err); return *this; }
        Error& withSecondary(string message, Position position) { notes.push_back(Error("", message, ERR_NOTE, position)); return *this; }
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