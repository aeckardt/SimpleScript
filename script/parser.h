#ifndef PARSER_H
#define PARSER_H

#include <list>
#include <memory>
#include <sstream>
#include <vector>

#include "lexer.h"

namespace ps
{

using std::list;
using std::string;
using std::stringstream;
using std::vector;

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

typedef list<tn::Token>::const_iterator token_index;

class Token
{
public:
    inline Token() { isValid = false; }
    inline Token(const token_index &v) { isValid = true; val = v; }

    inline bool hasValue() const { return isValid; }
    inline const tn::TokenId &id() const { return val->id; }

    inline Token &operator++() { ++val; return *this; }
    inline Token operator++(int) { return val++; }
    inline Token &operator--() { --val; return *this; }
    inline Token operator--(int) { return val--; }
    inline Token &operator=(const token_index &val) { this->val = val; isValid = true; return *this; }
    inline const string getText() const { return string(val->begin, val->end); }

private:
    token_index val;
    bool isValid;

    friend bool operator==(const Token &, const Token &);
    friend bool operator!=(const Token &, const Token &);
};

inline bool operator==(const Token &np1, const Token &np2) { return np1.val == np2.val; }
inline bool operator!=(const Token &np1, const Token &np2) { return np1.val != np2.val; }

struct Node
{
    ParserRule rule;
    list<Node> children;

    // parameter token, can be empty
    Token param;
};

typedef list<Node>::const_iterator tree_pos;

class Parser
{
public:
    void run(const tn::TokenList &tokens, Node &root);

    const string &getLastError() const { return error_msg; }

private:
    string error_msg;
    stringstream err_msg_ss;
    const tn::TokenList *ptokens;

    // current position
    tn::line_pos cur_line;
    Token cur_token;

    void pushError(const string &msg);

    Node &addNode(Node &parent, ParserRule rule);
    void expectToken(const vector<tn::TokenId> &ids);
    void readParam(Node &node);
    void readOperator(Node &node);

    void parseAssignment(Node &node);
    void parseExpr(Node &node);
    void parseFunction(Node &node);
    void parseIfStatement(Node &node);
    void parseLine(Node &parent);
    void parseSection(Node &node, uint32_t lvl, bool may_be_empty);

    inline const token_index lineBegin() const { return cur_line->tokens.begin(); }
    inline const token_index lineEnd() const { return cur_line->tokens.end(); }
};

} // namespace ps

#endif  // PARSER_H
