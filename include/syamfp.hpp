/**
 * @file syamfp.hpp
 * @author KATOI (y.yagi@gmail.com)
 * @brief Mathmatical Function Parser with Shunting Yard Algorithm
 * @version 1.0
 * @date 2024-12-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __SYAMFP_HPP__
#define __SYAMFP_HPP__

#include <algorithm>
#include <charconv>
#include <concepts>
#include <complex>
#include <cstdint>
#include <deque>
#include <errno.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <numbers>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SYAMFP
{

	/** @note usually, you need not to use this namespace */
	namespace Details
	{
		/** @brief value type used in constance value
		 *  @note  You developer can change the type if you want to use other type like float128_t in C++23. */
		using ValueType = double;

		template <typename Type>
		concept MathConcept
			 = std::is_convertible_v<ValueType, Type> /* This means T is required to be able to be rleated by ValueType. */
			&& requires(Type a, Type b) {
				/* and support functions bellow: */
				{ std::sin(a) }         -> std::convertible_to<Type>;
				{ std::cos(a) }         -> std::convertible_to<Type>;
				{ std::tan(a) }         -> std::convertible_to<Type>;
				{ std::asin(a) }        -> std::convertible_to<Type>;
				{ std::acos(a) }        -> std::convertible_to<Type>;
				{ std::atan(a) }        -> std::convertible_to<Type>;
				{ std::sinh(a) }        -> std::convertible_to<Type>;
				{ std::cosh(a) }        -> std::convertible_to<Type>;
				{ std::tanh(a) }        -> std::convertible_to<Type>;
				{ std::asinh(a) }       -> std::convertible_to<Type>;
				{ std::acosh(a) }       -> std::convertible_to<Type>;
				{ std::atanh(a) }       -> std::convertible_to<Type>;
				{ std::exp(a) }         -> std::convertible_to<Type>;
				{ std::log(a) }         -> std::convertible_to<Type>;
				{ std::log10(a) }       -> std::convertible_to<Type>;
				{ std::sqrt(a) }        -> std::convertible_to<Type>;
				{ std::pow(a, b) }      -> std::convertible_to<Type>;

				/* and support operators bellow: */
				{ a + b } -> std::convertible_to<Type>;
				{ a - b } -> std::convertible_to<Type>;
				{ a * b } -> std::convertible_to<Type>;
				{ a / b } -> std::convertible_to<Type>;
			};


		template <MathConcept Type>
		using Func = Type (*)(const std::vector<Type>&);

		enum class TokenType
		{
			Variable,   /* x, z, a, etc. */
			Constant,   /* pi, e, sqrt2, etc. */
			Real,       /* 0, -5, 3.14, etc. */
			Imaginary,  /* i, 3i, -2.5i, etc. */
			Operator,   /* only +, -, *, /, ^ */
			Func1,      /* sin, cos, exp, etc. */
			Func2,      /* pow, Bessel, etc. */
			Func3,      /* laguerre, etc. */
			LParen,     /* ( */
			RParen,     /* )*/
			Comma,      /* , */
		};


		template <MathConcept Type>
		struct Token
		{
			std::string str;
			TokenType   type;
			int         arg_num;
			Type        value;
			Func<Type>  func;

			Token() = default;
			Token(const std::string& str);
			Token(const std::string& str, TokenType type, int arg_num, const Type& val, Func<Type> func)
				: str(str), type(type), arg_num(arg_num), value(val), func(func) {}
		};

		template <MathConcept Type>
		bool operator==(const Token<Type>& T1, const Token<Type>& T2)
		{
			return (T1.str == T2.str) && (T1.type == T2.type) && (T1.arg_num == T2.arg_num)
				&& (T1.value == T2.value) && (T1.func == T2.func);
		}

		template <MathConcept Type>
		bool operator!=(const Token<Type>& T1, const Token<Type>& T2) { return !(T1 == T2); }


		template <MathConcept Type>
		std::unordered_map<std::string, Token<Type>>
		RESERVED_TOKEN =
		{
			/* Operator */
			{ "+", Token<Type>{ "+", TokenType::Operator, 2, 0, [](const std::vector<Type>& args){ return args[0] + args[1]; } } },
			{ "-", Token<Type>{ "-", TokenType::Operator, 2, 0, [](const std::vector<Type>& args){ return args[0] - args[1]; } } },
			{ "*", Token<Type>{ "*", TokenType::Operator, 2, 0, [](const std::vector<Type>& args){ return args[0] * args[1]; } } },
			{ "/", Token<Type>{ "/", TokenType::Operator, 2, 0, [](const std::vector<Type>& args){ return args[0] / args[1]; } } },
			{ "^", Token<Type>{ "/", TokenType::Operator, 2, 0, [](const std::vector<Type>& args){ return std::pow(args[0], args[1]); } } },

			{ "(", Token<Type>{ "(", TokenType::LParen, 0, 0, nullptr } },
			{ ")", Token<Type>{ ")", TokenType::RParen, 0, 0, nullptr } },
			{ ",", Token<Type>{ ",", TokenType::Comma,  0, 0, nullptr } },

			/* Constant */
			{	"pi",
				Token<Type>{ "pi", TokenType::Constant, 0, std::numbers::pi_v<ValueType>, nullptr }
			},
			{	"inv_pi",
				Token<Type>{ "inv_pi", TokenType::Constant, 0, std::numbers::inv_pi_v<ValueType>, nullptr }
			},
			{	"inv_sqrtpi",
				Token<Type>{ "inv_sqrtpi", TokenType::Constant, 0, std::numbers::inv_sqrtpi_v<ValueType>, nullptr }
			},
			{	"e",
				Token<Type>{ "e", TokenType::Constant, 0, std::numbers::e_v<ValueType>, nullptr }
			},
			{	"sqrt2",
				Token<Type>{ "sqrt2", TokenType::Constant, 0, std::numbers::sqrt2_v<ValueType>, nullptr }
			},
			{	"sqrt3",
				Token<Type>{ "sqrt3", TokenType::Constant, 0, std::numbers::sqrt3_v<ValueType>, nullptr }
			},
			{	"ln2",
				Token<Type>{ "ln2", TokenType::Constant, 0, std::numbers::ln2_v<ValueType>, nullptr }
			},
			{	"ln10",
				Token<Type>{ "ln10", TokenType::Constant, 0, std::numbers::ln10_v<ValueType>, nullptr }
			},
			{	"log2e",
				Token<Type>{ "log2e", TokenType::Constant, 0, std::numbers::log2e_v<ValueType>, nullptr }
			},
			{	"log10e",
				Token<Type>{ "log10e", TokenType::Constant, 0, std::numbers::log10e_v<ValueType>, nullptr }
			},
			{	"egamma",
				Token<Type>{ "egamma", TokenType::Constant, 0, std::numbers::egamma_v<ValueType>, nullptr }
			},
			{	"phi",
				Token<Type>{ "phi", TokenType::Constant, 0, std::numbers::phi_v<ValueType>, nullptr }
			},

			/* Func1 */
			{	"sin",
				Token<Type>{
					"sin", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::sin(args[0]); }
				}
			},
			{	"cos",
				Token<Type>{
					"cos", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::cos(args[0]); }
				}
			},
			{	"tan",
				Token<Type>{
					"tan", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::tan(args[0]); }
				}
			},
			{	"asin",
				Token<Type>{
					"asin", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::asin(args[0]); }
				}
			},
			{	"acos",
				Token<Type>{
					"acos", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::acos(args[0]); }
				}
			},
			{	"atan",
				Token<Type>{
					"atan", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::atan(args[0]); }
				}
			},
			{	"sinh",
				Token<Type>{
					"sinh", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::sinh(args[0]); }
				}
			},
			{	"cosh",
				Token<Type>{
					"cosh", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::cosh(args[0]); }
				}
			},
			{	"tanh",
				Token<Type>{
					"tanh", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::tanh(args[0]); }
				}
			},
			{	"asinh",
				Token<Type>{
					"asinh", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::asinh(args[0]); }
				}
			},
			{	"acosh",
				Token<Type>{
					"acosh", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::acosh(args[0]); }
				}
			},
			{	"atanh",
				Token<Type>{
					"atanh", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::atanh(args[0]); }
				}
			},
			{	"exp",
				Token<Type>{
					"exp", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::exp(args[0]); }
				}
			},
			{	"log",
				Token<Type>{
					"log", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::log(args[0]); }
				}
			},
			{	"log10",
				Token<Type>{
					"log10", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::log10(args[0]); }
				}
			},
			{	"ln",
				Token<Type>{
					"ln", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::log(args[0]); }
				}
			},
			{	"sqrt",
				Token<Type>{
					"sqrt", TokenType::Func1, 1, 0,
					[](const std::vector<Type>& args){ return std::sqrt(args[0]); }
				}
			},

			/* Func2 */
			{	"pow",
				Token<Type>{
					"pow", TokenType::Func2, 2, 0,
					[](const std::vector<Type>& args){ return std::pow(args[0], args[1]); }
				}
			},
		};

		template <MathConcept Type>
		const Token<Type>* LPAREN_p = &RESERVED_TOKEN<Type>.at("(");

		inline bool is_real(const std::string& token)
		{
			static const std::regex Real(R"(^[+-]?\d+(\.\d+)?([eE][+-]?\d+)?$)");
			return std::regex_match(token, Real);
		}

		inline bool is_imaginary(const std::string& token)
		{
			static const std::regex Imag(R"(^[+-]?\d*(\.\d+)?([eE][+-]?\d+)?i$)");
			return std::regex_match(token, Imag);
		}

		inline bool is_left_assoc(const std::string& Operator)
		{
			static const std::unordered_map<std::string, bool>
			LEFT_ASSOC_LIST =
			{
				{ "+",	true },
				{ "-",	true },
				{ "*",	true },
				{ "/",	true },
				{ "^",	false},
			};

			return LEFT_ASSOC_LIST.at(Operator);
		}

		inline int ret_preced(const std::string& Operator)
		{
			static const std::unordered_map<std::string, int>
			PRECED =
			{
				{ "+",	0 },
				{ "-",	0 },
				{ "*",	1 },
				{ "/",	1 },
				{ "^",	2 },
			};

			return PRECED.at(Operator);
		}


		template <MathConcept Type>
		Token<Type>::Token(const std::string& str)
			: str(str), type(TokenType::Variable), arg_num(0), value(0), func(nullptr)
		{
			using namespace std::literals::complex_literals;

			auto pair = RESERVED_TOKEN<Type>.find(str);
			if (pair != RESERVED_TOKEN<Type>.end()) {
				type    = pair->second.type;
				arg_num = pair->second.arg_num;
				value   = pair->second.value;
				func    = pair->second.func;
				return;
			}

			ValueType val;
			if (is_real(str)) {
				type = TokenType::Real;
				auto [ptr, err] = std::from_chars(str.c_str(), str.c_str() + str.length(), val);
				value = static_cast<Type>(val);
			} else if (is_imaginary(str)) {
				type = TokenType::Imaginary;
				auto [ptr, err] = std::from_chars(str.c_str(), str.c_str() + str.length() - 1, val);
				if (str.length() == 1) {
					val = 1.0;
				}
				value = static_cast<Type>(val * 1.0i);
			} else {
				type = TokenType::Variable;
			}
		}

		template <typename Type>
		using Tokens = std::deque<Token<Type>>;

		template <typename Type>
		Tokens<Type> devide_to_tokens(const std::string& formula)
		{
			Tokens<Type> tokens;
			std::string_view str;

			auto is_operator = [](const std::string_view& str) -> bool
			{
				auto pair = RESERVED_TOKEN<Type>.find(std::string(str));
				if (pair == RESERVED_TOKEN<Type>.end())
					return false;

				TokenType type = pair->second.type;
				return type == TokenType::Operator
					|| type == TokenType::LParen
					|| type == TokenType::RParen
					|| type == TokenType::Comma;
			};

			for (const char& c : formula) {
				if (std::isspace(c)) {
					if (!str.empty()) {
						tokens.push_back(Token<Type>(std::string(str)));
						str = {};
					}
					/* ignore spaces */
					continue;
				}

				if (str.empty()) {
					str = {&c, 1};
					continue;
				}

				if (is_operator(std::string_view{str.data(), str.length() + 1})) {
					str = {str.data(), str.length() + 1};
					continue;
				}

				if (is_operator(str) || is_operator(std::string_view{&c, 1})) {
					tokens.emplace_back(Token<Type>(std::string(str)));
					str = {&c, 1};
					continue;
				}

				str = {str.data(), str.length() + 1};
			}

			if (!str.empty()) {
				tokens.emplace_back(Token<Type>(std::string(str)));
			}

			return tokens;
		}


		template <MathConcept Type>
		using RPNs = std::vector<Token<Type>>;


		template <MathConcept Type>
		bool has_LParen(const RPNs<Type>& stack)
		{
			return std::ranges::find(stack, *LPAREN_p<Type>) != stack.end();
		}

		template <MathConcept Type>
		bool case_of_RParen(RPNs<Type>& rpn, RPNs<Type>& stack)
		{
			if (!has_LParen(stack))
				return false;

			while (true) {
				Token<Type> poped = stack.back();
				stack.pop_back();

				if (poped.type == TokenType::LParen)
					break;

				rpn.emplace_back(poped);
			}

			if (stack.empty())
				return true;

			Token<Type> token_before_RParen = stack.back();

			/* judge whether a token befor "(" is function or not by func pointer:
			 * only func type onjects have function */
			if (token_before_RParen.func != nullptr) {
				rpn.emplace_back(token_before_RParen);
				stack.pop_back();
			}

			return true;
		}

		template <MathConcept Type>
		bool case_of_Comma(RPNs<Type>& rpn, RPNs<Type>& stack)
		{
			if (!has_LParen(stack))
				return false;

			while (stack.back() != *LPAREN_p<Type>) {
				rpn.emplace_back(stack.back());
				stack.pop_back();
			}

			return true;
		}

		template <MathConcept Type>
		void case_of_Operator(RPNs<Type>& rpn, RPNs<Type>& stack, Token<Type> token,
		                      bool is_prev_token_operator)
		{
			if (is_prev_token_operator) {
				if (token.str == "+") {
					/* Delete the plus operator to convert "+x" --> "x".
					 * So token is not pushed into any stack. */
					return;
				}

				if (token.str == "-") {
					/* Convert "-X" --> "-1 * X", considering the case where X is a function */
					rpn.emplace_back(Token<Type>("-1"));
					token = Token<Type>("*"); /* The token becomes "-1 * X" from "-X", and this token is parsed in later process. */
				}
			}

			while (!stack.empty()) {
				Token<Type> poped = stack.back();
				if (poped.type != TokenType::Operator)
					break;

				int preced_t = ret_preced(token.str);
				int preced_p = ret_preced(poped.str);

				if (is_left_assoc(token.str)) {
					if (preced_t > preced_p) break;
				} else {
					if (preced_t >= preced_p) break;
				}

				rpn.emplace_back(poped);
				stack.pop_back();
			}

			stack.emplace_back(token);
		}

		template <MathConcept Type>
		const RPNs<Type> BAD_RPNs = RPNs<Type>();

		template <MathConcept Type>
		RPNs<Type> make_rpn(const std::string& formula)
		{
			Tokens<Type> tokens = devide_to_tokens<Type>(formula);
			RPNs<Type> rpn;
			RPNs<Type> stack;
			bool is_prev_token_operator = true;
			bool retval = true;

			while (!tokens.empty()) {
				Token<Type> token = tokens.front();
				tokens.pop_front();

				switch (token.type)
				{
				case TokenType::Variable :
				case TokenType::Constant :
				case TokenType::Real :
				case TokenType::Imaginary :
					is_prev_token_operator = false;
					rpn.emplace_back(token);
					break;

				case TokenType::Func1 :
				case TokenType::Func2 :
				case TokenType::Func3 :
					is_prev_token_operator = false;
					stack.emplace_back(token);
					break;

				case TokenType::LParen :
					is_prev_token_operator = true;
					stack.emplace_back(token);
					break;

				case TokenType::RParen :
					is_prev_token_operator = false;
					retval = case_of_RParen<Type>(rpn, stack);
					break;

				case TokenType::Comma :
					is_prev_token_operator = false;
					retval = case_of_Comma<Type>(rpn, stack);
					break;

				case TokenType::Operator :
					case_of_Operator<Type>(rpn, stack, token, is_prev_token_operator);
					is_prev_token_operator = true;
					break;
				}

				if (retval != true) {
					return BAD_RPNs<Type>;
				}
			}

			/* Push the remaining operators in thg stacks */
			while (!stack.empty()) {
				if (has_LParen<Type>(stack)) {
					return BAD_RPNs<Type>;
				}
				rpn.emplace_back(stack.back());
				stack.pop_back();
			}

			return rpn;
		}
	}

	template <Details::MathConcept Type>
	class VariableTable
	{
	private:
		std::unordered_map< std::string, Type > variables;

		template <typename STR, typename VAL>
		void insert_variable(const STR& str, const VAL& val)
		{
			variables[std::string(str)] = static_cast<Type>(val);
		}

		template <typename STR, typename VAL, typename... REST>
		void insert_variable(const STR& str, const VAL& val, const REST&... rest)
		{
			variables[std::string(str)] = static_cast<Type>(val);
			insert_variable(rest...); /* recursive call for the remaining argumets */
		}

	public:
		VariableTable() = default;
		VariableTable(const std::string& str, const Type& val)
		{
			variables[str] = val;
		}

		/**
		 * @brief Variadic constructor to create a VariableTable object with multiple variable name-value pairs.
		 *
		 * This constructor initializes the table with multiple variable name-value pairs provided as arguments.
		 *
		 * @tparam `STRING` The type of the std::string input.
		 * @tparam `TYPE` The type of the value input.
		 * @tparam `REST` Variadic arguments for additional variable name-value pairs.
		 * @param str1 The name of the first variable.
		 * @param val1 The value of the first variable.
		 * @param rest Subsequent variable name-value pairs.
		 */
		template <typename STRING, typename TYPE, typename... REST>
		VariableTable(const STRING& str1, const TYPE& val1, const REST&... rest)
		{
			insert_variable(str1, val1, rest...);
		}

		~VariableTable() = default;

		bool contains(const std::string& str) const noexcept
		{
			return variables.contains(str);
		}

		Type at(const std::string& str) const
		{
			return variables.at(str);
		}

		void add(const std::string& str, const Type& val)
		{
			insert_variable(str, val);
		}

		void clear_all(void)
		{
			variables->clear();
		}

		VariableTable<Type> operator+=(const std::pair<std::string, Type>& pair)
		{
			insert_variable(pair.first, pair.second);
			return *this;
		}

		Type& operator[](const std::string& str)
		{
			return variables[str];
		}

		/** @return `auto` return iterator of the first variable in this table */
		auto begin(void) const
		{
			return variables.begin();
		}

		/** @return `auto` return iterator of the end variable in this table */
		auto end(void) const
		{
			return variables.end();
		}
	};


	namespace Details
	{
		template <MathConcept Type>
		using Stack = std::deque<Type>;

		template <MathConcept Type>
		using CompiledFunc = std::function<void(Stack<Type>&, const VariableTable<Type>&)>;

		template <MathConcept Type>
		using CompiledRPN = std::vector<CompiledFunc<Type>>;

		using VariableList = std::unordered_set<std::string>;

		/**
		 * @brief parse rpn and make function object list
		 *
		 * @tparam `Type` MathConcept type
		 * @param[in] rpn
		 * @param[out] var_list this is inserted variable strings used in rpn. use it to check variable table.
		 * @return `CompiledRPN<Type>`
		 * @throw `std::invalid_argument` if the number of function argument is invalid
		 */
		template <MathConcept Type>
		CompiledRPN<Type> compile_RPN(const RPNs<Type>& rpn, VariableList& var_list)
		{
			int var_cnt = 0; /* for format check */
			CompiledRPN<Type> crpn;
			var_list.clear();

			for (const Details::Token<Type>& token : rpn) {
				switch (token.type)
				{
				case TokenType::Variable :
					var_list.insert(token.str);
					crpn.emplace_back([token](Stack<Type>& stack, const VariableTable<Type>& table)
					{
						stack.emplace_back(table.at(token.str));
					});
					break;
				case TokenType::Constant :
				case TokenType::Real :
				case TokenType::Imaginary :
					crpn.emplace_back([token](Stack<Type>& stack, const VariableTable<Type>& table)
					{
						stack.emplace_back(token.value);
					});
					break;
				case TokenType::Operator :
				case TokenType::Func1 :
				case TokenType::Func2 :
				case TokenType::Func3 :
					var_cnt -= token.arg_num;
					if (var_cnt < 0) {
						throw std::invalid_argument("Invalid formula: missing number of argument for " + token.str);
					}
					crpn.emplace_back([token](Stack<Type>& stack, const VariableTable<Type>& table)
					{
						std::vector<Type> args(token.arg_num);
						for (int n = token.arg_num - 1; n >= 0; --n) {
							args[n] = stack.back();
							stack.pop_back();
						}
						stack.emplace_back(token.func(args));
					});
					break;
				}
				var_cnt++;
			}

			if (var_cnt != 1) {
				throw std::invalid_argument("Invalid formula: some functions have too many arguments");
			}

			return crpn; /* return value equal to BAD_CRPN if rpn is BAD_RPNs */
		}
	}


	template <Details::MathConcept Type>
	class Syamfp
	{
	private:
		std::string formula;
		VariableTable<Type> table;
		Details::VariableList vars;
		Details::CompiledRPN<Type> crpn;

	public:
		Syamfp() = default;

		Syamfp(const std::string& formula, const VariableTable<Type>& table = VariableTable<Type>())
			: formula(formula), table(table), vars(), crpn() {};

		~Syamfp() = default;

		/**
		 * @brief parse and compile formula
		 * @param formula compiled formula
		 * @return `int` `0`:Success, `negative`:Error
		 */
		int parse(const std::string& formula)
		{
			auto rpn = Details::make_rpn<Type>(formula);
			if (rpn == Details::BAD_RPNs<Type>) {
				return -EINVAL;
			}

			try {
				auto crpn = Details::compile_RPN<Type>(rpn, vars);
				this->crpn = crpn;
				this->formula = formula;
				return 0;
			} catch (const std::exception& e) {
				return -EINVAL;
			}
		}

		/**
		 * @brief parse and compile formula
		 * @param formula compiled formula
		 * @param table used variable table
		 * @return `int` `0`:Success, `negative`:Error
		 */
		int parse(const std::string& formula, const VariableTable<Type>& table)
		{
			this->table = table;
			return parse(formula);
		}

		/** @brief register variable table */
		void regist(const VariableTable<Type>& table)
		{
			this->table = table;
		}

		/**
		 * @brief return functional object
		 *
		 * @param[in] variable_string variable string used as variable like "x", "z", etc.
		 * @return `auto` functional object
		 * @throw `std::runtime_error` if unknown variable is included in function
		 */
		auto ret_func(const std::string& variable_string) -> std::function<Type(const Type&)>
		{
			/* check variable list */
			for (const std::string& str : vars) {
				if (!table.contains(str) && (str != variable_string)) {
					throw std::runtime_error("Invalid function: some variable are not determined");
				}
			}

			/* copy member variable to use in lambda function */
			VariableTable<Type> Table = table;
			Details::CompiledRPN<Type> CRPN = crpn;
			return [Table, CRPN, variable_string](const Type& value) -> Type
			{
				Details::Stack<Type> stack;
				VariableTable<Type> table(Table);
				table += {variable_string, value};

				for (const Details::CompiledFunc<Type>& func : CRPN) {
					func(stack, table);
				}

				return stack.front();
			};
		}
	};

	template <Details::MathConcept Type>
	void add_custom_function(const std::string& name, Details::TokenType type, int arg_num, Details::Func<Type> lambda)
	{
		Details::RESERVED_TOKEN<Type>.insert({
			name,
			Details::Token<Type>(name, type, arg_num, static_cast<Details::ValueType>(0.0), lambda)
		});
	}
}

#endif /* end of __SYAMFP_HPP__ */
