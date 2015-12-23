CXX = clang++
CXXFLAGS = -g -std=c++14

TARGETS = any expr intbasic

#.SUFFIXES:

# prevent intermediate files from being deleted.
.SECONDARY:


all : $(TARGETS)

clean :
	$(RM) -- $(TARGETS) $(TARGETS:=.cpp) $(TARGETS:=.h) $(TARGETS:=.out) intbasic_lexer.cpp


intbasic : intbasic.cpp intbasic_lexer.cpp
intbasic_lexer.cpp : intbasic_lexer.ragel intbasic.cpp

% : %.o
	$(CXX) $^ -o $@
	$(RM) -- $^

#%.o : %.cpp
#	$(CXX) $(CXXFLAGS) $< -o $@

any.cpp : any.lemon
	../lemon++ nconflicts=2 $<


%.cpp : %.lemon
	../lemon++ $(LEMONFLAGS) $<


%.cpp : %.ragel
	ragel -p -T1 $< -o $@