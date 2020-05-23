
#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <limits>
#include <vector>
#include <fstream>
#include <functional>
#include <algorithm>

using std::string;
using std::exception;
using std::vector;

enum ErrorCode {
    ERR_INTERNAL,
    ERR_GENERAL  = 1000,
    WRN_GENERAL  = 2000,
    NOTE_GENERAL = 3000,
    HELP_GENERAL = 4000,
    ERR_UNKNOWN  = 5000,
};

namespace ErrorReporter {

    class SourceFile {
    public: 
        virtual string str() = 0;
    };

    /*
        A minimal SourceFile class to get you started
    */
    class SimpleFile : public SourceFile {
    private:
        string _path;
    public:     
        SimpleFile(string path) : _path(path) {};
        string str() { return _path; };
    };

    /* 
        Basic information about the position of a token or node 

        example: { line=2, start=2, end=5 }
        means:   "on second line, from the 3rd character to (and excluding) the 6th character"
    */
    class Position {
    public:
        unsigned int line;
        unsigned int start;
        unsigned int end;
        SourceFile* file;
    };

    // bool hasHelpArticle(ErrorCode errTy);

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

    const Position POS_NONE = {0, 0, 0, NULL};

    /* Pretty-prints all errors reported so far */
    void showAll();

    /* 
        Create and save an error/warning.
        Returns the Error created.
    */
    Error& report(Error err);
    Error& report(string msg, ErrorCode errCode, Position pos);
    Error& report(string msg, string subMsg, ErrorCode errCode, Position pos);
}