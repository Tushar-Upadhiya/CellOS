# ---- Toolchain ----
CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# ---- Target chip flags ----
MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

# ---- Project files ----
SRC_C   = cos_vector.c main.c
SRC_ASM = startup.s
LDSCRIPT = my_linker.ld

OBJ = $(SRC_C:.c=.o) $(SRC_ASM:.s=.o)

TARGET = cellos

# ---- Build rules ----
all: $(TARGET).bin

%.o: %.c
	$(CC) $(MCU_FLAGS) -c $< -o $@

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