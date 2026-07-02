CC = sdcc
CFLAGS = -mstm8 --opt-code-size --std-c23
LDFLAGS = -mstm8 -lstm8 --verbose
BUILD_DIR = build

TARGET = rvswd

SRCS = $(wildcard *.c)
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.rel)))

.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).ihx

$(BUILD_DIR)/$(TARGET).ihx: $(OBJS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.rel: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(BUILD_DIR)/$(TARGET).ihx
	stm8flash -c stlinkv2 -p stm8s103f3 -w $(BUILD_DIR)/$(TARGET).ihx

clean:
	rm -f $(BUILD_DIR)/*.{ihx,asm,lst,rel,sym,map,lk,cdb,rst}
