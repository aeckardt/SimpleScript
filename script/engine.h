#ifndef ENGINE_H
#define ENGINE_H

#include <string>

#include <QMainWindow>

#include "treewalker.h"

using namespace tw;

class ImageObject : public tw::ParameterObject
{
public:
    ~ImageObject() override {}
    void copyTo(void *&) const override;

    QImage image;
};

class ScriptEngine
{
public:
    ScriptEngine(QMainWindow *parent);

    inline void run(const std::string &str) { tw.run(str); }

    inline void setOutput(const OutputFnc &output) { this->output = output; }
    inline void setErrorOutput(const OutputFnc &fnc) { tw.setErrorOutput(fnc); }

    inline void print(const Parameter &param) { if (output != nullptr) { output(param); } }
    inline void print(std::string &str) { Parameter param; param.assign(str); print(param); }

private:
    TreeWalker tw;
    OutputFnc output;
    QMainWindow *mainWindow;

    friend bool cmdRegister(const ParameterList &, Parameter &);
    friend bool cmdSelect(const ParameterList &, Parameter &);
};

#endif // ENGINE_H
