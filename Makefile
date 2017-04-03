
virtmem: main.o page_table.o disk.o program.o
	gcc main.o page_table.o disk.o program.o -o virtmem

main.o: src/main.c
	gcc -Wall -g -c src/main.c -o main.o

page_table.o: src/page_table.c
	gcc -Wall -g -c src/page_table.c -o page_table.o

disk.o: src/disk.c
	gcc -Wall -g -c src/disk.c -o disk.o

program.o: src/program.c
	gcc -Wall -g -c src/program.c -o program.o


clean:
	rm -f *.o virtmem
