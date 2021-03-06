#include <unordered_map>
#include <unordered_set>

#include <math.h>

#include "parameter.h"
#include "astwalker.h"

using namespace tw;

const std::unordered_map<int32_t, std::string> type_names = {
    {Empty,     "Empty"}, {String,    "String"},  {Int,         "Int"},
    {Float,     "Float"}, {Boolean,  "Boolean"},  {Point,     "Point"},
    {Rect,       "Rect"}, {DateTime, "DateTime"}, {Object,   "Object"}};

const std::unordered_set<std::string> keywords = {
    "if", "else", "print"};

// Comparison range for floating types
#define FLOAT_CMP_EPSILON 0.00001

bool floatEqual(const double &f1, const double &f2)
{
    return fabs(f1 - f2) < FLOAT_CMP_EPSILON;
}

inline void ASTWalker::errorMsg(const char *msg) const
{
    if (output_fnc != nullptr) {
        Parameter param;
        param.assign(std::string(msg));
        output_fnc(param, Qt::darkRed);
    }
}

// void(0) is used to enforce semicolon after the macro
#define errorMsgf(format, ...) \
{ char *buffer = new char[strlen(format) * 2 + 50]; sprintf(buffer, format, __VA_ARGS__); errorMsg(buffer); } (void)0

Parameter ASTWalker::getConstValue(const Node &node)
{
    const std::string &content = node.param.getText();
    Parameter param;

    switch (node.param.id()) {
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
        // This should not happen, because the parser delivers only
        // the above three tokens to the ConstValue rule
        errorMsg("param_token.id out of range");
        break;
    }

    return param;
}

Parameter::Type ASTWalker::getParamType(const Node &node)
{
    switch (node.param.id()) {
    case lx::Integer:
        return Int;
    case lx::Float:
        return Float;
    case lx::String:
        return String;
    default:
        errorMsg("param_token.id out of range");
        return Empty;
    }
}

bool ASTWalker::run(const std::string &str)
{
    lx::TokenList tokens;
    lx::Lexer lexer;
    lexer.tokenize(str, tokens);

    if (!lexer.getLastError().empty()) {
        errorMsg("Error lexing:");
        errorMsg(lexer.getLastError().c_str());
        return false;
    }

    Node ast_root;
    ps::Parser parser;
    parser.createAST(tokens, ast_root);

    if (!parser.getLastError().empty()) {
        errorMsg("Error parsing:");
        errorMsg(parser.getLastError().c_str());
        return false;
    }

    if (!validate(ast_root)) {
        errorMsg("Error validating syntax");
        return false;
    } else
        if (!traverse(ast_root)) {
            errorMsg("Error running script");
            return false;
        }

    return true;
}

