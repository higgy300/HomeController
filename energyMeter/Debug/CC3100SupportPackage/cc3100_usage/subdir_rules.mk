################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
CC3100SupportPackage/cc3100_usage/%.obj: ../CC3100SupportPackage/cc3100_usage/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.5.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/juanh/Desktop/msp432_driverlib_3_21_00_05/inc" --include_path="C:/Users/juanh/Desktop/Design 2/CC3100SupportPackage" --include_path="C:/Users/juanh/workspace_v8/test" --include_path="C:/ti/simplelink_msp432p4_sdk_2_40_00_10/source" --include_path="C:/ti/simplelink_msp432p4_sdk_2_40_00_10/source/third_party/CMSIS/Include" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.5.LTS/include" --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="CC3100SupportPackage/cc3100_usage/$(basename $(<F)).d_raw" --obj_directory="CC3100SupportPackage/cc3100_usage" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


