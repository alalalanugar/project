#include "FileCatch.h"

std::string FileCatch::getPath(const std::string& path)
{
    static std::string(*pathBuilder)(std::string const&) = FileCatch::getPathBuilder();
    return (*pathBuilder)(path);
}

std::string const& FileCatch::getRoot()
{
#pragma warning(suppress : 4996)
    static char const* envRoot = getenv("LOGL_ROOT_PATH");
    static char const* givenRoot = (envRoot != nullptr ? envRoot : "D:/MSc/physically-rendering");
    static std::string root = (givenRoot != nullptr ? givenRoot : "");
    return root;
}

std::string FileCatch::getPathRelativeRoot(const std::string& path)
{
    return getRoot() + std::string("/") + path;
}

std::string FileCatch::getPathRelativeBinary(const std::string& path)
{
    return "../../../" + path;
}