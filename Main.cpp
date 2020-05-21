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
        ERR_WARNING,
        { 4, 9, 13, file }
    ));
    ErrorReporter::report(ErrorReporter::Error(
        "a secondary",
        "a string",
        ERR_NOTE,
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
        .withSecondary("a relevant include", { 1, 0, 8, file })
        .withSecondary("start of function `main`", { 2, 15, 16, file })
        .withSecondary("a type", { 4, 4, 8, file })
        .withSecondary("assignment", { 4, 14, 15, file })

        .withSecondary("relevant include in another file\nwith another line\nand another", {1, 0, 8, new SourceFile("ErrorReporter.cpp")})
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
    example1();
    example2();
    return 0;
}