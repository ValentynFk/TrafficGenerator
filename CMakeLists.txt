cmake_minimum_required(VERSION 3.14)
project(TrafficGenerator)

set(CMAKE_CXX_STANDARD 11)

#==========================================================================================================#
# Add major files to the project
#==========================================================================================================#

include_directories ("${PROJECT_SOURCE_DIR}")
set (PROJECT_LIBRARIES
        ${PROJECT_LIBRARIES}
        trafficgenerator.h
        customgraphics.h
        plot.h
        )
set (PROJECT_CODE
        ${PROJECT_CODE}
        customgraphics.cpp
        main.cpp
        )

add_executable(TrafficGenerator ${PROJECT_CODE} ${PROJECT_LIBRARIES})
target_link_libraries(TrafficGenerator -lopengl32 -lglew32 -lfreeglut -lglu32)