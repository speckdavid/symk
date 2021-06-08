#ifndef SDAC_PARSER_PARSER_H
#define SDAC_PARSER_PARSER_H

#include "catamorph/expression.h"
#include <map>
#include <stack>
#include <string>

namespace sdac_parser {
static std::map<std::string, int> op_precedence{
    {"sentinel", 0}, {"==", 1}, {">", 2}, {">=", 3}, {"<", 4},
    {"<=", 5}, {"-", 6}, {"+", 7}, {"/", 8}, {"*", 9},
    {"||", 10}, {"&&", 11}, {"!", 12}, {"abs", 13}
};

enum Type {
    OP,
    CONST,
    LPAREN,
    RPAREN,
    LSQRBRACK,
    RSQRBRACK,
    VAR,
    END,
    INVALID
};

struct Token {
    Token(Type _type = INVALID, std::string val = "")
        : type(_type), value(val) {}

    Type type;

    bool binary = true;

    // Integer value for constants, name for variables, operator name for
    // operators, empty string otherwise
    std::string value;
};

// Generates tokens from a given string
class Lexer {
public:
    Lexer(std::string const &s)
        : input(s), previousToken(Token(Type::END)), returnPrevToken(false) {}

    // Generates the next token and removes it from the input string
    Token getNextToken();

    // After revert the next token returned will be the previous token
    void revert() {
        returnPrevToken = true;
    }

    // Input string, modified during token generation
    std::string input;

private:
    // The previous token in case we want to "unget" the current token, maybe
    // because we went too far
    Token previousToken;
    bool returnPrevToken;
};

class Parser {
    // EBNF of our context free grammar:
    // sign = "+" | "-"
    // op =  sign | "*" | "/" | "&&" | "||" | "!" | "=="
    // digit = "0"|...|"9"
    // number = [sign] {digit}[.{digit}]
    // variable = [{digit}]{"a"|"A",...,"z"|"Z"|"_"}[{digit}]
    // expression = number | [sign] variable | "(" op {expression} ")"
    // paranthExpression = "(" expression ")"

public:
    Expression parse(std::string const &input) const;

private:
    Expression parseExpression(Lexer &lexer) const;
    Expression parseOpExpression(Lexer &lexer) const;
};

class InfixParser {
    // EBNF
    // Expression --> (P | LogicEXP) (BinaryOP P)*
    // P --> variable| number | "(" Expression ")" | UnaryOP P | LogixEXP
    // BinaryOP --> "+" | "-" | "*" | "/" | ">" | ">=" | "<" | "<="
    // UnaryOP --> "-" | "abs"
    //
    // LogicEXP --> ("[" LP (LogicBinaryOP LP)* "]")*
    // LP --> variable | number | LogicUnaryOP LP | LogicEXP | "(" Expression
    // ")"
    // LogicBinaryOP --> "&&" | "||" | "=="
    // LogicUnaryOP "!"

public:
    // Parses the string and returns an expression or throws an error
    Expression parse(std::string const &input);

private:
    void E(Lexer &lexer);
    void P(Lexer &lexer);

    void LP(Lexer &lexer);
    void LogicEXP(Lexer &lexer);

    bool isBinaryOperator(Token const &token);
    bool isUnaryOperator(Token const &token);
    bool isLogicalBinaryOperator(Token const &token);
    bool isLogicalUnaryOperator(Token const &token);
    void expect(Type type, Lexer &lexer);
    void pushOperator(Token const &token);
    void popOperator();
    bool hasHigherPrecedence(Token const &first, Token const &second);

    // Creates an expression of type op(lhs, rhs)
    Expression createExpression(Expression const &lhs, Token op,
                                Expression const &rhs) const;

    Expression createUnaryExpression(Expression const &exp, Token op) const;

    std::stack<Token> operators;
    std::stack<Expression> operands;

    Token next;
    void consume(Lexer &lexer);
};
}

#endif
