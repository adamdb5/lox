package net.adambruce.lox;

/**
 * Interprets a Lox expression..
 */
public class Interpreter implements Expr.Visitor<Object> {

    /**
     * Interprets the given Lox expression.
     * @param expression the Lox expression.
     */
    void interpret(Expr expression) {
        try {
            Object value = evaluate(expression);
            System.out.println(stringify(value));
        } catch (RuntimeError error) {
            Lox.runtimeError(error);
        }
    }

    @Override
    public Object visitBinaryExpr(Expr.Binary expr) {
        Object left = evaluate(expr.left);
        Object right = evaluate(expr.right);

        switch (expr.operator.type) {
            /* Subtraction. */
            case MINUS -> {
                checkNumberOperands(expr.operator, left, right);
                return (double)left - (double)right;
            }

            /* Addition (numbers and strings). */
            case PLUS  -> {
                if (left instanceof Double && right instanceof Double) {
                    return (double)left + (double)right;
                }
                if (left instanceof String && right instanceof String) {
                    return (String)left + (String)right;
                }

                /* Mixed doubles and strings. */
                if (left instanceof String && right instanceof Double) {
                    return (String)left + stringify(right);
                }
                if (left instanceof Double && right instanceof String) {
                    return stringify(left) + (String)right;
                }

                throw new RuntimeError(expr.operator, "Operands must be two numbers or two strings.");
            }

            /* Division. */
            case SLASH -> {
                checkNumberOperands(expr.operator, left, right);

                if (right instanceof Double && (double)right == 0) {
                    throw new RuntimeError(expr.operator, "Cannot divide by zero.");
                }

                return (double)left / (double)right;
            }

            /* Multiplication. */
            case STAR  -> {
                checkNumberOperands(expr.operator, left, right);
                return (double)left * (double)right;
            }

            /* Greater than. */
            case GREATER -> {
                checkNumberOperands(expr.operator, left, right);
                return (double)left >  (double)right;
            }

            /* Greater than or equal to. */
            case GREATER_EQUAL -> {
                checkNumberOperands(expr.operator, left, right);
                return (double)left >= (double)right;
            }

            /* Less than. */
            case LESS -> {
                checkNumberOperands(expr.operator, left, right);
                return (double)left <  (double)right;
            }

            /* Less than or equal to. */
            case LESS_EQUAL -> {
                checkNumberOperands(expr.operator, left, right);
                return (double)left <= (double)right;
            }

            /* Not equal. */
            case BANG_EQUAL -> { return !isEqual(left, right); }

            /* Equal. */
            case EQUAL_EQUAL -> { return isEqual(left, right); }
        }

        return null;
    }

    @Override
    public Object visitGroupingExpr(Expr.Grouping expr) {
        return evaluate(expr.expression);
    }

    @Override
    public Object visitLiteralExpr(Expr.Literal expr) {
        return expr.value;
    }

    @Override
    public Object visitUnaryExpr(Expr.Unary expr) {
        Object right = evaluate(expr.right);

        switch (expr.operator.type) {
            case BANG  -> { return !isTruthy(right); }
            case MINUS -> {
                checkNumberOperand(expr.operator, right);
                return -(double)right;
            }
        }

        return null;
    }

    /**
     * Determines whether the given operand is a number.
     * @param operator the expression's operator.
     * @param operand the operand.
     */
    private void checkNumberOperand(Token operator, Object operand) {
        if (operand instanceof Double) return;
        throw new RuntimeError(operator, "Operand must be a number.");
    }

    /**
     * Determines whether both operands are numbers.
     * @param operator the expression's operator.
     * @param left the left expression.
     * @param right the right expression.
     */
    private void checkNumberOperands(Token operator, Object left, Object right) {
        if (left instanceof Double && right instanceof Double) return;
        throw new RuntimeError(operator, "Operands must be numbers.");
    }

    /**
     * Evaluates an expression.
     * TODO: Work out how this works.
     * @param expr the expression to evaluate.
     * @return the result of the expression.
     */
    private Object evaluate(Expr expr) {
        return expr.accept(this);
    }

    /**
     * Determines whether a object should be considered true or false.
     * @param object the object.
     * @return whether the object is considered true or false.
     */
    private boolean isTruthy(Object object) {
        if (object == null) return false;
        if (object instanceof Boolean) return (boolean)object;
        return true;
    }

    /**
     * Determines whether two objects are equal.
     * @param a the first object.
     * @param b the second object.
     * @return whether the objects are equal.
     */
    private boolean isEqual(Object a, Object b) {
        if (a == null && b == null) return true;
        if (a == null) return false;

        return a.equals(b);
    }

    /**
     * Converts an object to a string.
     * @param object the object.
     * @return the string representation of the object.
     */
    private String stringify(Object object) {
        if (object == null) return "nil";

        if (object instanceof Double) {
            String text = object.toString();
            if (text.endsWith(".0")) {
                text = text.substring(0, text.length() - 2);
            }
            return text;
        }
        return object.toString();
    }
}
