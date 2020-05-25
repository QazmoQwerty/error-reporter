/*
 * Copyright (c) 2020 Shalev Don Meiri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef DIAGNOSTIC_REPORTER_HPP_INCLUDED
#define DIAGNOSTIC_REPORTER_HPP_INCLUDED

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

/**
 * A simple implementation for pretty error diagnostics.
 * Used for the Dino compiler.
 */
namespace reporter {

    /**
     * Utility namespace which deals with terminal colors.
     */
    namespace colors {
        /**
         * Utility class which represents a terminal color.
         * @note 'color' can mean both "red/green" and "bold/italic".
         */ 
        class Color {
            unsigned char _code;
        public:
            /** 
             * @param code the ANSI escape code corresponding to this color.
             */
            Color(unsigned char code) : _code(code) {}

            /** 
             * Colors a string using an ANSI escape sequence based on _color, to be displayed in the terminal.
             * @param str string to be colored.
             * @return the colored string.
             */
            std::string operator()(std::string str) const {
                return "\x1B[" + std::to_string(_code) + "m" + str + "\x1B[0m";
            }
        };

        const Color bold      (1);
        const Color weak      (2);
        const Color italic    (3);
        const Color underline (4);

        const Color black   (30);
        const Color red     (31);
        const Color green   (32);
        const Color yellow  (33);
        const Color blue    (34);
        const Color magenta (35);
        const Color cyan    (36);
        const Color white   (37);
    }

    /////////////////////////////////////////////////////////////////////////

    /**
     * Abstract class which represents a source file.
     * Class contains only one member, 'str()', which is opened and displayed by the reporter
     */
    class SourceFile {
    public: 
        /**
         * @return file path to be opened and dispayed by the reporter.
         */
        virtual std::string str() = 0;
    };

    /**
     * A minimal SourceFile class to get you started
     */
    class SimpleFile : public SourceFile {
    private:
        std::string _path;
    public:
        /**
         * @param path path to the source file.
         */
        SimpleFile(std::string path) : _path(path) {};

        /**
         * @return path to the source file.
         */
        std::string str() { return _path; };
    };

    /////////////////////////////////////////////////////////////////////////

    /**
     * TODO - explain
     */
    enum DiagnosticType {
        INTERNAL_ERROR,
        ERROR,
        WARNING,
        NOTE,
        HELP,
        UNKNOWN,
    };

    /**
     * A location of source code.
     * @example `{ 2, 3, 5 }` -> on 2nd line, from the 4th character to (and excluding) the 6th character"
     */
    class Location {
    public:
        unsigned int line;  /// line in the file (line count starts at 1).  
        unsigned int start; /// index of the first character of the line to be included (starts at 0).
        unsigned int end;   /// index of the first character of the line to be excluded (starts at 0).
        SourceFile* file;   /// file this location is in.

        /** 
         * Constructs a Location in `file` at `line`, from `start` to (and excluding) `end`.
         * @param line line in the file (line count starts at 1).  
         * @param start index of the first character of the line to be included (starts at 0).
         * @param end index of the first character of the line to be excluded (starts at 0).
         * @param file file this location is in.
         */
        Location(unsigned int line, unsigned int start, unsigned int end, SourceFile* file)
                : line(line), start(start), end(end), file(file) {
            if (this->end <= this->start)
                this->end = this->start + 1;
        }

        /**
         * Constructs a Location in `file` at `line`, at the character with index of `loc`.
         * @param line line in the file (line count starts at 1).  
         * @param loc index of the only character of the line to be included (starts at 0).
         * @param file file this location is in.
         */
        Location(unsigned int line, unsigned int loc, SourceFile* file) : Location(line, loc, loc + 1, file) {}

        /**
         * Constructs a non-location
         * Used for errors which are not bound to a specific location in the source code.
         */
        Location() : Location(0, 0, 0, nullptr) {}
    };

