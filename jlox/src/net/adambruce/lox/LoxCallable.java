package net.adambruce.lox;

import java.util.List;

/**
 * A callable object in Lox.
 */
public interface LoxCallable {

    /**
     * Returns the callable's arity (parameter count).
     * @return the callable's arity.
     */
    int arity();

    /**
     * Interprets the callable using the given interpreter and arguments.
     * @param interpreter the interpreter.
     * @param arguments the callable's arguments.
     * @return the return value of the callable.
     */
    Object call(Interpreter interpreter, List<Object> arguments);

}
