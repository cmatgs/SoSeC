cmake_minimum_required(VERSION 3.16)

project(SoSeSta
  VERSION 0.1.0
  LANGUAGES CXX
)

# -------------------------
# Build-Optionen
# -------------------------
option(SOSESTA_USE_MOCK "Build with mock hardware (no real libs required)" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# RPATH (Linux/RPi)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(APPEND CMAKE_INSTALL_RPATH "/usr/local/lib")

# -------------------------
# wxWidgets 3.2 Auto-Detection
# -------------------------
# Der Nutzer kann wxWidgets_CONFIG_EXECUTABLE weiterhin selbst setzen.
if (NOT DEFINED wxWidgets_CONFIG_EXECUTABLE)
  if (WIN32)
    # MSYS2 UCRT64 – bevorzugter Pfad
    if (EXISTS "C:/msys64/ucrt64/bin/wx-config")
      set(wxWidgets_CONFIG_EXECUTABLE "C:/msys64/ucrt64/bin/wx-config" CACHE FILEPATH "Path to wx-config" FORCE)
    elseif (EXISTS "/ucrt64/bin/wx-config")
      # Falls der Aufruf im MSYS2-UCRT64-Shell-Kontext ist
      set(wxWidgets_CONFIG_EXECUTABLE "/ucrt64/bin/wx-config" CACHE FILEPATH "Path to wx-config" FORCE)
    else()
      message(FATAL_ERROR "wx-config nicht gefunden! Bitte installieren: pacman -S mingw-w64-ucrt-x86_64-wxWidgets3.2")
    endif()
  elseif (UNIX)
    if (EXISTS "/usr/local/bin/wx-config")
      set(wxWidgets_CONFIG_EXECUTABLE "/usr/local/bin/wx-config" CACHE FILEPATH "Path to wx-config" FORCE)
    elseif (EXISTS "/usr/bin/wx-config")
      set(wxWidgets_CONFIG_EXECUTABLE "/usr/bin/wx-config" CACHE FILEPATH "Path to wx-config" FORCE)
    else()
      message(FATAL_ERROR "wx-config nicht gefunden! Bitte wxWidgets 3.2 installieren.")
    endif()
  endif()
endif()

find_package(wxWidgets 3.2 REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# -------------------------
# Quellen nach deiner Struktur
# └───src
#     ├───app
#     │   ├───core
#     │   └───data
#     ├───config
#     ├───gui
#     ├───hw
#     │   ├───mock
#     │   └───real
#     │       ├───daq
#     │       ├───leds
#     │       ├───mux
#     │       ├───power
#     │       └───relays
#     ├───services
# -------------------------

# Kern-Quellen (plattformunabhängig)
set(SOSESTA_CORE_SRCS
  # Main
  src/main.cpp

  # App
  src/app/core/App.cpp
  src/app/core/State.cpp

  # GUI
  src/gui/MainFrame.cpp
  src/gui/ChannelWidget.cpp
  src/gui/ConfigEditor.cpp

  # Services
  src/services/CsvExporter.cpp
  src/services/LoggerService.cpp
  src/services/TestRunner.cpp

  # Hardware Factory (erzeugt Mock oder Real)
  src/hw/HardwareFactory.cpp
)

# Mock-Quellen
set(SOSESTA_MOCK_SRCS
  src/hw/mock/MockHardware.cpp
)

# Real-Quellen (falls später implementiert)
set(SOSESTA_REAL_SRCS
  src/hw/real/RealHardware.cpp
  src/hw/real/relays/RelayController.cpp
  src/hw/real/leds/LEDStrip.cpp
  src/hw/real/daq/RedLabDAQ.cpp
  src/hw/real/mux/TCA9548A.cpp
  src/hw/real/power/INA219.cpp
)

# -------------------------
# Executable
# -------------------------
add_executable(sosesta
  ${SOSESTA_CORE_SRCS}
)

# Includes
target_include_directories(sosesta PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/app
  ${CMAKE_CURRENT_SOURCE_DIR}/src/app/core
  ${CMAKE_CURRENT_SOURCE_DIR}/src/app/data
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw/mock
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw/real
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw/real/relays
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw/real/leds
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw/real/daq
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw/real/mux
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hw/real/power
  ${CMAKE_CURRENT_SOURCE_DIR}/src/services
)

# wxWidgets
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

  # Ab hier: optionale Real-Dependencies für Linux/RPi.
  # Unter MSYS2/Windows normalerweise nicht vorhanden/benötigt.
  if (UNIX AND NOT APPLE)
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
    find_library(WS2811_LIBRARY ws2811 PATHS /usr/local/lib /lib /usr/lib)
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
endif()

# Installation
install(TARGETS sosesta RUNTIME DESTINATION bin)

# -------------------------
# Build-Beispiele
# -------------------------
# MSYS2 UCRT64 (Windows, Mock):
#   cmake -B build -G Ninja
#   cmake --build build
#
# Linux (Mock):
#   cmake -B build -G Ninja -DSOSESTA_USE_MOCK=ON
#   cmake --build build
#
# Linux (Real):
#   cmake -B build -G Ninja -DSOSESTA_USE_MOCK=OFF
#   cmake --build build
