#ifndef ENGINE_H
#define ENGINE_H

#include <string>

#include <QMainWindow>

#include "treewalker.h"

class ScriptEngine
{
public:
    ScriptEngine(QMainWindow *parent);

    void run(const std::string &str) { tw.run(str); }
    void setOutput(const tw::OutputFnc &output) { this->output = output; tw.setErrorOutput(output); }

private:
    tw::TreeWalker tw;
    tw::OutputFnc output;
    QMainWindow *mainWindow;

    inline void print(const tw::Parameter &param) { if (output != nullptr) { output(param, QColor(17, 17, 17)); } }

    friend bool cmdPrint(const tw::ParameterList &, tw::Parameter &);
    friend bool cmdSelect(const tw::ParameterList &, tw::Parameter &);
};

#endif // ENGINE_H
