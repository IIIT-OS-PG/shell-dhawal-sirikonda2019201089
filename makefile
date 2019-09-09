CPP = g++  #compiler
CFLAGS = -Wall -g
TARGET = /common/dhsh #target file name
 
all:
	$(CPP) ${CFLAGS} main.cpp dhsh.cpp trie.cpp -o $(TARGET)
 
clean:
	rm $(TARGET)
