PROJECT =       FlowBench GUI
TEMPLATE =      app
TARGET          += 
DEPENDPATH      += .
QT              += widgets
QT              += serialport

OBJECTS_DIR     = obj
MOC_DIR         = moc
UI_DIR          = ui
LANGUAGE        = C++
CONFIG          += qt warn_on release

# Input
FORMS           += flowbench-main.ui
HEADERS         += flowbench-main.h
SOURCES         += flowbench.cpp
SOURCES         += flowbench-main.cpp
