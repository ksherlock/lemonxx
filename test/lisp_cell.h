#ifndef __lisp_cell_h__
#define __lisp_cell_h__


#include <new>
#include <string>
#include <vector>
#include <unordered_map>

namespace lisp {

	typedef class cell * cell_ptr;
	typedef class symbol_cell * symbol_cell_ptr;
	typedef std::vector<symbol_cell_ptr> hideset;

	typedef std::unordered_map<symbol_cell_ptr, cell_ptr> environment;

	extern environment env;

	void initialize(std::size_t);
	void clear_temp_list();

	class cell {
	public:
		enum type {
			undefined,
			atom,
			symbol,
			integer,
			builtin,
			lambda,
			function, // call a function.
			pair,
			unary,
			binary,
			conditional,
		};



		static void *operator new(std::size_t);
		static void operator delete(void *);



		cell_ptr substitute(const environment &e) {
			hideset hs;
			return substitute(hs, e);
		}

		cell_ptr evaluate() {
			environment e;
			return evaluate(e);
		}

		virtual std::string to_string() const;
		virtual cell_ptr substitute(hideset &hs, const environment &) { return this; }
		virtual cell_ptr evaluate(const environment &) { return this; }
		virtual cell_ptr label(symbol_cell_ptr) { return this; }

		virtual int to_int() const;
		virtual bool to_bool() const;
		virtual symbol_cell_ptr to_symbol();
		virtual cell_ptr to_atom();
		virtual std::pair<cell_ptr, cell_ptr> to_pair() const;

		//virtual bool to_bool() const;


		template<type T>
		bool is() const { return type == T; }

	protected:
		cell(type t) : type(t)
		{};
		virtual ~cell()
		{}

		friend class unary_cell;
		friend class binary_cell;
		friend class lambda_cell;
		friend class pair_cell;
		friend class conditional_cell;
		friend class function_cell;
		virtual void mark() { alive = true; }

	private:
		bool alive = false;
		type type = undefined;

		friend void garbage_collect();

	};


	class atom_cell : public cell {

	public:

		atom_cell(const std::string &s) : cell(cell::atom), value(s)
		{ }

		virtual std::string to_string() const override final;
		virtual cell_ptr to_atom() override final;
		virtual bool to_bool() const override final;

	private:
		std::string value;
	};


	class symbol_cell : public cell {

	public:

		symbol_cell(const std::string &s) : cell(cell::symbol), value(s)
		{}

		virtual std::string to_string() const override final;
		virtual cell_ptr substitute(hideset &hs, const environment &e) override final;
		virtual cell_ptr evaluate(const environment &e) override final;

		virtual symbol_cell_ptr to_symbol() override final;

	private:
		std::string value;
	};


	class integer_cell : public cell {

	public:

		integer_cell(int i = 0) : cell(cell::integer), value(i)
		{}

		virtual std::string to_string() const override final;
		virtual int to_int() const override final;

	private:
		int value;

	};

	class pair_cell : public cell {

	public:

		pair_cell(cell_ptr a, cell_ptr b) : cell(cell::pair), car(a), cdr(b)
		{}

		virtual std::string to_string() const override final;
		virtual cell_ptr substitute(hideset &hs, const environment &e) override final;
		virtual cell_ptr evaluate(const environment &e) override final;

		virtual int to_int() const override final;
		virtual bool to_bool() const override final;
		virtual cell_ptr to_atom() override final;

		virtual std::pair<cell_ptr, cell_ptr> to_pair() const override final;
	protected:
		virtual void mark() override final;


	private:
		cell_ptr car;
		cell_ptr cdr;
	};

	class unary_cell : public cell {
	public:
		unary_cell(int op, cell_ptr rhs) : cell(cell::unary), op(op), car(rhs)
		{}

		virtual cell_ptr substitute(hideset &hs, const environment &e) override final;
		virtual cell_ptr evaluate(const environment &e) override final;

