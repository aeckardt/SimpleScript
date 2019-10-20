#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "script.h"

static ScriptEngine *pengine;

//#define SCRIPT_LOG_MESSAGE

bool cmdOpenURL(const ParameterList &params, Parameter &)
{
#ifdef SCRIPT_LOG_MESSAGE
    std::cout << "opening URL: " << params[0].asString() << std::endl;
#endif
#if !defined(_WIN32)
    system(("open " + params[0].asString()).c_str());
#else
    system(("start " + params[0].asString()).c_str());
#endif
    return true;
}

bool cmdPrint(const ParameterList &params, Parameter &)
{
    if (params.empty())
    {
        std::cout << std::endl;
        return true;
    }

    std::cout << params[0] << std::endl;

    return true;
}

bool cmdSleep(const ParameterList &params, Parameter &)
{
    useconds_t msec;
    switch (params[0].getType())
    {
    case Int:
        msec = static_cast<useconds_t>(params[0].asInt() * 1000000);
        break;
    case Float:
        msec = static_cast<useconds_t>(params[0].asFloat() * 1000000.0);
        break;
    default:
        msec = 0;
        break;
    }

#ifdef SCRIPT_LOG_MESSAGE
    std::cout << "Sleeping milliseconds: " << (msec / 1000) << std::endl;
#endif

    usleep(msec);

    return true;
}

ScriptEngine::ScriptEngine()
{
    tw.registerCommand("open",
        {{String}}, Empty,
        cmdOpenURL);

    tw.registerCommand("print",
        {{Empty, String, Int, Float, Point, Boolean}}, Empty,
        cmdPrint);

    tw.registerCommand("sleep",
        {{Int, Float}}, Empty,
        cmdSleep);

    pengine = this;
}
