#include "parser.h"
#include "catamorph/expression.h"
#include "catamorph/factories.h"
#include "parser.h"
#include "string_utils.h"

#include <assert.h>
#include <iostream>
#include <regex>

using namespace std;

namespace sdac_parser {
Token Lexer::getNextToken() {
    if (returnPrevToken) {
        returnPrevToken = false;
        return previousToken;
    }

    // trim surrounding parantheses
    StringUtils::trim(input);
    // Operator symbols and their regex checks. Each pair describes an operator,
    // where the first value corresponds to the operator symbol and the second
    // value to the regex which is used to check for the operator in the input
    // string.
    vector<pair<string, regex>> operator_regex_pairs;

    // Important: If a regex r1 is a specialization of a regex r2 (in the sense
    // that r1 will match if r2 matches, but r2 will not match if r1 matches),
    // then r2 should be checked before r1. Ideally, regexes should be defined
    // such that this can't happen.

    // Arithmetic operators
    operator_regex_pairs.emplace_back("+", regex("^\\+"));
    operator_regex_pairs.emplace_back("-", regex("^-"));
    operator_regex_pairs.emplace_back("*", regex("^\\*"));
    operator_regex_pairs.emplace_back("/", regex("^\\/"));
    // Logical operators
    operator_regex_pairs.emplace_back("&&", regex("^\\&\\&"));
    operator_regex_pairs.emplace_back("||", regex("^\\|\\|"));
    operator_regex_pairs.emplace_back("==", regex("^\\=\\="));
    operator_regex_pairs.emplace_back("!", regex("^\\!"));
    operator_regex_pairs.emplace_back(">=", regex("^\\>\\="));
    operator_regex_pairs.emplace_back(">", regex("^\\>"));
    operator_regex_pairs.emplace_back("<=", regex("^\\<\\="));
    operator_regex_pairs.emplace_back("<", regex("^\\<"));

    // Absolute amount function regex: abs(x)
    // We currently have to handle the absolute amount as a special case,
    // because we want to allow variables beginning with abs, e.g. abs(absinth).
    // Thus, we have to check if abs is followed by a parentheses and then
    // rewind the last parentheses.
    regex absRegex("^abs\\((.*)");

    // Regex for constant numbers.
    // Note that we use the passive group (?:subpattern) here, so that we only
    // have one backreference for the whole constant and not multiple
    // backreferences for individual constant parts
    regex constantRegex("^((?:[[:digit:]]+)(?:\\.(?:(?:[[:digit:]]+)?))?)");
    // Variables contain letters, numbers or "_",
    // but must not start with a number
    regex varRegex("^((?:[[:alpha:]_]+)(?:[[:alnum:]_]*))");
    // Brackets and Iverson brackets
    vector<pair<Type, regex>> bracket_regex_pairs;
    bracket_regex_pairs.emplace_back(Type::LPAREN, regex("^\\("));
    bracket_regex_pairs.emplace_back(Type::RPAREN, regex("^\\)"));
    bracket_regex_pairs.emplace_back(Type::LSQRBRACK, regex("^\\["));
    bracket_regex_pairs.emplace_back(Type::RSQRBRACK, regex("^\\]"));

    Token token;
    smatch match;

    // See above: we handle absolute amount as a special case
    if (regex_match(input, absRegex)) {
        token.type = Type::OP;
        token.value = "abs";
        // While we check matches with abs( we only want to prune abs, not (
        input = regex_replace(input, regex("abs(.*)"), "$1");
        previousToken = token;
        return token;
    }

    for (auto operator_regex_pair : operator_regex_pairs) {
        if (regex_search(input, match, operator_regex_pair.second)) {
            token.type = Type::OP;
            token.value = operator_regex_pair.first;
            input = input.substr(match.position() + match.length());
            previousToken = token;
            return token;
        }
    }
    for (auto bracket_regex_pair : bracket_regex_pairs) {
        if (regex_search(input, match, bracket_regex_pair.second)) {
            token.type = bracket_regex_pair.first;
            input = input.substr(match.position() + match.length());
            previousToken = token;
            return token;
        }
    }
    if (regex_search(input, match, constantRegex)) {
        token.type = Type::CONST;
        token.value = input.substr(0, match.position() + match.length());
        input = input.substr(match.position() + match.length());
    } else if (regex_search(input, match, varRegex)) {
        token.type = Type::VAR;
        token.value = input.substr(0, match.position() + match.length());
        input = input.substr(match.position() + match.length());
    } else if (input.empty()) {
        token.type = Type::END;
    }
    if (token.type == Type::INVALID) {
        string error = "Illegal token at start of substring: \"" + input;
        error += "\"";
        throw invalid_argument(error);
        exit(0);
    }
    previousToken = token;
    return token;
}

/*
 * PREFIX PARSER
 */

Expression Parser::parse(string const &input) const {
    // For user convenience we surround the input string with parantheses
    string expr = "(" + input + ")";
    Lexer lexer(expr);
    return parseExpression(lexer);
}

Expression Parser::parseExpression(Lexer &lexer) const {
    Token token = lexer.getNextToken();
    switch (token.type) {
    case Type::CONST:
        return Factories::cst(stod(token.value));
        break;
    case Type::VAR:
        return Factories::var(token.value);
        break;
    case Type::OP:
        lexer.revert();
        return parseOpExpression(lexer);
        break;
    case Type::LPAREN:
    {
        string const beforeRParen = lexer.input;
        Expression expression = parseExpression(lexer);
        token = lexer.getNextToken();
        if (token.type != Type::RPAREN) {
            throw invalid_argument("Missing ) for substring " + beforeRParen);
        }
        return expression;
        break;
    }
    case Type::RPAREN:
        throw invalid_argument("No matching ( for substring " + lexer.input);
        break;
    case Type::LSQRBRACK:
    {
        string const beforeRParen = lexer.input;
        Expression expression = parseExpression(lexer);
        token = lexer.getNextToken();
        if (token.type != Type::RSQRBRACK) {
            throw invalid_argument("Missing ] for substring " + beforeRParen);
        }
        return expression;
        break;
    }
    case Type::RSQRBRACK:
        throw invalid_argument("No matching [ for substring " + lexer.input);
    default:
        throw invalid_argument("Illegal expression: " + lexer.input);
        break;
    }
}

Expression Parser::parseOpExpression(Lexer &lexer) const {
    Token token = lexer.getNextToken();
    assert(token.type == Type::OP);
    string opType = token.value;

    string const beforeRParen = lexer.input;
    token = lexer.getNextToken();
    vector<Expression> exprs;
    while (token.type != Type::RPAREN) {
        if (token.type == Type::END) {
            throw invalid_argument("Missing ) for substring " + beforeRParen);
        }
        lexer.revert();
        exprs.push_back(parseExpression(lexer));
        token = lexer.getNextToken();
    }
    if (exprs.empty()) {
        throw invalid_argument("Empty operator: " + opType + " before" +
                               beforeRParen);
    }
    // Revert because the ")" is again checked in the "(" expression part
    lexer.revert();
    if (opType == "+") {
        return Factories::add(exprs);
    } else if (opType == "-") {
        return Factories::sub(exprs);
    } else if (opType == "*") {
        return Factories::mul(exprs);
    } else if (opType == ">") {
        return Factories::greater(exprs);
    } else if (opType == "<") {
        return Factories::less(exprs);
    } else if (opType == ">=") {
        return Factories::greater_equals(exprs);
    } else if (opType == "<=") {
        return Factories::less_equals(exprs);
    } else if (opType == "&&") {
        return Factories::land(exprs);
    } else if (opType == "||") {
        return Factories::lor(exprs);
    } else if (opType == "==") {
        return Factories::equals(exprs);
    } else if (opType == "!") {
        return Factories::lnot(exprs);
    } else {
        throw invalid_argument("Unknown operator:" + opType);
    }
}

/*
 * INFIX PARSER
 */

bool InfixParser::isBinaryOperator(Token const &token) {
    return token.value == "+" || token.value == "-" || token.value == "*" ||
           token.value == "/" || token.value == ">" || token.value == ">=" ||
           token.value == "<" || token.value == "<=";
}

bool InfixParser::isUnaryOperator(Token const &token) {
    return token.value == "-" || token.value == "abs";
}

bool InfixParser::hasHigherPrecedence(Token const &first, Token const &second) {
    if (!second.binary && second.value == "-") {
        return false;
    }
    if (!first.binary && first.value == "-") {
        return true;
    }

    return op_precedence.at(first.value) > op_precedence.at(second.value);
}

bool InfixParser::isLogicalBinaryOperator(Token const &token) {
    return token.value == "&&" || token.value == "||" || token.value == "==";
}

bool InfixParser::isLogicalUnaryOperator(Token const &token) {
    return token.value == "!";
}

void InfixParser::expect(Type type, Lexer &lexer) {
    if (type == next.type) {
        consume(lexer);
    } else {
        throw invalid_argument("Parser error : expected " +
                               to_string(type) + " was " +
                               to_string(next.type));
    }
}

void InfixParser::popOperator() {
    if ((isBinaryOperator(operators.top()) ||
         isLogicalBinaryOperator(operators.top())) &&
        operators.top().binary) {
        Expression lhs = operands.top();
        operands.pop();
        Expression rhs = operands.top();
        operands.pop();
        operands.push(createExpression(rhs, operators.top(), lhs));
        operators.pop();
    } else {
        Expression lhs = operands.top();
        operands.pop();
        operands.push(createUnaryExpression(lhs, operators.top()));
        operators.pop();
    }
}

void InfixParser::pushOperator(Token const &token) {
    while (hasHigherPrecedence(operators.top(), token)) {
        popOperator();
    }
    operators.push(token);
}

void InfixParser::consume(Lexer &lexer) {next = lexer.getNextToken();}

Expression InfixParser::parse(string const &input) {
    Lexer lexer(input);
    next = lexer.getNextToken();
    Token sentinel = Token(Type::OP, "sentinel");
    operators.push(sentinel);
    E(lexer);
    expect(Type::END, lexer);

    return operands.top();
}

void InfixParser::E(Lexer &lexer) {
    if (next.type == Type::LSQRBRACK) {
        LogicEXP(lexer);
    } else {
        P(lexer);
    }
    while (isBinaryOperator(next)) {
        pushOperator(next);
        consume(lexer);
        P(lexer);
    }

    while (operators.top().value != "sentinel") {
        popOperator();
    }
}

void InfixParser::P(Lexer &lexer) {
    if (next.type == Type::VAR) {
        // cout<< "VAR: "<<next.value<<endl;
        operands.push(Factories::var(next.value));
        consume(lexer);
    } else if (next.type == Type::CONST) {
        // cout<< "Const: "<<next.value<<endl;
        operands.push(Factories::cst(stod(next.value)));
        consume(lexer);
    } else if (next.type == Type::LPAREN) {
        // cout<< "LPAREN: "<<next.value<<endl;
        consume(lexer);
        Token sentinel = Token(Type::OP, "sentinel");
        operators.push(sentinel);
        E(lexer);
        expect(Type::RPAREN, lexer);
        operators.pop();
    } else if (isUnaryOperator(next)) {
        // cout<< "unary: "<<next.value<<endl;
        next.binary = false;
        pushOperator(next);
        consume(lexer);
        P(lexer);
    } else if (next.type == Type::LSQRBRACK) {
        // cout<< "L Exp: "<<next.value<<endl;
        LogicEXP(lexer);
    } else {
        throw invalid_argument(
                  "P Unknown Token \"" + to_string(next.type) + "\" with value: \"" +
                  next.value + "\" at " + lexer.input);
    }
}

void InfixParser::LogicEXP(Lexer &lexer) {
    if (next.type != Type::LSQRBRACK) {
        throw invalid_argument("expected [");
    }
    Token sentinel = Token(Type::OP, "sentinel");
    operators.push(sentinel);
    consume(lexer); // consume [
    LP(lexer);
    while (isLogicalBinaryOperator(next)) {
        pushOperator(next);
        consume(lexer);
        LP(lexer);
    }
    expect(Type::RSQRBRACK, lexer);
    while (operators.top().value != "sentinel") {
        popOperator();
    }
    operators.pop(); // remove [
}

void InfixParser::LP(Lexer &lexer) {
    if (next.type == Type::VAR) {
        operands.push(Factories::var(next.value));
        consume(lexer);
    } else if (next.type == Type::CONST) {
        operands.push(Factories::cst(stod(next.value)));
        consume(lexer);
    } else if (isLogicalUnaryOperator(next)) {
        next.binary = false;
        pushOperator(next);
        consume(lexer);
        LP(lexer);
    } else if (next.type == Type::LSQRBRACK) {
        LogicEXP(lexer);
    } else if (next.type == Type::LPAREN) {
        consume(lexer);
        Token sentinel = Token(Type::OP, "sentinel");
        operators.push(sentinel);
        E(lexer);
        expect(Type::RPAREN, lexer);
        operators.pop();
    } else {
        throw invalid_argument(" LP Unknown Token \"" +
                               to_string(next.type) + "\" with value \"" +
                               next.value + "\" at " + lexer.input);
    }
}

Expression InfixParser::createExpression(Expression const &lhs, Token op,
                                         Expression const &rhs) const {
    vector<Expression> exprs{lhs, rhs};
    if (op.value == "+") {
        return Factories::add(exprs);
    } else if (op.value == "-") {
        return Factories::sub(exprs);
    } else if (op.value == "*") {
        return Factories::mul(exprs);
    } else if (op.value == "/") {
        return Factories::div(exprs);
    } else if (op.value == ">") {
        return Factories::greater(exprs);
    } else if (op.value == "<") {
        return Factories::less(exprs);
    } else if (op.value == ">=") {
        return Factories::greater_equals(exprs);
    } else if (op.value == "<=") {
        return Factories::less_equals(exprs);
    } else if (op.value == "&&") {
        return Factories::land(exprs);
    } else if (op.value == "||") {
        return Factories::lor(exprs);
    } else if (op.value == "==") {
        return Factories::equals(exprs);
    } else {
        throw invalid_argument("Unknown binary operator:" + op.value);
    }
}

Expression InfixParser::createUnaryExpression(Expression const &exp,
                                              Token op) const {
    if (op.value == "!") {
        return Factories::lnot({exp});
    } else if (op.value == "-") {
        vector<Expression> exprs{Factories::cst(0), exp};
        return Factories::sub(exprs);
    } else if (op.value == "abs") {
        return Factories::abs({exp});
    } else {
        throw invalid_argument("Unknown unary operator:" + op.value);
    }
}
}
