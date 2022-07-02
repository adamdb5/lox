package net.adambruce.lox;

import java.util.HashMap;
import java.util.Map;

/**
 * The Lox environment for a scope.
 */
public class Environment {

    /** The scope's enclosed environment. */
    final Environment enclosing;

    /** The map of key value pairs for the scope's variables. */
    private final Map<String, Object> values = new HashMap<>();

    /**
     * Create a new environment with no enclosed scope.
     */
    Environment() {
        enclosing = null;
    }

    /**
     * Create a new environment with the given enclosed scope.
     * @param enclosing the enclosed scope.
     */
    Environment(Environment enclosing) {
        this.enclosing = enclosing;
    }

    /**
     * Defines a new variable.
     * @param name the variable's name.
     * @param value the variable's value.
     */
    void define(String name, Object value) {
        values.put(name, value);
    }

    /**
     * Gets the value of a variable from it's name.
     * @param name the variable's name.
     * @return the variable's value.
     * @throws RuntimeError the variable is not defined in the scope.
     */
    Object get(Token name) throws RuntimeError {
        if (values.containsKey(name.lexeme)) {
            return values.get(name.lexeme);
        }

        if (enclosing != null) return enclosing.get(name);

        throw new RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }

    /**
     * Assigns a new value to an existing variable.
     * @param name the variable's name.
     * @param value the variable's new value.
     * @throws RuntimeError the variable is not defined in the scope.
     */
    public void assign(Token name, Object value) throws RuntimeError {
        if (values.containsKey(name.lexeme)) {
            values.put(name.lexeme, value);
            return;
        }

        if (enclosing != null) {
            enclosing.assign(name, value);
            return;
        }

        throw new RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }
}
