#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <string>
#include <map>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "cv.h"

using lx::Lexer;
using lx::Token;
using lx::TokenId;

using ps::Parser;
using ps::Node;

namespace tw
{

enum ParameterType
{
    Empty,
    String,
    Int,
    Float,
    Point,
    Boolean
};

ParameterType getParameterType(const TokenId &token_id);

class Parameter
{
public:
    typedef ParameterType Type;

    Parameter() { value = nullptr; }
    Parameter(const Parameter &src) : Parameter() { operator=(src); }
    ~Parameter() { clear(); }

    void clear();
    bool empty() const { return value == nullptr; }

    const Type &getType() const { return type; }

    void assign(const std::string &str);
    void assign(const int32_t     &i);
    void assign(const double      &f);
    void assign(const cv::Point   &pt);
    void assign(const bool        &b);

    const std::string &asString() const   { return *static_cast<std::string*>(value); }
    int32_t            asInt() const;
    double             asFloat() const;
    const cv::Point   &asPoint() const    { return *static_cast<cv::Point*>(value); }
    bool               asBoolean() const  { return *static_cast<bool*>(value); }

    Parameter &operator=(const Parameter &src);

private:
    Type type;
    void *value;
};

std::ostream &operator <<(std::ostream &os, const Parameter &param);

typedef std::vector<Parameter> ParameterList;

typedef bool(* CommandFnc)(const ParameterList &, Parameter &);
struct Command
{
    std::vector<std::vector<Parameter::Type>> param_types;
    Parameter::Type return_type;
    CommandFnc callback_fnc;
};

class TreeWalker
{
public:
    bool run(const std::string &str);
    bool run(const char *filename);

    inline void registerCommand(const std::string &name, const std::vector<std::vector<Parameter::Type>> &param_types,
        const Parameter::Type &return_type, const CommandFnc &callback_fnc) { commands[name] = {param_types, return_type, callback_fnc}; }

private:
    Lexer lexer;
    Parser parser;

    std::map<std::string, Parameter> vars;
    std::map<std::string, Command> commands;

    Parameter return_value;

    Parameter& addParam(ParameterList &params) const;
    void getParam(const Node &node, Parameter &param);

    bool executeOperation(const lx::TokenId &op, const Parameter &p1, const Parameter &p2);

    bool traverse(const Node &node);
    bool traverseAssignment(const Node &node);
    bool traverseExpr(const Node &node);
    bool traverseFunction(const Node &node);
    bool traverseIfStatement(const Node &node);

    // the following types, variables and functions are solely used for validating the syntax
    typedef std::vector<ParameterType> ParameterTypeList;

    std::map<std::string, ParameterType> var_types;

    ParameterType return_value_type;

    void getParamType(const Node &node, ParameterType &param_type);

    bool validate(const Node &node);
    bool validateAssignment(const Node &node);
    bool validateCommand(const std::string &command, const ParameterTypeList &param_types);
    bool validateExpr(const Node &node);
    bool validateFunction(const Node &node);
    bool validateIfStatement(const Node &node);
    bool validateOperation(const lx::TokenId &op, const ParameterType &pt1, const ParameterType &pt2);
    bool validateParamType(const Node &node, ParameterTypeList *param_types = nullptr);
};

} // namespace tw

#endif // SCRIPTENGINE_H
