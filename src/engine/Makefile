OBJECTS = Engine.o

INCLUDE = -I../include/ -I../

CXXFLAGS = -Wall -g $(INCLUDE)

default: $(OBJECTS)	

clean:
	rm -f $(OBJECTS) *~

Instance.o: Engine.cpp 

runtest:
	cd test && gmake clean && gmake && gmake main && gmake test
