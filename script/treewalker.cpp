#include <math.h>
#include <sys/time.h>

#include "treewalker.h"

using lx::TokenList;

using namespace tw;

//*************************************************************************//
//
// Parameter class implementation
//
//*************************************************************************//

const std::map<Parameter::Type, std::string> type_names = {
    {Empty,     "Empty"}, {String,    "String"},  {Int,         "Int"},
    {Float,     "Float"}, {Boolean,  "Boolean"},  {Point,     "Point"},
    {Rect,       "Rect"}, {DateTime, "DateTime"}, {Object,   "Object"}};

ParameterType tw::getParameterType(const TokenId &token_id)
{
    switch (token_id)
    {
    case lx::Integer:
        return Int;
    case lx::Float:
        return Float;
    case lx::String:
        return String;
    default:
        return Empty;
    }
}

void Parameter::clear()
{
    if (empty())
        return;
    switch (type)
    {
    case Empty:
        // do nothing...
        break;
    case String:
        delete static_cast<std::string*>(value);
        break;
    case Int:
        delete static_cast<int32_t*>(value);
        break;
    case Float:
        delete static_cast<double*>(value);
        break;
    case Boolean:
        delete static_cast<bool*>(value);
        break;
    case Point:
        delete static_cast<QPoint*>(value);
        break;
    case Rect:
        delete static_cast<QRect*>(value);
        break;
    case DateTime:
        delete static_cast<QDateTime*>(value);
        break;
    case Object:
        delete static_cast<ParameterObject*>(value);
        break;
    }
    type = Empty;
    value = nullptr;
}

void Parameter::assign(const std::string &str)
{
    clear();
    value = new std::string(str);
    type = String;
}

void Parameter::assign(const int32_t &i)
{
    clear();
    value = new int32_t;
    *static_cast<int32_t*>(value) = i;
    type = Int;
}

void Parameter::assign(const double &f)
{
    clear();
    value = new double;
    *static_cast<double*>(value) = f;
    type = Float;
}

void Parameter::assign(const bool &b)
{
    clear();
    value = new bool;
    *static_cast<bool*>(value) = b;
    type = Boolean;
}

void Parameter::assign(const QPoint &pt)
{
    clear();
    value = new QPoint(pt);
    type = Point;
}

void Parameter::assign(const QRect &rect)
{
    clear();
    value = new QRect(rect);
    type = Rect;
}

void Parameter::assign(const QDateTime &dt)
{
    clear();
    value = new QDateTime(dt);
    type = DateTime;
}

double Parameter::asFloat() const
{
    if (type == Float)
        return *static_cast<double*>(value);
    else if (type == Int)
    {
        int32_t val = *static_cast<int32_t*>(value);
        return static_cast<double>(val);
    }
    return 0;
}

int32_t Parameter::asInt() const
{
    if (type == Int)
        return *static_cast<int32_t*>(value);
    else if (type == Float)
    {
        double val = *static_cast<double*>(value);
        return static_cast<int32_t>(val);
    }
    return 0;
}

Parameter &Parameter::operator=(const Parameter &src)
{
    clear();
    switch (src.getType())
    {
    case Empty:
        type = Empty;
        break;
    case String:
        assign(src.asString());
        break;
    case Int:
        assign(src.asInt());
        break;
    case Float:
        assign(src.asFloat());
        break;
    case Boolean:
        assign(src.asBoolean());
        break;
    case Point:
        assign(src.asPoint());
        break;
    case Rect:
        assign(src.asRect());
        break;
    case DateTime:
        assign(src.asDateTime());
        break;
    case Object:
        static_cast<ParameterObject*>(src.value)->copyTo(value);
        type = Object;
        break;
    }
    return *this;
}

