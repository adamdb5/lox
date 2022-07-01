package net.adambruce.lox;

import java.util.List;

import static net.adambruce.lox.TokenType.*;

/**
 * Parses a stream of Lox tokens and produces an AST.
 */
public class Parser {

    /** Used to catch internal parsing errors. */
    private static class ParseError extends RuntimeException {}

    /** The list of tokens being parsed. */
    private final List<Token> tokens;

    /** The index of the current token being parsed. */
    private int current = 0;

    Parser(List<Token> tokens) {
        this.tokens = tokens;
    }

    /**
     * Parses the tokens and produces a single expression representing the program as an AST.
     * If an error occurs during parsing, this will return null.
     * @return the expression.
     */
    Expr parse() {
        try {
            return expression();
        } catch (ParseError error) {
            return null;
        }
    }

    /**
     * TODO: Work out what's actually going on here!
     * @return
     */
    private Expr expression() {
        return equality();
    }

    /**
     * TODO: Work out what's actually going on here!
     * @return
     */
    private Expr equality() {
        Expr expr = comparison();
        while (match(BANG_EQUAL, EQUAL_EQUAL)) {
            Token operator = previous();
            Expr right = comparison();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    /**
     * TODO: Work out what's actually going on here!
     * @return
     */
    private Expr comparison() {
        Expr expr = term();

        while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
            Token operator = previous();
            Expr right = term();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    /**
     * TODO: Work out what's actually going on here!
     * @return
     */
    private Expr term() {
        Expr expr = factor();

        while (match(MINUS, PLUS)) {
            Token operator = previous();
            Expr right = factor();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    /**
     * TODO: Work out what's actually going on here!
     * @return
     */
    private Expr factor() {
        Expr expr = unary();

        while (match(SLASH, STAR)) {
            Token operator = previous();
            Expr right = unary();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    /**
     * TODO: Work out what's actually going on here!
     * @return
     */
    private Expr unary() {
        if (match(BANG, MINUS)) {
            Token operator = previous();
            Expr right = unary();
            return new Expr.Unary(operator, right);
        }

        return primary();
    }

    /**
     * TODO: Work out what's actually going on here!
     * @return
     */
    private Expr primary() {
        if (match(FALSE)) return new Expr.Literal(false);
        if (match(TRUE)) return new Expr.Literal(true);
        if (match(NIL)) return new Expr.Literal(null);

        if (match(NUMBER, STRING)) {
            return new Expr.Literal(previous().literal);
        }

        if (match(LEFT_PAREN)) {
            Expr expr = expression();
            consume(RIGHT_PAREN, "Expect ')' after expression.");
            return new Expr.Grouping(expr);
        }

        throw error(peek(), "Expect expression.");
    }

    /**
     * Consumes a token of the given type. If the current token if not of the given type, an error will be thrown.
     * @param type the desired token type.
     * @param message the error message to throw, if the token does not match.
     * @return the current token.
     */
    private Token consume(TokenType type, String message) {
        if (check(type)) return advance();

        throw error(peek(), message);
    }

    /**
     * Checks if the current token is one of the given types. The token is not consumed.
     * @param types the list of desired token types.
     * @return if the current token matches one of the desired types.
     */
    private boolean match(TokenType... types) {
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }

    /**
     * Checks if the current token if of the given type.
     * @param type the desired token type.
     * @return if the current token does not match the desired type.
     */
    private boolean check(TokenType type) {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    /**
     * Consumes the current token and returns it.
     * @return the current token.
     */
    private Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    /**
     * Determines whether the parser has reached the end of the token list.
     * @return if the parser is at the end of the token list.
     */
    private boolean isAtEnd() {
        return peek().type == EOF;
    }

    /**
     * Returns the current token without consuming it.
     * @return the current token.
     */
    private Token peek() {
        return tokens.get(current);
    }

    /**
     * Returns the previously consumed token.
     * @return the previous token.
     */
    private Token previous() {
        return tokens.get(current - 1);
    }

    /**
     * Creates a new parse error with the given token and error message.
     * @param token the token that caused the error.
     * @param message the message to be displayed in the error.
     * @return the new parse error.
     */
    private ParseError error(Token token, String message) {
        Lox.error(token, message);
        return new ParseError();
    }

    /**
     * Synchronises the parser.
     * TODO: Learn what this means.
     */
    private void synchronize() {
        advance();
        while (!isAtEnd()) {
            if (previous().type == SEMICOLON) return;

            switch (peek().type) {
                case CLASS,
                    FOR,
                    FUN,
                    IF,
                    PRINT,
                    RETURN,
                    VAR,
                    WHILE -> { return; }
            }

            advance();
        }
    }

}
