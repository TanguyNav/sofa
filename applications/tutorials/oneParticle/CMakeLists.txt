cmake_minimum_required(VERSION 3.12)
project(oneParticle)

find_package(SofaGui REQUIRED)
find_package(SofaComponentAll REQUIRED)

add_executable(${PROJECT_NAME} oneParticle.cpp)
target_link_libraries(${PROJECT_NAME} SofaGui SofaComponentAll)
