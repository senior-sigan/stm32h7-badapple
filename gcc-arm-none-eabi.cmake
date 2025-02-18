# CMake toolchain definition for STM32CubeIDE

set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set (CMAKE_SYSTEM_NAME "Generic" CACHE STRING "")

# Skip link step during toolchain validation.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Specify toolchain. NOTE When building from inside STM32CubeIDE the location of the toolchain is resolved by the "MCU Toolchain" project setting (via PATH).  
set(STM32_TARGET_TRIPLET "arm-none-eabi")

set(TOOLCHAIN_SYSROOT  "${STM32_TOOLCHAIN_PATH}/${STM32_TARGET_TRIPLET}")
set(TOOLCHAIN_BIN_PATH "${STM32_TOOLCHAIN_PATH}/bin")
set(TOOLCHAIN_INC_PATH "${STM32_TOOLCHAIN_PATH}/${STM32_TARGET_TRIPLET}/include")
set(TOOLCHAIN_LIB_PATH "${STM32_TOOLCHAIN_PATH}/${STM32_TARGET_TRIPLET}/lib")

set(CMAKE_SYSROOT ${TOOLCHAIN_SYSROOT})

find_program(CMAKE_ASM_COMPILER NAMES ${STM32_TARGET_TRIPLET}-gcc     HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_C_COMPILER   NAMES ${STM32_TARGET_TRIPLET}-gcc     HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_CXX_COMPILER NAMES ${STM32_TARGET_TRIPLET}-g++     HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_OBJCOPY      NAMES ${STM32_TARGET_TRIPLET}-objcopy HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_OBJDUMP      NAMES ${STM32_TARGET_TRIPLET}-objdump HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_SIZE         NAMES ${STM32_TARGET_TRIPLET}-size    HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_DEBUGGER     NAMES ${STM32_TARGET_TRIPLET}-gdb     HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_CPPFILT      NAMES ${STM32_TARGET_TRIPLET}-c++filt HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_AR           NAMES ${STM32_TARGET_TRIPLET}-ar      HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_LINKER       NAMES ${STM32_TARGET_TRIPLET}-ld      HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_RANLIB       NAMES ${STM32_TARGET_TRIPLET}-ranlib  HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_STRIP        NAMES ${STM32_TARGET_TRIPLET}-ld      HINTS ${TOOLCHAIN_BIN_PATH})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)