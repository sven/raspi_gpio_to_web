CFLAGS = -Wall -Wextra -Werror

all:
	${CC} ${CFLAGS} -o raspi raspi.c -lwiringPi
