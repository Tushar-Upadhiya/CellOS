CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -g

INC_DIR     = Include
SRC_DIR     = src
DRIVER_DIR  = drivers
STARTUP_DIR = startup

SRC_C   = $(SRC_DIR)/main.c \
          $(SRC_DIR)/systick.c \
          $(SRC_DIR)/task.c \
          $(SRC_DIR)/mutex.c \
          $(DRIVER_DIR)/uart.c \
		  $(DRIVER_DIR)/i2c.c \
          $(STARTUP_DIR)/cos_vector.c

SRC_ASM = $(STARTUP_DIR)/startup.s \
          $(STARTUP_DIR)/context_switch.s

LDSCRIPT = my_linker.ld
INCLUDES = -I$(INC_DIR)

OBJ = $(notdir $(SRC_C:.c=.o)) $(notdir $(SRC_ASM:.s=.o))

VPATH = $(SRC_DIR):$(DRIVER_DIR):$(STARTUP_DIR)

TARGET = cellos

all: $(TARGET).bin

%.o: %.c
	$(CC) $(MCU_FLAGS) $(INCLUDES) -c $< -o $@

%.o: %.s
	$(AS) $(MCU_FLAGS) -c $< -o $@

$(TARGET).elf: $(OBJ)
	$(CC) $(MCU_FLAGS) -T $(LDSCRIPT) -nostdlib $(OBJ) -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

clean:
	rm -f *.o $(TARGET).elf $(TARGET).bin

.PHONY: all flash clean