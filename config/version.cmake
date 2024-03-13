cmake_minimum_required(VERSION 3.10)

set(VERSION_CMD "${CMAKE_CURRENT_SOURCE_DIR}/config/version.py")
set(MASTER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/config/MasterVersion")
set(TEMPLATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config/")
set(OUTFILE "${CMAKE_CURRENT_SOURCE_DIR}/kern/version.h")

add_custom_target(generate_version
    DEPENDS ${VERSION_CMD}
            ${CMAKE_CURRENT_SOURCE_DIR}/*
)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
set(PLATFORM "AARCH64_QEMU")

add_custom_command(TARGET generate_version
    PRE_BUILD
        COMMAND python3 ${VERSION_CMD} -m ${MASTER_VERSION} -t ${TEMPLATE_DIR} 
                        -o ${OUTFILE} -b ${BUILD_TYPE} -p ${PLATFORM}
)