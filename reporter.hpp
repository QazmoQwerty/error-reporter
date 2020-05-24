#ifndef ERROR_REPORTER_HPP_INCLUDED
#define ERROR_REPORTER_HPP_INCLUDED

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

namespace reporter {

    namespace colors {
        class Color {
            unsigned char _code;
        public:
            Color(unsigned char code) : _code(code) {}
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

    enum ErrorCode {
        INTERNAL_ERROR = 0,
        ERROR          = 1,
        WARNING        = 2,
        NOTE           = 3,
        HELP           = 4,
        UNKNOWN        = 5,
    };

    class SourceFile {
    public: 
        virtual std::string str() = 0;
    };

    /*
        A minimal SourceFile class to get you started
    */
    class SimpleFile : public SourceFile {
    private:
        std::string _path;
    public:     
        SimpleFile(std::string path) : _path(path) {};
        std::string str() { return _path; };
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

        Position(unsigned int line, unsigned int start, unsigned int end, SourceFile* file)
                : line(line), start(start), end(end), file(file) {
            if (this->end <= this->start)
                this->end = this->start + 1;
        }

        Position(unsigned int line, unsigned int pos, SourceFile* file) : Position(line, pos, pos + 1, file) {}

        Position() : Position(0, 0, 0, nullptr) {}
    };

    class Diagnostic {
    private:
        static std::vector<std::string> splitLines(const std::string& str);
        static std::string getLine(std::string fileName, int line);

        std::string msg;
        std::string subMsg;
        Position pos;
        std::vector<Diagnostic> secondaries;

        void sortSecondaries();
        void showSecondariesOnLine(std::ostream& out, std::string &line, size_t &i, unsigned int maxLine);
        void printIndent(std::ostream& out, unsigned int maxLine, bool showBar = true);
        void printIndentWithLineNum(std::ostream& out, unsigned int lineNum, unsigned int maxLine, bool showBar = true);
        void printIndentWithLineNum(std::ostream& out, unsigned int maxLine, bool showBar = true);
        void printPaddingLine(std::ostream& out, unsigned int maxLine, unsigned int line = 0, SourceFile *file = nullptr);

    protected:
        ErrorCode errTy;
        std::string code;
        virtual std::string tyToString();
        virtual std::string color(std::string str);
    
    public:
        Diagnostic(ErrorCode ty, std::string code, std::string message, std::string subMessage, Position position) 
               : msg(message), subMsg(subMessage), pos(position), errTy(ty), code(code) {};
        
        Diagnostic(ErrorCode ty, std::string message, std::string subMessage, Position position) : Diagnostic(ty, "", message, subMessage, position) {};
        Diagnostic(ErrorCode ty, std::string message, Position position) : Diagnostic(ty, message, "", position) {};
        Diagnostic(ErrorCode ty, std::string message) : Diagnostic(ty, message, {}) {};

        Diagnostic& print(std::ostream& out);
        Diagnostic& withNote(std::string message, Position position);
        Diagnostic& withHelp(std::string message, Position position);
        Diagnostic& withNote(std::string message);
        Diagnostic& withHelp(std::string message);
    };

    class Error : public Diagnostic {
    public:
        Error(std::string message)                                                              : Diagnostic(ERROR, message) {}
        Error(std::string message, Position position)                                           : Diagnostic(ERROR, message, position) {}
        Error(std::string message, std::string subMessage, Position position)                   : Diagnostic(ERROR, message, subMessage, position) {}
        Error(std::string code, std::string message, std::string subMessage, Position position) : Diagnostic(ERROR, code, message, subMessage, position) {}

        Error& print(std::ostream& out)                         { Diagnostic::print(out);                  return *this; };
        Error& withNote(std::string message, Position position) { Diagnostic::withNote(message, position); return *this; };
        Error& withHelp(std::string message, Position position) { Diagnostic::withHelp(message, position); return *this; };
        Error& withNote(std::string message)                    { Diagnostic::withNote(message);           return *this; };
        Error& withHelp(std::string message)                    { Diagnostic::withHelp(message);           return *this; };

    private:
        std::string tyToString() { return code == "" ? "Error" : "Error(" + code + ")"; }
        std::string color(std::string str) { return colors::bold(colors::red(str)); }
    };

