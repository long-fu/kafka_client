cmake_minimum_required(VERSION 3.0.0)
project(main VERSION 0.1.0 LANGUAGES C CXX)

# include(CTest)
# enable_testing()

include_directories(.)

add_executable(main main.c producer.c consumer.c cJSON_Utils.c cJSON.c http_parser.c soc.c http.c)
target_link_libraries(main PUBLIC rdkafka cap)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
