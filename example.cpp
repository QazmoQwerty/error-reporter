#include "reporter.hpp"

int main() {
    auto file = new reporter::SimpleFile("example.cpp");
    auto file2 = new reporter::SimpleFile("reporter.hpp");
    auto err = reporter::Error(
            "a complex error",
            "this is where the error is, hence the bold red",
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
                  "with another line\nand another", {4, 0, 8, file2})
        .withHelp("something important", {7, 0, 18, file2})
        
        .withHelp("a general help message,\nnot set to any specific position")
        .withNote("can also be a note");

    err.print(std::cerr);
    
    return 0;
}