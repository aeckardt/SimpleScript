#ifndef SCRIPT_H
#define SCRIPT_H

#include "treewalker.h"

using namespace tw;

class ScriptEngine
{
public:
    ScriptEngine();

    void run(const std::string &str) { tw.run(str); }
    void run(const char *filename) { tw.run(filename); }

private:
    TreeWalker tw;

    friend bool cmdRegister(const ParameterList &, Parameter &);
};

#endif // SCRIPT_H
