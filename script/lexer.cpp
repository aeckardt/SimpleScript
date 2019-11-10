#include <array>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <unordered_set>

#include "lexer.h"

using namespace lx;

static TokenId char_def[UCHAR_MAX + 1] = {
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Whitespace, Newline, Other, Other, Return, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Whitespace, Not, String, Comment, Other, Other, Other, Other,
    LeftParen, RightParen, Star, Plus, Comma, Minus, Float, Slash,
    Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer,
    Integer, Integer, Colon, Other, Other, Equal, Other, Other,
    Other, Term,  Term,  Term,  Term,  Term,  Term,  Term,
    Term,  Term,  Term,  Term,  Term,  Term,  Term,  Term,
    Term,  Term,  Term,  Term,  Term,  Term,  Term,  Term,
    Term,  Term,  Term,  Other, Other, Other, Other, Term,
    Other, Term,  Term,  Term,  Term,  Term,  Term,  Term,
    Term,  Term,  Term,  Term,  Term,  Term,  Term,  Term,
    Term,  Term,  Term,  Term,  Term,  Term,  Term,  Term,
    Term,  Term,  Term,  Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other};

const std::unordered_set<uint32_t> lx::operators = {
    EqualEqual, NotEqual, Plus, Minus, Star, Slash};

const std::unordered_set<uint32_t> lx::expr_tokens = {
    Term, Integer, Float, String, LeftParen, RightParen,
    EqualEqual, NotEqual, Plus, Minus, Star, Slash};

inline TokenId token_at(const token_pos it)
{
    return char_def[static_cast<unsigned char>(*it)];
}

inline void Lexer::pushError(const std::string &msg)
{
    if (error_msg.empty()) {
        char buf[20];
        sprintf(buf, "%d", (line->index + 1));
        error_msg = std::string("At line ") + buf + ": " + msg;
    }
}

inline void Lexer::readIndent(token_pos &it, const token_pos &end, uint32_t &spaces, uint32_t &tabs)
{
    while (it != end && token_at(it) == Whitespace) {
        if (*it == ' ')
            ++spaces;
        else
            ++tabs;
        ++it;
    }
}

inline void Lexer::readName(token_pos &it, const token_pos &end, Token &token)
{
    // *it is in [A-Za-z_]
    token.begin = it++;
    while (it != end && (token_at(it) == Term || token_at(it) == Integer))
        it++;
    token.id = Term;
    token.end = it;
}

inline void Lexer::readNumber(token_pos &it, const token_pos &end, Token &token)
{
    // *it is in [0-9.]
    bool contains_dot = *it == '.';
    token.begin = it++;
    if (!contains_dot) {
        while (it != end && token_at(it) == Integer)
            ++it;
        if (it != end && *it == '.') {
            ++it;
            contains_dot = true;
        }
    }
    while (it != end && token_at(it) == Integer)
        ++it;
    if (it == token.begin + 1 && contains_dot)
        pushError("A number needs to have at least one digit");
    token.id = contains_dot ? Float : Integer;
    token.end = it;
}

inline void Lexer::readString(token_pos &it, const token_pos &end, Token &token)
{
    // *it is '"'
    token.begin = it++;
    while (it != end && *it != '"' && *it != '\n')
        ++it;
    if (it == end || *it == '\n')
        pushError("End of string not found, missing '\"'");
    it++;
    token.id = String;
    token.end = it;
}

inline void Lexer::readSingleChar(token_pos &it, Token &token)
{
    token.begin = it;
    token.id = token_at(it);
    token.end = ++it;
}

inline void Lexer::readOperator(token_pos &it, const token_pos &end, Token &token)
{
    token.begin = it;
    token.id = token_at(it);
    token.end = ++it;

    if (it != end && *it == '=') {
        token.id = static_cast<TokenId>(static_cast<uint8_t>(token.id) + 1);
        token.end = ++it;
    }
}

inline void Lexer::readComment(token_pos &it, const token_pos &end)
{
    // *it is '#'
    ++it;
    while (it != end && *it != '\n')
        ++it;
}

void Lexer::run(const std::string &context, TokenList &tokens)
{
    error_msg.clear();

    token_pos       it  = context.begin();
    const token_pos end = context.end();

    uint32_t line_index = 0;

    uint32_t spaces_total = 0;
    uint32_t tabs_total   = 0;
    uint32_t spaces;
    uint32_t tabs;

    while (it != end) {

        tokens.push_back(Line());
        line = tokens.rbegin();
        line->index = line_index;

        spaces = 0;
        tabs   = 0;
        readIndent(it, end, spaces, tabs);

        while (it != end && *it != '\n') {

            switch (token_at(it)) {
            case Term: {
                line->tokens.push_back(Token());
                readName(it, end, *line->tokens.rbegin());
                continue;
            }
            case Integer:
            case Float: {
                line->tokens.push_back(Token());
                readNumber(it, end, *line->tokens.rbegin());
                break;
            }
            case String: {
                line->tokens.push_back(Token());
                readString(it, end, *line->tokens.rbegin());
                break;
            }
            case Comma:
            case Colon:
            case LeftParen:
            case RightParen: {
                line->tokens.push_back(Token());
                readSingleChar(it, *line->tokens.rbegin());
                break;
            }
            case Equal:      // Equal or EqualEqual
            case Not:        // Not or NotEqual
            case Star:       // Star or StarEqual
            case Slash:      // Slash or SlashEqual
            case Plus:       // Plus or PlusEqual
            case Minus: {    // Minus or MinusEqual
                line->tokens.push_back(Token());
                readOperator(it, end, *line->tokens.rbegin());
                break;
            }
            case Comment:
                readComment(it, end);
                break;
            case Whitespace: // ' ' or '\t' -> skip
                ++it;
                break;
            case Return:     // '\r' -> skip
                ++it;
                break;
            case Other: // not allowed
            default:    // no paths lead here, just to suppress warnings...
                return pushError("Invalid character");
            }
        }

        if (it != end && *it == '\n')
            ++it;

        if (line->tokens.size() == 0)
            tokens.pop_back();
        else {
            // check if indentation is correct
            if (spaces % 4 != 0)
                return pushError("Invalid indentation");
            spaces_total += spaces;
            tabs_total   += tabs;
            if ((spaces_total > 0) && (tabs_total > 0))
                return pushError("Invalid indentation, inconsistent use of tabs and spaces");
            line->indent_level = tabs + spaces / 4;
        }

        if (!error_msg.empty())
            return;

        ++line_index;
    }
}
