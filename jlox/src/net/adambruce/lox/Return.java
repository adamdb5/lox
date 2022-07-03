package net.adambruce.lox;

/**
 * The exception used to raise the return value out of a callable.
 */
public class Return extends RuntimeException {

    /** The return value. */
    final Object value;

    Return(Object value) {
        super(null, null, false, false);
        this.value = value;
    }

}
