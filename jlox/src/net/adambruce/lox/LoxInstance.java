package net.adambruce.lox;

import java.util.HashMap;
import java.util.Map;

/**
 * A Lox class instance.
 */
public class LoxInstance {

    /** The class that this is an instance of. */
    private LoxClass klass;

    /** The fields held by this instance. */
    private final Map<String, Object> fields = new HashMap<>();

    LoxInstance(LoxClass klass) {
        this.klass = klass;
    }

    /**
     * Gets a field by name.
     * @param name the field name.
     * @return the field's value.
     */
    Object get(Token name) {
        if (fields.containsKey(name.lexeme)) {
            return fields.get(name.lexeme);
        }

        LoxFunction method = klass.findMethod(name.lexeme);
        if (method != null) return method.bind(this);

        throw new RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
    }

    /**
     * Sets a field..
     * @param name the field's name.
     * @param value the field's value.
     */
    void set(Token name, Object value) {
        fields.put(name.lexeme, value);
    }

    @Override
    public String toString() {
        return klass.name + " instance";
    }
}
