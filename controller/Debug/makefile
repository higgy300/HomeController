################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.5.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./AsciiLib.obj" \
"./ClockSys.obj" \
"./LCDLib.obj" \
"./_data_pack_.obj" \
"./adc14.obj" \
"./aes256.obj" \
"./bme280.obj" \
"./bme280_support.obj" \
"./comp_e.obj" \
"./cpu.obj" \
"./crc32.obj" \
"./cs.obj" \
"./demo_sysctl.obj" \
"./dma.obj" \
"./flash.obj" \
"./fpu.obj" \
"./gpio.obj" \
"./i2c.obj" \
"./i2c_driver.obj" \
"./interrupt.obj" \
"./main.obj" \
"./mpu.obj" \
"./pcm.obj" \
"./pmap.obj" \
"./pss.obj" \
"./ref_a.obj" \
"./reset.obj" \
"./rtc_c.obj" \
"./spi.obj" \
"./sysctl.obj" \
"./system_msp432p401r.obj" \
"./systick.obj" \
"./timer32.obj" \
"./timer_a.obj" \
"./uart.obj" \
"./uart_driver.obj" \
"./wdt_a.obj" \
"./WiFi/board.obj" \
"./WiFi/cc3100_usage.obj" \
"./WiFi/device.obj" \
"./WiFi/driver.obj" \
"./WiFi/flowcont.obj" \
"./WiFi/fs.obj" \
"./WiFi/netapp.obj" \
"./WiFi/netcfg.obj" \
"./WiFi/nonos.obj" \
"./WiFi/socket.obj" \
"./WiFi/spawn.obj" \
"./WiFi/spi_cc3100.obj" \
"./WiFi/wlan.obj" \
"./ccs/startup_msp432p401r_ccs.obj" \
"../msp432p401r.cmd" \
$(GEN_CMDS__FLAG) \
-l"ti/display/lib/display.aem4f" \
-l"ti/grlib/lib/ccs/m4f/grlib.a" \
-l"third_party/spiffs/lib/ccs/m4f/spiffs.a" \
-l"ti/drivers/lib/drivers_msp432p401x.aem4f" \
-l"third_party/fatfs/lib/ccs/m4f/fatfs.a" \
-l"ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include WiFi/subdir_vars.mk
-include ccs/subdir_vars.mk
-include subdir_rules.mk
-include WiFi/subdir_rules.mk
-include ccs/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
hub.out \

EXE_OUTPUTS__QUOTED += \
"hub.out" \

BIN_OUTPUTS += \
hub.hex \

BIN_OUTPUTS__QUOTED += \
"hub.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "hub.out"

# Tool invocations
hub.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.5.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on -z -m"hub.map" --stack_size=512 -i"C:/ti/simplelink_msp432p4_sdk_2_40_00_10/source" -i"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.5.LTS/lib" --diag_wrap=off --display_error_number --warn_sections --xml_link_info="hub_linkInfo.xml" --rom_model -o "hub.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

hub.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.5.LTS/bin/armhex"  -o "hub.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "AsciiLib.obj" "ClockSys.obj" "LCDLib.obj" "_data_pack_.obj" "adc14.obj" "aes256.obj" "bme280.obj" "bme280_support.obj" "comp_e.obj" "cpu.obj" "crc32.obj" "cs.obj" "demo_sysctl.obj" "dma.obj" "flash.obj" "fpu.obj" "gpio.obj" "i2c.obj" "i2c_driver.obj" "interrupt.obj" "main.obj" "mpu.obj" "pcm.obj" "pmap.obj" "pss.obj" "ref_a.obj" "reset.obj" "rtc_c.obj" "spi.obj" "sysctl.obj" "system_msp432p401r.obj" "systick.obj" "timer32.obj" "timer_a.obj" "uart.obj" "uart_driver.obj" "wdt_a.obj" "WiFi\board.obj" "WiFi\cc3100_usage.obj" "WiFi\device.obj" "WiFi\driver.obj" "WiFi\flowcont.obj" "WiFi\fs.obj" "WiFi\netapp.obj" "WiFi\netcfg.obj" "WiFi\nonos.obj" "WiFi\socket.obj" "WiFi\spawn.obj" "WiFi\spi_cc3100.obj" "WiFi\wlan.obj" "ccs\startup_msp432p401r_ccs.obj" 
	-$(RM) "AsciiLib.d" "ClockSys.d" "LCDLib.d" "_data_pack_.d" "adc14.d" "aes256.d" "bme280.d" "bme280_support.d" "comp_e.d" "cpu.d" "crc32.d" "cs.d" "demo_sysctl.d" "dma.d" "flash.d" "fpu.d" "gpio.d" "i2c.d" "i2c_driver.d" "interrupt.d" "main.d" "mpu.d" "pcm.d" "pmap.d" "pss.d" "ref_a.d" "reset.d" "rtc_c.d" "spi.d" "sysctl.d" "system_msp432p401r.d" "systick.d" "timer32.d" "timer_a.d" "uart.d" "uart_driver.d" "wdt_a.d" "WiFi\board.d" "WiFi\cc3100_usage.d" "WiFi\device.d" "WiFi\driver.d" "WiFi\flowcont.d" "WiFi\fs.d" "WiFi\netapp.d" "WiFi\netcfg.d" "WiFi\nonos.d" "WiFi\socket.d" "WiFi\spawn.d" "WiFi\spi_cc3100.d" "WiFi\wlan.d" "ccs\startup_msp432p401r_ccs.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

