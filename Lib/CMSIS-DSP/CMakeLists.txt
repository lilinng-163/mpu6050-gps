cmake_minimum_required(VERSION 3.22)

enable_language(C CXX)

set(CMSIS_DSP_SRC E:/git_code/CMSIS-DSP/Source)

file(GLOB BASIC_SRC     ${CMSIS_DSP_SRC}/BasicMathFunctions/*.c)
file(GLOB BAYES_SRC     ${CMSIS_DSP_SRC}/BayesFunctions/*.c)
file(GLOB COMMON_SRC    ${CMSIS_DSP_SRC}/CommonTables/*.c)
file(GLOB COMPLEX_SRC   ${CMSIS_DSP_SRC}/ComplexMathFunctions/*.c)
file(GLOB CTRL_SRC      ${CMSIS_DSP_SRC}/ControllerFunctions/*.c)
file(GLOB DIST_SRC      ${CMSIS_DSP_SRC}/DistanceFunctions/*.c)
file(GLOB FAST_SRC      ${CMSIS_DSP_SRC}/FastMathFunctions/*.c)
file(GLOB FILTER_SRC    ${CMSIS_DSP_SRC}/FilteringFunctions/*.c)
file(GLOB INTERP_SRC    ${CMSIS_DSP_SRC}/InterpolationFunctions/*.c)
# MatrixFunctions needs to exclude NEON files
file(GLOB MATRIX_SRC    ${CMSIS_DSP_SRC}/MatrixFunctions/*.c)
list(FILTER MATRIX_SRC EXCLUDE REGEX "_neon")
file(GLOB QUAT_SRC      ${CMSIS_DSP_SRC}/QuaternionMathFunctions/*.c)
file(GLOB STAT_SRC      ${CMSIS_DSP_SRC}/StatisticsFunctions/*.c)
file(GLOB SUPPORT_SRC   ${CMSIS_DSP_SRC}/SupportFunctions/*.c)
file(GLOB SVM_SRC       ${CMSIS_DSP_SRC}/SVMFunctions/*.c)
file(GLOB TRANSFORM_SRC ${CMSIS_DSP_SRC}/TransformFunctions/*.c)
file(GLOB WINDOW_SRC    ${CMSIS_DSP_SRC}/WindowFunctions/*.c)

add_library(cmsis-dsp STATIC)
target_include_directories(cmsis-dsp PUBLIC
    E:/git_code/CMSIS-DSP/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    PRIVATE E:/git_code/CMSIS-DSP/PrivateInclude)
target_compile_definitions(cmsis-dsp PUBLIC ARM_MATH_CM3)
target_sources(cmsis-dsp PRIVATE
    ${BASIC_SRC} ${BAYES_SRC} ${COMMON_SRC} ${COMPLEX_SRC}
    ${CTRL_SRC} ${DIST_SRC} ${FAST_SRC} ${FILTER_SRC}
    ${INTERP_SRC} ${MATRIX_SRC} ${QUAT_SRC} ${STAT_SRC}
    ${SUPPORT_SRC} ${SVM_SRC} ${TRANSFORM_SRC} ${WINDOW_SRC})