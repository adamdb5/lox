package net.adambruce.lox;

/**
 * Represents a Lox token.
 * Each token stores its type, lexeme, literal and line number.
 */
public class Token {

    /** The token's type. */
    final TokenType type;

    /** The token's lexeme (text representation). */
    final String lexeme;

    /** The token's literal value. */
    final Object literal;

    /** The line within the program that this token appears. */
    final int line;

    Token(TokenType type, String lexeme, Object literal, int line) {
        this.type = type;
        this.lexeme = lexeme;
        this.literal = literal;
        this.line = line;
    }

    @Override
    public String toString() {
        return type + " " + lexeme + " " + literal;
    }

}
