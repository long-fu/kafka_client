cmake_minimum_required(VERSION 3.0.0)
project(alarm_send VERSION 0.1.0 LANGUAGES C CXX)

# include(CTest)
# enable_testing()

include_directories(.)

add_executable(alarm_send main.c soc.c client.c http.c producer.c)
# add_executable(main bind_ifname.c)
target_link_libraries(alarm_send PUBLIC rdkafka)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
