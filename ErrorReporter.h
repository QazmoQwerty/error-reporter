
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

namespace ErrorReporter 
{
    const uint INTERNAL = 0;
    const uint WARNING = 1;
    const uint NOTE = 2;
    const uint ERROR = 3;

    /* Basic information about the position of a token or node */
    class Position 
    {
    public:
        int line;
        int startPos;
        int endPos;
        SourceFile* file;
    };

    /* Represents dino compile time errors */
    class Error {
    public:
        string msg;
        string subMsg;
        uint errTy;
        Position pos;
        vector<Error> notes;

        Error(string message, uint type, Position position) : msg(message), subMsg(), errTy(type), pos(position) {};
        Error(string message, string subMessage, uint type, Position position) : msg(message), subMsg(subMessage), errTy(type), pos(position) {};
        void addNote(Error note) { notes.push_back(note); }
        void show();
        void showBasic();
        string tyToString();
        void sortSecondaries();
        void showAsSecondary();
        string color(string str);
        void printIndent(bool showLine = false);

        Error& withSecondary(Error err) { notes.push_back(err); return *this; }

        Error& withSecondary(string message, Position position) { notes.push_back(Error(message, NOTE, position)); return *this; }
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
        If isFatal is (true), the function will throw a value.
    */
    Error report(string msg, uint errCode, Position pos, bool isFatal = false);

    /*
        Report an INTERNAL error - these should never be shown to a user of the compiler (in theory at least).
    */
    Error reportInternal(string msg, uint errCode = INTERNAL, Position pos = POS_NONE);
}

#define FATAL_ERROR(_string) throw ErrorReporter::reportInternal(std::string(_string) + " at " + __func__ + ":" + std::to_string(__LINE__) + " in " + __FILE__ , ERR_INTERNAL)

#define TODO FATAL_ERROR("TODO reached");
#define UNREACHABLE FATAL_ERROR("unreachable reached");
#define ASSERT(cond) if(!(cond)) FATAL_ERROR("assertion failed")