    /**
     * These are all the parts which are rendered by `print`:
     *
     *      header╶─╴│ Error(E308): a complex error
     *         top╶─╴│   ╭─ example.cpp ─╴
     *               │   │ 
     *     snippet╶─╴│ 1 │ #include "reporter.hpp"
     *   secondary╶─╴│   │ ~~~~~~~~ a relevant include
     *     padding╶─╴│ 2 │ 
     *     snippet╶─╴│ 3 │ int main() {
     *   secondary╶─╴│   │            ~ curly brace
     *     padding╶─╴│ ⋯
     *  subMessage╶─╴│   │          this is where the error is, hence the bold red
     *               │   │          vvvv
     *     snippet╶─╴│ 7 │     auto file = new reporter::SimpleFile("example.cpp");
     *             ┌╴│   │     ~~~~ ~~~~ ~               ~~~~~~~~~~                
     *             │ │   │     │    │    │               ╰ a help message
     * secondaries╶┤ │   │     │    │    ╰ assignment
     *             │ │   │     │    ╰ a variable
     *             └╴│   │     ╰ a type
     *      bottom╶─╴│───╯
     *   secondary╶─╴│     • Help: a general help message,
     *               │             not set to any specific location
     */
    class Diagnostic {
    private:
        std::string msg;
        std::string subMsg;
        Location loc;
        DiagnosticType errTy;
        std::string code;
        std::vector<Diagnostic> secondaries;

        /* returns whether the two diagnostics are on the same line */
        static bool onSameLine(Diagnostic& a, Diagnostic& b) {
            return a.loc.file == b.loc.file && a.loc.line == b.loc.line;
        }

        /* split a string into its lines */
        static std::vector<std::string> splitLines(const std::string& str) {
            std::vector<std::string> strings;
            std::string::size_type loc = 0;
            std::string::size_type prev = 0;
            while ((loc = str.find("\n", prev)) != std::string::npos) {
                strings.push_back(str.substr(prev, loc - prev));
                prev = loc + 1;
            }
            // To get the last substring (or only, if delimiter is not found)
            strings.push_back(str.substr(prev));
            return strings;
        }

        /* get a specific line from a text file */
        static std::string getLine(std::string fileName, int line) {
            std::fstream file(fileName);
            file.seekg(std::ios::beg);
            for (int i=0; i < line - 1; ++i)
                file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::string ret;
            std::getline(file, ret);
            return ret;
        }

        /* prints `count` lines of whitespace */
        static void indent(std::ostream& out, std::string& line, unsigned int count) {
            for (unsigned int i = 0; i < count; i++)
                out << (line[i] == '\t' ? "\t" : " ");
        }

        std::string tyToString() {
            std::string str;
            switch (errTy) {
                case ERROR:   return code == "" ? "Error" : "Error(" + code + ")";
                case WARNING: return code == "" ? "Warning" : "Warning(" + code + ")";
                case NOTE:    return code == "" ? "Note" : "Note(" + code + ")";;
                case HELP:    return code == "" ? "Help" : "Help(" + code + ")";;
                default:      return code == "" ? "Internal Error" : "Internal Error(" + code + ")";
            }
        }

        /* gets a string and colors it based on `errTy`*/
        std::string color(std::string str) {
            switch (errTy) {
                default:      return colors::bold(colors::red(str));
                case WARNING: return colors::bold(colors::yellow(str));
                case NOTE:    return colors::bold(colors::black(str));
                case HELP:    return colors::bold(colors::blue(str));
            }
        }

        /* sort the vector of secondary messages based on the order we want to be printing them */
        void sortSecondaries() {
            auto file = loc.file;
            std::sort(
                std::begin(secondaries), std::end(secondaries), 
                [file](Diagnostic &a, Diagnostic &b) {
                    if (!a.loc.file) 
                        return false;
                    if (!b.loc.file) 
                        return true;
                    if (a.loc.file == file && b.loc.file != file)
                        return true;
                    if (a.loc.file != file && b.loc.file == file)
                        return false;
                    if (a.loc.file != b.loc.file)
                        return a.loc.file->str() < b.loc.file->str();
                    if (a.loc.line == b.loc.line)
                        return a.loc.start > b.loc.start; 
                    return a.loc.line < b.loc.line;
                }
            );
        }
        
