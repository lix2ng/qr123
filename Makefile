all: qr

CC = gcc
CFLAGS = -pipe -Wall -Wpadded -std=gnu99 -Os -DUT_QR123

OBJS = qr123.o
qr123.o: qr123.c qr123.h

.c.o:
	$(CC) -c $(CFLAGS) $<

qr:	$(OBJS)
	$(CC) -o $@ $^ 

clean:
	@rm -f qr qr.exe $(OBJS)

# vim: set syn=make noet ts=8 tw=80:
