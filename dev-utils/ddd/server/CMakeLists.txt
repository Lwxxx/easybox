cmake_minimum_required(VERSION 2.8.1)

project(dds)

set(SRC_LIST
  ddd.c
  ddd_socket.c
  component/command.c
  component/log.c
  component/portmap.c
  component/tftp.c
  component/monitor.c
  component/monitor/cpu.c
  component/monitor/memory.c
  )

include_directories(.)
include_directories(component)
include_directories(component/monitor)

add_executable(dds ${SRC_LIST})
target_link_libraries(dds pthread)
install(TARGETS dds DESTINATION /usr/bin)
