#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <list>
#include <string>

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

// isOperator is equivalent to id is in {Plus, Minus, Star, Slash}
inline bool isOperator(const TokenId &id) { return (id >= Plus && id <= SlashEqual && (static_cast<uint32_t>(id) % 2 == 0)) || id == EqualEqual || id == NotEqual; }

// isExprToken is equivalent to id is in {Term, Integer, Float, String, LeftParen, RightParen, EqualEqual, NotEqual, Plus, Minus, Star, Slash}
inline bool isExprToken(const TokenId &id) { return isOperator(id) || (id >= Term && id <= RightParen); }

typedef std::string::const_iterator token_pos;

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

} // namespace tn

#endif // TOKENIZER_H
