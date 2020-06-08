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

        /* font style attributes (bold/italic/etc.) */
        namespace attributes {
            const uint8_t bold      = 1 << 0;
            const uint8_t weak      = 1 << 1;
            const uint8_t italic    = 1 << 2;
            const uint8_t underline = 1 << 3;
            const uint8_t blink     = 1 << 4;
            const uint8_t reverse   = 1 << 5;
            const uint8_t cross     = 1 << 6;

            // inherit is used to tell the reporter to use the diagnostic's type's color.
            const uint8_t inherit   = 1 << 7;
        }

        /**
         * Utility class which represents a terminal color.
         * @note "color" can mean both "red/green" and "bold/italic".
         */ 
        class Color {
            uint8_t _fg; // foreground color (ansi codes 30-37)
            uint8_t _bg; // background color (ansi codes 40-47)
            uint8_t _attributes;
        public:
            Color() : _fg(0), _bg(0), _attributes(0) {}
            Color(uint8_t fg, uint8_t bg) : _fg(fg), _bg(bg), _attributes(0) {}
            Color(uint8_t fg, uint8_t bg, uint8_t attributes) : _fg(fg), _bg(bg), _attributes(attributes) {}
            Color(const Color color, uint8_t attributes) : _fg(color._fg), _bg(color._bg), _attributes(color._attributes | attributes) {}

            /** 
             * @return this color, in addition with the specified attributes.
             */
            Color with(uint8_t attributes) const {
                return Color(*this, attributes);
            }

            /** 
             * @return this color, in addition with the specified attributes.
             */
            Color operator&(uint8_t attributes) const {
                return with(attributes);
            }

            /** 
             * @return this color, with `color`'s foreground, background, and attributes. Any unspecified options will remain unchanged.
             */
            Color with(const Color color) const {
                return Color(
                    color._fg ? color._fg : _fg,
                    color._bg ? color._bg : _bg,
                    color._attributes | _attributes
                );
            }

            /** 
             * @return this color, with `color`'s foreground, background, and attributes. Any unspecified options will remain unchanged.
             */
            Color operator&(const Color color) const {
                return with(color);
            }

            /** 
             * Check color equality.
             * @return true if background color, foreground color, and attributes are the same.
             */
            bool operator==(const Color color) const {
                return _bg == color._bg &&
                       _fg == color._fg &&
                       _attributes == color._attributes;
            }

            /** 
             * Colors a string using an ANSI escape sequence, to be displayed in the terminal.
             * @param str string to be colored.
             * @return the colored string.
             */
            std::string operator()(std::string str) const {
                if (_attributes & attributes::bold)      str = "\x1B[1m" + str + "\x1B[0m";
                if (_attributes & attributes::weak)      str = "\x1B[2m" + str + "\x1B[0m";
                if (_attributes & attributes::italic)    str = "\x1B[3m" + str + "\x1B[0m";
                if (_attributes & attributes::underline) str = "\x1B[4m" + str + "\x1B[0m";
                if (_attributes & attributes::blink)     str = "\x1B[5m" + str + "\x1B[0m";
                if (_attributes & attributes::reverse)   str = "\x1B[9m" + str + "\x1B[0m";
                if (_fg) str = "\x1B[" + std::to_string(_fg) + "m" + str + "\x1B[0m";
                if (_bg) str = "\x1B[" + std::to_string(_bg) + "m" + str + "\x1B[0m";
                return str;
            }
        };

        const Color none; // default terminal color
        const Color inherit   = none & attributes::inherit; // inherit is used to tell the reporter to use the diagnostic's type's color
        const Color bold      = none & attributes::bold;
        const Color weak      = none & attributes::weak;
        const Color italic    = none & attributes::italic;
        const Color underline = none & attributes::underline;
        const Color blink     = none & attributes::blink;
        const Color reverse   = none & attributes::reverse;

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

        virtual ~SourceFile() {}

        /* get a specific line from the file */
        virtual std::string getLine(uint32_t line) {
            std::fstream file(str());
            file.seekg(std::ios::beg);
            for (uint32_t i=0; i < line - 1; ++i)
                file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::string ret;
            std::getline(file, ret);
            return ret;
        }
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
    enum class DiagnosticType {
        INTERNAL_ERROR,
        ERROR,
        WARNING,
        NOTE,
        HELP,
        UNKNOWN
    };

    /**
     * A location of source code.
     * Example: `{ 2, 3, 5 }` -> on 2nd line, from the 4th character to (and excluding) the 6th character"
     */
    class Location {
    public:
        uint32_t line;  /// line in the file (line count starts at 1).  
        uint32_t start; /// index of the first character of the line to be included (starts at 0).
        uint32_t end;   /// index of the first character of the line to be excluded (starts at 0).
        SourceFile* file;   /// file this location is in.

        /** 
         * Constructs a Location in `file` at `line`, from `start` to (and excluding) `end`.
         * @param _line line in the file (line count starts at 1).  
         * @param _start index of the first character of the line to be included (starts at 0).
         * @param _end index of the first character of the line to be excluded (starts at 0).
         * @param _file file this location is in.
         */
        Location(uint32_t _line, uint32_t _start, uint32_t _end, SourceFile* _file)
                : line(_line), start(_start), end(_end), file(_file) {
            if (this->end <= this->start)
                this->end = this->start + 1;
        }

        /**
         * Constructs a Location in `file` at `line`, at the character with index of `loc`.
         * @param lineNum line in the file (line count starts at 1).  
         * @param loc index of the only character of the line to be included (starts at 0).
         * @param src file this location is in.
         */
        Location(uint32_t lineNum, uint32_t loc, SourceFile* src) : Location(lineNum, loc, loc + 1, src) {}

        /**
         * Constructs a non-location
         * Used for errors which are not bound to a specific location in the source code.
         */
        Location() : Location(0, 0, 0, nullptr) {}

        bool operator==(Location& other) { 
            return other.file == file && other.start == start &&
                   other.line == line && other.end  == end; 
        }
        bool operator!=(Location& other) { 
            return !operator==(other);
        }
    };

    /**
     * RICH:
     *     Error(E308): a rich error
     *        ╭─ test.cpp ─╴
     *        │ 
     *      4 │     int n = 10;
     *        │     ^^^ a submessage
     *     ───╯
     * 
     * SHORT:
     *     test.cpp:4:4:7: Error(E308): a rich error
     */
    enum class DisplayStyle { RICH, SHORT };

    /**
     * TODO - explain
     */
    class Config {
    public:
        DisplayStyle style;
        uint32_t tabWidth;
        struct {
            colors::Color error = colors::fgred & colors::bold;
            colors::Color warning = colors::fgyellow & colors::bold;
            colors::Color note = colors::fgblack & colors::bold;
            colors::Color help = colors::fgblue & colors::bold;
            colors::Color message = colors::bold; // the color of the main error message
            colors::Color border = colors::inherit;
            colors::Color lineNum = colors::inherit;
            colors::Color highlightLineNum = colors::inherit;
        } colors;

        struct {
            /*
                    ╭─ file.xyz ─
                    │ ┐
                    │ ├─ borderTop
                    │ ┘
               17   │   int n = 20
            └┬┘  └┬┘│└┬┘
             │    │ │ ╰ borderLeft
             │    ╰ afterLineNum
             ╰ beforeLineNum
                    │ ┐
                    │ ├─ borderBottom
                    │ ┘
                 ───╯
            */
            uint8_t beforeLineNum = 1;
            uint8_t afterLineNum = 1;
            uint8_t borderTop = 1;
            uint8_t borderLeft = 1;
            uint8_t borderBottom = 0;
        } padding;

        struct {
            std::string errorName = "Error";
            std::string warningName = "Warning";
            std::string noteName = "Note";
            std::string helpName = "Help";
            std::string internalErrorName = "Internal Error";

            std::string shortModeLineSeperator = " / "; // seperates multi-line diagnostic messages

            wchar_t errCodeBracketLeft  = L'(';
            wchar_t errCodeBracketRight = L')';

            std::string beforeFileName  = "╭─ ";
            std::string afterFileName   = " ─╴";

            wchar_t borderVertical      = L'│';
            wchar_t borderHorizontal    = L'─';
            wchar_t borderBottomRight   = L'╯';
            wchar_t noteBullet          = L'•';

            wchar_t lineVertical        = L'│';
            std::string lineBottomLeft  = "╰ ";

            wchar_t arrowDown           = L'v';
            wchar_t arrowUp             = L'^';
            wchar_t underline1          = L'~';
            wchar_t underline2          = L'=';
            wchar_t underline3          = L'#';
            wchar_t underline4          = L'*';
            wchar_t underlineA          = L'-';
            wchar_t underlineB          = L'+';
        } chars;

        Config() : style(DisplayStyle::RICH), tabWidth(4) { }
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

        /* unicode code point to string */
        static std::string toString(char32_t cp)
        {
            uint8_t c[5] = { 0 };
            if      (cp<=0x7F) { c[0] = static_cast<uint8_t>(cp); } 
            else if (cp<=0x7FF) { c[0] = static_cast<uint8_t>((cp>>6)+192); c[1] = static_cast<uint8_t>((cp&63)+128); }
            else if (0xd800<=cp && cp<=0xdfff) { /*invalid block of utf8*/ } 
            else if (cp <= 0xFFFF)   { 
                c[0] = static_cast<uint8_t>((cp>>12)+224);
                c[1] = static_cast<uint8_t>(((cp>>6)&63)+128);
                c[2] = static_cast<uint8_t>((cp&63)+128); 
            } 
            else if (cp <= 0x10FFFF) { 
                c[0] = static_cast<uint8_t>((cp>>18)+240);
                c[1] = static_cast<uint8_t>(((cp>>12)&63)+128); 
                c[2] = static_cast<uint8_t>(((cp>>6)&63)+128);
                c[3] = static_cast<uint8_t>((cp&63)+128); }
            return std::string(reinterpret_cast<char*>(c));
        }

        /* repeat `s` `n` times */
        static std::string repeat(std::string s, size_t n) 
        { 
            std::string s1 = s; 
            for (size_t i=1; i<n;i++)  s += s1;
            return s; 
        }

        /* replace all occurences of `from` in `str` with `to` */
        static std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return str;
        }

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

        static uint32_t tabWidth(const Config& config, size_t pos) {
            if (config.tabWidth == 0) return 0;
            return config.tabWidth - pos % config.tabWidth;
        }

        /* prints `count` characters of whitespace */
        static void indent(const Config& config, std::ostream& out, std::string& line, uint32_t count, size_t start = 0) {
            for (uint32_t i = 0; i < count; i++)
                out << (line[start + i] == '\t' ? std::string(tabWidth(config, start + i), ' ') : " ");
        }

        static void printLine(const Config& config, std::ostream& out, std::string& line) {
            for (size_t i = 0; i < line.size(); i++) {
                if (line[i] == '\t')
                    out << std::string(tabWidth(config, i), ' ');
                else out << line[i];
            }
            out << "\n";
        }

        /* get corresponding underline character based intensity level */
        static std::string getUnderline(const Config& config, int8_t level, std::string& line, size_t idx) {
            std::string ret = "";
            switch (level) {
                case -1: ret = toString(config.chars.lineVertical); break;
                case 0:  ret = " "; break;
                case 1:  ret = toString(config.chars.underline1); break;
                case 2:  ret = toString(config.chars.underline2); break;
                case 3:  ret = toString(config.chars.underline3); break;
                case 4:  ret = toString(config.chars.underline4); break;
                default: ret = toString(level%2 ? config.chars.underlineA : config.chars.underlineB); break;
            }
            if (line[idx] == '\t')
                return repeat(ret, tabWidth(config, idx));
            return ret;
        }

        /* return errTy' string representation + the error code if one exists */
        std::string tyToString(const Config& config) {
            std::string str;
            switch (errTy) {
                case DiagnosticType::INTERNAL_ERROR: 
                case DiagnosticType::UNKNOWN: str = config.chars.internalErrorName; break;
                case DiagnosticType::ERROR:   str = config.chars.errorName;         break;
                case DiagnosticType::WARNING: str = config.chars.warningName;       break;
                case DiagnosticType::NOTE:    str = config.chars.noteName;          break;
                case DiagnosticType::HELP:    str = config.chars.helpName;          break;
            }
            if (code != "")
                return str + toString(config.chars.errCodeBracketLeft) + code  + toString(config.chars.errCodeBracketRight);
            else return str;
        }

        /* returns errTy's color */
        const colors::Color& color(const Config& config) {
            switch (errTy) {
                case DiagnosticType::INTERNAL_ERROR: 
                case DiagnosticType::UNKNOWN: 
                case DiagnosticType::ERROR:   return config.colors.error;
                case DiagnosticType::WARNING: return config.colors.warning;
                case DiagnosticType::NOTE:    return config.colors.note;
                case DiagnosticType::HELP:    return config.colors.help;
            }
            return colors::none;
        }

        /* returns errTy's color if `c` is `colors::inherit` (otherwise returns c)*/
        const colors::Color& maybeInherit(const Config& config, const colors::Color& c) {
            if (c == colors::inherit)
                return color(config);
            else return c;
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
        
        /* prints the bars on the left with the correct indentation */
        void printLeft(const Config& config, std::ostream& out, uint32_t maxLine, bool printBar = true) {
            out << std::string(std::to_string(maxLine).size() + config.padding.beforeLineNum + config.padding.afterLineNum, ' ');
            if (printBar)
                out << maybeInherit(config, config.colors.border)(toString(config.chars.borderVertical) + std::string(config.padding.borderLeft, ' '));
        }

        /* prints the `╭─ file.xyz ─╴` at the start of the file's diagnostics */
        void printTop(const Config& config, std::ostream& out, SourceFile* file, uint32_t maxLine) {
            printLeft(config, out, maxLine, false);
            out << maybeInherit(config, config.colors.border)(config.chars.beforeFileName) 
                << file->str() << maybeInherit(config, config.colors.border)(config.chars.afterFileName) << "\n";
        }

        /* prints the border at the end of the file's diagnostics */
        void printBottom(const Config& config, std::ostream& out, uint32_t maxLine) {
            for (uint8_t i = 0; i < config.padding.borderBottom; i++) {
                printLeft(config, out, maxLine);
                out << "\n";
            }
            for (size_t i = 0; i < std::to_string(maxLine).size() + config.padding.beforeLineNum + config.padding.afterLineNum; i++)
                out << maybeInherit(config, config.colors.border)(toString(config.chars.borderHorizontal));
            out << maybeInherit(config, config.colors.border)(toString(config.chars.borderBottomRight)) << "\n";
        }

        /* prints the bars on the left with the correct indentation + with the line number */
        void printLeftWithLineNum(const Config& config, std::ostream& out, uint32_t lineNum, uint32_t maxLine, bool printBar = true) {
            auto targetSize = std::to_string(maxLine).size() + config.padding.beforeLineNum + config.padding.afterLineNum;
            auto str = std::string(config.padding.beforeLineNum, ' ') + std::to_string(lineNum) + std::string(config.padding.afterLineNum, ' ');
            while (str.size() < targetSize)
                str += " ";
            if (lineNum == loc.line)
                out << maybeInherit(config, config.colors.highlightLineNum)(str);
            else out << maybeInherit(config, config.colors.lineNum)(str);
            if (printBar)
                    out << maybeInherit(config, config.colors.border)(toString(config.chars.borderVertical) + std::string(config.padding.borderLeft, ' '));
        }

        /* a 'padding' line is an irrelevant line in between two other relevant lines */
        void printPadding(const Config& config, std::ostream& out, uint32_t maxLine, uint32_t lastLine, uint32_t currLine, SourceFile *file) {
            if (lastLine + 2 == currLine) {
                printLeftWithLineNum(config, out, currLine - 1, maxLine);
                out << file->getLine(currLine - 1) << "\n";
            } else {
                switch (std::to_string(maxLine).size() + config.padding.beforeLineNum + config.padding.afterLineNum) {
                    case 3:  out << " " << color(config)("⋯") << "\n"; break;
                    case 4:  out << " " << color(config)("··") << "\n"; break;
                    default: out << " " << color(config)("···") << "\n"; break;
                }   
            }
        }

        /* prints all secondary messages on the current line */
        void printSecondariesOnLine(const Config& config, std::ostream& out, std::string &line, size_t &i, uint32_t maxLine, bool shownAbove) {
            auto &first = secondaries[i];
            if (!shownAbove && first.loc == loc) { i++; return; }
            printLeft(config, out, maxLine);
            
            if (i + 1 >= secondaries.size() || !onSameLine(first, secondaries[i + 1])) {
                // only one secondary concerning this line
                indent(config, out, line, first.loc.start);
                for (auto idx = first.loc.start; idx < first.loc.end; idx++)
                    out << first.color(config)(getUnderline(config, 1, line, idx));

                auto lines = splitLines(first.msg);
                for (auto& sec : first.secondaries)
                    for (auto str : splitLines(sec.msg))
                        lines.push_back(sec.color(config)(str));
                
                for (size_t idx = 0; idx < lines.size(); idx++) {
                    if (idx != 0) {
                        printLeft(config, out, maxLine);
                        indent(config, out, line, first.loc.end);
                    }
                    out << " " << first.color(config)(lines[idx]) << "\n";
                }
                i++;
            } else {
                std::vector<std::vector<Diagnostic*>> toRender;
                uint8_t depth = 0;
                auto idx = i;
                while (idx < secondaries.size() && onSameLine(secondaries[idx], first))
                    idx++;

                for (; idx > i; idx--) {
                    if (toRender.size() == 0)
                        toRender.push_back({ &secondaries[idx-1] });
                    else if (secondaries[idx-1].loc.start <  toRender[depth].back()->loc.end && 
                             secondaries[idx-1].loc.end   >= toRender[depth].back()->loc.end) {
                        if (++depth == toRender.size())
                            toRender.push_back({});
                        toRender[depth].push_back(&secondaries[idx-1]);
                    } else {
                        while (depth != 0 && secondaries[idx-1].loc.start >= toRender[depth-1].back()->loc.end)
                            depth--;
                        toRender[depth].push_back(&secondaries[idx-1]);
                    }
                }

                for (size_t j = 0; j < toRender.size(); j++) {
                    if (j != 0) {
                        out << "\n";
                        printLeft(config, out, maxLine);
                    }
                    for (size_t lineIdx = 0; lineIdx < line.size(); lineIdx++) {
                        int8_t count = 0;
                        Diagnostic* firstFound = nullptr;
                        for (auto curr : toRender[j])
                            if (curr->loc.start <= lineIdx && lineIdx < curr->loc.end) {
                                count++;
                                if (!firstFound) firstFound = curr;
                            }
                        if (!firstFound)
                            for (auto k = j; count == 0 && k > 0; k--) {
                                for (auto diag : toRender[k-1])
                                    if (diag->loc.start == lineIdx) {
                                        count = -1;
                                        firstFound = diag;
                                        break;
                                    }
                            }
                        if (firstFound)
                            out << firstFound->color(config)(getUnderline(config, count, line, lineIdx));
                        else out << getUnderline(config, count, line, lineIdx);
                    }
                }
                
                out << "\n";
                for (; i < secondaries.size() && onSameLine(secondaries[i], first); i++) {
                    printLeft(config, out, maxLine);
                    for (size_t j = 0; j < secondaries[i].loc.start; j++) {
                        bool b = false;
                        for (auto idx = i; !b && idx < secondaries.size() && onSameLine(secondaries[idx], first); idx++)
                            if (secondaries[idx].loc.start == j) {
                                out << secondaries[idx].color(config)(toString(config.chars.lineVertical));
                                if (line[j] == '\t' && tabWidth(config, j) > 1)
                                    out << std::string(tabWidth(config, j) - 1, ' ');
                                b = true;
                            }
                        if (!b) indent(config, out, line, 1, j);
                    }
                    auto lines = splitLines(secondaries[i].msg);
                    for (auto& sec : secondaries[i].secondaries)
                        for (auto str : splitLines(sec.msg))
                            lines.push_back(sec.color(config)(str));

                    for (size_t idx = 0; idx < lines.size(); idx++) {
                        if (idx == 0)
                            out << secondaries[i].color(config)(config.chars.lineBottomLeft + lines[idx]) << "\n";
                        else {
                            printLeft(config, out, maxLine);
                            for (size_t j = 0; j < secondaries[i].loc.start; j++) {
                                bool b = false;

                                for (auto k = i; !b && k < secondaries.size() && onSameLine(secondaries[k], first); k++)
                                    if (secondaries[k].loc.start == j) {
                                        out << secondaries[k].color(config)(toString(config.chars.lineVertical));
                                        if (line[j] == '\t' && tabWidth(config, j) > 1)
                                            out << std::string(tabWidth(config, j) - 1, ' ');
                                        b = true;
                                    }
                                
                                if (!b) indent(config, out, line, 1, j);
                            }
                            out << secondaries[i].color(config)("  " + lines[idx]) << "\n";
                        }
                    }
                }
            }
        }

    protected:
        Diagnostic(DiagnosticType ty, std::string message, std::string subMessage, std::string diagCode, Location location) 
               : msg(message), subMsg(subMessage), loc(location), errTy(ty), code(diagCode) {}
        Diagnostic(DiagnosticType ty, std::string message, std::string subMessage, Location location) : Diagnostic(ty, message, subMessage, "", location) {}
        Diagnostic(DiagnosticType ty, std::string message, Location location) : Diagnostic(ty, message, "", location) {}
        Diagnostic(DiagnosticType ty, std::string message) : Diagnostic(ty, message, {}) {}

    public:
        /**
         * Pretty-print the diagnostic.
         * @param out stream in which to print the error.
         * @return the object which this function was called upon.
         */
        Diagnostic& print(std::ostream& out, const Config& config) {

            // sort the vector of secondary messages based on the order we want to be printing them 
            sortSecondaries();

            if (config.style == DisplayStyle::SHORT) {
                if (loc.file)
                    out << loc.file->str() << ":" << loc.line << ":" << loc.start << ":" << loc.end << ": ";
                out << color(config)(tyToString(config) + ": ") 
                    << maybeInherit(config, config.colors.message)(replaceAll(msg, "\n", config.chars.shortModeLineSeperator)) << "\n";
                for (auto& i : secondaries)
                {
                    if (i.loc.file) 
                        out << i.loc.file->str() << ":" << i.loc.line << ":" << i.loc.start << ":" << i.loc.end << ": ";
                    out << i.color(config)(i.tyToString(config) + ": ") 
                        << replaceAll(i.msg, "\n", config.chars.shortModeLineSeperator) << "\n";
                }
                return *this;
            }

            // find the maximum line (to know by how much to indent the bars)
            auto maxLine = loc.line;
            for (auto& secondary : secondaries)
                if (secondary.loc.line > maxLine)
                    maxLine = secondary.loc.line;

            // by default we're pointing at the error location from below the code snippet
            bool printAbove = false; 

            // if there are any messages on the line of the error, point to the error from above instead
            for (auto& i : secondaries)
                if (onSameLine(i, *this) && i.loc != loc) {
                    printAbove = true;
                    break;
                }

            // print the main error message
            if (msg != "")
                out << color(config)(tyToString(config) + ": ") << maybeInherit(config, config.colors.message)(msg) << "\n";

            size_t i = 0; // current index in `secondaries`
            uint32_t lastLine = 0; // the last line we rendered
            std::string line = ""; // the snippet we're going to print

            // skip to after the diagnostic's location is printed if it has no location 
            if (loc.file == nullptr) goto afterSubMsg;

            // print the file the error is in
            printTop(config, out, loc.file, maxLine);

            // top padding
            for (uint8_t idx = 0; idx < config.padding.borderTop - 1; idx++) {
                printLeft(config, out, maxLine);
                out << "\n";
            }

            // first print all messages in the main file which come before the error
            while (i < secondaries.size() && secondaries[i].loc.file == loc.file && secondaries[i].loc.line < loc.line) {
                auto &secondary = secondaries[i];

                if (lastLine == 0 && config.padding.borderTop != 0) { // if we're rendering the first line in the file, print an empty line
                    printLeft(config, out, maxLine); 
                    out << "\n";
                } else if (lastLine != 0 && lastLine < secondary.loc.line - 1)
                    printPadding(config, out, maxLine, lastLine, secondary.loc.line, secondary.loc.file);

                lastLine = secondary.loc.line;
                line = loc.file->getLine(secondary.loc.line);
                printLeftWithLineNum(config, out, secondary.loc.line, maxLine);
                printLine(config, out, line);
                printSecondariesOnLine(config, out, line, i, maxLine, printAbove);
            }

            line = loc.file->getLine(loc.line);

            if (lastLine == 0 && !printAbove && config.padding.borderTop != 0) {
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
                        out << color(config)(currLine) << "\n";
                    }
                }
                
                printLeft(config, out, maxLine);

                indent(config, out, line, loc.start);
                for (auto j = loc.start; j < loc.end; j++)
                    if (line[j] == '\t')
                        out << color(config)(repeat(toString(config.chars.arrowDown), tabWidth(config, j)));
                    else out << color(config)(toString(config.chars.arrowDown));
                out << "\n";
            }
            
            printLeftWithLineNum(config, out, loc.line, maxLine);
            printLine(config, out, line);

            if (!printAbove) {
                printLeft(config, out, maxLine);
                indent(config, out, line, loc.start);
                for (auto j = loc.start; j < loc.end; j++)
                    if (line[j] == '\t')
                        out << color(config)(repeat(toString(config.chars.arrowUp), tabWidth(config, j)));
                    else out << color(config)(toString(config.chars.arrowUp));
                if (subMsg == "")
                    out << "\n";
                else {
                    auto split = splitLines(subMsg);
                    for (size_t k = 0; k < split.size(); k++) {
                        if (k) {
                            printLeft(config, out, maxLine);
                            indent(config, out, line, loc.end);
                            // out << std::string(loc.end - loc.start, ' ');
                        }
                        out << " " << color(config)(split[k]) << "\n";
                    }
                }
                for (size_t j = i; j < secondaries.size() && secondaries[j].loc.file == loc.file && secondaries[j].loc.line == loc.line; j++) {
                    if (secondaries[j].loc == loc) {
                        for (auto str : splitLines(secondaries[j].msg)) {
                            printLeft(config, out, maxLine);
                            indent(config, out, line, loc.end);
                            out << " " << secondaries[j].color(config)(str) << "\n";    
                        }
                    }
                }
            }
        
            if (i < secondaries.size() && onSameLine(secondaries[i], *this))
                printSecondariesOnLine(config, out, line, i, maxLine, printAbove);

        afterSubMsg:    
            auto currFile = loc.file;
            while (i < secondaries.size() && secondaries[i].loc.file) {
                auto &secondary = secondaries[i];
                if (currFile == nullptr || secondary.loc.file->str() != currFile->str()) {
                    if (currFile != nullptr)                    
                        printBottom(config, out, maxLine);
                    currFile = secondary.loc.file;
                    printTop(config, out, currFile, maxLine);
                    for (uint8_t k = 0; k < config.padding.borderTop; k++) {
                        printLeft(config, out, maxLine);
                        out << "\n";
                    }
                } else if (lastLine < secondary.loc.line - 1)
                    printPadding(config, out, maxLine, lastLine, secondary.loc.line, secondary.loc.file);

                lastLine = secondary.loc.line;
                line = currFile->getLine(secondary.loc.line);
                printLeftWithLineNum(config, out, secondary.loc.line, maxLine);
                printLine(config, out, line);
                printSecondariesOnLine(config, out, line, i, maxLine, printAbove);
            }
            if (currFile != nullptr)
                printBottom(config, out, maxLine); 
            for (; i < secondaries.size(); i++) {
                auto& secondary = secondaries[i];
                printLeft(config, out, maxLine, false);
                out << secondary.color(config)(toString(config.chars.noteBullet) + " " + secondary.tyToString(config) + secondary.color(config)(": "));

                auto lines = splitLines(secondary.msg);
                for (auto& sec : secondary.secondaries)
                    for (auto str : splitLines(sec.msg))
                        lines.push_back(sec.color(config)(str));
                
                for (size_t idx = 0; idx < lines.size(); idx++) {
                    if (idx != 0) {
                        printLeft(config, out, maxLine, false);
                        out << std::string(secondary.tyToString(config).size() + 4, ' ');
                    }
                    printLine(config, out, lines[idx]);
                }
            }
            return *this;
        }

        Diagnostic& print(std::ostream& out, const Config&& config = Config()) { return print(out, config); }

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
        inline Diagnostic& withNote(std::string message) { return withNote(message, {}); }

        /**
         * Adds a secondary help message to the diagnostic without a specific location.
         * @param message the help message.
         * @return the object which this function was called upon.
         */
        Diagnostic& withHelp(std::string message) { return withHelp(message, {}); }

    private:
        Diagnostic& with(Diagnostic diag) {
            if (diag.loc.file != nullptr) 
                for (auto& i : secondaries)
                    if (i.loc == diag.loc) {
                        i.with(diag);
                        return *this;
                    }
            secondaries.push_back(diag);
            return *this; 
        }
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
        DiagnosticTy<T>& print(std::ostream& out, const Config& config)             { Diagnostic::print(out, config); return *this; }
        DiagnosticTy<T>& print(std::ostream& out, const Config&& config = Config()) { Diagnostic::print(out, config); return *this; }

        /**
         * Adds a secondary note message to the diagnostic at `location`.
         * @param message the note message.
         * @param location source code location of the note message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withNote(std::string message, Location location) { Diagnostic::withNote(message, location); return *this; }

        /**
         * Adds a secondary help message to the diagnostic at `location`.
         * @param message the help message.
         * @param location source code location of the help message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withHelp(std::string message, Location location) { Diagnostic::withHelp(message, location); return *this; }

        /**
         * Adds a secondary note message to the diagnostic without a specific location.
         * @param message the note message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withNote(std::string message) { Diagnostic::withNote(message); return *this; }

        /**
         * Adds a secondary help message to the diagnostic without a specific location.
         * @param message the help message.
         * @return the object which this function was called upon.
         */
        DiagnosticTy<T>& withHelp(std::string message) { Diagnostic::withHelp(message); return *this; }
    };

    /////////////////////////////////////////////////////////////////////////

    /**
     * An `internal error` diagnostic type. 
     * These should (ideally) never be shown to the client, rather they should be used as a debugging tool for the compiler developer.
     */
    typedef DiagnosticTy<DiagnosticType::INTERNAL_ERROR> InternalError;

    /**
     * An `error` diagnostic type. 
     * These are usually errors which stop the compilation process from being competed.
     */
    typedef DiagnosticTy<DiagnosticType::ERROR> Error;

    /**
     * A `warning` diagnostic type.
     * These don't necessarily halt the compilation process, but they hint at a possible error in the programmer's code.
     */
    typedef DiagnosticTy<DiagnosticType::WARNING> Warning;

    /**
     * A `note` diagnostic type.
     * Should be used to supplement the `Error`/`Warning` diagnostic, and give useful inforation to solve the issue.
     */
    typedef DiagnosticTy<DiagnosticType::NOTE> Note;

    /**
     * A `help` diagnostic type.
     * Usually used to give useful hints at how to fix an issue.
     */
    typedef DiagnosticTy<DiagnosticType::HELP> Help;

    Diagnostic& Diagnostic::withNote(std::string message, Location location) { return with(Note(message, location)); }
    Diagnostic& Diagnostic::withHelp(std::string message, Location location) { return with(Help(message, location)); }
}

#endif /* DIAGNOSTIC_REPORTER_HPP_INCLUDED */