    class Warning : public Diagnostic {
    public:
        Warning(std::string message)                                                              : Diagnostic(WARNING, message) {}
        Warning(std::string message, Position position)                                           : Diagnostic(WARNING, message, position) {}
        Warning(std::string message, std::string subMessage, Position position)                   : Diagnostic(WARNING, message, subMessage, position) {}
        Warning(std::string code, std::string message, std::string subMessage, Position position) : Diagnostic(WARNING, code, message, subMessage, position) {}

        Warning& print(std::ostream& out)                         { Diagnostic::print(out);                  return *this; };
        Warning& withNote(std::string message, Position position) { Diagnostic::withNote(message, position); return *this; };
        Warning& withHelp(std::string message, Position position) { Diagnostic::withHelp(message, position); return *this; };
        Warning& withNote(std::string message)                    { Diagnostic::withNote(message);           return *this; };
        Warning& withHelp(std::string message)                    { Diagnostic::withHelp(message);           return *this; };
    private:
        std::string tyToString() { return code == "" ? "Warning" : "Warning(" + code + ")"; }
        std::string color(std::string str) { return colors::bold(colors::yellow(str)); }
    };

    class Note : public Diagnostic {
    public:
        Note(std::string message)                                                              : Diagnostic(NOTE, message) {}
        Note(std::string message, Position position)                                           : Diagnostic(NOTE, message, position) {}
        Note(std::string message, std::string subMessage, Position position)                   : Diagnostic(NOTE, message, subMessage, position) {}
        Note(std::string code, std::string message, std::string subMessage, Position position) : Diagnostic(NOTE, code, message, subMessage, position) {}

        Note& print(std::ostream& out)                         { Diagnostic::print(out);                  return *this; };
        Note& withNote(std::string message, Position position) { Diagnostic::withNote(message, position); return *this; };
        Note& withHelp(std::string message, Position position) { Diagnostic::withHelp(message, position); return *this; };
        Note& withNote(std::string message)                    { Diagnostic::withNote(message);           return *this; };
        Note& withHelp(std::string message)                    { Diagnostic::withHelp(message);           return *this; };
    private:
        std::string tyToString() { return code == "" ? "Note" : "Note(" + code + ")"; }
        std::string color(std::string str) { return colors::bold(colors::black(str)); }
    };

    class Help : public Diagnostic {
    public:
        Help(std::string message)                                                              : Diagnostic(HELP, message) {}
        Help(std::string message, Position position)                                           : Diagnostic(HELP, message, position) {}
        Help(std::string message, std::string subMessage, Position position)                   : Diagnostic(HELP, message, subMessage, position) {}
        Help(std::string code, std::string message, std::string subMessage, Position position) : Diagnostic(HELP, code, message, subMessage, position) {}

        Help& print(std::ostream& out)                         { Diagnostic::print(out);                  return *this; };
        Help& withNote(std::string message, Position position) { Diagnostic::withNote(message, position); return *this; };
        Help& withHelp(std::string message, Position position) { Diagnostic::withHelp(message, position); return *this; };
        Help& withNote(std::string message)                    { Diagnostic::withNote(message);           return *this; };
        Help& withHelp(std::string message)                    { Diagnostic::withHelp(message);           return *this; };
    private:
        std::string tyToString() { return code == "" ? "Help" : "Help(" + code + ")"; }
        std::string color(std::string str) { return colors::bold(colors::blue(str)); }
    };

    /******************** implementations ********************/

    std::string Diagnostic::getLine(std::string fileName, int line) {
        std::fstream file(fileName);
        file.seekg(std::ios::beg);
        for (int i=0; i < line - 1; ++i)
            file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::string ret;
        std::getline(file, ret);
        return ret;
    }
    
    std::vector<std::string> Diagnostic::splitLines(const std::string& str) {
        std::vector<std::string> strings;
        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = str.find("\n", prev)) != std::string::npos) {
            strings.push_back(str.substr(prev, pos - prev));
            prev = pos + 1;
        }
        // To get the last substring (or only, if delimiter is not found)
        strings.push_back(str.substr(prev));
        return strings;
    }

    #define ON_SAME_LINE(a, b) ((a).pos.file == (b).pos.file && (a).pos.line == (b).pos.line)

