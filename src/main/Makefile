INCLUDE=-I../include -I../include/rep -I../

LIBS= ../activity/Activity.o ../rep/Instance.o ../engine/Engine.o ../fwk/*.o

CXXFLAGS = -Wall -g $(INCLUDE)

default: test1 example ourclient

test1:	test1.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

example:	example.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^
ourclient:	ourclient.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f test1 test1.o example example.o ourclient ourclient.o *~

example.o: example.cpp
test1.o: test1.cpp 
ourclient.o: ourclient.cpp
runtest:
	./test_driver.pl test_manifest

