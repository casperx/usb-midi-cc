add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/main.cc)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/main.cc
  ${CMAKE_CURRENT_LIST_DIR}/config.cc
  ${CMAKE_CURRENT_LIST_DIR}/quota.cc
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
  # EMULATED
  # DEMO
)

target_link_libraries(
  ${PROJECT_NAME}
  pico_stdlib
  pico_rand
  hardware_pio
  hardware_flash
  tinyusb_device
)

target_link_options(
  ${PROJECT_NAME}
  PRIVATE
  "-Wl,--print-memory-usage"
)

add_subdirectory(pio)
add_subdirectory(usb)

pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 0)
