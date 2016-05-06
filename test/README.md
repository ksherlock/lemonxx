# lisp

An implementation of lisp, as originally intended. (_i.e._, m-expressions).
Demonstrates an object-oriented parser (`lempar.cxx`, `lemon_base.h`)

    x: (1, 2, 3, 4)       # a list
    cadr[x] : car[cdr[x]] # function definition
    y: 1+2*3+4            # math

    car[x]                # call a function
    cdr[x]
    cadr[x]


# intbasic

Apple Integer Basic. Accepts or rejects but doesn't build a parse tree.

# expr

Converts infix expressions on the command-line to lisp. Uses smart pointers.

# any

A very early test. Tokens have a deleted copy constructor/operator= to verify
move assignment works.

