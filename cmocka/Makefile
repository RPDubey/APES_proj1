tests: tests.c tmp102Sensor.o adps9301Sensor.o i2cWrapper.o client_test.o
	gcc -o $@ $^ -lcmocka -lm -lrt -pthread

tmp102Sensor.o:../sensors/tmp102Sensor.c ../sensors/i2cWrapper.c
	gcc -c -o $@ $< -lcmocka

adps9301Sensor.o:../sensors/adps9301Sensor.c ../sensors/i2cWrapper.o
	gcc -c -o $@ $< -lcmocka -lm

i2cWrapper.o:../sensors/i2cWrapper.c
	gcc -c -o $@ $< -lcmocka

client_test.o:../socket_client/client_test.c
	gcc -c -o client_test.o ../socket_client/client_test.c -lrt -pthread


#client_test.o:client_test.c
#	gcc -c -o client_test.o client_test.c -lrt -pthread


clean:
	-rm -f *.o tests
