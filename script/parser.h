#ifndef PARSER_H
#define PARSER_H

#include <list>
#include <memory>
#include <sstream>
#include <vector>

#include "lexer.h"

namespace ps
{

enum ParserRule : uint32_t
{
    Section,
    IfStatement,
    Assignment,
    Variable,
    ConstValue,
    Function,
    Operator,
    Expr,
};

typedef std::list<lx::Token>::const_iterator token_index;

class Token
{
public:
    inline Token() { isValid = false; }
    inline Token(const token_index &v) { isValid = true; val = v; }

    inline bool hasValue() const { return isValid; }
    inline const lx::TokenId &id() const { return val->id; }

    inline Token &operator++() { ++val; return *this; }
    inline Token operator++(int) { return val++; }
    inline Token &operator--() { --val; return *this; }
    inline Token operator--(int) { return val--; }
    inline Token &operator=(const token_index &val) { this->val = val; isValid = true; return *this; }
    inline const std::string getText() const { return std::string(val->begin, val->end); }

private:
    token_index val;
    bool isValid;

    friend bool operator==(const Token &, const Token &);
    friend bool operator!=(const Token &, const Token &);
};

inline bool operator==(const Token &np1, const Token &np2)
{ return np1.val == np2.val; }

inline bool operator!=(const Token &np1, const Token &np2)
{ return np1.val != np2.val; }

struct Node
{
    ParserRule rule;
    std::list<Node> children;

    // parameter token, can be empty
    Token param;
};

typedef std::list<Node>::const_iterator tree_pos;

class Parser
{
public:
    void run(const lx::TokenList &tokens, Node &root);

    const std::string &getLastError() const
    { return error_msg; }

private:
    std::string error_msg;
    std::stringstream err_msg_ss;
    const lx::TokenList *tokens;

    // current position
    lx::line_pos cur_line;
    Token cur_token;

    void pushError(const std::string &msg);

    Node &addNode(Node &parent, ParserRule rule);
    void expectToken(const std::vector<lx::TokenId> &ids);
    void readParam(Node &node);
    void readOperator(Node &node);

    void parseAssignment(Node &node);
    void parseExpr(Node &node);
    void parseFunction(Node &node);
    void parseIfStatement(Node &node);
    void parseLine(Node &parent);
    void parseSection(Node &node, uint32_t lvl, bool may_be_empty);

    inline const token_index lineBegin() const
    { return cur_line->tokens.begin(); }

    inline const token_index lineEnd() const
    { return cur_line->tokens.end(); }
};

} // namespace ps

#endif  // PARSER_H
