CFLAGS = \
	-Wall -Wextra -Werror

INC_PATH = \
	-I wiringPi/wiringPi

LIB_PATH = \
	-L wiringPi/wiringPi


all: wiringPi/wiringPi/libwiringPi.so
	gcc -DRASPI_GPIO ${CFLAGS} ${INC_PATH} ${LIB_PATH} -o raspi raspi.c -lwiringPi -lpthread


standalone:
	gcc ${CFLAGS} ${INC_PATH} ${LIB_PATH} -o raspi raspi.c


wiringPi/wiringPi/libwiringPi.so: wiringPi/wiringPi/libwiringPi.so.2.25
	ln -s libwiringPi.so.2.25 wiringPi/wiringPi/libwiringPi.so
