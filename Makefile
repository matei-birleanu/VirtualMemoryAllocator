CC=gcc
CFLAGS=-Wall -Wextra -std=c99


OBJ += main.o vma.o

%.o: %.c
	$(CC) -g -c -o $@ $< $(CFLAGS) -lm
	
build: $(OBJ) 	
	$(CC) -g -o vma $^ $(CFLAGS) -lm
	
vma: build

.PHONY: clean

run_vma:
	./vma
	
clean:
	rm -f *.o vma	
			
