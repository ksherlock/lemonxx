CXX = clang++
CXXFLAGS = -g -std=c++11


any : any.cpp 


clean :
	$(RM) any any.cpp

any.cpp : any.lemon
	../lemon nconflicts=2 $<

%.cpp : %.lemon
	../lemon $(LEMONFLAGS) $<
