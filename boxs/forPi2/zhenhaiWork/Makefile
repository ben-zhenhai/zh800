
MACHINE	= m3050

CFLAGS	= -O2 -D$(MACHINE) -DLOCALTEST
LDFLAGS	= -lm -lpthread -lwiringPi -lcurl

SRC		= main.c standerInput.c eeprom.c ftp.c lock.c lcd.c $(MACHINE)/$(MACHINE).c
OBJ		= $(SRC:%.c=%.o)
EXE		= binary

.PHONY: all clean

$(EXE): $(OBJ)
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $@

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXE)

clean:
	@rm -rf $(OBJ) *.o $(EXE)
