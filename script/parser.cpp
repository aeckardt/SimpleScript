#include <map>

#include "parser.h"

#define ASSERT(expr) { expr; if (!error_msg.empty()) return; }

using namespace tn;
using namespace ps;

static const std::map<TokenId, std::string> token_desc = {
    {Term,    "Term"}, {Integer, "Integer"}, {Float,   "Float"}, {String,    "String"},
    {Comma,    "','"}, {Colon,       "':'"}, {LeftParen, "'('"}, {RightParen,   "')'"},
    {Equal,    "'='"}, {EqualEqual, "'=='"}, {Not,       "'!'"}, {NotEqual,    "'!='"},
    {Star,     "'*'"}, {StarEqual,  "'*='"}, {Slash,     "'/'"}, {SlashEqual,  "'/='"},
    {Plus,     "'+'"}, {PlusEqual,  "'+='"}, {Minus,     "'-'"}, {MinusEqual,  "'-='"}};

inline Node &Parser::addNode(Node &parent, ParserRule rule)
{
    parent.children.push_back(Node());
    Node &child = parent.children.back();
    child.rule = rule;
    return child;
}

inline void Parser::expectToken(const std::vector<TokenId> &ids)
{
    if (cur_token != lineEnd())
    {
        for (const TokenId &id : ids)
        {
            if (cur_token.id() == id)
                return;
        }

        // handle error
        err_msg_ss.clear();
        err_msg_ss << "Invalid token, expected {";
    }
    else
    {
        err_msg_ss.clear();
        err_msg_ss << "Unexpected end of line, expected {";
    }

    bool first = true;
    for (const TokenId &id : ids)
    {
        if (!first) err_msg_ss << ", ";
        err_msg_ss << token_desc.at(id);
        first = false;
    }
    err_msg_ss << "}";
    if (cur_token != lineEnd())
        err_msg_ss << ", found " << token_desc.at(cur_token.id());

    return pushError(err_msg_ss.str());
}

inline void Parser::readOperator(Node &node)
{
    node.rule = Operator;
    node.param = cur_token++;
    return;
}

inline void Parser::readParam(Node &node)
{
    ASSERT(expectToken({Term, Integer, Float, String}))

    if (cur_token.id() != Term)
    {
        node.rule = ConstValue;
        node.param = cur_token++;
        return;
    }
    else
    {
        if (++cur_token != lineEnd() && cur_token.id() == LeftParen)
        {
            node.rule = Function;
            --cur_token;
            ASSERT(parseFunction(node))
        }
        else
        {
            node.rule = Variable;
            node.param = (--cur_token)++;
            return;
        }
    }
}

void Parser::parse(const TokenList &tokens, Node &root)
{
    error_msg.clear();

    root.rule = Section;
    ptokens = &tokens;
    cur_line = ptokens->begin();
    parseSection(root, 0, true);
}

void Parser::parseAssignment(Node &node)
{
    // add parameter node
    ASSERT(readParam(addNode(node, Variable)))

    // expression is assignment
    ASSERT(expectToken({Equal}))
    ++cur_token;

    ASSERT(parseExpr(node))
}

void Parser::parseExpr(Node &node)
{
    bool isExpression = false;
    Node temp_node;

    static const std::map<TokenId, uint32_t> precedence = {
        {EqualEqual, 1},
        {NotEqual, 1},
        {Plus, 2,},
        {Minus, 2},
        {Star, 3},
        {Slash, 3}};

    static const std::map<TokenId, bool> assoc_left = {
        {EqualEqual, true},
        {NotEqual, true},
        {Plus, true},
        {Minus, true},
        {Star, true},
        {Slash, true}};

    // use shunting yard algorithm to build RPN

    std::vector<Node> stack;
    uint32_t lvl = 0;
    int32_t nparams = 0;
    while (cur_token != lineEnd() && isExprToken(cur_token.id()))
    {
        Node it_node;
        it_node.param = cur_token;
        if (cur_token.id() == Term  || cur_token.id() == Integer ||
            cur_token.id() == Float || cur_token.id() == String)
        {
            ASSERT(readParam(it_node))
            temp_node.children.push_back(it_node);
            ++nparams;
            if (nparams > 1)
                return pushError("Invalid expression, expected operator");
            --cur_token;
        }
        else if (isOperator(cur_token.id()))
        {
            --nparams;
            if (nparams < 0)
                return pushError("Invalid expression, expected parameter");
            isExpression = true;
            if (stack.size() > 0)
            {
                while (stack.size() > 0 && ((stack.back().param.id() != LeftParen) &&
                       (precedence.at(stack.back().param.id())  > precedence.at(it_node.param.id()) ||
                       (precedence.at(stack.back().param.id()) == precedence.at(it_node.param.id()) && assoc_left.at(it_node.param.id())))))
                {
                    temp_node.children.push_back(stack.back());
                    stack.pop_back();
                }
            }

            it_node.rule = Operator;
            stack.push_back(it_node);
        }
        else if (cur_token.id() == LeftParen)
        {
            stack.push_back(it_node);
            ++lvl;
        }
        else if (cur_token.id() == RightParen)
        {
            if (lvl == 0)
                break;
            while (stack.size() > 0 && stack.back().param.id() != LeftParen)
            {
                temp_node.children.push_back(stack.back());
                stack.pop_back();
            }

            if (stack.size() > 0 && stack.back().param.id() == LeftParen)
            {
                stack.pop_back();
                --lvl;
            }
            else
                return pushError("Closed parenthesis without left parenthesis");
        }
        ++cur_token;
    }

    if (nparams == 0)
        return pushError("Invalid expression, expected parameter");

    while (stack.size() > 0)
    {
        temp_node.children.push_back(stack.back());
        stack.pop_back();
    }

    if (isExpression)
    {
        temp_node.rule = Expr;
        node.children.push_back(temp_node);
    }
    else
    {
        for (const Node &child : temp_node.children)
            node.children.push_back(child);
    }
}

