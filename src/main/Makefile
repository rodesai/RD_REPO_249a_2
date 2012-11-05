INCLUDE=-I../include -I../

LIBS=../rep/Instance.o ../engine/Engine.o ../fwk/*.o

CXXFLAGS = -Wall -g $(INCLUDE)

default: test1 example

test1:	test1.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

example:	example.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f test1 test1.o example example.o *~

example.o: example.cpp
test1.o: test1.cpp 
