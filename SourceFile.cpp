#include "SourceFile.h"
#include <unistd.h>

// SourceFile::SourceFile(string path) : _path(path), _backendDIFile(NULL) {}

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

#define PATH_MAX 1000

string SourceFile::getPath() 
{ 
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    auto idx = _path.find_last_of("/\\");
    if (idx == _path.npos)
        return string(cwd);
    else return string(cwd) + "/" + _path.substr(0, idx); 
}