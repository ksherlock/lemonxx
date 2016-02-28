#include "lisp_cell.h"
#include <cstdlib>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <numeric>

#include "lisp.h"

namespace lisp {

	environment env;

	namespace {

		typedef typename std::aligned_union<1,
			cell, atom_cell, symbol_cell, integer_cell, pair_cell, 
			unary_cell, binary_cell, conditional_cell, lambda_cell, builtin_cell, 
			function_cell
			>::type chunk;



		cell_ptr kTRUE = nullptr;
		cell_ptr kFALSE = nullptr;
		cell_ptr kUNDEFINED = nullptr;
		cell_ptr kNIL = nullptr;

		inline cell_ptr null_to_nil(cell_ptr c) { return c == nullptr ? c : kNIL; }
		inline cell_ptr nil_to_null(cell_ptr c) { return c == kNIL ? nullptr : c; }
	

		std::vector<cell_ptr> temp_list;
		static std::vector<void *> free_list;
		static std::vector<chunk> storage;


		template<class T>
		T *temp(T *t) { temp_list.push_back(t); return t; }

		std::array<integer_cell, 256> numbers;


		void arity_error(const std::string &s) {
			throw std::runtime_error(s);
		}




		cell_ptr builtin_eq(const std::vector<cell_ptr> &argv, const environment &e) {
			if (argv.size() != 2) arity_error("function arity error: eq requires two parameters.");

			auto a = argv[0] ? argv[0]->evaluate(e) : nullptr;
			auto b = argv[1] ? argv[1]->evaluate(e) : nullptr;

			return a == b ? kTRUE : kFALSE;
		}

		cell_ptr builtin_car(const std::vector<cell_ptr> &argv, const environment &e) {
			if (argv.size() != 1) arity_error("function arity error: car requires one parameter.");

			auto tmp = argv[0] ? argv[0]->evaluate(e) : nullptr;
			if (!tmp) return tmp;
			auto p = tmp->to_pair();

			return p.first;
		}

		cell_ptr builtin_cdr(const std::vector<cell_ptr> &argv, const environment &e) {
			if (argv.size() != 1) arity_error("function arity error: cdr requires one parameter.");

			auto tmp = argv[0] ? argv[0]->evaluate(e) : nullptr;
			if (!tmp) return tmp;
			auto p = tmp->to_pair();

			return p.second;
		}

		cell_ptr builtin_atom(const std::vector<cell_ptr> &argv, const environment &e) {

			if (argv.size() != 1) arity_error("function arity error: atom requires one parameter.");

			auto tmp = argv[0] ? argv[0]->evaluate(e) : nullptr;
			if (!tmp) return tmp;

			return tmp->is<cell::atom>() ? kTRUE : kFALSE;
		}

		cell_ptr builtin_cons(const std::vector<cell_ptr> &argv, const environment &e) {
			if (argv.size() != 2) arity_error("function arity error: cons requires two parameters.");

			auto a = argv[0] ? argv[0]->evaluate(e) : nullptr;
			auto b = argv[1] ? argv[1]->evaluate(e) : nullptr;

			return make_pair(a, b);
		}

		cell_ptr builtin_label(const std::vector<cell_ptr> &argv, const environment &e) {
			if (argv.size() != 2) arity_error("function arity error: label requires two parameters.");

			auto label = argv[0] ? argv[0]->to_symbol() : nullptr;
			auto f = argv[1] ? argv[1]->evaluate(e) : nullptr;
			return f ? f->label(label) : nullptr;
		}

		cell_ptr builtin_list(const std::vector<cell_ptr> &argv, const environment &e) {
			return std::accumulate(argv.rbegin(), argv.rend(), (cell_ptr)nullptr,
				[&](cell_ptr cdr, cell_ptr car){
					car = car->evaluate(e);
					return make_pair(car, cdr);
				});
		}


		builtin_cell builtin_eq_cell("eq", 2, builtin_eq);
		builtin_cell builtin_atom_cell("atom", 1, builtin_atom);
		builtin_cell builtin_cons_cell("cons", 2, builtin_cons);
		builtin_cell builtin_car_cell("car", 1, builtin_car);
		builtin_cell builtin_cdr_cell("cdr", 1, builtin_cdr);
		builtin_cell builtin_label_cell("label", 2, builtin_label);
		builtin_cell builtin_list_cell("list", 0, builtin_list);

		std::unordered_map<std::string, cell_ptr> atom_map;
		std::unordered_map<std::string, symbol_cell_ptr> symbol_map;
	};



	void clear_temp_list() { temp_list.clear(); }

