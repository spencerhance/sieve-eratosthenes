all:
	gcc sieve.c -o sieve -lpthread -lm
clean:
	rm sieve
