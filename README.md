# lemon--

An experimental fork of the
[lemon parser generator](http://www.hwaci.com/sw/lemon/lemon.html)
that hopes to be more compatible with c++11.  Placement constructors and
destructors are automatically generated, so you shouldn't need to deal with
pointers as much (unless, of course, you want to!)


## contrived example


###lemon--
	%token_type {int}
    %type number_list{std::vector<int>}
    // or use a std::shared_ptr or std::unique_ptr...

    program ::= number_list(L). {

    	for (auto i : L) {
    		printf("%d\n", i);
    	}
    }
    number_list(LHS) ::= number_list(L) NUMBER(N). {
    	LHS = std::move(L);
    	LHS.push_back(N);
    }
    number_list(LHS) ::= NUMBER(N). {
    	LHS.push_back(N);
    }

The left-hand-side variable (`LHS`) will be default constructed before the code
is executed.  The right-hand-side variables (`L`, and `N`) will be destructed 
after the code is executed. (Lemon zero-initializes the left-hand side memory
and  will only call the %destructor on the right-hand side for tokens that
aren't referenced).


###lemon

	%token_type {int}
    %type number_list{std::vector<int> *}
    %destructor number_list { delete $$; }

    program ::= number_list(L). {

    	for (auto i : *L) {
    		printf("%d\n", i);
    	}
    	delete L;
    }
    number_list(LHS) ::= number_list(L) NUMBER(N). {
    	LHS = L;
    	LHS->push_back(N);
    }
    number_list(LHS) ::= NUMBER(N). {
    	LHS = new std::vector<int>();
    	LHS->push_back(N);
    }


Ok, the lemon-- example isn't much of an improvement here.  But it might
be an improvement for more complicated scenarios!
