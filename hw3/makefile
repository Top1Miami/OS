CXXFLAGS+= -fsanitize=address -Wall
CXX=gcc

all: hw3.o 
	
run: hw3.o
	./hw3.o

hw3.o: hw3.c 
	$(CXX) $^ $(CXXFLAGS) -o hw3.o

clean:
	rm -f *.o