.default: all

all: dining-philosopher

clean:
	rm -f dining-philosopher *.o

dining-philosopher: DiningPhilosopher.o
	gcc -g -pthread -Wall -Werror -O -o $@ $^
%.o: %.c
	gcc -g -pthread -Wall -Werror -O -c $^