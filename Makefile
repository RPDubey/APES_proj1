#*******************************************************************************
   #Filename:Makefile
   #Brief:
   #Author:Ravi Dubey
   #Date:3/14/2018
 #*****************************************************************************/

main.elf:main.c lightTask.o tempTask.o signals.o logTask.o notification.o socketTask.o  i2cWrapper.o tmp102Sensor.o adps9301Sensor.o
	gcc -g -o $@ $^ -lpthread -lrt -lm

signals.o:signals.c
	gcc -c -o $@ $<

logTask.o:logTask.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread

lightTask.o:lightTask.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread -lrt

tempTask.o:tempTask.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread -lrt

notification.o:notification.c messageQue.h includes.h
	gcc -c -o $@ $<  -lpthread -lrt

socketTask.o:socketTask.c includes.h
	gcc -c -o $@ $<  -lpthread

tmp102Sensor.o:./sensors/tmp102Sensor.c ./sensors/i2cWrapper.c
	gcc -c -o $@ $<

adps9301Sensor.o:./sensors/adps9301Sensor.c ./sensors/i2cWrapper.o
	gcc -c -o $@ $< -lm

i2cWrapper.o:./sensors/i2cWrapper.c
	gcc -c -o $@ $<

clean:
	rm -f *.o *.elf logfile.txt ./sensors/*.o ./socket_client/*.o ./cmocka/*.o
	rm -f ./socket_client/*.elf ./cmocka/tests
