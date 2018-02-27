main.elf:main.c thread1.o thread2.o signals.o
	gcc -o $@ $^ -lpthread -lrt

signals.o:signals.c
	gcc -c -o $@ $<

thread1.o:thread1.c
	gcc -c -o $@ $<  -lpthread -lrt

thread2.o:thread2.c
	gcc -c -o $@ $<  -lpthread -lrt

clean:
	rm -f *.o *.elf
