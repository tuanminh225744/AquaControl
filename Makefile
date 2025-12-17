# ========================
# Biến
# ========================
CC = gcc
CFLAGS = -Wall -g -Icommon
LDFLAGS = -lpthread

# Controller
CONTROLLER = controller
CONTROLLER_SRC = controller_app/src
CONTROLLER_OBJS = $(CONTROLLER_SRC)/controller.c $(CONTROLLER_SRC)/set_device.c $(CONTROLLER_SRC)/get_device_info.c $(CONTROLLER_SRC)/manual_device_control.c $(CONTROLLER_SRC)/turn_on_off.c $(COMMON_SRC)/network_utils.c

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
$(CONTROLLER): $(CONTROLLER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# ========================
# Build mỗi device riêng
# ========================
# Pattern: mỗi thiết bị có main riêng -> tạo 1 executable
$(DEVICES):
	$(CC) $(CFLAGS) $(DEVICE_SRC)/device_server.c $(DEVICE_SRC)/device.c $(DEVICE_SRC)/device/$@_device.c $(COMMON_SRC)/network_utils.c -o $@ $(LDFLAGS)

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
