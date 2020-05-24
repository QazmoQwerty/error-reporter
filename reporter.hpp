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

#ifndef ERROR_REPORTER_HPP_INCLUDED
#define ERROR_REPORTER_HPP_INCLUDED

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
        INTERNAL_ERROR = 0,
        ERROR          = 1,
        WARNING        = 2,
        NOTE           = 3,
        HELP           = 4,
        UNKNOWN        = 5,
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
     * TODO - explain
     */
    class Diagnostic {
    private:
        static std::vector<std::string> splitLines(const std::string& str);
        static std::string getLine(std::string fileName, int line);
        static bool onSameLine(Diagnostic& a, Diagnostic& b);

        std::string msg;
        std::string subMsg;
        Location loc;
        std::vector<Diagnostic> secondaries;

        void sortSecondaries();
        void showSecondariesOnLine(std::ostream& out, std::string &line, size_t &i, unsigned int maxLine);
        void printIndent(std::ostream& out, unsigned int maxLine, bool showBar = true);
        void printIndentWithLineNum(std::ostream& out, unsigned int lineNum, unsigned int maxLine, bool showBar = true);
        void printIndentWithLineNum(std::ostream& out, unsigned int maxLine, bool showBar = true);
        void printPaddingLine(std::ostream& out, unsigned int maxLine, unsigned int line = 0, SourceFile *file = nullptr);

    protected:
        DiagnosticType errTy;
        std::string code;
        std::string tyToString();
        std::string color(std::string str);

        Diagnostic(DiagnosticType ty, std::string code, std::string message, std::string subMessage, Location location) 
               : msg(message), subMsg(subMessage), loc(location), errTy(ty), code(code) {};
        Diagnostic(DiagnosticType ty, std::string message, std::string subMessage, Location location) : Diagnostic(ty, "", message, subMessage, location) {};
        Diagnostic(DiagnosticType ty, std::string message, Location location) : Diagnostic(ty, message, "", location) {};
        Diagnostic(DiagnosticType ty, std::string message) : Diagnostic(ty, message, {}) {};

    public:
        /**
         * Pretty-print the diagnostic.
         * @param out stream in which to print the error.
         * @return the object which this function was called upon.
         */
        Diagnostic& print(std::ostream& out);

        /**
         * Adds a secondary note message to the diagnostic at `location`.
         * @param message the note message.
         * @param location source code location of the note message.
         * @return the object which this function was called upon.
         */
        Diagnostic& withNote(std::string message, Location location);

        /**
         * Adds a secondary help message to the diagnostic at `location`.
         * @param message the help message.
         * @param location source code location of the help message.
         * @return the object which this function was called upon.
         */
        Diagnostic& withHelp(std::string message, Location location);

        /**
         * Adds a secondary note message to the diagnostic without a specific location.
         * @param message the note message.
         * @return the object which this function was called upon.
         */
        Diagnostic& withNote(std::string message);

        /**
         * Adds a secondary help message to the diagnostic without a specific location.
         * @param message the help message.
         * @return the object which this function was called upon.
         */
        Diagnostic& withHelp(std::string message);        
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
         * @param subMessage the secondary message which is shown directly next to the source code.
         * @param location the location the diagnostic is concerning.
         */
        DiagnosticTy<T>(std::string message, std::string subMessage, Location location) : Diagnostic(T, message, subMessage, location) {}

        /**
         * Constructs a diagnostic at a specific source code location with both a primary message and a submessage, as well as a custom error code.
         * @param code the error code, can be anything but is usually something like `"E101"` or `"W257"`, for example.
         * @param message the diagnostic message - should essentially be the 'title' of the diagnostic without going into too much detail.
         * @param subMessage the secondary message which is shown directly next to the source code.
         * @param location the location the diagnostic is concerning.
         */
        DiagnosticTy<T>(std::string code, std::string message, std::string subMessage, Location location) : Diagnostic(T, code, message, subMessage, location) {}

        /**
         * Pretty-print the diagnostic.
         * @param out stream in which to print the error.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& print(std::ostream& out)                         { Diagnostic::print(out);                  return *this; };

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
        DiagnosticTy<T>& withNote(std::string message)                    { Diagnostic::withNote(message);           return *this; };

        /**
         * Adds a secondary help message to the diagnostic without a specific location.
         * @param message the help message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withHelp(std::string message)                    { Diagnostic::withHelp(message);           return *this; };
    };

    /////////////////////////////////////////////////////////////////////////

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

    /////////////////////////////////////////////////////////////////////////

    /*************************** implementations ***************************/

    /* get a specific line from a text file */
    std::string Diagnostic::getLine(std::string fileName, int line) {
        std::fstream file(fileName);
        file.seekg(std::ios::beg);
        for (int i=0; i < line - 1; ++i)
            file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::string ret;
        std::getline(file, ret);
        return ret;
    }
    
    /* split a string into its lines */
    std::vector<std::string> Diagnostic::splitLines(const std::string& str) {
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

    /* returns whether the two diagnostics are on the same line */
    bool Diagnostic::onSameLine(Diagnostic& a, Diagnostic& b) {
        return a.loc.file == b.loc.file && a.loc.line == b.loc.line;
    }

    /////////////////////////////////////////////////////////////////////////

    Diagnostic& Diagnostic::withNote(std::string message, Location location) { secondaries.push_back(Note("", message, location)); return *this; }
    Diagnostic& Diagnostic::withHelp(std::string message, Location location) { secondaries.push_back(Help("", message, location)); return *this; }
    Diagnostic& Diagnostic::withNote(std::string message) { return withNote(message, {}); }
    Diagnostic& Diagnostic::withHelp(std::string message) { return withHelp(message, {}); }

    /////////////////////////////////////////////////////////////////////////

    Diagnostic& Diagnostic::print(std::ostream& out) {
        // find the maximum line (to know by how much to indent the bars)
        auto maxLine = loc.line;
        for (auto& secondary : secondaries)
            if (secondary.loc.line > maxLine)
                maxLine = secondary.loc.line;

        sortSecondaries();
        bool showAbove = false; // if there are any messages on the line of the error, point to the error from above instead

        for (size_t i = 0; !showAbove && i < secondaries.size(); i++)
            if (onSameLine(secondaries[i], *this))
                showAbove = true;

        // print the main error message
        if (msg != "")
            out << color(tyToString() + ": ") << colors::bold(msg) << "\n";

        if (loc.file == nullptr) return *this;

        // print the file the error is in
        printIndent(out, maxLine, false);
        out << color("╭─ ") << loc.file->str() << color(" ─╴") << "\n";

        bool isFirst = true;
        unsigned int lastLine = 0;
        size_t i = 0;

        // first print all messages in the main file which come before the error
        while (i < secondaries.size() && secondaries[i].loc.file == loc.file && secondaries[i].loc.line < loc.line) {
            auto &secondary = secondaries[i];
            
            if (isFirst) {
                printIndent(out, maxLine);
                out << "\n";
                isFirst = false;
            } else if (lastLine < secondary.loc.line - 1)  {
                if (lastLine == secondary.loc.line - 2)
                    printPaddingLine(out, maxLine, secondary.loc.line - 1, secondary.loc.file);
                else printPaddingLine(out, maxLine);
            }
            lastLine = secondary.loc.line;
            std::string line = getLine(loc.file->str(), secondary.loc.line);
            printIndentWithLineNum(out, secondary.loc.line, maxLine);
            out << line << "\n";
            showSecondariesOnLine(out, line, i, maxLine);
        }

        std::string line = getLine(loc.file->str(), loc.line);

        if (isFirst && !showAbove) {
            printIndent(out, maxLine);
            out << "\n";
        } else if (lastLine != 0 && lastLine < loc.line - 1) {
            if (lastLine == loc.line - 2)
                printPaddingLine(out, maxLine, loc.line - 1, loc.file);
            else printPaddingLine(out, maxLine);
        }
        lastLine = loc.line;
        
        if (showAbove) {
            if (subMsg != "") {
                for (auto currLine : splitLines(subMsg)) {
                    printIndent(out, maxLine);
                    for (unsigned int i = 0; i < loc.start; i++)
                        out << (line[i] == '\t' ? "\t" : " ");
                    out << color(currLine);
                    out << "\n";
                }
            }
            
            printIndent(out, maxLine);

            for (unsigned int i = 0; i < loc.start; i++)
                out << (line[i] == '\t' ? "\t" : " ");
            for (unsigned int i = 0; i < loc.end - loc.start; i++)
                out << color("v");
            out << "\n";
        }
        
        printIndentWithLineNum(out, maxLine);
        out << line << "\n";

        if (!showAbove && subMsg != "") {
            auto split = splitLines(subMsg);
            for (size_t i = 0; i < split.size(); i++) {
                printIndent(out, maxLine);
                for (unsigned int i = 0; i < loc.start; i++)
                    out << (line[i] == '\t' ? "\t" : " ");
                for (unsigned int j = 0; j < loc.end - loc.start; j++)
                    out << color(i == 0 ? "^" : " ");
                out << " ";
                out << color(split[i]) << "\n";
            }
        }
      
        if (i < secondaries.size() && onSameLine(secondaries[i], *this))
            showSecondariesOnLine(out, line, i, maxLine);
        
        auto currFile = loc.file;
        while (i < secondaries.size() && secondaries[i].loc.file) {
            auto &secondary = secondaries[i];

            if (secondary.loc.file->str() != currFile->str()) {
                currFile = secondary.loc.file;
                for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++) out << color("─");
                out << color("╯") << "\n";
                printIndent(out, maxLine, false);
                out << color("╭─ ") << currFile->str() << color(" ─╴") << "\n";
                printIndent(out, maxLine);
                out << "\n";
            } else if (lastLine == secondary.loc.line - 2)
                printPaddingLine(out, maxLine, secondary.loc.line - 1, secondary.loc.file);
            else if (lastLine < secondary.loc.line - 1)
                printPaddingLine(out, maxLine);

            lastLine = secondary.loc.line;
            std::string line = getLine(currFile->str(), secondary.loc.line);
            printIndentWithLineNum(out, secondary.loc.line, maxLine);
            out << line << "\n";
            showSecondariesOnLine(out, line, i, maxLine);
        }

        for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++) out << color("─");
        out << color("╯") << "\n"; 
        for (; i < secondaries.size(); i++) {
            auto& secondary = secondaries[i];
            printIndent(out, maxLine, false);
            out << secondary.color("• " + secondary.tyToString() + secondary.color(": "));
            auto lines = splitLines(secondary.subMsg);
            for (size_t idx = 0; idx < lines.size(); idx++) {
                if (idx != 0) {
                    printIndent(out, maxLine, false);
                    for (size_t j = 0; j < secondary.tyToString().size() + 4; j++)
                        out << " ";
                }
                out << lines[idx] << "\n";
            }
        }
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////

    /* show all the secondary messages on the current line */
    void Diagnostic::showSecondariesOnLine(std::ostream& out, std::string &line, size_t &i, unsigned int maxLine) {
        auto &first = secondaries[i];
        printIndent(out, maxLine);
        if (i + 1 >= secondaries.size() || !onSameLine(first, secondaries[i + 1])) {
            // only one secondary concerning this line
            for (size_t idx = 0; idx < first.loc.start; idx++)
                out << (line[idx] == '\t' ? '\t' : ' ');
            for (size_t idx = 0; idx < first.loc.end - first.loc.start; idx++)
                out << first.color("~");
            auto lines = splitLines(first.subMsg);
            for (size_t idx = 0; idx < lines.size(); idx++) {
                if (idx != 0) {
                    printIndent(out, maxLine);
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
                printIndent(out, maxLine);
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
                        printIndent(out, maxLine);
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

    /////////////////////////////////////////////////////////////////////////

    /* sort the vector of secondary messages based on the order we want to be printing them */
    void Diagnostic::sortSecondaries() {
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

    /////////////////////////////////////////////////////////////////////////

    /* a 'padding' line is an irrelevant line in between two other relevant lines */
    void Diagnostic::printPaddingLine(std::ostream& out, unsigned int maxLine, unsigned int line, SourceFile *file) {
        unsigned int targetSize = std::to_string(maxLine).size() + 2;
        if (line == 0) {
            switch (targetSize) {
                case 3:  out << " " << color("⋯") << "\n"; break;
                case 4:  out << " " << color("··") << "\n"; break;
                default: out << " " << color("···") << "\n"; break;
            }   
        } else {
            auto str = " " + std::to_string(line) + " ";
            while (str.size() < targetSize)
                str += " ";
            str += "│ ";
            out << color(str);
            if (file) out << getLine(file->str(), line);
            out << "\n";
        }
    }

    /* prints the bars on the left with the correct indentation */
    void Diagnostic::printIndent(std::ostream& out, unsigned int maxLine, bool showBar) {
        for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++)
            out << " ";
        if (showBar)
            out << color("│ ");
    }

    /////////////////////////////////////////////////////////////////////////

    /* prints the bars on the left with the correct indentation + with the line number */
    void Diagnostic::printIndentWithLineNum(std::ostream& out, unsigned int lineNum, unsigned int maxLine, bool showBar) {
        unsigned int targetSize = std::to_string(maxLine).size() + 2;
        auto str = " " + std::to_string(lineNum) + " ";
        while (str.size() < targetSize)
            str += " ";
        out << color(str);
        if (showBar)
            out << color("│ ");
    }

    /* prints the bars on the left with the correct indentation + with the line number */
    void Diagnostic::printIndentWithLineNum(std::ostream& out, unsigned int maxLine, bool showBar) {
        printIndentWithLineNum(out, loc.line, maxLine, showBar);
    }

    /////////////////////////////////////////////////////////////////////////

    std::string Diagnostic::tyToString() {
        std::string str;
        switch (errTy) {
            case ERROR:   return code == "" ? "Error" : "Error(" + code + ")";
            case WARNING: return code == "" ? "Warning" : "Warning(" + code + ")";
            case NOTE:    return code == "" ? "Note" : "Note(" + code + ")";;
            case HELP:    return code == "" ? "Help" : "Help(" + code + ")";;
            default:      return code == "" ? "Internal Error" : "Internal Error(" + code + ")";
        }
    }

    std::string Diagnostic::color(std::string str) {
        switch (errTy) {
            default:      return colors::bold(colors::red(str));
            case WARNING: return colors::bold(colors::yellow(str));
            case NOTE:    return colors::bold(colors::black(str));
            case HELP:    return colors::bold(colors::blue(str));
        }
    }
}

#endif /* ERROR_REPORTER_HPP_INCLUDED */