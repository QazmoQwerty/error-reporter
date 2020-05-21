#include "SourceFile.h"

SourceFile::SourceFile(string path) : _path(path) {}

string removeExtension(string const & filename)
{
    typename string::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != string::npos ? filename.substr(0, p) : filename;
}

string SourceFile::getOriginalPath() 
{
    return _path; 
}

string SourceFile::getName() 
{ 
    return _path.substr(_path.find_last_of("/\\") + 1);
}