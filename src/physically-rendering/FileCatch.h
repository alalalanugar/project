#pragma once

#include <string>
#include <cstdlib>

class FileCatch
{
public:
    static std::string getPath(const std::string& path);

private:

    static std::string const& getRoot();

    typedef std::string(*Builder) (const std::string& path);
    static Builder getPathBuilder()
    {
        if (getRoot() != "")
            return &FileCatch::getPathRelativeRoot;
        else
            return &FileCatch::getPathRelativeBinary;
    }

    static std::string getPathRelativeRoot(const std::string& path);

    static std::string getPathRelativeBinary(const std::string& path);
};