void Parser::parseFunction(Node &node)
{
    // it is established that the first token is the name of a function

    // add function name
    node.param = cur_token++;

    // skip left parenthesis
    ASSERT(expectToken({LeftParen}))
    ++cur_token;

    // check next token
    ASSERT(expectToken({Term, Integer, Float, String, LeftParen, RightParen}))

    if (cur_token.id() != RightParen)
        ASSERT(parseExpr(node)) // add next parameter
    else
    {
        ++cur_token;
        return;
    }

    while (true)
    {
        ASSERT(expectToken({Comma, RightParen}))
        if ((cur_token++).id() != RightParen)
            ASSERT(parseExpr(node)) // add next parameter
        else
            return;
    }
}

void Parser::parseIfStatement(Node &node)
{
    // it is established that the token at it is 'if'
    ++cur_token;

    // parse expression (which is evaluated as true / false at runtime)
    ASSERT(parseExpr(node))

    ASSERT(expectToken({Colon}))
    ++cur_token;

    if (cur_token != lineEnd())
        return pushError("Invalid token, expected end of line");

    uint32_t if_indent = (cur_line++)->indent_level;
    ASSERT(parseSection(addNode(node, Section), if_indent + 1, false))

    if (cur_line == ptokens->end())
    {
        --cur_line;
        cur_token = lineEnd();
        return;
    }
    cur_token = lineBegin();

    ASSERT(expectToken({Term}))
    const std::string &name = (cur_token++).getText();

    if (name == "else")
    {
        ASSERT(expectToken({Colon}))
        ++cur_token;

        if (cur_token != lineEnd())
            return pushError("Invalid token, expected end of line");

        ++cur_line;
        ASSERT(parseSection(addNode(node, Section), if_indent + 1, false))
    }

    --cur_line;
    cur_token = lineEnd();
}

void Parser::parseLine(Node &parent)
{
    cur_token = lineBegin();

    ASSERT(expectToken({Term}))
    const std::string &name = cur_token.getText();

    if (name == "if")
        ASSERT(parseIfStatement(addNode(parent, IfStatement)))
    else
    {
        ++cur_token;
        ASSERT(expectToken({LeftParen, Equal}))
        if (cur_token.id() == LeftParen)
        {
            Node &func_node = addNode(parent, Function);
            --cur_token;
            ASSERT(parseFunction(func_node))
        }
        else
        {
            --cur_token;
            ASSERT(parseAssignment(addNode(parent, Assignment)))
        }
    }

    if (cur_token != lineEnd())
        pushError("Unexpected token " + token_desc.at(cur_token.id()) + ", expected end of line");
}

void Parser::parseSection(Node &node, uint32_t lvl, bool may_be_empty)
{
    while (cur_line != ptokens->end() && cur_line->indent_level == lvl)
    {
        ASSERT(parseLine(node))
        if (cur_line != ptokens->end())
            ++cur_line;
    }

    if (cur_line != ptokens->end() && cur_line->indent_level > lvl)
        return pushError("Invalid indentation");

    if (!may_be_empty && node.children.empty())
    {
        --cur_line; // for the error message
        return pushError("Section is empty, needs to have at least one statement");
    }
}

inline void Parser::pushError(const std::string &msg)
{
    if (error_msg.empty())
    {
        char buf[20];
        sprintf(buf, "%d", (cur_line->index + 1));
        error_msg = std::string("At line ") + buf + ": " + msg;
    }
}

