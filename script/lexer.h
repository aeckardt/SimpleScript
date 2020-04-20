#ifndef LEXER_H
#define LEXER_H

#include <list>
#include <string>
#include <unordered_set>

namespace lx
{

enum TokenId : uint32_t
{
    AlphaNumeric,
    Integer,
    Float,
    String,

    LeftParen,
    RightParen,
    Equal,
    EqualEqual,
    Not,
    NotEqual,
    Plus,
    PlusEqual,
    Minus,
    MinusEqual,
    Star,
    StarEqual,
    Slash,
    SlashEqual,

    Comma,
    Colon,

    Comment,
    Whitespace,
    Return,
    Newline,
    Other,
    End
};

typedef std::string::const_iterator token_pos;

extern const std::unordered_set<uint32_t> operators;
extern const std::unordered_set<uint32_t> expr_tokens;

inline bool isOperator(TokenId id)
{ return operators.find(id) != operators.end(); }

inline bool isExprToken(TokenId id)
{ return expr_tokens.find(id) != expr_tokens.end(); }

struct Token
{
    TokenId id;
    token_pos begin;
    token_pos end;
};

struct Line
{
    uint32_t index;
    uint32_t indent_level;
    std::list<Token> tokens;
};

typedef std::list<Line> TokenList;
typedef TokenList::const_iterator line_pos;

class Lexer
{
public:
    void tokenize(const std::string &context, TokenList &tokens);

    const std::string &getLastError() const
    { return error_msg; }

private:
    std::string error_msg;

    Line *cur_line;
    token_pos it;
    token_pos end;

    void pushError(const std::string &msg);

    Token &newToken() { return cur_line->tokens.emplace_back(); }

    void readIndent(uint32_t &spaces, uint32_t &tabs);
    void readName();
    void readNumber();
    void readString();
    void readSingleChar();
    void readOperator();
    void skipComment();
};

} // namespace lx

#endif // LEXER_H