    Diagnostic& Diagnostic::withNote(std::string message, Position position) { secondaries.push_back(Note("", message, position)); return *this; }
    Diagnostic& Diagnostic::withHelp(std::string message, Position position) { secondaries.push_back(Help("", message, position)); return *this; }
    Diagnostic& Diagnostic::withNote(std::string message) { return withNote(message, {}); }
    Diagnostic& Diagnostic::withHelp(std::string message) { return withHelp(message, {}); }

    Diagnostic& Diagnostic::print(std::ostream& out) {
        // find the maximum line (to know by how much to indent the bars)
        auto maxLine = pos.line;
        for (auto& secondary : secondaries)
            if (secondary.pos.line > maxLine)
                maxLine = secondary.pos.line;

        sortSecondaries();
        bool showAbove = false; // if there are any messages on the line of the error, point to the error from above instead

        for (size_t i = 0; !showAbove && i < secondaries.size(); i++)
            if (ON_SAME_LINE(secondaries[i], *this))
                showAbove = true;

        // print the main error message
        if (msg != "")
            out << color(tyToString() + ": ") << colors::bold(msg) << "\n";

        if (pos.file == nullptr) return *this;

        // print the file the error is in
        printIndent(out, maxLine, false);
        out << color("╭─ ") << pos.file->str() << color(" ─╴") << "\n";

        bool isFirst = true;
        unsigned int lastLine = 0;
        size_t i = 0;

        // first print all messages in the main file which come before the error
        while (i < secondaries.size() && secondaries[i].pos.file == pos.file && secondaries[i].pos.line < pos.line) {
            auto &secondary = secondaries[i];
            
            if (isFirst) {
                printIndent(out, maxLine);
                out << "\n";
                isFirst = false;
            } else if (lastLine < secondary.pos.line - 1)  {
                if (lastLine == secondary.pos.line - 2)
                    printPaddingLine(out, maxLine, secondary.pos.line - 1, secondary.pos.file);
                else printPaddingLine(out, maxLine);
            }
            lastLine = secondary.pos.line;
            std::string line = getLine(pos.file->str(), secondary.pos.line);
            printIndentWithLineNum(out, secondary.pos.line, maxLine);
            out << line << "\n";
            showSecondariesOnLine(out, line, i, maxLine);
        }

        std::string line = getLine(pos.file->str(), pos.line);

        if (isFirst && !showAbove) {
            printIndent(out, maxLine);
            out << "\n";
        } else if (lastLine != 0 && lastLine < pos.line - 1) {
            if (lastLine == pos.line - 2)
                printPaddingLine(out, maxLine, pos.line - 1, pos.file);
            else printPaddingLine(out, maxLine);
        }
        lastLine = pos.line;
        
        if (showAbove) {
            if (subMsg != "") {
                for (auto currLine : splitLines(subMsg)) {
                    printIndent(out, maxLine);
                    for (unsigned int i = 0; i < pos.start; i++)
                        out << (line[i] == '\t' ? "\t" : " ");
                    out << color(currLine);
                    out << "\n";
                }
            }
            
            printIndent(out, maxLine);

            for (unsigned int i = 0; i < pos.start; i++)
                out << (line[i] == '\t' ? "\t" : " ");
            for (unsigned int i = 0; i < pos.end - pos.start; i++)
                out << color("v");
            out << "\n";
        }
        
        printIndentWithLineNum(out, maxLine);
        out << line << "\n";

        if (!showAbove && subMsg != "") {
            auto split = splitLines(subMsg);
            for (int i = 0; i < split.size(); i++) {
                printIndent(out, maxLine);
                for (unsigned int i = 0; i < pos.start; i++)
                    out << (line[i] == '\t' ? "\t" : " ");
                for (unsigned int j = 0; j < pos.end - pos.start; j++)
                    out << color(i == 0 ? "^" : " ");
                out << " ";
                out << color(split[i]) << "\n";
            }
        }
      
        if (i < secondaries.size() && ON_SAME_LINE(secondaries[i], *this))
            showSecondariesOnLine(out, line, i, maxLine);
        
        auto currFile = pos.file;
        while (i < secondaries.size() && secondaries[i].pos.file) {
            auto &secondary = secondaries[i];

            if (secondary.pos.file->str() != currFile->str()) {
                currFile = secondary.pos.file;
                for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++) out << color("─");
                out << color("╯") << "\n";
                printIndent(out, maxLine, false);
                out << color("╭─ ") << currFile->str() << color(" ─╴") << "\n";
                printIndent(out, maxLine);
                out << "\n";
            }

            if (lastLine == secondary.pos.line - 2)
                printPaddingLine(out, maxLine, secondary.pos.line - 1, secondary.pos.file);
            else if (lastLine < secondary.pos.line - 1)
                printPaddingLine(out, maxLine);

            lastLine = secondary.pos.line;
            std::string line = getLine(currFile->str(), secondary.pos.line);
            printIndentWithLineNum(out, secondary.pos.line, maxLine);
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

    void Diagnostic::showSecondariesOnLine(std::ostream& out, std::string &line, size_t &i, unsigned int maxLine) {
        auto &first = secondaries[i];
        printIndent(out, maxLine);
        if (i + 1 >= secondaries.size() || !ON_SAME_LINE(first, secondaries[i + 1])) {
            // only one secondary concerning this line
            for (size_t idx = 0; idx < first.pos.start; idx++)
                out << (line[idx] == '\t' ? '\t' : ' ');
            for (size_t idx = 0; idx < first.pos.end - first.pos.start; idx++)
                out << first.color("~");
            auto lines = splitLines(first.subMsg);
            for (size_t idx = 0; idx < lines.size(); idx++) {
                if (idx != 0) {
                    printIndent(out, maxLine);
                    for (size_t j = 0; j < first.pos.end; j++)
                        out << (line[j] == '\t' ? '\t' : ' ');
                }
                out << " " << first.color(lines[idx]) << "\n";
            }
            i++;
        } else {
            for (size_t lineIdx = 0; lineIdx < line.size(); lineIdx++) {
                bool b = false;
                for (auto idx = i; !b && idx < secondaries.size() && ON_SAME_LINE(secondaries[idx], first); idx++)
                    if (secondaries[idx].pos.start <= lineIdx && lineIdx < secondaries[idx].pos.end) {
                        out << secondaries[idx].color("~");
                        b = true;
                    }
                if (!b) out << " ";
            }
            out << "\n";
            for (; i < secondaries.size() && ON_SAME_LINE(secondaries[i], first); i++) {
                printIndent(out, maxLine);
                for (size_t j = 0; j < secondaries[i].pos.start; j++) {
                    bool b = false;
                    for (auto idx = i; !b && idx < secondaries.size() && ON_SAME_LINE(secondaries[idx], first); idx++)
                        if (secondaries[idx].pos.start == j) {
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
                        for (size_t j = 0; j < secondaries[i].pos.start; j++) {
                            bool b = false;

                            for (auto idx = i; !b && idx < secondaries.size() && ON_SAME_LINE(secondaries[idx], first); idx++)
                                if (secondaries[idx].pos.start == j) {
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

    void Diagnostic::sortSecondaries() {
        auto file = pos.file;
        std::sort(
            std::begin(secondaries), std::end(secondaries), 
            [file](Diagnostic &a, Diagnostic &b) {
                if (!a.pos.file) 
                    return false;
                if (!b.pos.file) 
                    return true;
                if (a.pos.file == file && b.pos.file != file)
                    return true;
                if (a.pos.file != file && b.pos.file == file)
                    return false;
                if (a.pos.file != b.pos.file)
                    return a.pos.file->str() < b.pos.file->str();
                if (a.pos.line == b.pos.line)
                    return a.pos.start > b.pos.start; 
                return a.pos.line < b.pos.line;
            }
        );
    }

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

    void Diagnostic::printIndent(std::ostream& out, unsigned int maxLine, bool showBar) {
        for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++)
            out << " ";
        if (showBar)
            out << color("│ ");
    }

    void Diagnostic::printIndentWithLineNum(std::ostream& out, unsigned int lineNum, unsigned int maxLine, bool showBar) {
        unsigned int targetSize = std::to_string(maxLine).size() + 2;
        auto str = " " + std::to_string(pos.line) + " ";
        while (str.size() < targetSize)
            str += " ";
        out << color(str);
        if (showBar)
            out << color("│ ");
    }

    void Diagnostic::printIndentWithLineNum(std::ostream& out, unsigned int maxLine, bool showBar) {
        printIndentWithLineNum(out, pos.line, maxLine, showBar);
    }

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