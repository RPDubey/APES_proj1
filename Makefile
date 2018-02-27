main.elf:main.c lightTask.o tempTask.o signals.o logtask.o
	gcc -o $@ $^ -lpthread -lrt

signals.o:signals.c
	gcc -c -o $@ $<

logTask.o:logTask.c
	gcc -c -o $@ $<

lightTask.o:lightTask.c
	gcc -c -o $@ $<  -lpthread -lrt

tempTask.o:tempTask.c
	gcc -c -o $@ $<  -lpthread -lrt

clean:
	rm -f *.o *.elf
