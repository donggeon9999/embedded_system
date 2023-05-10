default: main.o device.o io.o sema.o
	arm-none-linux-gnueabi-gcc -static -Wall -lm -o hw1_20181664.out main.o device.o io.o sema.o 
	@echo ">>> BUILD COMPLETE (hw1_20181664.out)"
	@echo ">>> Type 'adb push ./hw1_20181664.out /data/local/tmp' to push to ARM board."
	@echo ">>> Or do 'make push' to build and push."



main.o: main.h device.h io.h sema.h main.c
	arm-none-linux-gnueabi-gcc -static -Wall -lm -c main.c

device.o: device.c
	arm-none-linux-gnueabi-gcc -static -Wall -lm -c device.c
io.o: io.h sema.h device.h io.c
	arm-none-linux-gnueabi-gcc -static -Wall -lm -c io.c
sema.o: sema.c
	arm-none-linux-gnueabi-gcc -static -Wall -lm -c sema.c


clean:
	-rm *.o
	-rm *.out
	@echo ">>> Cleaned up project"
