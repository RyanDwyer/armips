#pragma once

#include <cassert>
#include <list>
#include <string>
#include <variant>
#include <vector>

class TextFile;

enum class TokenType
{
	Invalid,
	Identifier,
	Integer,
	String,
	Float,
	LParen,
	RParen,
	Plus,
	Minus,
	Mult,
	Div,
	Mod,
	Caret,
	Tilde,
	LeftShift,
	RightShift,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Equal,
	NotEqual,
	BitAnd,
	BitOr,
	LogAnd,
	LogOr,
	Exclamation,
	Question,
	Colon,
	LBrack,
	RBrack,
	Comma,
	Assign,
	Equ,
	EquValue,
	Hash,
	LBrace,
	RBrace,
	Dollar,
	NumberString,
	Degree,
	Separator
};

struct Token
{
	friend class Tokenizer;

	std::wstring getOriginalText() const
	{
		return originalText;
	}

	template<typename T>
	void setValue(T value, std::wstring originalText)
	{
		this->value = std::move(value);
		this->originalText = std::move(originalText);
	}

	const std::wstring &identifierValue() const
	{
		assert(std::holds_alternative<std::wstring>(value));
		return std::get<std::wstring>(value);
	}

	const std::wstring &stringValue() const
	{
		assert(std::holds_alternative<std::wstring>(value));
		return std::get<std::wstring>(value);
	}

	int64_t intValue() const
	{
		assert(std::holds_alternative<int64_t>(value));
		return std::get<int64_t>(value);
	}

	double floatValue() const
	{
		assert(std::holds_alternative<double>(value));
		return std::get<double>(value);
	}

	size_t line = 0;
	size_t column = 0;
	TokenType type = TokenType::Invalid;

protected:
	bool checked = false;

	using ValueType = std::variant<std::monostate, int64_t, double, std::wstring>;
	ValueType value;
	std::wstring originalText;
};

typedef std::list<Token> TokenList;

struct TokenizerPosition
{
	friend class Tokenizer;

	TokenizerPosition previous()
	{
		TokenizerPosition pos = *this;
		pos.it--;
		return pos;
	}
private:
	TokenList::iterator it;
};

class Tokenizer
{
public:
	Tokenizer();
	const Token& nextToken();
	const Token& peekToken(int ahead = 0);
	void eatToken() { eatTokens(1); }
	void eatTokens(int num);
	bool atEnd() { return position.it == tokens.end(); }
	TokenizerPosition getPosition() { return position; }
	void setPosition(TokenizerPosition pos) { position = pos; }
	void skipLookahead();
	std::vector<Token> getTokens(TokenizerPosition start, TokenizerPosition end) const;
	void registerReplacement(const std::wstring& identifier, std::vector<Token>& tokens);
	void registerReplacement(const std::wstring& identifier, const std::wstring& newValue);
	void registerReplacementString(const std::wstring& identifier, const std::wstring& newValue);
	void registerReplacementInteger(const std::wstring& identifier, int64_t newValue);
	void registerReplacementFloat(const std::wstring& identifier, double newValue);
	static size_t addEquValue(const std::vector<Token>& tokens);
	static void clearEquValues() { equValues.clear(); }
	void resetLookaheadCheckMarks();
protected:
	void clearTokens() { tokens.clear(); };
	void resetPosition() { position.it = tokens.begin(); } 
	void addToken(Token token);
private:
	bool processElement(TokenList::iterator& it);

	TokenList tokens;
	TokenizerPosition position;

	struct Replacement
	{
		std::wstring identifier;
		std::vector<Token> value;
	};

	Token invalidToken;
	std::vector<Replacement> replacements;
	static std::vector<std::vector<Token>> equValues;
};

class FileTokenizer: public Tokenizer
{
public:
	bool init(TextFile* input);
protected:
	Token loadToken();
	bool isInputAtEnd();

	void skipWhitespace();
	void createToken(TokenType type, size_t length);
	void createToken(TokenType type, size_t length, int64_t value);
	void createToken(TokenType type, size_t length, double value);
	void createToken(TokenType type, size_t length, const std::wstring& value);
	void createToken(TokenType type, size_t length, const std::wstring& value, size_t valuePos, size_t valueLength);
	void createTokenCurrentString(TokenType type, size_t length);

	bool convertInteger(size_t start, size_t end, int64_t& result);
	bool convertFloat(size_t start, size_t end, double& result);
	bool parseOperator();

	TextFile* input;
	std::wstring currentLine;
	size_t lineNumber;
	size_t linePos;
	
	Token token;
	bool equActive;
};

class TokenStreamTokenizer: public Tokenizer
{
public:
	void init(const std::vector<Token>& tokens)
	{
		clearTokens();

		for (const Token &tok: tokens)
			addToken(tok);
		
		resetPosition();
	}
};
