package net.adambruce.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static net.adambruce.lox.TokenType.*;

/**
 * Scans a Lox program and produces a stream of Tokens.
 */
public class Scanner {

    /** The Lox program source code. */
    private final String source;

    /** The list of tokens that have been created. */
    private final List<Token> tokens = new ArrayList<>();

    /** The start position of the current token. */
    private int start = 0;

    /** The current position of the cursor. */
    private int current = 0;

    /** The current line that is being scanned. */
    private int line = 1;

    /** A map of keywords and their corresponding token types. */
    private static final Map<String, TokenType> keywords;

    static {
        keywords = new HashMap<>();
        keywords.put("and", AND);
        keywords.put("class", CLASS);
        keywords.put("else", ELSE);
        keywords.put("false", FALSE);
        keywords.put("for", FOR);
        keywords.put("fun", FUN);
        keywords.put("if", IF);
        keywords.put("nil", NIL);
        keywords.put("or", OR);
        keywords.put("print", PRINT);
        keywords.put("return", RETURN);
        keywords.put("super", SUPER);
        keywords.put("this", THIS);
        keywords.put("true", TRUE);
        keywords.put("var", VAR);
        keywords.put("while", WHILE);
    }


    Scanner(String source) {
        this.source = source;
    }

    /**
     * Traverses the given source and produces a list of tokens.
     * @return a list of tokens.
     */
    List<Token> scanTokens() {
        while(!isAtEnd()) {
            start = current;
            scanToken();
        }
        tokens.add(new Token(EOF, "", null, line));
        return tokens;
    }

    /**
     * Determines whether the scanner has reached the end of the program's source.
     * @return true if at end of source.
     */
    private boolean isAtEnd() {
        return current >= source.length();
    }

    /**
     * Scans a single token and adds it to the list of tokens.
     */
    private void scanToken() {
        char c = advance();
        switch (c) {

            /* Single character tokens .*/
            case '(' -> addToken(LEFT_PAREN);
            case ')' -> addToken(RIGHT_PAREN);
            case '{' -> addToken(LEFT_BRACE);
            case '}' -> addToken(RIGHT_BRACE);
            case ',' -> addToken(COMMA);
            case '.' -> addToken(DOT);
            case '-' -> addToken(MINUS);
            case '+' -> addToken(PLUS);
            case ';' -> addToken(SEMICOLON);
            case '*' -> addToken(STAR);

            /* One or two character tokens (requires one character lookahead). */
            case '!' -> addToken(match('=') ? BANG_EQUAL    : BANG);
            case '=' -> addToken(match('=') ? EQUAL_EQUAL   : EQUAL);
            case '<' -> addToken(match('=') ? LESS_EQUAL    : LESS);
            case '>' -> addToken(match('=') ? GREATER_EQUAL : GREATER);

            /* Division and comments. */
            case '/' -> {

                /* Single line comment (consume tokens until end of line or end of file). */
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd()) advance();

                /* Multi line comment (consume tokens until closing delimiter), requires two character lookahead. */
                } else if (match('*')) {
                    int blockCommentDepth = 1;
                    advance();

                    while (!isAtEnd()) {
                        /* Increment block comment depth/ */
                        if (peek() == '/' && peekNext() == '*') {
                            blockCommentDepth++;
                            current += 2;
                        }

                        /* Decrement block comment depth. */
                        else if (peek() == '*' && peekNext() == '/') {
                            blockCommentDepth--;
                            current += 2;
                        }

                        /* If all opened comments have been closed, break the loop. */
                        else if (blockCommentDepth == 0) {
                            break;
                        }

                        /* If we're at a newline, increment the line count. */
                        else if (advance() == '\n')
                            line++;
                    }

                    if (isAtEnd()) {
                        Lox.error(line, "Not all nested comments were closed.");
                    }

                /* Division. */
                } else {
                    addToken(SLASH);
                }
            }

            /* Ignore whitespace characters. */
            case ' ', '\r', '\t' -> {}

            /* If the character is a newline, increment the line counter. */
            case '\n' -> line++;

            /* If the character is a double quote, read until the closing double quote, and create a string. */
            case '"' -> string();

            /* Attempt to parse as keyword or identifier. */
            default -> {
                if (isDigit(c)) {
                    number();
                } else if (isAlpha(c)) {
                    identifier();
                } else {
                    Lox.error(line, "Unexpected character.");
                }
            }
        }
    }

    /**
     * Determines whether a character is alphanumeric.
     * @param c the character to check.
     * @return if the character is alphanumeric.
     */
    private boolean isAlphaNumeric(char c) {
        return isAlpha(c) || isDigit(c);
    }

    /**
     * Detmerines whether a character is a letter or an underscore.
     * @param c the character to check.
     * @return if the character is a letter or an underscore.
     */
    private boolean isAlpha(char c) {
        return (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
    }

    /**
     * Reads an alphanumeric lexeme and produces a keyword token (if valid) otherwise an identifier token.
     * The new token is then added to the list of tokens.
     */
    private void identifier() {
        while(isAlphaNumeric(peek())) advance();

        String text = source.substring(start, current);
        TokenType type = keywords.get(text);
        if (type == null) type = IDENTIFIER;

        addToken(type);
    }

    /**
     * Reads a numeric (integer or floating point) lexeme and produces a numeric token.
     */
    private void number() {
        while (isDigit(peek())) advance();

        /* Look for fractional part. */
        if (peek() == '.' && isDigit(peekNext())) {

            /* Consume the '.'. */
            advance();

            while (isDigit(peek())) advance();
        }

        addToken(NUMBER, Double.parseDouble(source.substring(start, current)));
    }

    /**
     * Determines whether a character is a digit (0-9 decimal)
     * @param c the character to check.
     * @return if the character is a number.
     */
    private boolean isDigit(char c) {
        return c >= '0' && c <= '9';
    }

    /**
     * Reads in a string, up to the terminating double quote.
     */
    private void string() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') line++;
            advance();
        }

        /* Reached end of file without terminating the string. */
        if (isAtEnd()) {
            Lox.error(line, "Unterminated string.");
            return;
        }

        /* Consume the terminating double quote. */
        advance();

        /* Strip the double quotes. */
        String value = source.substring(start + 1, current - 1);
        addToken(STRING, value);
    }

    /**
     * Determines whether the current character matches the expected character.
     * @param expected the expected character.
     * @return if the current character matches the expected character.
     */
    private boolean match(char expected) {
        if (isAtEnd()) return false;
        if (source.charAt(current) != expected) return false;

        current++;
        return true;
    }

    /**
     * Returns the current character without consuming it.
     * @return the current character.
     */
    private char peek() {
        if (isAtEnd()) return '\0';
        return source.charAt(current);
    }

    /**
     * Returns the next character without consuming it.
     * @return the next character.
     */
    private char peekNext() {
        if (current + 1 >= source.length()) return '\0';
        return source.charAt(current + 1);
    }

    /**
     * Comsumes the current character and returns it.
     * @return the current character.
     */
    private char advance() {
        return source.charAt(current++);
    }

    /**
     * Adds a new token into the list of tokens, with a null literal.
     * @param type the new token type.
     */
    private void addToken(TokenType type) {
        addToken(type, null);
    }

    /**
     * Adds a new token into the list of tokens.
     * @param type the new token type.
     * @param literal the new token's literal value.
     */
    private void addToken(TokenType type, Object literal) {
        String text = source.substring(start, current);
        tokens.add(new Token(type, text, literal, line));
    }

}
