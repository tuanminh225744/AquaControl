CONTROLLER = controller
DEVICE = device

CONTROLLER_SRC = controller_app/src
DEVICE_SRC = device_simulator/src
COMMON_INC = common

CC = gcc
CFLAGS = -Wall -g -I$(COMMON_INC)

all: $(CONTROLLER) $(DEVICE)

$(CONTROLLER): $(CONTROLLER_SRC)/main.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread

$(DEVICE): $(DEVICE_SRC)/main.c $(DEVICE_SRC)/device.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

clean:
	rm -f $(CONTROLLER) $(DEVICE)
	rm -f $(CONTROLLER_SRC)/*.o
	rm -f $(DEVICE_SRC)/*.o

.PHONY: all clean