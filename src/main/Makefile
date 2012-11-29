INCLUDE=-I../include -I../include/rep -I../

LIBS= ../activity/Activity.o ../rep/Instance.o ../engine/Engine.o ../fwk/*.o

CXXFLAGS = -Wall -g $(INCLUDE)

default: test-cases test1 example ourclient experiment verification adaptive

test_client: test_client.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

test1:	test1.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

example:	example.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^
ourclient:	ourclient.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

test-cases: test-cases.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

experiment: experiment.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

verification: verification.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

adaptive: adaptive.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f test1 test1.o example test-cases test-cases.o example.o ourclient ourclient.o adaptive.o apative experiment experiment.o *~

example.o: example.cpp
test1.o: test1.cpp 
ourclient.o: ourclient.cpp
runtest:
	./test_driver.pl test_manifest

