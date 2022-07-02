package net.adambruce.lox;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

/**
 * Entrypoint for the Lox runtime.
 */
public class Lox {

    /** The Lox interpreter. */
    private static final Interpreter interpreter = new Interpreter();

    /** Flag indicating that an error has occurred. */
    static boolean hadError = false;

    /** Flag indicating that a runtime error has occurred. */
    static boolean hadRuntimeError = false;

    public static void main(String[] args) throws IOException {
        if (args.length > 1) {
            System.out.println("Usage: jlox [script]");
            System.exit(64);
        } else if (args.length == 1) {
            runFile(args[0]);
        } else {
            runPrompt();
        }
    }

    /**
     * Run a Lox program from a file.
     * @param path the path to the file.
     * @throws IOException if an exception occurs when reading the file.
     */
    private static void runFile(String path) throws IOException {
        byte[] bytes = Files.readAllBytes(Paths.get(path));
        run(new String(bytes, Charset.defaultCharset()));

        if (hadError) System.exit(65);
        if (hadRuntimeError) System.exit(70);
    }

    /**
     * Run the Lox REPL.
     * @throws IOException if an exception occurs when reading from stdin.
     */
    private static void runPrompt() throws IOException {
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);

        for(;;) {
            System.out.print("> ");
            String line = reader.readLine();
            if (line == null)
                break;
            run(line);
            hadError = false;
        }
    }

    /**
     * Run the given Lox source code.
     * @param source the Lox source code.
     */
    private static void run(String source) {
        Scanner scanner = new Scanner(source);
        List<Token> tokens = scanner.scanTokens();
        Parser parser = new Parser(tokens);
        List<Stmt> statements = parser.parse();

        if (hadError) return;

        interpreter.interpret(statements);
    }

    /**
     * Reports the given error using the given line number.
     * @param line the line where the error occurred.
     * @param message the message to display.
     */
    static void error(int line, String message) {
        report(line, "", message);
    }

    /**
     * Reports the given error using the given token.
     * @param token the token where the error occurred.
     * @param message the message to display.
     */
    static void error(Token token, String message) {
        if (token.type == TokenType.EOF) {
            report(token.line, " at end", message);
        } else {
            report(token.line, " at '" + token.lexeme + "'", message);
        }
    }

    /**
     * Displays a runtime error and sets the hadRuntimeError flag.
     * @param error the runtime error.
     */
    static void runtimeError(RuntimeError error) {
        System.err.println(error.getMessage() +
            "\n[line " + error.token.line + "]");
        hadRuntimeError = true;
    }

    /**
     * Displays an error and sets the hadError flag.
     * @param line the line where the error occurred.
     * @param where the location where the error occurred.
     * @param message the message to display.
     */
    private static void report(int line, String where, String message) {
        System.err.println("[line " + line + "] Error" + where  + ": " + message);
        hadError = true;
    }

}
