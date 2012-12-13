SRC= simulateur.c
DEP= ${SRC:.c=.dep}
OBJ= ${SRC:.c=.o}
CXX	 = gcc
LFLAGS   = -lm 
CXXFLAGS = -Wall -Wextra -Wshadow -Wunreachable-code -Winline -O0 -g $(pkg-config --cflags --libs gtk+-3.0)


all: depend exe Makefile

exe: $(OBJ) $(HDR) 
	${CXX} $(CXXFLAGS) -o $@ $(OBJ) $(LFLAGS)  $(LIB)

clean: 
	rm -f $(OBJ) $(DEP)

destroy: clean
	rm -f exe 

depend: ${DEP}

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $<

%.dep: %.c
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

-include ${DEP}
