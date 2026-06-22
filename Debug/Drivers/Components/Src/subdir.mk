################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Components/Src/esp32_at.c \
../Drivers/Components/Src/esp32_at_io.c 

OBJS += \
./Drivers/Components/Src/esp32_at.o \
./Drivers/Components/Src/esp32_at_io.o 

C_DEPS += \
./Drivers/Components/Src/esp32_at.d \
./Drivers/Components/Src/esp32_at_io.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Components/Src/%.o Drivers/Components/Src/%.su Drivers/Components/Src/%.cyclo: ../Drivers/Components/Src/%.c Drivers/Components/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I"C:/Users/Akhilesh/Desktop/ET_Pro_iot/App/Inc" -I"C:/Users/Akhilesh/Desktop/ET_Pro_iot/Drivers/Components/Inc" -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Components-2f-Src

clean-Drivers-2f-Components-2f-Src:
	-$(RM) ./Drivers/Components/Src/esp32_at.cyclo ./Drivers/Components/Src/esp32_at.d ./Drivers/Components/Src/esp32_at.o ./Drivers/Components/Src/esp32_at.su ./Drivers/Components/Src/esp32_at_io.cyclo ./Drivers/Components/Src/esp32_at_io.d ./Drivers/Components/Src/esp32_at_io.o ./Drivers/Components/Src/esp32_at_io.su

.PHONY: clean-Drivers-2f-Components-2f-Src

