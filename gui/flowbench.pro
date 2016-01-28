PROJECT =       FlowBench GUI
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
FORMS           += flowbench-main.ui
HEADERS         += flowbench-main.h
HEADERS         += serialport.h
SOURCES         += flowbench.cpp
SOURCES         += flowbench-main.cpp
SOURCES         += serialport.cpp