std::ostream &tw::operator<<(std::ostream &os, const Parameter &param)
{
    switch (param.getType())
    {
    case Empty:
        return os;
    case String:
        return os << param.asString();
    case Int:
        return os << param.asInt();
    case Float:
        return os << param.asFloat();
    case Boolean:
        return os << (param.asBoolean() ? "true" : "false");
    case Point: {
        const QPoint &pt = param.asPoint();
        return os << "(" << pt.x() << ", " << pt.y() << ")";
    }
    case Rect: {
        const QRect &rect = param.asRect();
        return os << "(" << rect.x() << ", " << rect.y() << ", " << rect.width() << ", " << rect.height() << ")";
    }
    case DateTime: {
        const QDateTime &dt = param.asDateTime();
        return os << dt.toString("yyyy-MM-dd HH:mm:ss").toStdString();
    }
    default:
        return os;
    }
}

ParameterObject::~ParameterObject()
{
}

//*************************************************************************//
//
// TreeWalker class implementation
//
//*************************************************************************//

// Comparison range for floating types
#define FLOAT_CMP_EPSILON 0.0001

bool floatEqual(const double &f1, const double &f2)
{
    return fabs(f1 - f2) < FLOAT_CMP_EPSILON;
}

TreeWalker::TreeWalker()
{
    output_fnc = nullptr;
}

inline Parameter &TreeWalker::addParam(ParameterList &params) const
{
    params.push_back(Parameter());
    return *params.rbegin();
}

inline void TreeWalker::errorMsg(const char *msg) const
{
    if (output_fnc != nullptr)
    {
        Parameter param;
        param.assign(std::string(msg));
        output_fnc(param, Qt::darkRed);
    }
}

#define ERROR_MSG(...) { std::stringstream ss; ss << __VA_ARGS__; errorMsg(ss.str().c_str()); }

