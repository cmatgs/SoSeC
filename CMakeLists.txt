cmake_minimum_required(VERSION 3.16)

project(SoSeSta
  VERSION 0.1.0
  LANGUAGES CXX
)

# -------------------------
# Build-Optionen
# -------------------------
option(SOSESTA_USE_MOCK "Build with mock hardware (no real libs required)" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(APPEND CMAKE_INSTALL_RPATH "/usr/local/lib")

# -------------------------
# wxWidgets 3.2
# -------------------------
if(NOT DEFINED wxWidgets_CONFIG_EXECUTABLE AND EXISTS "/usr/local/bin/wx-config")
  set(wxWidgets_CONFIG_EXECUTABLE "/usr/local/bin/wx-config" CACHE FILEPATH "Path to wx-config")
endif()

find_package(wxWidgets 3.2 REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# -------------------------
# Kern-Quellen
# -------------------------
set(SOSESTA_CORE_SRCS
  # Haupt-Quellen
  src/main.cpp
  src/app/App.cpp   # <— Pfadfix

  # GUI
  src/gui/MainFrame.cpp
  src/gui/ChannelWidget.cpp
  src/gui/ConfigEditor.cpp

  # Hardware-Fabrik
  src/hw/HardwareFactory.cpp
)

# Real-Quellen (nur wenn echte Hardware)
set(SOSESTA_REAL_SRCS
  src/hw/RealHardware.cpp
  src/hw/relays/RelayController.cpp
  src/hw/leds/LEDStrip.cpp
  src/hw/daq/RedLabDAQ.cpp
  src/hw/mux/TCA9548A.cpp
  src/hw/power/INA219.cpp
)

# Mock-Quellen (nur wenn Simulation)
set(SOSESTA_MOCK_SRCS
  src/hw/mock/MockHardware.cpp
)

# -------------------------
# Executable
# -------------------------
add_executable(sosesta
  ${SOSESTA_CORE_SRCS}
)

# Includes
target_include_directories(sosesta PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/app
  ${CMAKE_CURRENT_SOURCE_DIR}/gui
  ${CMAKE_CURRENT_SOURCE_DIR}/hw
  ${CMAKE_CURRENT_SOURCE_DIR}/hw/relays
  ${CMAKE_CURRENT_SOURCE_DIR}/hw/leds
  ${CMAKE_CURRENT_SOURCE_DIR}/hw/daq
  ${CMAKE_CURRENT_SOURCE_DIR}/hw/mux
  ${CMAKE_CURRENT_SOURCE_DIR}/hw/power
  ${CMAKE_CURRENT_SOURCE_DIR}/hw/mock
)

# wxWidgets binden
target_link_libraries(sosesta PRIVATE ${wxWidgets_LIBRARIES})

# -------------------------
# Modus: Mock vs. Real
# -------------------------
if(SOSESTA_USE_MOCK)
  message(STATUS "Building with MOCK hardware")
  target_compile_definitions(sosesta PRIVATE USE_MOCK)
  target_sources(sosesta PRIVATE ${SOSESTA_MOCK_SRCS})
else()
  message(STATUS "Building with REAL hardware")
  target_sources(sosesta PRIVATE ${SOSESTA_REAL_SRCS})

  # libgpiod
  find_path(GPIOD_INCLUDE_DIR gpiod.h)
  find_library(GPIOD_LIBRARY gpiod)
  if(NOT GPIOD_INCLUDE_DIR OR NOT GPIOD_LIBRARY)
    message(FATAL_ERROR "libgpiod nicht gefunden. Installiere: sudo apt install -y gpiod libgpiod-dev")
  endif()
  target_include_directories(sosesta PRIVATE ${GPIOD_INCLUDE_DIR})
  target_link_libraries(sosesta PRIVATE ${GPIOD_LIBRARY})

  # rpi_ws281x
  target_include_directories(sosesta PRIVATE /usr/local/include)
  find_library(WS2811_LIBRARY ws2811 PATHS /usr/local/lib)
  if(NOT WS2811_LIBRARY)
    message(FATAL_ERROR "ws2811 nicht gefunden. Bitte installieren.")
  endif()
  target_link_libraries(sosesta PRIVATE ${WS2811_LIBRARY})

  # ULDAQ (optional)
  find_library(ULDAQ_LIBRARY uldaq)
  if(ULDAQ_LIBRARY)
    target_link_libraries(sosesta PRIVATE ${ULDAQ_LIBRARY})
  else()
    message(WARNING "ULDAQ nicht gefunden – RedLabDAQ wird ggf. nicht gelinkt.")
  endif()
endif()

# Installation
install(TARGETS sosesta RUNTIME DESTINATION bin)

# -------------------------
# Build-Befehle
# -------------------------
# Mock:
#   cmake -B build -G Ninja -DSOSESTA_USE_MOCK=ON -DwxWidgets_CONFIG_EXECUTABLE=/usr/local/bin/wx-config
#   cmake --build build
#
# Real:
#   cmake -B build -G Ninja -DwxWidgets_CONFIG_EXECUTABLE=/usr/local/bin/wx-config
#   cmake --build build
