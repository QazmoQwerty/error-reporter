#include "ErrorReporter.h"

namespace ErrorReporter 
{
    void Error::show()
    {
        if (notes.size() == 0)
            return showBasic();
        
        std::cout << color(tyToString() + ": ") << BOLD(msg) << "\n";
        if (pos.file == NULL)
            return;
        // uncomment this for uglier, but clickable links
        // std::cout << color(" --> ") << "./" << pos.file->getOriginalPath() << ":" << pos.line << ":" << pos.startPos << "\n";
        for (int i = 1; i < std::to_string(pos.line).size(); i++) std::cout << " ";
        std::cout << color(" --> ") << pos.file->getOriginalPath() << "\n";

        
        string line = getLine(pos.file->getOriginalPath(), pos.line);

        printIndent();
        if (subMsg != "")
        {
            for (int i = 0; i < pos.startPos; i++)
                std::cout << (line[i] == '\t' ? "    " : " ");
            std::cout << color(subMsg);
        }
        std::cout << "\n";
        printIndent();
        for (int i = 0; i < pos.startPos; i++)
            std::cout << (line[i] == '\t' ? "    " : " ");
        for (int i = 0; i < pos.endPos - pos.startPos; i++)
            std::cout << color("v");
        
        std::cout << "\n";
        printIndent(true);
        std::cout << line << "\n";

        sortSecondaries();
        for (auto note : notes)
        {
            if (note.pos.line == pos.line)
            {
                printIndent();
                for (int i = 0; i < note.pos.startPos; i++)
                    std::cout << (line[i] == '\t' ? "    " : " ");
                for (int i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                    std::cout << note.color("^");
                std::cout << " " << note.color(note.msg) << "\n";
            }
            else note.show();
        }
        std::cout << "\n";
    }

    void Error::showBasic()
    {
        std::cout << color(tyToString() + ": ") << BOLD(msg) << "\n";
        if (pos.file == NULL)
            return;
        // uncomment this for uglier, but clickable links
        // std::cout << color(" --> ") << "./" << pos.file->getOriginalPath() << ":" << pos.line << ":" << pos.startPos << "\n";
        for (int i = 1; i < std::to_string(pos.line).size(); i++) std::cout << " ";
        std::cout << color(" --> ") << pos.file->getOriginalPath() << "\n";
        printIndent();
        std::cout << "\n";
        string line = getLine(pos.file->getOriginalPath(), pos.line);
        printIndent(true);
        std::cout << line << "\n";
        printIndent();
        for (int i = 0; i < pos.startPos; i++)
            std::cout << (line[i] == '\t' ? "    " : " ");
        for (int i = 0; i < pos.endPos - pos.startPos; i++)
            std::cout << color("^");
        
        if (subMsg != "")
            std::cout << " " << color(subMsg);
    }


    void Error::sortSecondaries()
    {
        std::sort(
            std::begin(notes), std::end(notes), 
            [](Error a, Error b) {return a.pos.startPos > b.pos.startPos; }
        );
    }

    // void sort() {
    //     std::sort(std::begin(vec ), std::end(vec ), [](int a, int b) {return a > b; });
    // }
    

    vector<Error> errors;

    Error report(string msg, uint errCode, Position pos, bool isFatal)
    {
        errors.push_back(Error(msg, errCode, pos));
        if (isFatal)
            throw "[Aborting due to previous error]";
        return errors.back();
    }

    Error reportInternal(string msg, uint errCode, Position pos)
    {
        errors.push_back({ msg, errCode, pos });
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
        switch (errTy)
        {
            case INTERNAL: return "Internal Error";
            case WARNING: return "Warning";
            case NOTE: return "Note";
            default: return "Error";
        }
    }

    string Error::color(string str)
    {
        switch (errTy)
        {
            default: return BOLD(FRED(str));
            case WARNING: return BOLD(FBLU(str));
            case NOTE: return BOLD(FBLK(str));
        }
    }

    void Error::printIndent(bool showLine) {
        if (showLine)
        {
            auto str = " " + std::to_string(pos.line) + " ";
            std::cout << color(str);
        }
        else 
        {
            for (uint i = 0; i < std::to_string(pos.line).size() + 2; i++)
                std::cout << " ";
        }
        std::cout << color("| ");
    }

    void show(Error &err) 
    {
        // if (err.pos.file != NULL)
        // {   
        //     std::cout << color(" --> ", err) << BOLD("`" + err.pos.file->getOriginalPath() + "`, line " + std::to_string(err.pos.line)) "\n";
        //     printIndent(err);
        //     std::cout << "\n";
        //     string line = getLine(err.pos.file->getOriginalPath(), err.pos.line);
        //     printIndent(err, true);
        //     std::cout << line << "\n";
        //     printIndent(err);
        //     for (int i = 0; i < err.pos.startPos; i++)
        //     {
        //         if (line[i] == '\t')
        //             std::cout << "\t";    
        //         else std::cout << " ";
        //     }
        //     for (int i = 0; i < err.pos.endPos - err.pos.startPos; i++)
        //         std::cout << color("^", err);
        //     std::cout << "\n";
        // }

        // std::cout << color(toString(err) + ": ", err) << err.msg << "\n";        
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