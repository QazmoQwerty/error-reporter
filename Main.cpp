#include "ErrorReporter.h"

int main() {
    auto fl = new SourceFile("ErrorReporter.cpp");
    ErrorReporter::Error(
        "types don't match", 
        "types don't match",
        ErrorReporter::ERROR,
        {11, 21, 23, fl}
    ).withSecondary(
        "right type is `long`",
        {11, 24, 28, fl}
    ).withSecondary(
        "left type is `string`",
        {11, 12, 20, fl}
    ).withSecondary(
        "look here",
        {5, 12, 20, fl}
    ).show();
    // ErrorReporter::report("this is a warning", 1, {7, 12, 20, new SourceFile("ErrorReporter.cpp")});
    // ErrorReporter::report("this is a note", 2, {4, 4, 20, new SourceFile("Main.cpp")});
    // ErrorReporter::report("this is an error", 12, {20, 4, 6, new SourceFile("ErrorReporter.h")});
    // ErrorReporter::showAll();
    return 0;
}