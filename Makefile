# ========================
# Biến
# ========================
CC = gcc
CFLAGS = -Wall -g -Icommon
LDFLAGS = -lpthread

# Controller
CONTROLLER = controller
CONTROLLER_SRC = controller_app/src

# Devices
DEVICE_SRC = device_simulator/src
COMMON_SRC = common
DEVICES = aerator feeder sensor water_pump pH_regulator

# ========================
# Target mặc định: build tất cả
# ========================
all: $(CONTROLLER) $(DEVICES)

# ========================
# Build controller
# ========================
$(CONTROLLER): $(CONTROLLER_SRC)/main.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# ========================
# Build mỗi device riêng
# ========================
# Pattern: mỗi thiết bị có main riêng -> tạo 1 executable
$(DEVICES):
	$(CC) $(CFLAGS) \
		$(DEVICE_SRC)/device_server.c \
		$(DEVICE_SRC)/device.c \
 		$(DEVICE_SRC)/device/$@_device.c \
		-o $@ $(LDFLAGS)

# ========================
# Xóa file build
# ========================
clean:
	rm -f $(CONTROLLER)
	rm -f $(DEVICES)
	rm -f $(CONTROLLER_SRC)/*.o
	rm -f $(DEVICE_SRC)/*.o
	rm -f $(DEVICE_SRC)/device/*.o

# ========================
# Không phải file
# ========================
.PHONY: all clean