	void initialize(std::size_t size) {

		size = std::min(size, (std::size_t)100);

		free_list.reserve(size);

		storage.reserve(size);
		storage.resize(size);
		std::transform(storage.begin(), storage.end(), std::back_inserter(free_list),
			[](chunk &x) { return &x; });


		env.emplace(make_symbol("eq"), &builtin_eq_cell);
		env.emplace(make_symbol("car"), &builtin_car_cell);
		env.emplace(make_symbol("cdr"), &builtin_cdr_cell);
		env.emplace(make_symbol("atom"), &builtin_atom_cell);
		env.emplace(make_symbol("cons"), &builtin_cons_cell);
		env.emplace(make_symbol("label"), &builtin_label_cell);
		env.emplace(make_symbol("list"), &builtin_list_cell);

		kTRUE = make_atom("T");
		kFALSE = make_atom("F");
		kUNDEFINED = make_atom("UNDEFINED");
		kNIL = make_atom("NIL");


		for (int i = 0; i < 256; ++i) {
			numbers[i] = integer_cell(i);
		}

	}

	void garbage_collect() {

		//
		if (!free_list.empty()) return;

		for (auto &x : storage) {
			cell_ptr cp = (cell_ptr)&x;
			cp->alive = false;
		}

		for (auto &kv : env) {
			kv.first->mark();
			if (kv.second) kv.second->mark();
		}

		for (auto x : temp_list) {
			if (x) x->mark(); 
		}


		for (auto &x : storage) {
			cell_ptr cp = (cell_ptr)&x;
			if (!cp->alive) {
				delete cp;
				free_list.push_back(&x);
			}
		}
	}

	void *cell::operator new(std::size_t size) {

		if (free_list.empty()) garbage_collect();
		if (free_list.empty()) throw std::bad_alloc();
		void *vp = free_list.back();
		//temp_list.push_back(vp);
		free_list.pop_back();
		return vp;
	}

	void cell::operator delete(void *vp) {
		// remove from temp_list, just in case... should never happen.
		//std::erase(temp_list.begin(), std::remove(temp_list.begin(), temp_list.end(), vp));
		//free_list.push_back(vp);
	}


	cell_ptr make_atom(const std::string &s) {
		auto &map = atom_map;

		auto iter = map.find(s);
		if (iter != map.end()) return iter->second;
		auto tmp = new atom_cell(s);
		map.emplace(s, tmp);
		return tmp;
	}

	symbol_cell_ptr make_symbol(const std::string &s) {
		auto &map = symbol_map;

		auto iter = map.find(s);
		if (iter != map.end()) return iter->second;
		auto tmp = new symbol_cell(s);
		map.emplace(s, tmp);
		return tmp;
	}


	cell_ptr make_integer(int i) {
		if (i >= 0 && i < numbers.size()) return &numbers[i];

		return temp(new integer_cell(i));
	}


	cell_ptr make_pair(cell_ptr a, cell_ptr b) {
		return temp(new pair_cell(a, b));
	}

	cell_ptr make_unary(int op, cell_ptr rhs) {
		return temp(new unary_cell(op, rhs));
	}

	cell_ptr make_binary(int op, cell_ptr lhs, cell_ptr rhs) {
		return temp(new binary_cell(op, lhs, rhs));
	}

	cell_ptr make_conditional(std::vector<cell_ptr> &&pe) {
		return temp(new conditional_cell(std::move(pe)));
	}

	cell_ptr make_lambda(symbol_cell_ptr label, std::vector<symbol_cell_ptr> &&parameters, cell_ptr body) {
		return temp(new lambda_cell(label, std::move(parameters), body));
	}

	cell_ptr make_lambda(symbol_cell_ptr label, const std::vector<symbol_cell_ptr> &parameters, cell_ptr body) {
		return temp(new lambda_cell(label, parameters, body));
	}

	cell_ptr make_function(cell_ptr function, std::vector<cell_ptr> &&parameters) {
		return temp(new function_cell(function, std::move(parameters)));
	}




#pragma mark - to_string

	std::string cell::to_string() const {
		char *tmp = nullptr;
		int size = asprintf(&tmp, "#<cell %p>", this);
		std::string s(tmp, tmp+size);
		free(tmp);
		return s;
	}

	std::string integer_cell::to_string() const {
		return std::to_string(value);
	}

	std::string atom_cell::to_string() const {
		return value;
	}

	std::string symbol_cell::to_string() const {
		return value;
	}

	std::string pair_cell::to_string() const {
		std::string tmp;
		tmp.push_back('(');
		if (car) {
			tmp.append(car->to_string());
		}
		if (cdr) {
			tmp.append(" . ");
			tmp.append(cdr->to_string());
		}
		tmp.push_back(')');
		return tmp;
	}


