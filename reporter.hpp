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
        namespace attributes {
            const unsigned char bold      = 1 << 0;
            const unsigned char weak      = 1 << 1;
            const unsigned char italic    = 1 << 2;
            const unsigned char underline = 1 << 3;
            const unsigned char blink     = 1 << 4;
            const unsigned char reverse   = 1 << 5;
            const unsigned char cross     = 1 << 6;
        };

        /**
         * Utility class which represents a terminal color.
         * @note 'color' can mean both "red/green" and "bold/italic".
         */ 
        class Color {
            uint8_t fg, bg, attributes;
        public:
            Color() : fg(0), bg(0), attributes(0) {}

            /** 
             * @param code the ANSI escape code corresponding to this color.
             */
            Color(uint8_t fg, uint8_t bg) : fg(fg), bg(bg), attributes(0) {}
            Color(uint8_t fg, uint8_t bg, uint8_t attributes) : fg(fg), bg(bg), attributes(attributes) {}
            Color(const Color color, uint8_t attributes) : fg(color.fg), bg(color.bg), attributes(color.attributes | attributes) {}

            Color with(uint8_t attributes) const {
                return Color(*this, attributes);
            }

            Color with(const Color color) const {
                return Color(
                    color.fg ? color.fg : fg,
                    color.bg ? color.bg : bg,
                    color.attributes | attributes
                );
            }

            Color operator&(const Color color) const {
                return with(color);
            }

            /** 
             * Colors a string using an ANSI escape sequence, to be displayed in the terminal.
             * @param str string to be colored.
             * @return the colored string.
             */
            std::string operator()(std::string str) const {
                if (attributes & attributes::bold)      str = "\x1B[1m" + str + "\x1B[0m";
                if (attributes & attributes::weak)      str = "\x1B[2m" + str + "\x1B[0m";
                if (attributes & attributes::italic)    str = "\x1B[3m" + str + "\x1B[0m";
                if (attributes & attributes::underline) str = "\x1B[4m" + str + "\x1B[0m";
                if (attributes & attributes::blink)     str = "\x1B[5m" + str + "\x1B[0m";
                if (attributes & attributes::reverse)   str = "\x1B[9m" + str + "\x1B[0m";
                if (fg) str = "\x1B[" + std::to_string(fg) + "m" + str + "\x1B[0m";
                if (bg) str = "\x1B[" + std::to_string(bg) + "m" + str + "\x1B[0m";
                return str;
            }
        };
        const Color none;

        const Color bold      (none, attributes::bold);
        const Color weak      (none, attributes::weak);
        const Color italic    (none, attributes::italic);
        const Color underline (none, attributes::underline);
        const Color blink     (none, attributes::blink);
        const Color reverse   (none, attributes::reverse);

        const Color fgblack   (30, 0);
        const Color fgred     (31, 0);
        const Color fggreen   (32, 0);
        const Color fgyellow  (33, 0);
        const Color fgblue    (34, 0);
        const Color fgmagenta (35, 0);
        const Color fgcyan    (36, 0);
        const Color fgwhite   (37, 0);

        const Color bgblack   (0, 40);
        const Color bgred     (0, 41);
        const Color bggreen   (0, 42);
        const Color bgyellow  (0, 43);
        const Color bgblue    (0, 44);
        const Color bgmagenta (0, 45);
        const Color bgcyan    (0, 46);
        const Color bgwhite   (0, 47);
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

        virtual ~SourceFile() {}; 
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
        SimpleFile(std::string path) : _path(path) {}

        /**
         * @return path to the source file.
         */
        std::string str() { return _path; }
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

    enum class DisplayStyle { RICH, SHORT };

    class Config {
    public:
        DisplayStyle style;
        unsigned int tabWidth;
        struct {
            colors::Color error = colors::fgred & colors::bold;
            colors::Color warning = colors::fgyellow & colors::bold;
            colors::Color note = colors::fgblack & colors::bold;
            colors::Color help = colors::fgblue & colors::bold;
        } colors;
        
        struct {
            uint8_t left = 1;
            uint8_t right = 1;
        } lineNumPadding;

        Config() : style(DisplayStyle::RICH), tabWidth(4) { }
        Config(DisplayStyle style, unsigned int tabWidth) : style(style), tabWidth(tabWidth) {}

        static const Config& getDefault() {
            static const Config ret = Config();
            return ret;
        }
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
        static void indent(const Config& config, std::ostream& out, std::string& line, unsigned int count) {
            for (unsigned int i = 0; i < count; i++)
                out << (line[i] == '\t' ? std::string(config.tabWidth, ' ') : " ");
        }

        static std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return str;
        }

        static void printLine(const Config& config, std::ostream& out, std::string& line) {
            out << replaceAll(line, "\t", std::string(config.tabWidth, ' ')) << "\n";
        }

        static std::string getUnderline(const Config& config, unsigned char level) {
            switch (level) {
                case 0: return " ";
                case 1: return "~";
                case 2: return "=";
                case 3: return "#";
                case 4: return "*";
                default: return level%2 ? "-" : "+";
            }
        }

        std::string tyToString(const Config& config) {
            std::string str;
            switch (errTy) {
                case ERROR:   str = "Error";   break;
                case WARNING: str = "Warning"; break;
                case NOTE:    str = "Note";    break;
                case HELP:    str = "Help";    break;
                default:      str = "Internal Error"; break;
            }
            if (code != "")
                return str + "(" + code  + ")";
            else return str;
        }

        /* gets a string and colors it based on `errTy`*/
        std::string color(const Config& config, std::string str) {
            switch (errTy) {
                default:      return config.colors.error(str);
                case WARNING: return config.colors.warning(str);
                case NOTE:    return config.colors.note(str);
                case HELP:    return config.colors.help(str);
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
        void printPadding(const Config& config, std::ostream& out, unsigned int maxLine, unsigned int lastLine, unsigned int currLine, SourceFile *file) {
            unsigned int targetSize = std::to_string(maxLine).size() + config.lineNumPadding.left + config.lineNumPadding.right;
            if (lastLine + 2 == currLine) {
                auto str = std::string(config.lineNumPadding.left, ' ') + std::to_string(currLine - 1) + std::string(config.lineNumPadding.right, ' ');
                while (str.size() < targetSize)
                    str += " ";
                str += "│ ";
                out << color(config, str) << getLine(file->str(), currLine - 1) << "\n";            
            } else {
                switch (targetSize) {
                    case 3:  out << " " << color(config, "⋯") << "\n"; break;
                    case 4:  out << " " << color(config, "··") << "\n"; break;
                    default: out << " " << color(config, "···") << "\n"; break;
                }   
            }
        }

        /* prints the bars on the left with the correct indentation */
        void printLeft(const Config& config, std::ostream& out, unsigned int maxLine, bool printBar = true) {
            out << std::string(std::to_string(maxLine).size() + config.lineNumPadding.left + config.lineNumPadding.right, ' ');
            if (printBar)
                out << color(config, "│ ");
        }

        /* prints the `╭─ file.xyz ─╴` at the start of the file's diagnostics */
        void printTop(const Config& config, std::ostream& out, SourceFile* file, unsigned int maxLine) {
            printLeft(config, out, maxLine, false);
            out << color(config, "╭─ ") << file->str() << color(config, " ─╴") << "\n";
        }

        /* prints the `──╯` at the end of the file's diagnostics */
        void printBottom(const Config& config, std::ostream& out, unsigned int maxLine) {
            for (unsigned int i = 0; i < std::to_string(maxLine).size() + config.lineNumPadding.left + config.lineNumPadding.right; i++) 
                out << color(config, "─");
            out << color(config, "╯") << "\n";
        }

        /* prints the bars on the left with the correct indentation + with the line number */
        void printLeftWithLineNum(const Config& config, std::ostream& out, unsigned int lineNum, unsigned int maxLine, bool printBar = true) {
            unsigned int targetSize = std::to_string(maxLine).size() + config.lineNumPadding.left + config.lineNumPadding.right;
            auto str = std::string(config.lineNumPadding.left, ' ') + std::to_string(lineNum) + std::string(config.lineNumPadding.right, ' ');
            while (str.size() < targetSize)
                str += " ";
            out << color(config, str);
            if (printBar)
                out << color(config, "│ ");
        }

        /* prints all secondary messages on the current line */
        void printSecondariesOnLine(const Config& config, std::ostream& out, std::string &line, size_t &i, unsigned int maxLine) {
            auto &first = secondaries[i];
            printLeft(config, out, maxLine);
            if (i + 1 >= secondaries.size() || !onSameLine(first, secondaries[i + 1])) {
                // only one secondary concerning this line
                indent(config, out, line, first.loc.start);
                for (size_t idx = 0; idx < first.loc.end - first.loc.start; idx++)
                    out << first.color(config, getUnderline(config, 1));
                auto lines = splitLines(first.subMsg);
                for (size_t idx = 0; idx < lines.size(); idx++) {
                    if (idx != 0) {
                        printLeft(config, out, maxLine);
                        for (size_t j = 0; j < first.loc.end; j++)
                            out << (line[j] == '\t' ? std::string(config.tabWidth, ' ') : " ");
                    }
                    out << " " << first.color(config, lines[idx]) << "\n";
                }
                i++;
            } else {
                for (size_t lineIdx = 0; lineIdx < line.size(); lineIdx++) {
                    unsigned char count = 0;
                    Diagnostic* firstFound = nullptr;
                    for (auto idx = i; idx < secondaries.size() && onSameLine(secondaries[idx], first); idx++)
                        if (secondaries[idx].loc.start <= lineIdx && lineIdx < secondaries[idx].loc.end) {
                            count++;
                            if (!firstFound) firstFound = &secondaries[idx];
                        }
                    if (!firstFound)
                        out << getUnderline(config, count);
                    else out << firstFound->color(config, getUnderline(config, count));
                }
                out << "\n";
                for (; i < secondaries.size() && onSameLine(secondaries[i], first); i++) {
                    printLeft(config, out, maxLine);
                    for (size_t j = 0; j < secondaries[i].loc.start; j++) {
                        bool b = false;
                        for (auto idx = i; !b && idx < secondaries.size() && onSameLine(secondaries[idx], first); idx++)
                            if (secondaries[idx].loc.start == j) {
                                out << secondaries[idx].color(config, "│");
                                b = true;
                            }
                        if (!b) out << " ";
                    }
                    auto lines = splitLines(secondaries[i].subMsg);
                    for (size_t idx = 0; idx < lines.size(); idx++) {
                        if (idx == 0)
                            out << secondaries[i].color(config, "╰ " + lines[idx]) << "\n";
                        else {
                            printLeft(config, out, maxLine);
                            for (size_t j = 0; j < secondaries[i].loc.start; j++) {
                                bool b = false;

                                for (auto idx = i; !b && idx < secondaries.size() && onSameLine(secondaries[idx], first); idx++)
                                    if (secondaries[idx].loc.start == j) {
                                        out << secondaries[idx].color(config, "│");
                                        b = true;
                                    }
                                if (!b) out << " ";
                            }
                            out << secondaries[i].color(config, "  " + lines[idx]) << "\n";
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
        Diagnostic& print(std::ostream& out, const Config& config = Config::getDefault()) {

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
                out << color(config, tyToString(config) + ": ") << colors::bold(msg) << "\n";

            // we're finished if the diagnostic has no location
            if (loc.file == nullptr) return *this;

            // print the file the error is in
            printTop(config, out, loc.file, maxLine);

            size_t i = 0; // current index in `secondaries`
            unsigned int lastLine = 0; // the last line we rendered

            // first print all messages in the main file which come before the error
            while (i < secondaries.size() && secondaries[i].loc.file == loc.file && secondaries[i].loc.line < loc.line) {
                auto &secondary = secondaries[i];

                if (lastLine == 0) { // if we're rendering the first line in the file, print an empty line
                    printLeft(config, out, maxLine); 
                    out << "\n";
                } else if (lastLine < secondary.loc.line - 1)
                    printPadding(config, out, maxLine, lastLine, secondary.loc.line, secondary.loc.file);

                lastLine = secondary.loc.line;
                std::string line = getLine(loc.file->str(), secondary.loc.line);
                printLeftWithLineNum(config, out, secondary.loc.line, maxLine);
                printLine(config, out, line);
                printSecondariesOnLine(config, out, line, i, maxLine);
            }

            std::string line = getLine(loc.file->str(), loc.line);

            if (lastLine == 0 && !printAbove) {
                printLeft(config, out, maxLine);
                out << "\n";
            } else if (lastLine != 0 && lastLine < loc.line - 1)
                printPadding(config, out, maxLine, lastLine, loc.line, loc.file);
            lastLine = loc.line;
            
            if (printAbove) {
                if (subMsg != "") {
                    for (auto currLine : splitLines(subMsg)) {
                        printLeft(config, out, maxLine);
                        indent(config, out, line, loc.start);
                        out << color(config, currLine);
                        out << "\n";
                    }
                }
                
                printLeft(config, out, maxLine);

                indent(config, out, line, loc.start);
                for (unsigned int i = 0; i < loc.end - loc.start; i++)
                    out << color(config, "v");
                out << "\n";
            }
            
            printLeftWithLineNum(config, out, loc.line, maxLine);
            printLine(config, out, line);

            if (!printAbove && subMsg != "") {
                auto split = splitLines(subMsg);
                for (size_t i = 0; i < split.size(); i++) {
                    printLeft(config, out, maxLine);
                    indent(config, out, line, loc.start);
                    for (unsigned int j = 0; j < loc.end - loc.start; j++)
                        out << color(config, i == 0 ? "^" : " ");
                    out << " ";
                    out << color(config, split[i]) << "\n";
                }
            }
        
            if (i < secondaries.size() && onSameLine(secondaries[i], *this))
                printSecondariesOnLine(config, out, line, i, maxLine);
            
            auto currFile = loc.file;
            while (i < secondaries.size() && secondaries[i].loc.file) {
                auto &secondary = secondaries[i];

                if (secondary.loc.file->str() != currFile->str()) {
                    currFile = secondary.loc.file;
                    printBottom(config, out, maxLine);
                    printTop(config, out, currFile, maxLine);
                    printLeft(config, out, maxLine);
                    out << "\n";
                } else if (lastLine < secondary.loc.line - 1)
                    printPadding(config, out, maxLine, lastLine, secondary.loc.line, secondary.loc.file);

                lastLine = secondary.loc.line;
                std::string line = getLine(currFile->str(), secondary.loc.line);
                printLeftWithLineNum(config, out, secondary.loc.line, maxLine);
                printLine(config, out, line);
                printSecondariesOnLine(config, out, line, i, maxLine);
            }

            printBottom(config, out, maxLine); 
            for (; i < secondaries.size(); i++) {
                auto& secondary = secondaries[i];
                printLeft(config, out, maxLine, false);
                out << secondary.color(config, "• " + secondary.tyToString(config) + secondary.color(config, ": "));
                auto lines = splitLines(secondary.subMsg);
                for (size_t idx = 0; idx < lines.size(); idx++) {
                    if (idx != 0) {
                        printLeft(config, out, maxLine, false);
                        for (size_t j = 0; j < secondary.tyToString(config).size() + 4; j++)
                            out << " ";
                    }
                    printLine(config, out, lines[idx]);
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