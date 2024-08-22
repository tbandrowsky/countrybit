
#ifndef CORONA_EXPRESSIONS_HPP
#define CORONA_EXPRESSIONS_HPP

namespace corona
{

    enum TokenType { VALUE, QUERY_EXPRESSION, PLUS, MINUS, MUL, DIV, LPAREN, RPAREN, END };

    class Token {
    public:
        TokenType type;
        json value;        
    };

    class Lexer 
    {

        bool parse_value(Token& _dest, const char* _src, const char** _endsrc )
        {
            json_parser jp;
            std::shared_ptr<json_value> _value;

            if (jp.parse_value(_value, _src, _endsrc)) {
                _dest.type = VALUE;
                _dest.value = _value;
                return true;
            }
            return false;
        }

    public:

        json_parser jp;

        Lexer(const std::string& text) : text(text), pos(0), current_char(text[pos]) {}

        Token get_next_token() {
            json empty;
            while (current_char != '\0') {
                if (isspace(current_char)) {
                    skip_whitespace();
                    continue;
                }

                const char* pc = pcurrent_char;
                Token value_token;

                if (parse_value(value_token, pcurrent_char, &pc)) {
                    set(pc);
                    return value_token;
                }
                if (current_char == '+') {
                    advance();
                    return Token{ PLUS, empty };
                }
                if (current_char == '-') {
                    advance();
                    return Token{ MINUS, empty };
                }
                if (current_char == '*') {
                    advance();
                    return Token{ MUL, empty };
                }
                if (current_char == '/') {
                    advance();
                    return Token{ DIV, empty };
                }
                if (current_char == '(') {
                    advance();
                    return Token{ LPAREN, empty };
                }
                if (current_char == ')') {
                    advance();
                    return Token{ RPAREN, empty };
                }
                throw std::runtime_error("Invalid character");
            }
            return Token{ END, empty };
        }

    private:
        std::string text;
        size_t pos;
        char current_char;
        char* pcurrent_char;

        void set(const char *_endptr) {
            pos = _endptr - text.c_str();
            if (pos < text.size()) {
                current_char = text[pos];
                pcurrent_char = &text[pos];
            }
            else {
                current_char = '\0';
                pcurrent_char = nullptr;
            }
        }

        void advance() {
            pos++;
            if (pos < text.size()) {
                current_char = text[pos];
                pcurrent_char = &text[pos];
            }
            else {
                current_char = '\0';
                pcurrent_char = nullptr;
            }
        }

        void skip_whitespace() {
            while (current_char != '\0' and isspace(current_char)) {
                advance();
            }
        }
    };

    class Parser {
    public:
        Parser(Lexer& lexer) : lexer(lexer), current_token(lexer.get_next_token()) {}

        json expr() {
            json result = term();
            while (current_token.type == PLUS or current_token.type == MINUS) {
                Token token = current_token;
                if (token.type == PLUS) {
                    eat(PLUS);
                    result = result + term();
                }
                else if (token.type == MINUS) {
                    eat(MINUS);
                    result = result - term();
                }
            }
            return result;
        }

    private:
        Lexer& lexer;
        Token current_token;

        void eat(TokenType token_type) {
            if (current_token.type == token_type) {
                current_token = lexer.get_next_token();
            }
            else {
                throw std::runtime_error("Invalid syntax");
            }
        }

        json factor() {
            Token token = current_token;
            if (token.type == VALUE) {
                eat(VALUE);
                return token.value;
            }
            else if (token.type == LPAREN) {
                eat(LPAREN);
                json result = expr();
                eat(RPAREN);
                return result;
            }
            else if (token.type == PLUS) {
                eat(PLUS);
                return factor();
            }
            else if (token.type == MINUS) {
                eat(MINUS);
                json t = factor();
                t = negate(t);
                return t;
            }
            throw std::runtime_error("Invalid syntax");
        }

        json term() {
            json result = factor();
            while (current_token.type == MUL or current_token.type == DIV) {
                Token token = current_token;
                if (token.type == MUL) {
                    eat(MUL);
                    result = result * factor();
                }
                else if (token.type == DIV) {
                    eat(DIV);
                    result = result / factor();
                }
            }
            return result;
        }
    };

    int test_calculator() {
        std::string text;
        while (true) {
            try {
                std::cout << "calc> ";
                std::getline(std::cin, text);
                if (text.empty()) continue;
                Lexer lexer(text);
                Parser parser(lexer);
                int result = parser.expr();
                std::cout << result << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        return 0;
    }
}

#endif