        /* a 'padding' line is an irrelevant line in between two other relevant lines */
        void printPadding(std::ostream& out, unsigned int maxLine, unsigned int lastLine, unsigned int currLine, SourceFile *file) {
            unsigned int targetSize = std::to_string(maxLine).size() + 2;
            if (lastLine + 2 == currLine) {
                auto str = " " + std::to_string(currLine - 1) + " ";
                while (str.size() < targetSize)
                    str += " ";
                str += "│ ";
                out << color(str) << getLine(file->str(), currLine - 1) << "\n";            
            } else {
                switch (targetSize) {
                    case 3:  out << " " << color("⋯") << "\n"; break;
                    case 4:  out << " " << color("··") << "\n"; break;
                    default: out << " " << color("···") << "\n"; break;
                }   
            }
        }

        /* prints the bars on the left with the correct indentation */
        void printLeft(std::ostream& out, unsigned int maxLine, bool printBar = true) {
            for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++)
                out << " ";
            if (printBar)
                out << color("│ ");
        }

        /* prints the `╭─ file.xyz ─╴` at the start of the file's diagnostics */
        void printTop(std::ostream& out, SourceFile* file, unsigned int maxLine) {
            printLeft(out, maxLine, false);
            out << color("╭─ ") << file->str() << color(" ─╴") << "\n";
        }

