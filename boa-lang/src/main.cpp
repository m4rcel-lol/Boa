// Boa Language Interpreter - Main Entry Point
// Native compiled interpreter for the Boa programming language.
// No Python, Node, JVM, or other high-level runtimes are used.

#include "boa/interpreter.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static void print_usage() {
    std::cout << "Boa Language Interpreter v0.1.0\n"
              << "Usage:\n"
              << "  boa                   Start interactive REPL\n"
              << "  boa <file.boa>        Run a Boa script\n"
              << "  boa --help            Show this help\n"
              << "  boa --version         Show version\n";
}

static void print_version() {
    std::cout << "Boa v0.1.0 (native interpreter)\n";
}

static std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: cannot open file '" << path << "'\n";
        std::exit(1);
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return content;
}

static std::string extract_dir(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return ".";
    return path.substr(0, pos);
}

static int run_file(const std::string& path) {
    std::string source = read_file(path);
    boa::Interpreter interp;
    interp.set_base_dir(extract_dir(path));
    try {
        interp.run(source, path);
    } catch (const boa::LexerError& e) {
        std::cerr << "SyntaxError: " << e.what() << "\n";
        return 1;
    } catch (const boa::ParseError& e) {
        std::cerr << "ParseError: " << e.what() << "\n";
        return 1;
    } catch (const boa::BoaRuntimeError& e) {
        std::cerr << "RuntimeError: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

static void run_repl() {
    std::cout << "Boa v0.1.0 REPL (type :help for commands, Ctrl+C to exit)\n";
    boa::Interpreter interp;
    std::string line;

    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line)) break;

        if (line.empty()) continue;

        // REPL commands
        if (line == ":help") {
            std::cout << "REPL commands:\n"
                      << "  :help            Show this help\n"
                      << "  :run <file>      Run a Boa script file\n"
                      << "  :load <file>     Load and execute a file in current session\n"
                      << "  :doc <symbol>    Show documentation for a symbol\n"
                      << "  :quit            Exit the REPL\n";
            continue;
        }
        if (line == ":quit" || line == ":exit") break;

        if (line.substr(0, 5) == ":run ") {
            std::string path = line.substr(5);
            run_file(path);
            continue;
        }
        if (line.substr(0, 6) == ":load ") {
            std::string path = line.substr(6);
            try {
                std::string source = read_file(path);
                interp.set_base_dir(extract_dir(path));
                interp.run(source, path);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
            continue;
        }
        if (line.substr(0, 5) == ":doc ") {
            std::string sym = line.substr(5);
            auto val = interp.global_env()->get(sym);
            if (val) {
                std::cout << sym << " : " << boa::value_type_name(val->type) << "\n";
            } else {
                std::cout << "Symbol '" << sym << "' not found\n";
            }
            continue;
        }

        // Handle multi-line input (lines ending with ':')
        std::string source = line;
        while (!source.empty() && source.back() == ':') {
            std::cout << "... ";
            std::string cont;
            if (!std::getline(std::cin, cont)) break;
            if (cont.empty()) break;
            source += "\n" + cont;
        }

        try {
            auto result = interp.run(source);
            if (result && result->type != boa::ValueType::None) {
                std::cout << result->to_string() << "\n";
            }
        } catch (const boa::LexerError& e) {
            std::cerr << "SyntaxError: " << e.what() << "\n";
        } catch (const boa::ParseError& e) {
            std::cerr << "ParseError: " << e.what() << "\n";
        } catch (const boa::BoaRuntimeError& e) {
            std::cerr << "RuntimeError: " << e.what() << "\n";
        }
    }
    std::cout << "\nGoodbye!\n";
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        run_repl();
        return 0;
    }

    std::string arg = argv[1];
    if (arg == "--help" || arg == "-h") {
        print_usage();
        return 0;
    }
    if (arg == "--version" || arg == "-v") {
        print_version();
        return 0;
    }

    return run_file(arg);
}
