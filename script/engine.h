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

    void run(const std::string &str) { tw.run(str); }
    void setOutput(const OutputFnc &output) { this->output = output; tw.setErrorOutput(output); }

private:
    TreeWalker tw;
    OutputFnc output;
    QMainWindow *mainWindow;

    inline void print(const Parameter &param) { if (output != nullptr) { output(param, QColor(17, 17, 17)); } }

    friend bool cmdPrint(const ParameterList &, Parameter &);
    friend bool cmdSelect(const ParameterList &, Parameter &);
};

#endif // ENGINE_H
