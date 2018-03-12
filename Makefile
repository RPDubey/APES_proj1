

main.elf:main.c lightTask.o tempTask.o signals.o logTask.o errorhandling.o socketTask.o includes.h
	gcc -g -o $@ $^ -lpthread -lrt

signals.o:signals.c
	gcc -c -o $@ $<

logTask.o:logTask.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread

lightTask.o:lightTask.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread -lrt

tempTask.o:tempTask.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread -lrt

errorhandling.o:errorhandling.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread -lrt

socketTask.o:socketTask.c includes.h
	gcc -c -o $@ $<  -lpthread

clean:
	rm -f *.o *.elf logfile.txt
