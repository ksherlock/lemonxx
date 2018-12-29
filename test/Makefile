CXX = clang++
CXXFLAGS = -g -std=c++14
CCFLAGS = -g

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

error-1.o : error.c
	$(CC) $(CCFLAGS) -c $< -o $@

error-2.o : error.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

error.cpp : error.lemon ../lemon++ ../lempar.cxx 
	../lemon++ -T../lempar.cxx $(LEMONFLAGS) $<


% : %.o
	$(CXX) $(LDFLAGS) $^ -o $@
	#$(RM) -- $^

#%.o : %.cpp
#	$(CXX) $(CXXFLAGS) -c $< -o $@

any.cpp : any.lemon ../lemon++ ../lempar.cpp
	../lemon++ -T../lempar.cpp nconflicts=2 $<

lisp.cpp : lisp.lemon lisp_main.h lisp_cell.h ../lemon++ ../lempar.cxx 
	../lemon++ -T../lempar.cxx $<

%.cpp : %.lemon ../lemon++ ../lempar.cpp 
	../lemon++ -T../lempar.cpp $(LEMONFLAGS) $<

%.c : %.lemon ../lemon ../lempar.c 
	../lemon -T../lempar.c $(LEMONFLAGS) $<

# %.cxx : %.lemon ../lemon++ ../lempar.cxx 
# 	../lemon++ -T../lempar.cxx $(LEMONFLAGS) $<

%.cpp : %.ragel
	ragel -p -T1 $< -o $@