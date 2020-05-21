#include "ErrorReporter.h"
void example1() 
{
    auto file = new SourceFile("Main.cpp");
    ErrorReporter::report(ErrorReporter::Error(
        "an error",
        "a type",
        ERR_GENERAL,
        { 4, 4, 8, file }
    ));
    ErrorReporter::report(ErrorReporter::Error(
        "a warning",
        "a variable",
        WRN_GENERAL,
        { 4, 9, 13, file }
    ));
    ErrorReporter::report(ErrorReporter::Error(
        "a secondary",
        "a string",
        NOTE_GENERAL,
        { 4, 31, 41, file }
    ));
}

void example2() 
{
    auto file = new SourceFile("Main.cpp");
    ErrorReporter::report(
        ErrorReporter::Error(
            "a complex error",
            "a variable",
            ERR_GENERAL,
            { 4, 9, 13, file }
        )
        .withNote("a relevant include", { 1, 0, 8, file })
        .withNote("curly brace!", { 3, 1, 2, file })
        .withNote("a type", { 4, 4, 8, file })
        .withNote("assignment", { 4, 14, 15, file })
        .withNote("a variable", { 4, 9, 13, file })
        .withNote("a string", { 4, 31, 41, file })

        .withNote("relevant include in another file\nwith another line\nand another", {1, 0, 8, new SourceFile("ErrorReporter.cpp")})
    );
}

void example3() 
{
    auto file = new SourceFile("Main.cpp");
    ErrorReporter::report(
        ErrorReporter::Error(
            "a complex error",
            "a variable",
            ERR_GENERAL,
            { 1, 9, 13, file }
        )
        .withNote("a relevant include", { 1, 0, 8, file })
        .withNote("curly brace!", { 3, 1, 2, file })
        .withNote("a type", { 4, 4, 8, file })
        .withNote("assignment", { 4, 14, 15, file })
        .withNote("a variable", { 4, 9, 13, file })
        .withNote("a string", { 4, 31, 41, file })

        .withNote("relevant include in another file\nwith another line\nand another", {1, 0, 8, new SourceFile("ErrorReporter.cpp")})
    );
}

void example4() 
{
    auto file = new SourceFile("Main.cpp");
    auto file2 = new SourceFile("ErrorReporter.cpp");
    ErrorReporter::report(
        ErrorReporter::Error(
            "a complex error",
            "this is where the error is, hence the bold red",
            ERR_GENERAL,
            { 4, 9, 13, file }
        )
        .withHelp("a general help message,\nnot set to any specific position")
        .withNote("can also be a note")

        .withNote("a relevant include", { 1, 0, 8, file })
        .withNote("curly brace!", { 3, 0, 1, file })
        .withNote("a type", { 4, 4, 8, file })
        .withNote("assignment", { 4, 14, 15, file })
        .withNote("a variable with a very long explanation\nwhich requires an especially\nlarge number of lines", { 4, 9, 13, file })
        .withHelp("a help message", { 4, 31, 41, file })

        .withNote("relevant include in another file\nwith another line\nand another", {1, 0, 8, file2})

        .withHelp("something important", {5, 4, 25, file2})
    );
}

void helpExample() 
{
    std::cout << BOLD("\nHelp for: \n    `")
              << BOLD(FYEL("Warning(E101): ")) << BOLD("line break negator `..` has no effect`\n\n")
              << "The operator `..` causes the compiler to ignore any newlines occuring before or after the `..`.\n"
              << "This is used to split a single statement into multiple lines.\n\n"
              << BOLD(FBLU("  ╭─ ")) << BOLD("Correct") << BOLD(FBLU(" ─╴")) << "\n"
              << BOLD(FBLU("  │ ")) << "\n"
              << BOLD(FBLU("5 │ ")) << "    a.." << BOLD(FBLK(" <-- line break is ignored")) << "\n"
              << BOLD(FBLU("6 │ ")) << "        := 10\n"
              << BOLD(FBLU("  │ ")) << "\n"
              << BOLD(FBLU("──╯ ")) << "\n\n"

              << "A warning is shown when `..` " << ITLC("doesn't")  << " come before/after a newline, and therefore has no effect:\n\n"
              << BOLD(FBLU("  ╭─ ")) << BOLD("Incorrect") << BOLD(FBLU(" ─╴")) << "\n"
              << BOLD(FBLU("  │ ")) << "\n"
              << BOLD(FBLU("5 │ ")) <<           "    a := ..10" << "\n"
              << BOLD(FBLU("  │ ")) << BOLD(FBLK("         ^^ has no effect, since it does not come before/after a newline")) << "\n"
              << BOLD(FBLU("──╯ ")) << "\n\n"

              << BOLD(FBLK("Note:")) << " `..` might easily be confused with the vararg type operator `...`:\n\n"
              << BOLD(FBLU("  ╭─ ")) << BOLD("Example") << BOLD(FBLU(" ─╴")) << "\n"
              << BOLD(FBLU("  │ ")) "\n"
              << BOLD(FBLU("5 │ ")) <<           "    void Foo(int.. n) {" << "\n"
              << BOLD(FBLU("  │ ")) << BOLD(FBLK("                ^^ should be `...`")) << "\n"
              << BOLD(FBLU("──╯ ")) << "\n";
}

int main() {
    // helpExample();
    // example1();
    // example2();
    example4();
    return 0;
}