	protected:
		virtual void mark() override final;

	private:
		int op;
		cell_ptr car;
	};

	class binary_cell : public cell {
	public:
		binary_cell(int op, cell_ptr lhs, cell_ptr rhs) : cell(cell::binary), op(op), car(lhs), cdr(rhs)
		{}

		virtual cell_ptr substitute(hideset &hs, const environment &e) override final;
		virtual cell_ptr evaluate(const environment &e) override final;

	protected:
		virtual void mark() override final;

	private:
		int op;
		cell_ptr car;
		cell_ptr cdr;
	};


	class conditional_cell : public cell {
	public:
		conditional_cell(std::vector<cell_ptr> &&pe) : cell(cell::conditional), pe(std::move(pe))
		{}

		virtual cell_ptr substitute(hideset &hs, const environment &e) override final;
		virtual cell_ptr evaluate(const environment &e) override final;

	protected:
		virtual void mark() override final;

	private:
		std::vector<cell_ptr> pe;

	};

	class lambda_cell : public cell {

	public:
		lambda_cell(symbol_cell_ptr name, std::vector<symbol_cell_ptr> &&parameters, cell_ptr body)
			: cell(cell::lambda),
			name(name),
			parameters(std::move(parameters)),
			body(body)
		{}

		lambda_cell(symbol_cell_ptr name, const std::vector<symbol_cell_ptr> &parameters, cell_ptr body)
			: cell(cell::lambda),
			name(name),
			parameters(std::move(parameters)),
			body(body)
		{}

		virtual std::string to_string() const override final;
		virtual cell_ptr substitute(hideset &hs, const environment &e) override final;
		virtual cell_ptr evaluate(const environment &e) override final;
		virtual cell_ptr label(symbol_cell_ptr) override final;

	protected:
		virtual void mark() override final;

	protected:

		cell_ptr call(const std::vector<cell_ptr> &args);

	private:

		friend class function_cell;

		std::vector<symbol_cell_ptr> parameters;
		symbol_cell_ptr name;
		cell_ptr body;

	};

	class builtin_cell : public cell {
	public:
		typedef cell_ptr (*function_proto)(const std::vector<cell_ptr> &, const environment &e);

		builtin_cell(std::string name, int arity, function_proto function) :
			cell(cell::builtin), name(name), arity(arity), function(function)
		{}

		virtual std::string to_string() const override final;

	protected:
		cell_ptr call(const std::vector<cell_ptr> &args, const environment &e);

	private:
		friend class function_cell;

		std::string name;
		int arity;
		function_proto function;
	};

	class function_cell : public cell {

	public:
		function_cell(cell_ptr function, std::vector<cell_ptr> &&arguments) :
			cell(cell::function),
			function(function),
			arguments(std::move(arguments))
		{}

		virtual cell_ptr substitute(hideset &hs, const environment &e) override final;
		virtual cell_ptr evaluate(const environment &e) override final;

	protected:
		virtual void mark() override final;

	private:
		cell_ptr function; // lambda or built-in
		std::vector<cell_ptr> arguments;
	};




	cell_ptr make_pair(cell_ptr a, cell_ptr b);
	cell_ptr make_atom(const std::string &s);
	symbol_cell_ptr make_symbol(const std::string &s);
	cell_ptr make_integer(int i);
	cell_ptr make_unary(int op, cell_ptr rhs);
	cell_ptr make_binary(int op, cell_ptr lhs, cell_ptr rhs);
	cell_ptr make_conditional(std::vector<cell_ptr> &&pe);
	cell_ptr make_lambda(symbol_cell_ptr label, std::vector<symbol_cell_ptr> &&parameters, cell_ptr body);
	cell_ptr make_lambda(symbol_cell_ptr label, const std::vector<symbol_cell_ptr> &parameters, cell_ptr body);
	cell_ptr make_function(cell_ptr function, std::vector<cell_ptr> &&parameters);


} // namespace

#endif
