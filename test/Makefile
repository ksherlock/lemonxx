CXX = clang++
CXXFLAGS = -g -std=c++14

TARGETS = any expr intbasic lisp

#.SUFFIXES:

# prevent intermediate files from being deleted.
.SECONDARY:


all : $(TARGETS)

clean :
	$(RM) -- $(TARGETS) $(TARGETS:=.cpp) $(TARGETS:=.h) $(TARGETS:=.out) \
	intbasic_lexer.cpp lisp_main.cpp lisp_main.o


intbasic : intbasic.cpp intbasic_lexer.cpp
intbasic_lexer.cpp : intbasic_lexer.ragel intbasic.cpp

lisp : lisp.o lisp_main.o lisp_cell.o
lisp_cell.o : lisp_cell.cpp lisp_cell.h lisp.h
lisp_main.o : lisp_main.cpp lisp_cell.h lisp.h

% : %.o
	$(CXX) $(LDFLAGS) $^ -o $@
	#$(RM) -- $^

#%.o : %.cpp
#	$(CXX) $(CXXFLAGS) -c $< -o $@

any.cpp : any.lemon
	../lemon++ nconflicts=2 $<

lisp.cpp lisp.h : lisp.lemon lisp_main.h lisp_cell.h lempar.cxx
	../lemon++ -Tlempar.cxx $<

%.cpp : %.lemon
	../lemon++ $(LEMONFLAGS) $<


%.cpp : %.ragel
	ragel -p -T1 $< -o $@