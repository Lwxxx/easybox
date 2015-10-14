cmake_minimum_required(VERSION 2.8.1)

project(ddd)

set(SRC_LIST
  ddd.c
  ddd_socket.c
  component/command.c
  component/log.c
  component/monitor.c
  component/portmap.c
  component/tftp.c
  )

include_directories(.)
include_directories(component)

add_executable(ddd ${SRC_LIST})
target_link_libraries(ddd pthread)
install(TARGETS ddd DESTINATION /usr/bin)
