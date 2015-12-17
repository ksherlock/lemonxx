CXX = clang++
CXXFLAGS = -g -std=c++14

all : any expr

any : any.cpp 
expr: expr.cpp

clean :
	$(RM) any any.cpp any.h expr expr.cpp expr.h

any.cpp : any.lemon
	../lemon++ nconflicts=2 $<

%.cpp : %.lemon
	../lemon++ $(LEMONFLAGS) $<
