#include <iostream>
#include <string>
#include <unordered_set>

#include "lexer.h"

using namespace lx;

#define AlphabetChar AlphaNumeric
#define Digit        Integer
#define Dot          Float
#define Quote        String

static TokenId char_def[UCHAR_MAX + 1] = {
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Whitespace, Newline, Other, Other, Return, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Other, Other, Other, Other, Other, Other, Other, Other,
    Whitespace, Not, Quote, Comment, Other, Other, Other, Other,
    LeftParen, RightParen, Star, Plus, Comma, Minus, Dot, Slash,
    Digit, Digit, Digit, Digit, Digit, Digit, Digit, Digit,
    Digit, Digit, Colon, Other, Other, Equal, Other, Other,
    Other, AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,
    AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,
    AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,
    AlphabetChar,  AlphabetChar,  AlphabetChar,  Other, Other, Other, Other, AlphabetChar,
    Other, AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,
    AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,
    AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,  AlphabetChar,
    AlphabetChar,  AlphabetChar,  AlphabetChar,  Other, Other, Other, Other, Other,
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
    AlphabetChar, Digit, Dot, Quote, LeftParen, RightParen,
    EqualEqual, NotEqual, Plus, Minus, Star, Slash};

inline TokenId token_at(const token_pos it)
{
    return char_def[static_cast<unsigned char>(*it)];
}

inline void Lexer::pushError(const std::string &msg)
{
    if (error_msg.empty()) {
        char buf[20];
        sprintf(buf, "%d", (cur_line->index + 1));
        error_msg = std::string("At line ") + buf + ": " + msg;
    }
}

inline void Lexer::readIndent(uint32_t &spaces, uint32_t &tabs)
{
    while (it != end && token_at(it) == Whitespace) {
        if (*it == ' ')
            ++spaces;
        else
            ++tabs;
        ++it;
    }
}

inline void Lexer::readName()
{
    Token &token = newToken();

    // *it is in [A-Za-z_]
    token.begin = it++;
    while (it != end && (token_at(it) == AlphabetChar || token_at(it) == Digit))
        it++;
    token.id = AlphaNumeric;
    token.end = it;
}

inline void Lexer::readNumber()
{
    Token &token = newToken();

    // *it is in [0-9.]
    bool contains_dot = *it == '.';

    token.begin = it++;
    if (!contains_dot) {
        while (it != end && token_at(it) == Digit)
            ++it;
        if (it != end && *it == '.') {
            ++it;
            contains_dot = true;
        }
    }
    while (it != end && token_at(it) == Digit)
        ++it;
    if (it == token.begin + 1 && contains_dot)
        pushError("A number needs to have at least one digit");
    token.id = contains_dot ? Float : Integer;
    token.end = it;
}

inline void Lexer::readString()
{
    Token &token = newToken();

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

inline void Lexer::readSingleChar()
{
    cur_line->tokens.push_back({token_at(it), it, ++it});
}

inline void Lexer::readOperator()
{
    Token &token = newToken();

    token.begin = it;
    token.id = token_at(it);
    token.end = ++it;

    if (it != end && *it == '=') {
        token.id = static_cast<TokenId>(static_cast<uint32_t>(token.id) + 1);
        token.end = ++it;
    }
}

inline void Lexer::skipComment()
{
    // *it is '#'
    ++it;
    while (it != end && *it != '\n')
        ++it;
}

void Lexer::run(const std::string &context, TokenList &tokens)
{
    error_msg.clear();

    it  = context.begin();
    end = context.end();

    uint32_t line_index = 0;

    uint32_t spaces_total = 0;
    uint32_t tabs_total   = 0;
    uint32_t spaces;
    uint32_t tabs;

    while (it != end) {

        cur_line = &tokens.emplace_back();
        cur_line->index = line_index;

        spaces = 0;
        tabs   = 0;
        readIndent(spaces, tabs);

        while (it != end && *it != '\n') {

            switch (token_at(it)) {
            case AlphabetChar:
                readName();
                break;
            case Digit:
            case Dot:
                readNumber();
                break;
            case Quote:
                readString();
                break;
            case Comma:
            case Colon:
            case LeftParen:
            case RightParen:
                readSingleChar();
                break;
            case Equal:      // Equal or EqualEqual
            case Not:        // Not or NotEqual
            case Star:       // Star or StarEqual
            case Slash:      // Slash or SlashEqual
            case Plus:       // Plus or PlusEqual
            case Minus:      // Minus or MinusEqual
                readOperator();
                break;
            case Comment:
                skipComment();
                break;
            case Whitespace: // ' '  or '\t' -> skip
            case Return:     // '\r'         -> skip
                ++it;
                break;
            case Other: // not allowed
            default:    // no paths lead here, just to suppress warnings...
                return pushError("Invalid character");
            }
        }

        if (it != end && *it == '\n')
            ++it;

        if (cur_line->tokens.size() == 0)
            tokens.pop_back();
        else {
            // check if indentation is correct
            if (spaces % 4 != 0)
                return pushError("Invalid indentation");
            spaces_total += spaces;
            tabs_total   += tabs;
            if ((spaces_total > 0) && (tabs_total > 0))
                return pushError("Invalid indentation, inconsistent use of tabs and spaces");
            cur_line->indent_level = tabs + spaces / 4;
        }

        if (!error_msg.empty())
            return;

        ++line_index;
    }
}
