package net.adambruce.lox;

/**
 * Lox runtime error.
 */
public class RuntimeError extends RuntimeException {

    /** The token that caused the runtime error. */
    final Token token;

    RuntimeError(Token token, String message) {
        super(message);
        this.token = token;
    }

}