bool TreeWalker::executeOperation(const lx::TokenId &op, const Parameter &p1, const Parameter &p2)
{
    switch (op)
    {
    case lx::Plus:
    {
        if (p1.getType() != p2.getType())
        {
            if ((p1.getType() == Int   && p2.getType() == Float) ||
                (p1.getType() == Float && p2.getType() == Int))
            {
                return_value.assign(p1.asFloat() + p2.asFloat());
                return true;
            }
            else
                return false;
        }

        switch (p1.getType())
        {
        case String:
            return_value.assign(p1.asString() + p2.asString());
            break;
        case Int:
            return_value.assign(p1.asInt() + p2.asInt());
            break;
        case Float:
            return_value.assign(p1.asFloat() + p2.asFloat());
            break;
        case Boolean:
            return_value.assign(p1.asBoolean() + p2.asBoolean());
            break;
        case Point:
            return_value.assign(p1.asPoint() + p2.asPoint());
            break;
        default:
            return false;
        }
        return true;
    }
    case lx::Minus:
    {
        if (p1.getType() != p2.getType())
        {
            if ((p1.getType() == Int   && p2.getType() == Float) ||
                (p1.getType() == Float && p2.getType() == Int))
            {
                return_value.assign(p1.asFloat() - p2.asFloat());
                return true;
            }
            else
                return false;
        }

        switch (p1.getType())
        {
        case Int:
            return_value.assign(p1.asInt() - p2.asInt());
            break;
        case Float:
            return_value.assign(p1.asFloat() - p2.asFloat());
            break;
        case Boolean:
            return_value.assign(p1.asBoolean() - p2.asBoolean());
            break;
        case Point:
            return_value.assign(p1.asPoint() - p2.asPoint());
            break;
        default:
            return false;
        }
        return true;
    }
    case lx::Star:
    {
        if (p1.getType() != p2.getType())
        {
            if ((p1.getType() == Int   && p2.getType() == Float) ||
                (p1.getType() == Float && p2.getType() == Int))
            {
                return_value.assign(p1.asFloat() * p2.asFloat());
                return true;
            }
            else if (p1.getType() == Point && p2.getType() == Int)
                return_value.assign(p1.asPoint() * p2.asInt());
            else if (p1.getType() == Point && p2.getType() == Float)
                return_value.assign(p1.asPoint() * p2.asFloat());
            else if (p1.getType() == Int   && p2.getType() == Point)
                return_value.assign(p1.asInt()   * p2.asPoint());
            else if (p1.getType() == Float && p2.getType() == Point)
                return_value.assign(p1.asFloat() * p2.asPoint());
            else
                return false;
            return true;
        }

        switch (p1.getType())
        {
        case Int:
            return_value.assign(p1.asInt() * p2.asInt());
            break;
        case Float:
            return_value.assign(p1.asFloat() * p2.asFloat());
            break;
        case Boolean:
            return_value.assign(p1.asBoolean() * p2.asBoolean());
            break;
        default:
            return false;
        }
        return true;
    }
    case lx::Slash:
    {
        if (p2.getType() == Int)
        {
            if (p2.asInt() == 0)
            {
                errorMsg("Division by zero not allowed");
                return false;
            }
        }
        else if (p2.getType() == Float)
        {
            if (fabs(p2.asFloat()) == 0)
            {
                errorMsg("Division by zero not allowed");
                return false;
            }
        }

        if (p1.getType() != p2.getType())
        {
            if ((p1.getType() == Int   && p2.getType() == Float) ||
                (p1.getType() == Float && p2.getType() == Int))
            {
                return_value.assign(p1.asFloat() / p2.asFloat());
                return true;
            }
            else if (p1.getType() == Point && p2.getType() == Int)
                return_value.assign(p1.asPoint() / p2.asInt());
            else if (p1.getType() == Point && p2.getType() == Float)
                return_value.assign(p1.asPoint() / p2.asFloat());
            else
                return false;
            return true;
        }

        switch (p1.getType())
        {
        case Int:
            return_value.assign(p1.asInt() / p2.asInt());
            break;
        case Float:
            return_value.assign(p1.asFloat() / p2.asFloat());
            break;
        default:
            return false;
        }
        return true;
    }
    case lx::EqualEqual:
    {
        if (p1.getType() != p2.getType())
        {
            if ((p1.getType() == Int   && p2.getType() == Float) ||
                (p1.getType() == Float && p2.getType() == Int))
            {
                return_value.assign(floatEqual(p1.asFloat(), p2.asFloat()));
                return true;
            }
            else
                return false;
        }

        switch (p1.getType())
        {
        case String:
            return_value.assign(p1.asString() == p2.asString());
            return true;
        case Int:
            return_value.assign(p1.asInt() == p2.asInt());
            return true;
        case Float:
            return_value.assign(floatEqual(p1.asFloat(), p2.asFloat()));
            return true;
        case Boolean:
            return_value.assign(p1.asBoolean() == p2.asBoolean());
            return true;
        case Point:
            return_value.assign(p1.asPoint() == p2.asPoint());
            return true;
        case Rect:
            return_value.assign(p1.asRect() == p2.asRect());
            return true;
        case DateTime:
            return_value.assign(p1.asDateTime() == p2.asDateTime());
            return true;
        default:
            return false;
        }
    }
    case lx::NotEqual:
    {
        if (p1.getType() != p2.getType())
        {
            if ((p1.getType() == Int   && p2.getType() == Float) ||
                (p1.getType() == Float && p2.getType() == Int))
            {
                return_value.assign(!floatEqual(p1.asFloat(), p2.asFloat()));
                return true;
            }
            else
                return false;
        }

        switch (p1.getType())
        {
        case String:
            return_value.assign(p1.asString() != p2.asString());
            return true;
        case Int:
            return_value.assign(p1.asInt() != p2.asInt());
            return true;
        case Float:
            return_value.assign(!floatEqual(p1.asFloat(), p2.asFloat()));
            return true;
        case Boolean:
            return_value.assign(p1.asBoolean() != p2.asBoolean());
            return true;
        case Point:
            return_value.assign(p1.asPoint() != p2.asPoint());
            return true;
        case Rect:
            return_value.assign(p1.asRect() != p2.asRect());
            return true;
        case DateTime:
            return_value.assign(p1.asDateTime() != p2.asDateTime());
            return true;
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

void TreeWalker::getParam(const Node &node, Parameter &param)
{
    const std::string &content = node.param.getText();

    switch (node.param.getId())
    {
    case lx::Term:
        param = vars.find(content)->second;
        break;
    case lx::Integer:
        param.assign(atoi(content.c_str()));
        break;
    case lx::Float:
        param.assign(atof(content.c_str()));
        break;
    case lx::String:
        param.assign(std::string(content.begin() + 1, content.end() - 1));
        break;
    default:
        errorMsg("param_token.id out of range");
        break;
    }
}

void TreeWalker::getParamType(const Node &node, ParameterType &param_type)
{
    const std::string &content = node.param.getText();

    switch (node.param.getId())
    {
    case lx::Term:
        param_type = vars.find(content)->second.getType();
        return;
    case lx::Integer:
        param_type = Parameter::Type::Int;
        return;
    case lx::Float:
        param_type = Parameter::Type::Float;
        return;
    case lx::String:
        param_type = Parameter::Type::String;
        return;
    default:
        errorMsg("param_token.id out of range");
        break;
    }
}

bool TreeWalker::run(const std::string &str)
{
    TokenList tokens;
    Lexer lexer;
    lexer.lex(str, tokens);

    if (!lexer.getLastError().empty())
    {
        errorMsg("Error lexing:");
        errorMsg(lexer.getLastError().c_str());
        return false;
    }

    Node ast_root;
    Parser parser;
    parser.parse(tokens, ast_root);
    if (!parser.getLastError().empty())
    {
        errorMsg("Error parsing:");
        errorMsg(parser.getLastError().c_str());
        return false;
    }

    if (validate(ast_root))
    {
        if (!traverse(ast_root))
        {
            errorMsg("Error running script");
            return false;
        }
    }
    else
    {
        errorMsg("Error validating syntax");
        return false;
    }

    return true;
}

bool TreeWalker::traverse(const Node &node)
{
    switch (node.rule)
    {
    case ps::Section:
        for (auto const &child : node.children)
            if (!traverse(child))
                return false;
        break;
    case ps::IfStatement:
        return traverseIfStatement(node);
    case ps::Assignment:
        return traverseAssignment(node);
    case ps::Function:
        return traverseFunction(node);
    case ps::Expr:
        return traverseExpr(node);
    default:
        return false;
    }

    return true;
}

bool TreeWalker::traverseAssignment(const Node &node)
{
    const Node &var_node = *node.children.begin();
    const std::string &var_name = var_node.param.getText();

    const Node &src_node = *node.children.rbegin();
    if (src_node.rule == ps::Function || src_node.rule == ps::Expr)
    {
        if (!traverse(src_node))
            return false;
        vars[var_name] = return_value;
    }
    else if (src_node.rule == ps::Variable)
    {
        const std::string &src_name = src_node.param.getText();
        return_value = vars[src_name];
    }
    else if (src_node.rule == ps::Param)
        getParam(src_node, return_value);

    vars[var_name] = return_value;

    return_value.clear();

    return true;
}

bool TreeWalker::traverseExpr(const Node &node)
{
    return_value.clear();

    std::vector<Parameter> stack;
    for (const Node &child : node.children)
    {
        if (lx::isOperator(child.param.getId()))
        {
            Parameter p2 = *stack.rbegin();
            stack.pop_back();

            Parameter p1 = *stack.rbegin();
            stack.pop_back();

            if (!executeOperation(child.param.getId(), p1, p2))
                return false;
            stack.push_back(return_value);

            continue;
        }

        switch (child.rule)
        {
        case ps::Function:
        case ps::Expr:
            if (!traverse(child))
                return false;
            stack.push_back(return_value);
            break;
        case ps::Variable:
        {
            const std::string &var_name = child.param.getText();
            stack.push_back(vars[var_name]);
            break;
        }
        case ps::Param: {
            getParam(child, addParam(stack));
            break;
        }
        default:
            return false;
        }
    }

    if (stack.size() != 1)
    {
        errorMsg("Unable to evaluate expression");
        return false;
    }

    return_value = stack[0];
    return true;
}

bool TreeWalker::traverseFunction(const Node &node)
{
    ParameterList params;
    for (const Node &child : node.children)
    {
        return_value.clear();
        if (child.rule == ps::Function || child.rule == ps::Expr)
        {
            if (!traverse(child))
                return false;
            params.push_back(return_value);
        }
        else if (child.rule == ps::Param || child.rule == ps::Variable)
        {
            params.push_back(Parameter());
            Parameter &param = *params.rbegin();
            getParam(child, param);
        }
    }

    const std::string &cmd_name = node.param.getText();

    return_value.clear();

    // existance of the command is already proven in validityCheck
    if (!commands[cmd_name].callback_fnc(params, return_value))
        return false;

    return true;
}

bool TreeWalker::traverseIfStatement(const Node &node)
{
    // Check expression in if-statement
    ps::tree_pos tp = node.children.begin();

    const Node &expr_node = *tp;
    if (expr_node.rule == ps::Function || expr_node.rule == ps::Expr)
    {
        if (!traverse(expr_node))
            return false;
    }
    else if (expr_node.rule == ps::Variable || expr_node.rule == ps::Param)
        getParam(expr_node, return_value);

    // evaluate expression
    bool exprIsTrue;
    if (return_value.empty())
        exprIsTrue = false;
    else if (return_value.getType() != Boolean)
        exprIsTrue = true;
    else
        exprIsTrue = return_value.asBoolean();

    return_value.clear();

    if (exprIsTrue)
    {
        // execute if-section
        const Node &section_node = *(++tp);
        if (!traverse(section_node))
            return false;
    }
    else if (node.children.size() == 3)
    {
        // an else-section exists
        const Node &else_section_node = *(++++tp);
        if (!traverse(else_section_node))
            return false;
    }

    return true;
}

bool TreeWalker::validate(const Node &node)
{
    switch (node.rule)
    {
    case ps::Section:
        for (auto const &child : node.children)
            if (!validate(child))
                return false;
        break;
    case ps::Assignment:
        return validateAssignment(node);
    case ps::Expr:
        return validateExpr(node);
    case ps::IfStatement:
        return validateIfStatement(node);
    case ps::Function:
        return validateFunction(node);
    case ps::Variable:
    case ps::Param:
        return validateParamType(node);
    default:
        errorMsg("Unspecified rule");
        return false;
    }

    return true;
}

bool TreeWalker::validateAssignment(const Node &node)
{
    if (node.children.size() < 2)
    {
        errorMsg("Invalid assignment");
        return false;
    }

    const Node &var_node = *node.children.begin();
    if (var_node.rule != ps::Variable)
    {
        errorMsg("Expression is not assignable, lvalue needs to be variable");
        return false;
    }

    const Node &src_node = *node.children.rbegin();
    if (src_node.rule != ps::Variable && src_node.rule != ps::Param &&
        src_node.rule != ps::Function && src_node.rule != ps::Expr)
    {
        errorMsg("Invalid rvalue for assignment");
        return false;
    }

    if (src_node.rule == ps::Function || src_node.rule == ps::Expr)
    {
        if (!validate(src_node))
            return false;
    }
    else if (src_node.rule == ps::Variable)
    {
        const std::string &var_name = src_node.param.getText();
        if (var_types.find(var_name) == var_types.end())
        {
            ERROR_MSG("Variable '" << var_name << "' not found")
            return false;
        }
        return_value_type = var_types[var_name];
    }
    else // rule is Int, Float or String
        return_value_type = getParameterType(src_node.param.getId());

    const std::string &var_name = var_node.param.getText();
    var_types[var_name] = return_value_type;

    return true;
}

bool TreeWalker::validateCommand(const std::string &command, const ParameterTypeList &param_types)
{
    // check if function with that name exists
    if (commands.find(command) == commands.end())
    {
        ERROR_MSG("Unknown function '" << command << "'")
        return false;
    }

    const Command &cmd = commands.find(command)->second;

    if (cmd.param_types.size() < param_types.size())
    {
        ERROR_MSG("Too many arguments passed to function '" << command << "', expected " << cmd.param_types.size())
        return false;
    }

    // check if number and type of arguments are correct
    size_t index;
    bool validParam;
    for (index = 0; index < cmd.param_types.size(); index++)
    {
        validParam = false;
        for (const ParameterType &type : cmd.param_types[index])
        {
            if (param_types.size() <= index && type == Empty)
            {
                validParam = true;
                break;
            }
            else if (param_types.size() <= index)
            {
                ERROR_MSG("Error: Wrong number of arguments passed to function '" << command << "', expected " << cmd.param_types.size())
                return false;
            }
            else if (param_types[index] == type)
            {
                validParam = true;
                break;
            }
        }
        if (!validParam)
        {
            std::stringstream ss;
            ss << "Error: Wrong type of argument passed to function '" << command << "', expected ";
            if (cmd.param_types[index].size() > 1)
                ss << "{";
            bool first = true;
            for (const Parameter::Type &type : cmd.param_types[index])
            {
                if (!first) ss << ", ";
                ss << type_names.at(type);
                first = false;
            }
            if (cmd.param_types[index].size() > 1)
                ss << "}";
            ss << " at position " << (index + 1);
            errorMsg(ss.str().c_str());
            return false;
        }
    }

    return_value_type = cmd.return_type;

    return true;
}

bool TreeWalker::validateExpr(const Node &node)
{
    if (node.children.size() < 2)
    {
        errorMsg("Invalid expression, needs to have at least two operands");
        return false;
    }

    for (const Node &child : node.children)
    {
        if (!child.param.hasValue())
        {
            errorMsg("Invalid expression statement, needs to have a value");
            return false;
        }
    }

    // evaluating RPN (reverse polish notation)
    return_value_type = Empty;

    std::vector<ParameterType> stack;
    for (const Node &child : node.children)
    {
        if (lx::isOperator(child.param.getId()))
        {
            ParameterType p2 = *stack.rbegin();
            stack.pop_back();

            ParameterType p1 = *stack.rbegin();
            stack.pop_back();

            if (!validateOperation(child.param.getId(), p1, p2))
                return false;
            stack.push_back(return_value_type);

            continue;
        }

        switch (child.rule)
        {
        case ps::Function:
        case ps::Expr:
            if (!validate(child))
                return false;
            stack.push_back(return_value_type);
            break;
        case ps::Variable:
        {
            const std::string &var_name = child.param.getText();
            stack.push_back(var_types[var_name]);
            break;
        }
        case ps::Param: {
            stack.push_back(ParameterType());
            getParamType(child, *stack.rbegin());
            break;
        }
        default:
            return false;
        }
    }

    if (stack.size() != 1)
    {
        errorMsg("Unable to validate expression");
        return false;
    }

    return_value_type = stack[0];

    return true;
}

bool TreeWalker::validateFunction(const Node &node)
{
    std::vector<Parameter::Type> param_types;
    for (auto const &child : node.children)
    {
        if (child.rule != ps::Function && child.rule != ps::Param &&
            child.rule != ps::Variable && child.rule != ps::Expr)
        {
            errorMsg("Invalid function parameter");
            return false;
        }
        else if (child.rule == ps::Function || child.rule == ps::Expr)
        {
            return_value_type = Empty;
            if (!validate(child))
                return false;
            param_types.push_back(return_value_type);
        }
        else if (child.rule == ps::Param || child.rule == ps::Variable)
        {
            if (!validateParamType(child, &param_types))
                return false;
        }
        else
        {
            errorMsg("Invalid function parameter");
            return false;
        }
    }

    const std::string &cmd_name = node.param.getText();

    return validateCommand(cmd_name, param_types);
}

bool TreeWalker::validateIfStatement(const Node &node)
{
    if (node.children.size() < 2 || node.children.size() > 3)
    {
        errorMsg("Invalid if-statement");
        return false;
    }

    ps::tree_pos tp = node.children.begin();

    const Node &expr_node = *tp;
    if (!validate(expr_node))
        return false;

    const Node &section_node = *(++tp);
    if (!validate(section_node))
        return false;

    if (node.children.size() == 3)
    {
        const Node &else_section_node = *(++tp);
        if (!validate(else_section_node))
            return false;
    }

    return true;
}

bool TreeWalker::validateOperation(const lx::TokenId &op, const ParameterType &pt1, const ParameterType &pt2)
{
    switch (op)
    {
    case lx::Plus:
    {
        if (pt1 != pt2)
        {
            if ((pt1 == Int && pt2 == Float) || (pt1 == Float && pt2 == Int))
            {
                return_value_type = Float;
                return true;
            }
            else
            {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }
        else if (pt1 == Rect || pt1 == Object)
        {
            ERROR_MSG("Operator '+' not applicable for parameter type: " << type_names.at(pt1))
            return false;
        }

        return_value_type = pt1;
        return true;
    }
    case lx::Minus:
    {
        if (pt1 != pt2)
        {
            if ((pt1 == Int && pt2 == Float) || (pt1 == Float && pt2 == Int))
            {
                return_value_type = Float;
                return true;
            }
            else
            {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }

        if (pt1 == Int || pt1 == Float || pt1 == Point || pt1 == Boolean)
        {
            return_value_type = pt1;
            return true;
        }
        else
        {
            ERROR_MSG("Operator '-' not applicable for parameter type: " << type_names.at(pt1))
            return false;
        }
    }
    case lx::Star:
    {
        if (pt1 != pt2)
        {
            if ((pt1 == Int && pt2 == Float) ||
                (pt1 == Float && pt2 == Int))
            {
                return_value_type = Float;
                return true;
            }
            else if ((pt1 == Point && pt2 == Int) ||
                (pt1 == Point && pt2 == Float) ||
                (pt1 == Int   && pt2 == Point) ||
                (pt1 == Float && pt2 == Point))
            {
                return_value_type = Point;
                return true;
            }
            else
            {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }

        if (pt1 == Int || pt1 == Float || pt1 == Boolean)
        {
            return_value_type = pt1;
            return true;
        }
        else
        {
            ERROR_MSG("Operator '*' not applicable for parameter type: " << type_names.at(pt1))
            return false;
        }
    }
    case lx::Slash:
    {
        if (pt1 != pt2)
        {
            if ((pt1 == Int && pt2 == Float) ||
                (pt1 == Float && pt2 == Int))
            {
                return_value_type = Float;
                return true;
            }
            else if ((pt1 == Point && pt2 == Int) ||
                (pt1 == Point && pt2 == Float))
            {
                return_value_type = Point;
                return true;
            }
            else
            {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }

        if (pt1 == Int || pt1 == Float)
        {
            return_value_type = pt1;
            return true;
        }
        else
        {
            ERROR_MSG("Operator '/' not applicable for parameter type: " << type_names.at(pt1))
            return false;
        }
    }
    case lx::EqualEqual:
    case lx::NotEqual:
    {
        if (pt1 != pt2)
        {
            if ((pt1 == Int && pt2 == Float) ||
                (pt1 == Float && pt2 == Int))
            {
                return_value_type = Boolean;
                return true;
            }
            else
            {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }

        return_value_type = Boolean;
        return true;
    }
    default:
        return false;
    }
}

inline bool TreeWalker::validateParamType(const Node &node, ParameterTypeList *param_types)
{
    if (!node.param.hasValue())
    {
        errorMsg("No parameter found");
        return false;
    }

    switch (node.param.getId())
    {
    case lx::Term:
    {
        if (node.rule == ps::Param)
        {
            errorMsg("Token name cannot be used as parameter");
            return false;
        }
        const std::string &content = node.param.getText();
        if (var_types.find(content) == var_types.end())
        {
            ERROR_MSG("Variable '" << content << "' not found")
            return false;
        }
        if (param_types)
            param_types->push_back(var_types.find(content)->second);
        break;
    }
    case lx::Integer:
        if (param_types)
            param_types->push_back(Int);
        break;
    case lx::Float:
        if (param_types)
            param_types->push_back(Float);
        break;
    case lx::String:
    {
        // check string length >= 2 (for the quotes)
        if (node.param.getText().size() < 2)
        {
            errorMsg("String does not have quotes");
            return false;
        }
        if (param_types)
            param_types->push_back(String);
        break;
    }
    default:
        errorMsg("Invalid parameter");
        return false;
    }

    return true;
}
