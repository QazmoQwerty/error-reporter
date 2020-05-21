#include "ErrorReporter.h"

namespace ErrorReporter 
{

    Error::Error(string message, ErrorCode type, Position position) : msg(message), subMsg(), errTy(type), pos(position) {};
    Error::Error(string message, string subMessage, ErrorCode type, Position position) : msg(message), subMsg(subMessage), errTy(type), pos(position) {};
    Error& Error::withSecondary(Error err) { secondaries.push_back(err); return *this; }
    Error& Error::withSecondary(string message, Position position) { secondaries.push_back(Error("", message, ERR_NOTE, position)); return *this; }

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

    void Error::show(unsigned int maxLine)
    {
        // find the maximum line (to know by how much to indent the bars)
        if (pos.line > maxLine)
            maxLine = pos.line;
        for (auto secondary : secondaries)
            if (secondary.pos.line > maxLine)
                maxLine = secondary.pos.line;

        sortSecondaries();
        bool showAbove = false; // if there are any messages on the line of the error, point to the error from above instead

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
        for (auto secondary : secondaries) // first show all messages in the main file which come before the error
        {
            if (secondary.pos.file == pos.file && secondary.pos.line < pos.line && secondary.msg == "")
            {
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

                bool isFirstB = true;
                for (auto currLine : splitLines(secondary.subMsg))
                {
                    printIndent(maxLine);
                    for (unsigned int i = 0; i < secondary.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirstB)
                    {
                        for (unsigned int i = 0; i < secondary.pos.endPos - secondary.pos.startPos; i++)
                            std::cout << secondary.color("^");
                        std::cout << secondary.color(" ");
                        isFirstB = false;
                    }
                    std::cout << secondary.color(currLine);
                    std::cout << "\n";
                }
            }
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
        

        for (auto secondary : secondaries)
        {
            if (secondary.pos.line == pos.line)
            {
                printIndent(maxLine);
                for (unsigned int i = 0; i < secondary.pos.startPos; i++)
                    std::cout << (line[i] == '\t' ? "\t" : " ");
                for (unsigned int i = 0; i < secondary.pos.endPos - secondary.pos.startPos; i++)
                    std::cout << secondary.color("^");
                std::cout << " " << secondary.color(secondary.subMsg) << "\n";
            }
            else if (secondary.pos.line > pos.line)
            {
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
                bool isFirst = true;
                for (auto currLine : splitLines(secondary.subMsg))
                {
                    printIndent(maxLine);
                    for (unsigned int i = 0; i < secondary.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirst)
                    {
                        for (unsigned int i = 0; i < secondary.pos.endPos - secondary.pos.startPos; i++)
                            std::cout << secondary.color("^");
                        std::cout << secondary.color(" ");
                        isFirst = false;
                    }
                    std::cout << secondary.color(currLine);
                    std::cout << "\n";
                }
            }
        }
        for (unsigned int i = 0; i < std::to_string(pos.line).size() + 2; i++) std::cout << color("─");
        std::cout << color("╯") << "\n";
        for (auto secondary : secondaries)
            if (secondary.pos.file != pos.file)
                secondary.show(maxLine);
    }

    void Error::sortSecondaries()
    {
        auto file = pos.file;
        std::sort(
            std::begin(secondaries), std::end(secondaries), 
            [file](Error a, Error b) {
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
        // std::cout << "\n";
        return errors.back();
    }

    Error& report(string msg, ErrorCode errCode, Position pos)
    {
        return report(Error(msg, errCode, pos));
    }

    void reportAbort() 
    {
        errors.push_back(Error("aborting due to previous error", ERR_NOTE, POS_NONE));
        errors.back().show();
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
        if (ERR_GENERAL < errTy && errTy < ERR_WARNING)
            return "Error(E" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == ERR_GENERAL)
            return "Error";
        else if (ERR_WARNING < errTy && errTy < ERR_NOTE)
            return "Warning(W" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == ERR_WARNING)
            return "Warning";
        else if (ERR_NOTE < errTy && errTy < ERR_UNKNOWN)
            return "Note(N" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == ERR_NOTE)
            return "Note";
        return "internal error";
    }

    string Error::color(string str)
    {
        switch (errTy)
        {
            default: return BOLD(FRED(str));
            case ERR_WARNING: return BOLD(FYEL(str));
            case ERR_NOTE: return BOLD(FBLK(str));
        }
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