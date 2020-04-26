cmake_minimum_required(VERSION 3.15)

project(Encoder)
set(CMAKE_CXX_STANDARD 14)

add_executable(Encoder
  src/DCTTransform.cpp
  src/Encoder.cpp
  src/EntropyCoder.cpp
  src/IntraPredictor.cpp
  src/MotionCompensator.cpp
  src/Quantiser.cpp)

target_include_directories(Encoder PUBLIC include)
target_link_directories(Encoder PUBLIC lib)

if (APPLE)
    target_link_libraries(Encoder PUBLIC LibraryOSX)
elseif (UNIX)
    target_link_libraries(Encoder PUBLIC LibraryUNIX)
else()
  target_link_libraries(Encoder PUBLIC debug LibraryWINDeb)
  target_link_libraries(Encoder PUBLIC optimized LibraryWINRel)
endif()
