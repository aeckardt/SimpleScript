#ifndef LEXER_H
#define LEXER_H

#include <list>
#include <string>
#include <unordered_set>

namespace lx
{

enum TokenId : uint32_t
{
    Term,
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
    void run(const std::string &context, TokenList &tokens);

    const std::string &getLastError() const
    { return error_msg; }

private:
    std::string error_msg;
    std::list<Line>::reverse_iterator line;

    void pushError(const std::string &msg);

    void readIndent(token_pos &it, const token_pos &end, uint32_t &spaces, uint32_t &tabs);
    void readName(token_pos &it, const token_pos &end, Token &token);
    void readNumber(token_pos &it, const token_pos &end, Token &token);
    void readString(token_pos &it, const token_pos &end, Token &token);
    void readSingleChar(token_pos &it, Token &token);
    void readOperator(token_pos &it, const token_pos &end, Token &token);
    void readComment(token_pos &it, const token_pos &end);
};

} // namespace lx

#endif // LEXER_H