bool ASTWalker::traverse(const Node &node)
{
    switch (node.rule) {
    case ps::Section:
        for (auto const &child : node.children)
            if (!traverse(child))
                return false;
        return true;
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
}

bool ASTWalker::traverseAssignment(const Node &node)
{
    const Node &var_node = *node.children.begin();
    const std::string &var_name = var_node.param.getText();

    const Node &src_node = *node.children.rbegin();
    if (src_node.rule == ps::Function || src_node.rule == ps::Expr) {
        if (!traverse(src_node))
            return false;
    } else if (src_node.rule == ps::Variable)
        // create new variable as copy of rvalue variable
        return_value = vars[src_node.param.getText()];
    else if (src_node.rule == ps::ConstValue)
        // create new parameter containing the given value
        return_value = getConstValue(src_node);

    vars[var_name] = std::move(return_value);

    return true;
}

bool ASTWalker::traverseExpr(const Node &node)
{
    return_value.clear();

    ParameterList stack;
    for (const Node &child : node.children) {
        if (lx::isOperator(child.param.id())) {
            const Parameter p2 = stack.back();
            stack.pop_back();

            const Parameter p1 = stack.back();
            stack.pop_back();

            if (!traverseOperation(child.param.id(), p1, p2))
                return false;
            stack.push_back(std::move(return_value));

            continue;
        }

        switch (child.rule) {
        case ps::Function:
        case ps::Expr:
            if (!traverse(child))
                return false;
            stack.push_back(std::move(return_value));
            break;
        case ps::Variable: {
            const std::string &var_name = child.param.getText();
            stack.push_back(referenceTo(vars[var_name]));
            break;
        }
        case ps::ConstValue: {
            stack.push_back(getConstValue(child));
            break;
        }
        default:
            return false;
        }
    }

    if (stack.size() != 1) {
        errorMsg("Unable to evaluate expression");
        return false;
    }

    return_value = std::move(stack[0]);
    return true;
}

bool ASTWalker::traverseFunction(const Node &node)
{
    ParameterList params;
    for (const Node &child : node.children) {
        if (child.rule == ps::Function || child.rule == ps::Expr) {
            if (!traverse(child))
                return false;
            params.push_back(std::move(return_value));
        } else if (child.rule == ps::Variable)
            params.push_back(referenceTo(vars[child.param.getText()]));
        else if (child.rule == ps::ConstValue)
            params.push_back(getConstValue(child));
    }

    const std::string &cmd_name = node.param.getText();

    // existance of the command and correct types of the parameters
    // is already proven in the validity check
    if (!commands[cmd_name].callback_fnc(params, return_value))
        return false;

    return true;
}

bool ASTWalker::traverseIfStatement(const Node &node)
{
    // Check expression in if-statement
    ps::tree_pos tp = node.children.begin();

    const Node &expr_node = *tp;
    if (expr_node.rule == ps::Function || expr_node.rule == ps::Expr) {
        if (!traverse(expr_node))
            return false;
    } else if (expr_node.rule == ps::Variable)
        return_value = referenceTo(vars[expr_node.param.getText()]);
    else if (expr_node.rule == ps::ConstValue)
        return_value = getConstValue(expr_node);

    // evaluate expression
    bool exprIsTrue;
    if (return_value.empty())
        exprIsTrue = false;
    else if (return_value.type() != Boolean && return_value.type() != Int)
        exprIsTrue = true;
    else if (return_value.type() == Boolean)
        exprIsTrue = return_value.asBoolean();
    else
        exprIsTrue = return_value.asInt();

    return_value.clear();

    if (exprIsTrue) {
        // execute if-section
        const Node &section_node = *(++tp);
        if (!traverse(section_node))
            return false;
    } else if (node.children.size() == 3) {
        // an else-section exists
        const Node &else_section_node = *(++++tp);
        if (!traverse(else_section_node))
            return false;
    }

    return true;
}

bool ASTWalker::traverseOperation(const lx::TokenId &op, const Parameter &p1, const Parameter &p2)
{
    switch (op) {
    case lx::Plus: {
        if (p1.type() != p2.type()) {
            if ((p1.type() == Int   && p2.type() == Float) ||
                (p1.type() == Float && p2.type() == Int)) {

                return_value.assign(p1.asFloat() + p2.asFloat());
                return true;
            } else
                return false;
        }

        switch (p1.type()) {
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
    case lx::Minus: {
        if (p1.type() != p2.type()) {
            if ((p1.type() == Int   && p2.type() == Float) ||
                (p1.type() == Float && p2.type() == Int)) {

                return_value.assign(p1.asFloat() - p2.asFloat());
                return true;
            } else
                return false;
        }

        switch (p1.type()) {
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
    case lx::Star: {
        if (p1.type() != p2.type()) {
            if ((p1.type() == Int   && p2.type() == Float) ||
                (p1.type() == Float && p2.type() == Int)) {

                return_value.assign(p1.asFloat() * p2.asFloat());
                return true;
            } else if (p1.type() == Point && p2.type() == Int)
                return_value.assign(p1.asPoint() * p2.asInt());
            else if (p1.type() == Point && p2.type() == Float)
                return_value.assign(p1.asPoint() * p2.asFloat());
            else if (p1.type() == Int   && p2.type() == Point)
                return_value.assign(p1.asInt()   * p2.asPoint());
            else if (p1.type() == Float && p2.type() == Point)
                return_value.assign(p1.asFloat() * p2.asPoint());
            else
                return false;
            return true;
        }

        switch (p1.type()) {
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
    case lx::Slash: {
        if (p2.type() == Int) {
            if (p2.asInt() == 0) {
                errorMsg("Division by zero not allowed");
                return false;
            }
        } else if (p2.type() == Float) {
            if (fabs(p2.asFloat()) == 0) {
                errorMsg("Division by zero not allowed");
                return false;
            }
        }

        if (p1.type() != p2.type()) {
            if ((p1.type() == Int   && p2.type() == Float) ||
                (p1.type() == Float && p2.type() == Int)) {

                return_value.assign(p1.asFloat() / p2.asFloat());
                return true;
            } else if (p1.type() == Point && p2.type() == Int)
                return_value.assign(p1.asPoint() / p2.asInt());
            else if (p1.type() == Point && p2.type() == Float)
                return_value.assign(p1.asPoint() / p2.asFloat());
            else
                return false;
            return true;
        }

        switch (p1.type()) {
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
    case lx::EqualEqual: {
        if (p1.type() != p2.type()) {
            if ((p1.type() == Int   && p2.type() == Float) ||
                (p1.type() == Float && p2.type() == Int)) {

                return_value.assign(floatEqual(p1.asFloat(), p2.asFloat()));
                return true;
            } else
                return false;
        }

        switch (p1.type()) {
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
    case lx::NotEqual: {
        if (p1.type() != p2.type()) {
            if ((p1.type() == Int   && p2.type() == Float) ||
                (p1.type() == Float && p2.type() == Int)) {

                return_value.assign(!floatEqual(p1.asFloat(), p2.asFloat()));
                return true;
            } else
                return false;
        }

        switch (p1.type()) {
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

bool ASTWalker::validate(const Node &node)
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
    case ps::ConstValue:
        return validateParamType(node);
    default:
        errorMsg("Unspecified rule");
        return false;
    }

    return true;
}

bool ASTWalker::validateAssignment(const Node &node)
{
    if (node.children.size() < 2) {
        errorMsg("Invalid assignment");
        return false;
    }

    const Node &var_node = *node.children.begin();
    if (var_node.rule != ps::Variable) {
        errorMsg("Expression is not assignable, lvalue needs to be variable");
        return false;
    }

    const std::string &var_name = var_node.param.getText();
    if (keywords.find(var_name) != keywords.end()) {
        errorMsgf("Keyword '%s' is not allowed as variable name", var_name.c_str());
        return false;
    }

    const Node &src_node = *node.children.rbegin();
    if (src_node.rule != ps::Variable && src_node.rule != ps::ConstValue &&
        src_node.rule != ps::Function && src_node.rule != ps::Expr) {

        errorMsg("Invalid rvalue for assignment");
        return false;
    }

    if (src_node.rule == ps::Function || src_node.rule == ps::Expr) {
        if (!validate(src_node))
            return false;
    } else if (src_node.rule == ps::Variable) {
        const std::string &rvar_name = src_node.param.getText();
        if (var_types.find(rvar_name) == var_types.end()) {
            errorMsgf("Variable '%s' not found", rvar_name.c_str());
            return false;
        }
        ParameterType param_type = var_types.find(rvar_name)->second;
        if (param_type.basic_type == Object && !obj_types[param_type.obj_ref].copyable) {
            errorMsgf("Object of type '%s' is not copyable", obj_types[param_type.obj_ref].name.c_str());
            return false;
        }
        return_value_type = var_types[var_name];
    }
    else // rule is Int, Float or String
        return_value_type = getParamType(src_node);

    var_types[var_name] = return_value_type;

    return true;
}

bool ASTWalker::validateCommand(const std::string &command, const ParameterTypeList &param_types)
{
    // check if function with that name exists
    if (commands.find(command) == commands.end()) {
        errorMsgf("Unknown function '%s'", command.c_str());
        return false;
    }

    const Command &cmd = commands.find(command)->second;

    if (cmd.param_types.size() < param_types.size()) {
        errorMsgf("Too many arguments passed to function '%s', expected %lu",
                  command.c_str(), static_cast<unsigned long>(cmd.param_types.size()));
        return false;
    }

    // check if number and type of arguments are correct
    size_t index;
    bool validParam;
    for (index = 0; index < cmd.param_types.size(); index++) {
        validParam = false;
        for (const ParameterType &type : cmd.param_types[index]) {
            if (param_types.size() <= index && type.basic_type == Empty) {
                validParam = true;
                break;
            } else if (param_types.size() <= index) {
                errorMsgf("Error: Wrong number of arguments passed to function '%s', expected %lu",
                          command.c_str(), static_cast<unsigned long>(cmd.param_types.size()));
                return false;
            } else if (param_types[index] == type) {
                validParam = true;
                break;
            }
        }
        if (!validParam) {
            std::stringstream ss;
            ss << "Error: Wrong type of argument passed to function '" << command << "', expected ";
            if (cmd.param_types[index].size() > 1)
                ss << "{";
            bool first = true;
            for (const ParameterType &type : cmd.param_types[index]) {
                if (!first) ss << ", ";
                if (type.basic_type != Object)
                    ss << type_names.at(type.basic_type);
                else
                    ss << obj_types[type.obj_ref].name;
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

bool ASTWalker::validateExpr(const Node &node)
{
    if (node.children.size() < 2) {
        errorMsg("Invalid expression, needs to have at least two operands");
        return false;
    }

    for (const Node &child : node.children) {
        if (!child.param.hasValue()) {
            errorMsg("Invalid expression statement, needs to have a value");
            return false;
        }
    }

    // evaluating RPN (reverse polish notation)
    return_value_type = Empty;

    std::vector<ParameterType> stack;
    for (const Node &child : node.children) {
        if (lx::isOperator(child.param.id())) {
            ParameterType p2 = stack.back();
            stack.pop_back();

            ParameterType p1 = stack.back();
            stack.pop_back();

            if (!validateOperation(child.param.id(), p1.basic_type, p2.basic_type))
                return false;
            stack.push_back(return_value_type);

            continue;
        }

        switch (child.rule) {
        case ps::Function:
        case ps::Expr:
            if (!validate(child))
                return false;
            stack.push_back(return_value_type);
            break;
        case ps::Variable: {
            const std::string &var_name = child.param.getText();
            stack.push_back(var_types[var_name]);
            break;
        }
        case ps::ConstValue: {
            stack.push_back(getParamType(child));
            break;
        }
        default:
            return false;
        }
    }

    if (stack.size() != 1) {
        errorMsg("Unable to validate expression");
        return false;
    }

    return_value_type = stack[0];

    return true;
}

bool ASTWalker::validateFunction(const Node &node)
{
    std::vector<ParameterType> param_types;
    for (auto const &child : node.children) {
        if (child.rule != ps::Function && child.rule != ps::ConstValue &&
            child.rule != ps::Variable && child.rule != ps::Expr) {

            errorMsg("Invalid function parameter");
            return false;
        } else if (child.rule == ps::Function || child.rule == ps::Expr) {
            return_value_type = Empty;
            if (!validate(child))
                return false;
            param_types.push_back(return_value_type);
        } else if (child.rule == ps::ConstValue || child.rule == ps::Variable) {
            if (!validateParamType(child, &param_types))
                return false;
        } else {
            errorMsg("Invalid function parameter");
            return false;
        }
    }

    const std::string &cmd_name = node.param.getText();

    return validateCommand(cmd_name, param_types);
}

bool ASTWalker::validateIfStatement(const Node &node)
{
    if (node.children.size() < 2 || node.children.size() > 3) {
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

    if (node.children.size() == 3) {
        const Node &else_section_node = *(++tp);
        if (!validate(else_section_node))
            return false;
    }

    return true;
}

bool ASTWalker::validateOperation(const lx::TokenId &op, const Parameter::Type &pt1, const Parameter::Type &pt2)
{
    switch (op) {
    case lx::Plus: {
        if (pt1 != pt2) {
            if ((pt1 == Int && pt2 == Float) || (pt1 == Float && pt2 == Int)) {
                return_value_type = Float;
                return true;
            } else {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        } else if (pt1 == Rect || pt1 == Object) {
            errorMsgf("Operator '+' not applicable for parameter type: %s", type_names.at(pt1).c_str());
            return false;
        }

        return_value_type = pt1;
        return true;
    }
    case lx::Minus: {
        if (pt1 != pt2) {
            if ((pt1 == Int && pt2 == Float) || (pt1 == Float && pt2 == Int)) {
                return_value_type = Float;
                return true;
            } else {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }

        if (pt1 == Int || pt1 == Float || pt1 == Point || pt1 == Boolean) {
            return_value_type = pt1;
            return true;
        } else {
            errorMsgf("Operator '-' not applicable for parameter type: %s", type_names.at(pt1).c_str());
            return false;
        }
    }
    case lx::Star: {
        if (pt1 != pt2) {
            if ((pt1 == Int && pt2 == Float) ||
                (pt1 == Float && pt2 == Int)) {
                return_value_type = Float;
                return true;
            } else if ((pt1 == Point && pt2 == Int) ||
                (pt1 == Point && pt2 == Float) ||
                (pt1 == Int   && pt2 == Point) ||
                (pt1 == Float && pt2 == Point)) {
                return_value_type = Point;
                return true;
            } else {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }

        if (pt1 == Int || pt1 == Float || pt1 == Boolean) {
            return_value_type = pt1;
            return true;
        } else {
            errorMsgf("Operator '*' not applicable for parameter type: %s", type_names.at(pt1).c_str());
            return false;
        }
    }
    case lx::Slash: {
        if (pt1 != pt2) {
            if ((pt1 == Int && pt2 == Float) ||
                (pt1 == Float && pt2 == Int)) {
                return_value_type = Float;
                return true;
            } else if ((pt1 == Point && pt2 == Int) ||
                (pt1 == Point && pt2 == Float)) {
                return_value_type = Point;
                return true;
            } else {
                errorMsg("Parameter types in operation do not match");
                return false;
            }
        }

        if (pt1 == Int || pt1 == Float) {
            return_value_type = pt1;
            return true;
        } else {
            errorMsgf("Operator '/' not applicable for parameter type: %s", type_names.at(pt1).c_str());
            return false;
        }
    }
    case lx::EqualEqual:
    case lx::NotEqual: {
        if (pt1 != pt2) {
            if ((pt1 == Int && pt2 == Float) ||
                (pt1 == Float && pt2 == Int)) {
                return_value_type = Boolean;
                return true;
            } else {
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

inline bool ASTWalker::validateParamType(const Node &node, ParameterTypeList *param_types)
{
    if (!node.param.hasValue()) {
        errorMsg("No parameter found");
        return false;
    }

    switch (node.param.id()) {
    case lx::AlphaNumeric: {
        if (node.rule == ps::ConstValue) {
            errorMsg("Token name cannot be used as parameter");
            return false;
        }
        const std::string &content = node.param.getText();
        if (var_types.find(content) == var_types.end()) {
            errorMsgf("Variable '%s' not found", content.c_str());
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
        if (param_types)
            param_types->push_back(String);
        break;
    default:
        errorMsg("Invalid parameter");
        return false;
    }

    return true;
}
