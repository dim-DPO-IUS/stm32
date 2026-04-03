################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := HW 4_2 Boot
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
HW\ 4_2\ Boot.elf \

MAP_FILES += \
HW\ 4_2\ Boot.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
HW\ 4_2\ Boot.list \

OBJCOPY_HEX += \
HW\ 4_2\ Boot.hex \

OBJCOPY_BIN += \
HW\ 4_2\ Boot.bin \


# All Target
all: main-build

# Main-build Target
main-build: HW\ 4_2\ Boot.elf secondary-outputs

# Tool invocations
HW\ 4_2\ Boot.elf HW\ 4_2\ Boot.map: $(OBJS) $(USER_OBJS) /mnt/DATA-1/0Dev/MC-Dev/0-LESSONS/01/src/work4dak/stm32/HW\ 4_2\ Boot/STM32F411RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "HW 4_2 Boot.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/mnt/DATA-1/0Dev/MC-Dev/0-LESSONS/01/src/work4dak/stm32/HW 4_2 Boot/STM32F411RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="HW 4_2 Boot.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

HW\ 4_2\ Boot.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "HW 4_2 Boot.list"
	@echo 'Finished building: $@'
	@echo ' '

HW\ 4_2\ Boot.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "HW 4_2 Boot.hex"
	@echo 'Finished building: $@'
	@echo ' '

HW\ 4_2\ Boot.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "HW 4_2 Boot.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) HW\ 4_2\ Boot.bin HW\ 4_2\ Boot.elf HW\ 4_2\ Boot.hex HW\ 4_2\ Boot.list HW\ 4_2\ Boot.map HW\\ 4_2\\ Boot.elf default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
