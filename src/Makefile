CXXFLAGS = -Wall -g

OBJECTS = Instance.o

default:	test1 example

test1:	test1.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

example:	example.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f test1 test1.o $(OBJECTS) *~

Instance.o: Instance.cpp 
test1.o: test1.cpp 