        /* prints the `──╯` at the end of the file's diagnostics */
        void printBottom(std::ostream& out, unsigned int maxLine) {
            for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++) 
                out << color("─");
            out << color("╯") << "\n";
        }

        /* prints the bars on the left with the correct indentation + with the line number */
        void printLeftWithLineNum(std::ostream& out, unsigned int lineNum, unsigned int maxLine, bool printBar = true) {
            unsigned int targetSize = std::to_string(maxLine).size() + 2;
            auto str = " " + std::to_string(lineNum) + " ";
            while (str.size() < targetSize)
                str += " ";
            out << color(str);
            if (printBar)
                out << color("│ ");
        }


        /* prints all secondary messages on the current line */
        void printSecondariesOnLine(std::ostream& out, std::string &line, size_t &i, unsigned int maxLine) {
            auto &first = secondaries[i];
            printLeft(out, maxLine);
            if (i + 1 >= secondaries.size() || !onSameLine(first, secondaries[i + 1])) {
                // only one secondary concerning this line
                for (size_t idx = 0; idx < first.loc.start; idx++)
                    out << (line[idx] == '\t' ? '\t' : ' ');
                for (size_t idx = 0; idx < first.loc.end - first.loc.start; idx++)
                    out << first.color("~");
                auto lines = splitLines(first.subMsg);
                for (size_t idx = 0; idx < lines.size(); idx++) {
                    if (idx != 0) {
                        printLeft(out, maxLine);
                        for (size_t j = 0; j < first.loc.end; j++)
                            out << (line[j] == '\t' ? '\t' : ' ');
                    }
                    out << " " << first.color(lines[idx]) << "\n";
                }
                i++;
            } else {
                for (size_t lineIdx = 0; lineIdx < line.size(); lineIdx++) {
                    bool b = false;
                    for (auto idx = i; !b && idx < secondaries.size() && onSameLine(secondaries[idx], first); idx++)
                        if (secondaries[idx].loc.start <= lineIdx && lineIdx < secondaries[idx].loc.end) {
                            out << secondaries[idx].color("~");
                            b = true;
                        }
                    if (!b) out << " ";
                }
                out << "\n";
                for (; i < secondaries.size() && onSameLine(secondaries[i], first); i++) {
                    printLeft(out, maxLine);
                    for (size_t j = 0; j < secondaries[i].loc.start; j++) {
                        bool b = false;
                        for (auto idx = i; !b && idx < secondaries.size() && onSameLine(secondaries[idx], first); idx++)
                            if (secondaries[idx].loc.start == j) {
                                out << secondaries[idx].color("│");
                                b = true;
                            }
                        if (!b) out << " ";
                    }
                    auto lines = splitLines(secondaries[i].subMsg);
                    for (size_t idx = 0; idx < lines.size(); idx++) {
                        if (idx == 0)
                            out << secondaries[i].color("╰ ") << secondaries[i].color(lines[idx]) << "\n";
                        else {
                            printLeft(out, maxLine);
                            for (size_t j = 0; j < secondaries[i].loc.start; j++) {
                                bool b = false;

                                for (auto idx = i; !b && idx < secondaries.size() && onSameLine(secondaries[idx], first); idx++)
                                    if (secondaries[idx].loc.start == j) {
                                        out << secondaries[idx].color("│");
                                        b = true;
                                    }
                                if (!b) out << " ";
                            }
                            out << secondaries[i].color("  ") << secondaries[i].color(lines[idx]) << "\n";
                        }
                    }
                }
            }
        }

    protected:
        Diagnostic(DiagnosticType ty, std::string message, std::string subMessage, std::string code, Location location) 
               : msg(message), subMsg(subMessage), loc(location), errTy(ty), code(code) {};
        Diagnostic(DiagnosticType ty, std::string message, std::string subMessage, Location location) : Diagnostic(ty, message, subMessage, "", location) {};
        Diagnostic(DiagnosticType ty, std::string message, Location location) : Diagnostic(ty, message, "", location) {};
        Diagnostic(DiagnosticType ty, std::string message) : Diagnostic(ty, message, {}) {};

    public:
        /**
         * Pretty-print the diagnostic.
         * @param out stream in which to print the error.
         * @return the object which this function was called upon.
         */
        Diagnostic& print(std::ostream& out) {

            // find the maximum line (to know by how much to indent the bars)
            auto maxLine = loc.line;
            for (auto& secondary : secondaries)
                if (secondary.loc.line > maxLine)
                    maxLine = secondary.loc.line;

            // sort the vector of secondary messages based on the order we want to be printing them 
            sortSecondaries();

            // by default we're pointing at the error location from below the code snippet
            bool printAbove = false; 

            // if there are any messages on the line of the error, point to the error from above instead
            for (auto& i : secondaries)
                if (onSameLine(i, *this)) {
                    printAbove = true;
                    break;
                }

            // print the main error message
            if (msg != "")
                out << color(tyToString() + ": ") << colors::bold(msg) << "\n";

            // we're finished if the diagnostic has no location
            if (loc.file == nullptr) return *this;

            // print the file the error is in
            printTop(out, loc.file, maxLine);

            size_t i = 0; // current index in `secondaries`
            unsigned int lastLine = 0; // the last line we rendered

            // first print all messages in the main file which come before the error
            while (i < secondaries.size() && secondaries[i].loc.file == loc.file && secondaries[i].loc.line < loc.line) {
                auto &secondary = secondaries[i];

                if (lastLine == 0) { // if we're rendering the first line in the file, print an empty line
                    printLeft(out, maxLine); 
                    out << "\n";
                } else if (lastLine < secondary.loc.line - 1)
                    printPadding(out, maxLine, lastLine, secondary.loc.line, secondary.loc.file);

                lastLine = secondary.loc.line;
                std::string line = getLine(loc.file->str(), secondary.loc.line);
                printLeftWithLineNum(out, secondary.loc.line, maxLine);
                out << line << "\n";
                printSecondariesOnLine(out, line, i, maxLine);
            }

            std::string line = getLine(loc.file->str(), loc.line);

            if (lastLine == 0 && !printAbove) {
                printLeft(out, maxLine);
                out << "\n";
            } else if (lastLine != 0 && lastLine < loc.line - 1)
                printPadding(out, maxLine, lastLine, loc.line, loc.file);
            lastLine = loc.line;
            
            if (printAbove) {
                if (subMsg != "") {
                    for (auto currLine : splitLines(subMsg)) {
                        printLeft(out, maxLine);
                        indent(out, line, loc.start);
                        out << color(currLine);
                        out << "\n";
                    }
                }
                
                printLeft(out, maxLine);

                indent(out, line, loc.start);
                for (unsigned int i = 0; i < loc.end - loc.start; i++)
                    out << color("v");
                out << "\n";
            }
            
            printLeftWithLineNum(out, loc.line, maxLine);
            out << line << "\n";

            if (!printAbove && subMsg != "") {
                auto split = splitLines(subMsg);
                for (size_t i = 0; i < split.size(); i++) {
                    printLeft(out, maxLine);
                    indent(out, line, loc.start);
                    for (unsigned int j = 0; j < loc.end - loc.start; j++)
                        out << color(i == 0 ? "^" : " ");
                    out << " ";
                    out << color(split[i]) << "\n";
                }
            }
        
            if (i < secondaries.size() && onSameLine(secondaries[i], *this))
                printSecondariesOnLine(out, line, i, maxLine);
            
            auto currFile = loc.file;
            while (i < secondaries.size() && secondaries[i].loc.file) {
                auto &secondary = secondaries[i];

                if (secondary.loc.file->str() != currFile->str()) {
                    currFile = secondary.loc.file;
                    printBottom(out, maxLine);
                    printTop(out, currFile, maxLine);
                    printLeft(out, maxLine);
                    out << "\n";
                } else if (lastLine < secondary.loc.line - 1)
                    printPadding(out, maxLine, lastLine, secondary.loc.line, secondary.loc.file);

                lastLine = secondary.loc.line;
                std::string line = getLine(currFile->str(), secondary.loc.line);
                printLeftWithLineNum(out, secondary.loc.line, maxLine);
                out << line << "\n";
                printSecondariesOnLine(out, line, i, maxLine);
            }

            printBottom(out, maxLine); 
            for (; i < secondaries.size(); i++) {
                auto& secondary = secondaries[i];
                printLeft(out, maxLine, false);
                out << secondary.color("• " + secondary.tyToString() + secondary.color(": "));
                auto lines = splitLines(secondary.subMsg);
                for (size_t idx = 0; idx < lines.size(); idx++) {
                    if (idx != 0) {
                        printLeft(out, maxLine, false);
                        for (size_t j = 0; j < secondary.tyToString().size() + 4; j++)
                            out << " ";
                    }
                    out << lines[idx] << "\n";
                }
            }
            return *this;
        }

        /**
         * Adds a secondary note message to the diagnostic at `location`.
         * @param message the note message.
         * @param location source code location of the note message.
         * @return the object which this function was called upon.
         */
        inline Diagnostic& withNote(std::string message, Location location);
        
        /**
         * Adds a secondary help message to the diagnostic at `location`.
         * @param message the help message.
         * @param location source code location of the help message.
         * @return the object which this function was called upon.
         */
        inline Diagnostic& withHelp(std::string message, Location location);

        /**
         * Adds a secondary note message to the diagnostic without a specific location.
         * @param message the note message.
         * @return the object which this function was called upon.
         */
        inline Diagnostic& withNote(std::string message);

        /**
         * Adds a secondary help message to the diagnostic without a specific location.
         * @param message the help message.
         * @return the object which this function was called upon.
         */
        inline Diagnostic& withHelp(std::string message);        
    };

    /////////////////////////////////////////////////////////////////////////

    /**
     * TODO - explain
     */
    template<DiagnosticType T>
    class DiagnosticTy : public Diagnostic {
    public:
        /**
         * Constructs a minimal diagnostic message, without a specific source code location.
         * @param message the diagnostic message - should essentially be the 'title' of the diagnostic without going into too much detail.
         */
        DiagnosticTy<T>(std::string message) : Diagnostic(T, message) {}

        /**
         * Constructs a simple diagnostic with a message and a source code location.
         * @param message the diagnostic message - should essentially be the 'title' of the diagnostic without going into too much detail.
         * @param location the location the diagnostic is concerning.
         */
        DiagnosticTy<T>(std::string message, Location location) : Diagnostic(T, message, location) {}

        /**
         * Constructs a diagnostic at a specific source code location with both a primary message and a submessage.
         * @param message the diagnostic message - should essentially be the 'title' of the diagnostic without going into too much detail.
         * @param subMessage the secondary message which is printed directly next to the source code.
         * @param location the location the diagnostic is concerning.
         */
        DiagnosticTy<T>(std::string message, std::string subMessage, Location location) : Diagnostic(T, message, subMessage, location) {}

        /**
         * Constructs a diagnostic at a specific source code location with both a primary message and a submessage, as well as a custom error code.
         * @param message the diagnostic message - should essentially be the 'title' of the diagnostic without going into too much detail.
         * @param subMessage the secondary message which is printed directly next to the source code.
         * @param code the error code, can be anything but is usually something like `"E101"` or `"W257"`, for example.
         * @param location the location the diagnostic is concerning.
         */
        DiagnosticTy<T>(std::string message, std::string subMessage, std::string code, Location location) : Diagnostic(T, message, subMessage, code, location) {}

        /**
         * Pretty-print the diagnostic.
         * @param out stream in which to print the error.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& print(std::ostream& out) { Diagnostic::print(out); return *this; };

        /**
         * Adds a secondary note message to the diagnostic at `location`.
         * @param message the note message.
         * @param location source code location of the note message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withNote(std::string message, Location location) { Diagnostic::withNote(message, location); return *this; };

        /**
         * Adds a secondary help message to the diagnostic at `location`.
         * @param message the help message.
         * @param location source code location of the help message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withHelp(std::string message, Location location) { Diagnostic::withHelp(message, location); return *this; };

        /**
         * Adds a secondary note message to the diagnostic without a specific location.
         * @param message the note message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withNote(std::string message) { Diagnostic::withNote(message); return *this; };

        /**
         * Adds a secondary help message to the diagnostic without a specific location.
         * @param message the help message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withHelp(std::string message) { Diagnostic::withHelp(message); return *this; };
    };

    /////////////////////////////////////////////////////////////////////////

    /**
     * An `internal error` diagnostic type. 
     * These should (ideally) never be shown to the client, rather they should be used as a debugging tool for the compiler developer.
     */
    typedef DiagnosticTy<INTERNAL_ERROR> InternalError;

    /**
     * An `error` diagnostic type. 
     * These are usually errors which stop the compilation process from being competed.
     */
    typedef DiagnosticTy<ERROR> Error;

    /**
     * A `warning` diagnostic type.
     * These don't necessarily halt the compilation process, but they hint at a possible error in the programmer's code.
     */
    typedef DiagnosticTy<WARNING> Warning;

    /**
     * A `note` diagnostic type.
     * Should be used to supplement the `Error`/`Warning` diagnostic, and give useful inforation to solve the issue.
     */
    typedef DiagnosticTy<NOTE> Note;

    /**
     * A `help` diagnostic type.
     * Usually used to give useful hints at how to fix an issue.
     */
    typedef DiagnosticTy<HELP> Help;

    Diagnostic& Diagnostic::withNote(std::string message, Location location) { secondaries.push_back(Note("", message, location)); return *this; }
    Diagnostic& Diagnostic::withHelp(std::string message, Location location) { secondaries.push_back(Help("", message, location)); return *this; }
    Diagnostic& Diagnostic::withNote(std::string message) { return withNote(message, {}); }
    Diagnostic& Diagnostic::withHelp(std::string message) { return withHelp(message, {}); }
}

#endif /* DIAGNOSTIC_REPORTER_HPP_INCLUDED */