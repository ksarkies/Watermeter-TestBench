PROJECT =       Testbench GUI
TEMPLATE =      app
TARGET          += 
DEPENDPATH      += .

include(../auxiliary/qextserialport-v1.2/src/qextserialport.pri)

OBJECTS_DIR     = obj
MOC_DIR         = moc
UI_HEADERS_DIR  = ui
UI_SOURCES_DIR  = ui
LANGUAGE        = C++
CONFIG          += qt warn_on release

# Input
FORMS           += testbench-main.ui
HEADERS         += testbench-main.h
HEADERS         += serialport.h
SOURCES         += testbench.cpp
SOURCES         += testbench-main.cpp
SOURCES         += serialport.cpp

