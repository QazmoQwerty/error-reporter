#include "reporter.hpp"

int main() {
    auto file = new reporter::SimpleFile("example.cpp");
    auto file2 = new reporter::SimpleFile("reporter.hpp");
    auto err = reporter::Error(
            "a complex error",
            "this is where the error is, hence the bold red",
            "E308",
            { 4, 9, 13, file }
        )
        .withNote("a relevant include", { 1, 0, 8, file })
        .withNote("curly brace", { 3, 11, 12, file })
        .withNote("a type", { 4, 4, 8, file })
        .withNote("assignment", { 4, 14, 15, file })
        .withNote("a variable with a very long explanation\n"
                  "which requires an especially\n"
                  "large number of lines", { 4, 9, 13, file })
        .withHelp("a help message", { 4, 30, 40, file })

        .withNote("relevant include in another file\n"
                  "with another line\nand another", {26, 0, 8, file2})
        
        .withHelp("a general help message,\nnot set to any specific location")
        .withNote("can also be a note");

    err.print(std::cerr);

    std::cerr << "\n\n";

    reporter::SimpleFile file3("example.cpp");
    reporter::Error("an error", "here", { 4, 14, &file3 })
        .withHelp("help without a location")
        .withNote("a note without a location")
        .withNote("a note with a location", { 4, 14, 10, &file3 })
        .withHelp("help with a location", { 4, 4, 9, &file3 })
        .print(std::cerr);

    std::cerr << "\n\n";

    auto err2    = reporter::Error("an error", "here", { 4, 14, &file3 }).print(std::cerr);
    auto warn   = reporter::Warning("a warning", "here", { 4, 14, &file3 }).print(std::cerr);
    auto note   = reporter::Note("a note", "here", { 4, 14, &file3 }).print(std::cerr);
    auto help   = reporter::Help("some help", "here", { 4, 14, &file3 }).print(std::cerr);
    auto intern = reporter::InternalError("an internal compiler error", "here", { 4, 14, &file3 }).print(std::cerr);

    delete file;
    delete file2;
    return 0;
}