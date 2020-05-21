#include "ErrorReporter.h"

namespace ErrorReporter 
{
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

    void Error::show(uint maxLine)
    {
        if (pos.line > maxLine)
            maxLine = pos.line;
        
        if (notes.size() == 0)
            return showBasic(maxLine);

        for (auto note : notes)
            if (note.pos.file == pos.file && note.pos.line > pos.line)
                maxLine = note.pos.line;

        for (auto note : notes)
            if (note.pos.file == pos.file && note.pos.line == pos.line)
                goto normal;

        showBasic(maxLine);
        for (auto note : notes)
            if (note.pos.file != pos.file)
                note.show(maxLine);
        return;

        normal:

        std::cout << color(tyToString() + ": ") << BOLD(msg) << "\n";
        if (pos.file == NULL)
            return;
        for (uint i = 0; i < std::to_string(pos.line).size() + 2; i++) std::cout << " ";
        std::cout << color("╭─ ") << pos.file->getOriginalPath() << color(" ─╴") << "\n";

        bool isFirst = true;
        uint lastLine = 0;
        for (auto note : notes)
        {
            if (note.pos.file == pos.file && note.pos.line < pos.line && note.msg == "")
            {
                if (isFirst)
                {
                    printIndent(maxLine);
                    std::cout << "\n";
                    isFirst = false;
                }
                else if (lastLine < note.pos.line - 1) 
                {
                    if (lastLine == note.pos.line - 2)
                        printPaddingLine(maxLine, note.pos.line - 1, note.pos.file);
                    else printPaddingLine(maxLine);
                }
                lastLine = note.pos.line;
                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(maxLine, true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirstB = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent(maxLine);
                    for (uint i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirstB)
                    {
                        for (uint i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirstB = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }

        string line = getLine(pos.file->getOriginalPath(), pos.line);

        if (lastLine != 0 && lastLine < pos.line - 1) 
        {
            if (lastLine == pos.line - 2)
                printPaddingLine(maxLine, pos.line - 1, pos.file);
            else printPaddingLine(maxLine);
        }
        lastLine = pos.line;
        
        if (subMsg != "")
        {
            for (auto currLine : splitLines(subMsg))
            {
                printIndent(maxLine);
                for (uint i = 0; i < pos.startPos; i++)
                    std::cout << (line[i] == '\t' ? "\t" : " ");
                std::cout << color(currLine);
                std::cout << "\n";
            }
        }
        
        printIndent(maxLine);

        for (uint i = 0; i < pos.startPos; i++)
            std::cout << (line[i] == '\t' ? "\t" : " ");
        for (uint i = 0; i < pos.endPos - pos.startPos; i++)
            std::cout << color("v");
        
        std::cout << "\n";
        printIndent(maxLine, true);
        std::cout << line << "\n";

        sortSecondaries();

        for (auto note : notes)
        {
            if (note.pos.line == pos.line)
            {
                printIndent(maxLine);
                for (uint i = 0; i < note.pos.startPos; i++)
                    std::cout << (line[i] == '\t' ? "\t" : " ");
                for (uint i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                    std::cout << note.color("^");
                std::cout << " " << note.color(note.subMsg) << "\n";
            }
            else if (note.pos.line > pos.line)
            {
                if (lastLine < note.pos.line - 1) 
                {
                    if (lastLine == note.pos.line - 2)
                        printPaddingLine(maxLine, note.pos.line - 1, note.pos.file);
                    else printPaddingLine(maxLine);
                }
                lastLine = note.pos.line;
                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(maxLine, true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirst = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent(maxLine);
                    for (uint i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirst)
                    {
                        for (uint i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirst = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }
        for (uint i = 0; i < std::to_string(pos.line).size() + 2; i++) std::cout << color("─");
        std::cout << color("╯") << "\n";
        for (auto note : notes)
            if (note.pos.file != pos.file)
                note.show();
    }

    void Error::showBasic(uint maxLine)
    {
        if (msg != "")
            std::cout << color(tyToString() + ": ") << BOLD(msg) << "\n";
        if (pos.file == NULL)
            return;
        for (uint i = 0; i < std::to_string(pos.line).size() + 2; i++) std::cout << " ";
        std::cout << color("╭─ ") << pos.file->getOriginalPath() << color(" ─╴") << "\n";

        printIndent(maxLine);
        std::cout << "\n";
        uint lastLine = 0;
        for (auto note : notes)
        {
            if (note.pos.file == pos.file && note.pos.line < pos.line && note.msg == "")
            {
                if (lastLine != 0 && lastLine < note.pos.line - 1) 
                {
                    if (lastLine == note.pos.line - 2)
                        printPaddingLine(maxLine, note.pos.line - 1, note.pos.file);
                    else printPaddingLine(maxLine);
                }
                lastLine = note.pos.line;

                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(maxLine, true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirst = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent(maxLine);
                    for (uint i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirst)
                    {
                        for (uint i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirst = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }
        string line = getLine(pos.file->getOriginalPath(), pos.line);
        if (lastLine != 0 && lastLine < pos.line - 1) 
        {
            if (lastLine == pos.line - 2)
                printPaddingLine(maxLine, pos.line - 1, pos.file);
            else printPaddingLine(maxLine);
        }
        lastLine = pos.line;
        printIndent(maxLine, true);
        std::cout << line << "\n";
        printIndent(maxLine);
        for (uint i = 0; i < pos.startPos; i++)
            std::cout << (line[i] == '\t' ? "\t" : " ");
        for (uint i = 0; i < pos.endPos - pos.startPos; i++)
            std::cout << color("^");
        
        if (subMsg != "")
        {
            bool isFirst = true;
            for (auto currLine : splitLines(subMsg))
            {
                if (isFirst)
                {
                    std::cout << " " << color(currLine);
                    isFirst = false;
                }
                else
                {
                    std::cout << "\n";
                    printIndent(maxLine);
                    for (uint i = 0; i < pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    std::cout << color(currLine);
                }
                
            }
            

        }
        std::cout << "\n";
        for (auto note : notes)
        {
            if (note.pos.file == pos.file && note.pos.line > pos.line)
            {
                if (lastLine < note.pos.line - 1) 
                {
                    if (lastLine == note.pos.line - 2)
                        printPaddingLine(maxLine, note.pos.line - 1, note.pos.file);
                    else printPaddingLine(maxLine);
                }
                lastLine = note.pos.line;
                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(maxLine, true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirst = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent(maxLine);
                    for (uint i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirst)
                    {
                        for (uint i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirst = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }
        for (uint i = 0; i < std::to_string(pos.line).size() + 2; i++) std::cout << color("─");
        std::cout << color("╯") << "\n";
    }

    void Error::sortSecondaries()
    {
        auto file = pos.file;
        std::sort(
            std::begin(notes), std::end(notes), 
            [file](Error a, Error b) {
                if (a.pos.file == file && b.pos.file != file)
                    return true;
                if (a.pos.file != file && b.pos.file == file)
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
        for (uint i = 0; i < errors.size(); i++)
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

    void Error::printPaddingLine(uint maxLine, uint line, SourceFile *file) 
    {
        uint targetSize = std::to_string(maxLine).size() + 2;
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

    void Error::printIndent(uint maxLine, bool showLine) {
        uint targetSize = std::to_string(maxLine).size() + 2;
        if (showLine)
        {
            auto str = " " + std::to_string(pos.line) + " ";
            while (str.size() < targetSize)
                str += " ";
            std::cout << color(str);
        }
        else 
        {
            for (uint i = 0; i < targetSize; i++)
                std::cout << " ";
        }
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