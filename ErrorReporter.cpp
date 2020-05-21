#include "ErrorReporter.h"


namespace ErrorReporter 
{

    Error::Error(string message, ErrorCode type, Position position) : msg(message), subMsg(), errTy(type), pos(position) {};
    Error::Error(string message, string subMessage, ErrorCode type, Position position) : msg(message), subMsg(subMessage), errTy(type), pos(position) {};
    Error& Error::withNote(string message, Position position) { secondaries.push_back(Error("", message, NOTE_GENERAL, position)); return *this; }
    Error& Error::withHelp(string message, Position position) { secondaries.push_back(Error("", message, HELP_GENERAL, position)); return *this; }
    Error& Error::withNote(string message) { return withNote(message, POS_NONE); }
    Error& Error::withHelp(string message) { return withHelp(message, POS_NONE); }

    vector<string> splitLines(const string& str)
    {
        vector<string> strings;

        string::size_type pos = 0;
        string::size_type prev = 0;
        while ((pos = str.find("\n", prev)) != string::npos)
        {
            strings.push_back(str.substr(prev, pos - prev));
            prev = pos + 1;
        }

        // To get the last substring (or only, if delimiter is not found)
        strings.push_back(str.substr(prev));

        return strings;
    }

    void Error::show()
    {
        // find the maximum line (to know by how much to indent the bars)
        auto maxLine = pos.line;
        for (auto secondary : secondaries)
            if (secondary.pos.line > maxLine)
                maxLine = secondary.pos.line;

        sortSecondaries();
        bool showAbove = false; // if theERR_UNKNOWNre are any messages on the line of the error, point to the error from above instead

        for (size_t i = 0; !showAbove && i < secondaries.size(); i++)
            if (secondaries[i].pos.file == pos.file && secondaries[i].pos.line == pos.line)
                showAbove = true;  

        // show the main error message
        if (msg != "")
            std::cout << color(tyToString() + ": ") << BOLD(msg) << "\n";

        if (pos.file == NULL) return;


        // print the file the error is in
        printIndent(maxLine, false);
        std::cout << color("╭─ ") << pos.file->getOriginalPath() << color(" ─╴") << "\n";

        bool isFirst = true;
        unsigned int lastLine = 0;
        size_t i = 0;

        // first show all messages in the main file which come before the error
        while (i < secondaries.size() && secondaries[i].pos.file == pos.file && secondaries[i].pos.line < pos.line)
        {
            auto &secondary = secondaries[i];
            
            if (isFirst)
            {
                printIndent(maxLine);
                std::cout << "\n";
                isFirst = false;
            }
            else if (lastLine < secondary.pos.line - 1) 
            {
                if (lastLine == secondary.pos.line - 2)
                    printPaddingLine(maxLine, secondary.pos.line - 1, secondary.pos.file);
                else printPaddingLine(maxLine);
            }
            lastLine = secondary.pos.line;
            string line = getLine(pos.file->getOriginalPath(), secondary.pos.line);

            auto tmp = secondary.errTy;
            secondary.errTy = errTy;
            secondary.printIndentWithLineNum(maxLine);
            secondary.errTy = tmp;

            std::cout << line << "\n";

            showSecondariesOnLine(line, i, maxLine);
        }

        string line = getLine(pos.file->getOriginalPath(), pos.line);

        if (isFirst)
        {
            printIndent(maxLine);
            std::cout << "\n";
        }
        else if (lastLine != 0 && lastLine < pos.line - 1) 
        {
            if (lastLine == pos.line - 2)
                printPaddingLine(maxLine, pos.line - 1, pos.file);
            else printPaddingLine(maxLine);
        }
        lastLine = pos.line;
        
        if (showAbove)
        {
            if (subMsg != "")
            {
                for (auto currLine : splitLines(subMsg))
                {
                    printIndent(maxLine);
                    for (unsigned int i = 0; i < pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    std::cout << color(currLine);
                    std::cout << "\n";
                }
            }
            
            printIndent(maxLine);

            for (unsigned int i = 0; i < pos.startPos; i++)
                std::cout << (line[i] == '\t' ? "\t" : " ");
            for (unsigned int i = 0; i < pos.endPos - pos.startPos; i++)
                std::cout << color("v");
            std::cout << "\n";
        }
        
        printIndentWithLineNum(maxLine);
        std::cout << line << "\n";

        if (!showAbove)
        {
            if (subMsg != "")
            {
                auto split = splitLines(subMsg);
                for (int i = 0; i < split.size(); i++)
                {
                    printIndent(maxLine);
                    for (unsigned int i = 0; i < pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    for (unsigned int j = 0; j < pos.endPos - pos.startPos; j++)
                        std::cout << color(i == 0 ? "^" : " ");
                    std::cout << " ";
                    std::cout << color(split[i]) << "\n";
                }
            }
        }
      
        if (i < secondaries.size() && secondaries[i].pos.file == pos.file && secondaries[i].pos.line == pos.line)
            showSecondariesOnLine(line, i, maxLine);
        
        auto currFile = pos.file;
        while (i < secondaries.size() && secondaries[i].pos.file)
        {
            auto &secondary = secondaries[i];

            if (secondary.pos.file->getOriginalPath() != currFile->getOriginalPath())
            {
                currFile = secondary.pos.file;
                for (unsigned int i = 0; i < std::to_string(pos.line).size() + 2; i++) std::cout << color("─");
                std::cout << color("╯") << "\n";
                printIndent(maxLine, false);
                std::cout << color("╭─ ") << currFile->getOriginalPath() << color(" ─╴") << "\n";
                printIndent(maxLine);
                std::cout << "\n";
            }

            if (lastLine < secondary.pos.line - 1) 
            {
                if (lastLine == secondary.pos.line - 2)
                    printPaddingLine(maxLine, secondary.pos.line - 1, secondary.pos.file);
                else printPaddingLine(maxLine);
            }
            lastLine = secondary.pos.line;
            string line = getLine(pos.file->getOriginalPath(), secondary.pos.line);
            auto tmp = secondary.errTy;
            secondary.errTy = errTy;
            secondary.printIndentWithLineNum(maxLine);
            secondary.errTy = tmp;
            std::cout << line << "\n";
            showSecondariesOnLine(line, i, maxLine);
        }

        for (unsigned int i = 0; i < std::to_string(pos.line).size() + 2; i++) std::cout << color("─");
        std::cout << color("╯") << "\n"; 
        for (; i < secondaries.size(); i++)
        {
            auto& secondary = secondaries[i];
            printIndent(maxLine, false);
            std::cout << secondary.color("• " + secondary.tyToString() + secondary.color(": "));
            auto lines = splitLines(secondary.subMsg);
            for (uint idx = 0; idx < lines.size(); idx++)
            {
                if (idx != 0)
                {
                    printIndent(maxLine, false);
                    for (uint j = 0; j < secondary.tyToString().size() + 4; j++)
                        std::cout << " ";
                }
                std::cout << lines[idx] << "\n";
            }
        }
    }

    void Error::showSecondariesOnLine(string &line, size_t &i, unsigned int maxLine)
    {
        auto &first = secondaries[i];
        printIndent(maxLine);
        if (i + 1 >= secondaries.size() || secondaries[i + 1].pos.file != first.pos.file || secondaries[i + 1].pos.line != first.pos.line)
        {
            // only one secondary concerning this line
            for (size_t idx = 0; idx < first.pos.startPos; idx++)
                std::cout << (line[idx] == '\t' ? '\t' : ' ');
            for (size_t idx = 0; idx < first.pos.endPos - first.pos.startPos; idx++)
                std::cout << first.color("~");
            auto lines = splitLines(first.subMsg);
            for (size_t idx = 0; idx < lines.size(); idx++)
            {
                if (idx != 0)
                {
                    printIndent(maxLine);
                    for (size_t j = 0; j < first.pos.endPos; j++)
                        std::cout << (line[j] == '\t' ? '\t' : ' ');
                }
                std::cout << " " << first.color(lines[idx]) << "\n";
            }
            i++;
        }
        else 
        {
            for (size_t lineIdx = 0; lineIdx < line.size(); lineIdx++)
            {
                bool b = false;
                for (auto idx = i; !b && idx < secondaries.size() && secondaries[idx].pos.file == first.pos.file 
                    && secondaries[idx].pos.line == first.pos.line; idx++)
                    if (secondaries[idx].pos.startPos <= lineIdx && lineIdx < secondaries[idx].pos.endPos)
                    {
                        std::cout << secondaries[idx].color("~");
                        b = true;
                    }
                if (!b) std::cout << " ";
            }
            std::cout << "\n";
            for (; i < secondaries.size() && secondaries[i].pos.file == first.pos.file && secondaries[i].pos.line == first.pos.line; i++)
            {
                printIndent(maxLine);
                for (size_t j = 0; j < secondaries[i].pos.startPos; j++)
                {
                    bool b = false;
                    for (auto idx = i; !b && idx < secondaries.size() && secondaries[idx].pos.file == first.pos.file 
                        && secondaries[idx].pos.line == first.pos.line; idx++)
                        if (secondaries[idx].pos.startPos == j)
                        {
                            std::cout << secondaries[idx].color("│");
                            b = true;
                        }
                    if (!b) std::cout << " ";
                }
                auto lines = splitLines(secondaries[i].subMsg);
                for (size_t idx = 0; idx < lines.size(); idx++)
                {
                    if (idx == 0)
                        std::cout << secondaries[i].color("╰ ") << secondaries[i].color(lines[idx]) << "\n";
                    else
                    {
                        printIndent(maxLine);
                        for (size_t j = 0; j < secondaries[i].pos.startPos; j++)
                        {
                            bool b = false;
                            for (auto idx = i; !b && idx < secondaries.size() && secondaries[idx].pos.file == first.pos.file 
                                && secondaries[idx].pos.line == first.pos.line; idx++)
                                if (secondaries[idx].pos.startPos == j)
                                {
                                    std::cout << secondaries[idx].color("│");
                                    b = true;
                                }
                            if (!b) std::cout << " ";
                        }
                        std::cout << secondaries[i].color("  ") << secondaries[i].color(lines[idx]) << "\n";
                    }
                }
            }
        }
    }

    void Error::sortSecondaries()
    {
        auto file = pos.file;
        std::sort(
            std::begin(secondaries), std::end(secondaries), 
            [file](Error a, Error b) {
                if (!a.pos.file) 
                    return false;
                if (!b.pos.file) 
                    return true;
                if (a.pos.file == file && b.pos.file != file)
                    return true;
                if (a.pos.file != file && b.pos.file == file)
                    return false;
                if (a.pos.file != b.pos.file)
                    return a.pos.file->getOriginalPath() < b.pos.file->getOriginalPath();
                if (a.pos.line > b.pos.line)
                    return false;
                return a.pos.startPos > b.pos.startPos; 
            }
        );
    }

    vector<Error> errors;

    Error& report(Error err)
    {
        errors.push_back(err);
        err.show();
        return errors.back();
    }

    Error& report(string msg, ErrorCode errCode, Position pos)
    {
        return report(Error(msg, errCode, pos));
    }

    void reportAbort() 
    {
        report("aborting due to previous error", NOTE_GENERAL, POS_NONE);
    }

    Error& report(string msg, string subMsg, ErrorCode errCode, Position pos)
    {
        return report(Error(msg, subMsg, errCode, pos));
    }

    Error& reportInternal(string msg, ErrorCode errCode, Position pos)
    {
        errors.push_back({ msg, errCode, pos });
        errors.back().show();
        std::cout << "\n";
        return errors.back();
    }

    void showAll() 
    {
        for (unsigned int i = 0; i < errors.size(); i++)
        {
            if (i) std::cout << "\n";
            errors[i].show();
        }
    }

    string Error::tyToString() 
    {
        if (ERR_GENERAL < errTy && errTy < WRN_GENERAL)
            return "Error(E" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == ERR_GENERAL)
            return "Error";
        else if (WRN_GENERAL < errTy && errTy < NOTE_GENERAL)
            return "Warning(W" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == WRN_GENERAL)
            return "Warning";
        else if (NOTE_GENERAL < errTy && errTy < HELP_GENERAL)
            return "Note(N" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == NOTE_GENERAL)
            return "Note";
        else if (HELP_GENERAL < errTy && errTy < ERR_UNKNOWN)
            return "Help(H" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == HELP_GENERAL)
            return "Help";
        return "Internal Error";
    }

    string Error::color(string str)
    {
        if (WRN_GENERAL <= errTy && errTy < NOTE_GENERAL)
            return BOLD(FYEL(str));
        if (NOTE_GENERAL <= errTy && errTy < HELP_GENERAL)
            return BOLD(FBLK(str));
        if (HELP_GENERAL <= errTy && errTy < ERR_UNKNOWN)
            return BOLD(FBLU(str));
        return BOLD(FRED(str));
    }

    void Error::printPaddingLine(unsigned int maxLine, unsigned int line, SourceFile *file) 
    {
        unsigned int targetSize = std::to_string(maxLine).size() + 2;
        if (line == 0)
        {
            switch (targetSize)
            {
                case 3:  std::cout << " " << color("⋯") << "\n"; break;
                case 4:  std::cout << " " << color("··") << "\n"; break;
                default: std::cout << " " << color("···") << "\n"; break;
            }   
        }
        else
        {
            auto str = " " + std::to_string(line) + " ";
            while (str.size() < targetSize)
                str += " ";
            str += "│ ";
            std::cout << color(str);
            if (file) std::cout << getLine(file->getOriginalPath(), line);
            std::cout << "\n";
        }
    }

    void Error::printIndent(unsigned int maxLine, bool showBar) 
    {
        for (unsigned int i = 0; i < std::to_string(maxLine).size() + 2; i++)
            std::cout << " ";
        if (showBar)
            std::cout << color("│ ");
    }

    void Error::printIndentWithLineNum(unsigned int maxLine, bool showBar) 
    {
        unsigned int targetSize = std::to_string(maxLine).size() + 2;
        auto str = " " + std::to_string(pos.line) + " ";
        while (str.size() < targetSize)
            str += " ";
        std::cout << color(str);
        if (showBar)
            std::cout << color("│ ");
    }

    string getLine(string fileName, int line)
    {
        std::fstream file(fileName);
        file.seekg(std::ios::beg);
        for (int i=0; i < line - 1; ++i)
            file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        string ret;
        std::getline(file, ret);
        return ret;
    }
}