	std::string builtin_cell::to_string() const {
		char *tmp = nullptr;
		int size = asprintf(&tmp, "#<builtin %s>", name.c_str());
		std::string s(tmp, tmp+size);
		free(tmp);
		return s;
	}


	std::string lambda_cell::to_string() const {
		char *tmp = nullptr;
		int size = asprintf(&tmp, "#<lambda %s>", name ? name->to_string().c_str() : "");
		std::string s(tmp, tmp+size);
		free(tmp);
		return s;
	}

#pragma mark - to_xxx

symbol_cell_ptr cell::to_symbol() {
	throw std::runtime_error("Expected symbol.");
}

symbol_cell_ptr symbol_cell::to_symbol() {
	return this;
}

int cell::to_int() const {
	throw std::runtime_error("Expected integer.");
}

int integer_cell::to_int() const {
	return value;
}

int pair_cell::to_int() const {
	// allow (1) to be treated as an int.
	if (car && !cdr) return car->to_int();
	return cell::to_int();
}


bool cell::to_bool() const {
	throw std::runtime_error("Expected boolean.");
}


bool pair_cell::to_bool() const {
	// allow (1) to be treated as an int.
	if (car && !cdr) return car->to_bool();
	return cell::to_bool();
}

bool atom_cell::to_bool() const {
	if (this == kTRUE) return true;
	if (this == kFALSE) return false;
	return cell::to_bool();
}

cell_ptr cell::to_atom() {
	throw std::runtime_error("Expected atom.");
}

cell_ptr atom_cell::to_atom() {
	return this;
}

cell_ptr pair_cell::to_atom() {
	// allow (TRUE) to be treated as an int.
	if (car && !cdr) return car->to_atom();
	return cell::to_atom();
}

std::pair<cell_ptr, cell_ptr> cell::to_pair() const {
	throw std::runtime_error("Expected pair.");
}


std::pair<cell_ptr, cell_ptr> pair_cell::to_pair() const {
	return std::make_pair(car, cdr);
}



#pragma mark - substitute

cell_ptr symbol_cell::substitute(hideset &hs, const environment &e) {
	if (std::find(hs.begin(), hs.end(), this) != hs.end()) return this;
	auto iter = e.find(this);
	if (iter != e.end()) return iter->second;
	return this;
}

cell_ptr function_cell::substitute(hideset &hs, const environment &e) {

	auto f = function->substitute(hs, e);

	std::vector<cell_ptr> args;
	std::transform(arguments.begin(), arguments.end(), std::back_inserter(args),
		[&](cell_ptr cell) { return cell->substitute(hs, e); }
	);

	if (arguments == args && f == function) return this;
	return make_function(f, std::move(args));
}

cell_ptr lambda_cell::substitute(hideset &hs, const environment &e) {
	auto old_size = hs.size();
	if (name) hs.push_back(name);
	hs.insert(hs.end(), parameters.begin(), parameters.end());
	auto tmp = body->substitute(hs, e);
	hs.resize(old_size);

	if (tmp == body) return this;
	return make_lambda(name, parameters, tmp);
}

cell_ptr conditional_cell::substitute(hideset &hs, const environment &e) {

	std::vector<cell_ptr> tmp;
	std::transform(pe.begin(), pe.end(), std::back_inserter(tmp), [&](cell_ptr cell){
		return cell ? cell->substitute(hs, e) : cell;
	});

	if (pe == tmp) return this;
	return make_conditional(std::move(tmp));
}

cell_ptr pair_cell::substitute(hideset &hs, const environment &e) {
	auto a = car ? car->substitute(hs, e) : car;
	auto b = cdr ? cdr->substitute(hs, e) : cdr;
	if (a == car && b == cdr) return this;
	return make_pair(a, b);
}

cell_ptr binary_cell::substitute(hideset &hs, const environment &e) {
	auto a = car ? car->substitute(hs, e) : car;
	auto b = cdr ? cdr->substitute(hs, e) : cdr;
	if (a == car && b == cdr) return this;
	return make_binary(op, a, b);
}

cell_ptr unary_cell::substitute(hideset &hs, const environment &e) {
	auto a = car ? car->substitute(hs, e) : car;
	if (a == car) return this;
	return make_unary(op, a);
}


#pragma mark - evaluate

cell_ptr unary_cell::evaluate(const environment &e) {
	cell_ptr tmp = car->evaluate(e);
	// null ?
	switch(op) {

		case PLUS:
			return make_integer(tmp->to_int());

		case MINUS:
			return make_integer(-tmp->to_int());

		case NOT:
			return tmp->to_bool() ? kFALSE : kTRUE;
		default:
			throw std::runtime_error("invalid unary operation.");
	}
}

cell_ptr binary_cell::evaluate(const environment &e) {
	switch(op) {
		bool ok;
		case AND:
			ok = car->evaluate(e)->to_bool() && cdr->evaluate(e)->to_bool();
			return ok ? kTRUE: kFALSE;

		case OR:
			ok = car->evaluate(e)->to_bool() || cdr->evaluate(e)->to_bool();
			return ok ? kTRUE: kFALSE;

		case XOR:
			ok = car->evaluate(e)->to_bool() ^ cdr->evaluate(e)->to_bool();
			return ok ? kTRUE: kFALSE;
	}

	int a = car->evaluate(e)->to_int();
	int b = cdr->evaluate(e)->to_int();
	switch(op) {
		case PLUS: return make_integer(a + b);
		case MINUS: return make_integer(a - b);
		case TIMES: return make_integer(a * b);
		case DIVIDE: return make_integer(a / b);
		case EQ: return a == b ? kTRUE : kFALSE;
		case NE: return a != b ? kTRUE : kFALSE;
		case LT: return a < b ? kTRUE : kFALSE;
		case LE: return a <= b ? kTRUE : kFALSE;
		case GT: return a > b ? kTRUE : kFALSE;
		case GE: return a >= b ? kTRUE : kFALSE;
	default:
		throw std::runtime_error("invalid binary operation.");
	}
}


cell_ptr pair_cell::evaluate(const environment &e) {
	auto a = car ? car->evaluate(e) : car;
	auto b = cdr ? cdr->evaluate(e) : cdr;
	if (a == car && b == cdr) return this;
	return make_pair(a, b);
}

cell_ptr symbol_cell::evaluate(const environment &e) {
	auto iter = e.find(this);
	if (iter == e.end()) throw std::runtime_error("free variable - " + value);
	auto tmp = iter->second;
	return tmp->evaluate(); //? does it need e?  any parms should be terminals at this point.
}

cell_ptr conditional_cell::evaluate(const environment &env) {

	for (auto iter = pe.begin(); iter != pe.end(); ) {
		auto p = *iter++;
		auto e = *iter++;

		if (p->evaluate(env)->to_bool()) return e->evaluate(env);
	}
	throw std::runtime_error("condition did not have a true clause.");
}

cell_ptr function_cell::evaluate(const environment &env) {
	auto tmp = function->evaluate(env);

	if (!tmp) throw std::runtime_error("nil function.");

	if (tmp->is<cell::lambda>()) {
		auto f = (lambda_cell *)tmp;

		std::vector<cell_ptr> args;
		std::transform(arguments.begin(), arguments.end(), std::back_inserter(args),
			[&](cell_ptr cell) { return cell->evaluate(env); }
		);

		return f->call(args);
	}

	if (tmp->is<builtin>()) {
		auto f = (builtin_cell *)tmp;

		return f->call(arguments, env);
	}
	throw std::runtime_error("not a function.");
}

cell_ptr lambda_cell::evaluate(const environment &e) {
	hideset hs;
	return substitute(hs, e);
}

cell_ptr lambda_cell::label(symbol_cell_ptr n) {
	if (n == name) return this;
	return make_lambda(n, parameters, body);
}

cell_ptr lambda_cell::call(const std::vector<cell_ptr> &arguments) {
	if (arguments.size() != parameters.size())
		throw std::runtime_error("arity error.");

	environment env;
	if (name) env.emplace(name, this);
	for (int i = 0; i < parameters.size(); ++i) {
		env.emplace(parameters[i], arguments[i]);
	}
	return body->evaluate(env);
}

cell_ptr builtin_cell::call(const std::vector<cell_ptr> &arguments, const environment &e) {
	//if (arguments.size() != arity)
	//	throw std::runtime_error("arity error.");

	return function(arguments, e);
}

#pragma mark - mark

void unary_cell::mark() {
	cell::mark();
	if (car) car->mark();
}

void binary_cell::mark() {
	cell::mark();
	if (car) car->mark();
	if (cdr) cdr->mark();
}

void pair_cell::mark() {
	cell::mark();
	if (car) car->mark();
	if (cdr) cdr->mark();
}

void lambda_cell::mark() {
	cell::mark();
	if (name) name->mark();
	if (body) body->mark();
}

void function_cell::mark() {
	cell::mark();
	if (function) function->mark();
	for (auto x : arguments) { if (x) x->mark(); }
}

void conditional_cell::mark() {
	cell::mark();
	for (auto x : pe) { if (x) x->mark(); }
}


} // namespace
