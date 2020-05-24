#include "reporter.hpp"

void example1() {
    auto file = new reporter::SimpleFile("Main.cpp");
    reporter::report(reporter::Error(
        "an error",
        "a type",
        reporter::ERROR,
        { 4, 4, 8, file }
    ));
    reporter::report(reporter::Error(
        "a warning",
        "a variable",
        reporter::WARNING,
        { 4, 9, 13, file }
    )/*.withHelp("this is a message")*/);
    reporter::report(reporter::Error(
        "a note",
        "a string",
        reporter::NOTE,
        { 4, 31, 41, file }
    ));
    reporter::report(reporter::Error(
        "some help",
        "a type",
        reporter::HELP,
        { 4, 4, 8, file }
    ));
}

void example2() {
    auto file = new reporter::SimpleFile("Main.cpp");
    reporter::report(
        reporter::Error(
            "a complex error",
            "a variable",
            reporter::ERROR,
            { 4, 9, 13, file }
        )
        .withNote("a relevant include", { 1, 0, 8, file })
        .withNote("curly brace!", { 3, 1, 2, file })
        .withNote("a type", { 4, 4, 8, file })
        .withNote("assignment", { 4, 14, 15, file })
        .withNote("a variable", { 4, 9, 13, file })
        .withNote("a string", { 4, 31, 41, file })

        .withNote("relevant include in another file\nwith another line\nand another", {1, 0, 8, new reporter::SimpleFile("ErrorReporter.cpp")})
    );
}

void example3() {
    auto file = new reporter::SimpleFile("Main.cpp");
    reporter::report(
        reporter::Error(
            "a complex error",
            "a variable",
            reporter::ERROR,
            { 1, 9, 13, file }
        )
        .withNote("a relevant include", { 1, 0, 8, file })
        .withNote("curly brace!", { 3, 1, 2, file })
        .withNote("a type", { 4, 4, 8, file })
        .withNote("assignment", { 4, 14, 15, file })
        .withNote("a variable", { 4, 9, 13, file })
        .withNote("a string", { 4, 31, 41, file })

        .withNote("relevant include in another file\nwith another line\nand another", {1, 0, 8, new reporter::SimpleFile("ErrorReporter.cpp")})
    );
}

void example4() {
    auto file = new reporter::SimpleFile("Main.cpp");
    auto file2 = new reporter::SimpleFile("ErrorReporter.cpp");
    reporter::report(
        reporter::Error(
            "a complex error",
            "this is where the error is, hence the bold red",
            reporter::ERROR,
            { 4, 9, 13, file }
        )
        
        .withNote("a relevant include", { 1, 0, 8, file })
        .withNote("curly brace", { 3, 16, 17, file })
        .withNote("another curly brace", { 15, 8, 1, file })
        .withNote("a type", { 4, 4, 8, file })
        .withNote("assignment", { 4, 14, 15, file })
        .withNote("a variable with a very long explanation\nwhich requires an especially\nlarge number of lines", { 4, 9, 13, file })
        .withHelp("a help message", { 4, 31, 41, file })
        .withNote("relevant include in another file\nwith another line\nand another", {1, 0, 8, file2})
        .withHelp("something important", {5, 4, 25, file2})

        .withHelp("a general help message,\nnot set to any specific position")
        .withNote("can also be a note")
    );
}

// void helpExample() {
//     #include "TerminalColors.h"
//     std::cout << BOLD("\nHelp for: \n    `")
//               << BOLD(FYEL("Warning(E101): ")) << BOLD("line break negator `..` has no effect`\n\n")
//               << "The operator `..` causes the compiler to ignore any newlines occuring before or after the `..`.\n"
//               << "This is used to split a single statement into multiple lines.\n\n"
//               << BOLD(FBLU("  ╭─ ")) << BOLD("Correct") << BOLD(FBLU(" ─╴")) << "\n"
//               << BOLD(FBLU("  │ ")) << "\n"
//               << BOLD(FBLU("5 │ ")) << "    a.." << BOLD(FBLK(" <-- line break is ignored")) << "\n"
//               << BOLD(FBLU("6 │ ")) << "        := 10\n"
//               << BOLD(FBLU("  │ ")) << "\n"
//               << BOLD(FBLU("──╯ ")) << "\n\n"

//               << "A warning is shown when `..` " << ITLC("doesn't")  << " come before/after a newline, and therefore has no effect:\n\n"
//               << BOLD(FBLU("  ╭─ ")) << BOLD("Incorrect") << BOLD(FBLU(" ─╴")) << "\n"
//               << BOLD(FBLU("  │ ")) << "\n"
//               << BOLD(FBLU("5 │ ")) <<           "    a := ..10" << "\n"
//               << BOLD(FBLU("  │ ")) << BOLD(FBLK("         ^^ has no effect, since it does not come before/after a newline")) << "\n"
//               << BOLD(FBLU("──╯ ")) << "\n\n"

//               << BOLD(FBLK("Note:")) << " `..` might easily be confused with the vararg type operator `...`:\n\n"
//               << BOLD(FBLU("  ╭─ ")) << BOLD("Example") << BOLD(FBLU(" ─╴")) << "\n"
//               << BOLD(FBLU("  │ ")) "\n"
//               << BOLD(FBLU("5 │ ")) <<           "    void Foo(int.. n) {" << "\n"
//               << BOLD(FBLU("  │ ")) << BOLD(FBLK("                ^^ should be `...`")) << "\n"
//               << BOLD(FBLU("──╯ ")) << "\n";
// }

int main() {
    // helpExample();
    std::cout << "\n";
    example1();
    std::cout << "\n";
    // example2();
    example4();
    std::cout << "\n";
    return 0;
}