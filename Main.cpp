#include "ErrorReporter.h"

int main() {
    auto file1 = new SourceFile("Main.cpp");
    ErrorReporter::report(ErrorReporter::Error(
        "an error",
        "a type",
        ERR_GENERAL,
        { 4, 4, 8, file1 }
    ));
    ErrorReporter::report(ErrorReporter::Error(
        "a warning",
        "a variable",
        ERR_WARNING,
        { 4, 9, 14, file1 }
    ));
    ErrorReporter::report(ErrorReporter::Error(
        "a note",
        "a string",
        ERR_NOTE,
        { 4, 32, 42, file1 }
    ));

    ErrorReporter::report(ErrorReporter::Error(
        "a complex error",
        "a variable",
        ERR_GENERAL,
        { 4, 9, 14, file1 })
    .withSecondary("a relevant include", { 1, 0, 8, file1 })
    .withSecondary("start of function `main`", { 3, 11, 12, file1 })
    .withSecondary("a type", { 4, 4, 8, file1 })
    .withSecondary("assignment", { 4, 15, 16, file1 }));
    return 0;
}