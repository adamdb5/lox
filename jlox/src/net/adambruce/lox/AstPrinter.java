package net.adambruce.lox;

/**
 * Utility class to print an AST.
 */
public class AstPrinter implements Expr.Visitor<String> {

    /**
     * Returns a string representation of the given expression.
     * @param expr the expression.
     * @return the string representation.
     */
    String print(Expr expr) {
        return expr.accept(this);
    }


    @Override
    public String visitAssignExpr(Expr.Assign expr) {
        return parenthesize(expr.name.lexeme, expr);
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right);
    }

    @Override
    public String visitCallExpr(Expr.Call expr) {
        return parenthesize("fn " + expr.callee);
    }

    @Override
    public String visitGetExpr(Expr.Get expr) {
        return parenthesize("get " + expr.name);
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return parenthesize("group", expr.expression);
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        if (expr.value == null) return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitLogicalExpr(Expr.Logical expr) {
        return parenthesize(expr.operator.lexeme, expr);
    }

    @Override
    public String visitSetExpr(Expr.Set expr) {
        return parenthesize("set " + expr.name);
    }

    @Override
    public String visitSuperExpr(Expr.Super expr) {
        return parenthesize("super" + expr.method);
    }

    @Override
    public String visitThisExpr(Expr.This expr) {
        return parenthesize("this");
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        return parenthesize(expr.operator.lexeme, expr.right);
    }

    @Override
    public String visitVariableExpr(Expr.Variable expr) {
        return parenthesize(expr.name.lexeme, expr);
    }

    /**
     * Parenthesizes the given expressions, placing the name in the parentheses.
     * @param name the expression name (e.g +, -, ...)
     * @param exprs the list of expressions.
     * @return the string representation of the parenthesized expression.
     */
    private String parenthesize(String name, Expr... exprs) {
        StringBuilder builder = new StringBuilder();

        builder.append("(").append(name);
        for (Expr expr : exprs) {
            builder.append(" ");
            builder.append(expr.accept(this));
        }
        builder.append(")");

        return builder.toString();
    }

    public static void main(String[] args) {
        Expr expression = new Expr.Binary(
            new Expr.Unary(
                new Token(TokenType.MINUS, "-", null, 1),
                new Expr.Literal(123)),
            new Token(TokenType.STAR, "*", null, 1),
            new Expr.Grouping(
                new Expr.Literal(45.67)
            )
        );

        System.out.printf(new AstPrinter().print(expression));
    }
}
