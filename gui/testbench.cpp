/**
@mainpage Watermeter Testbench GUI
@version 1.0
@author Ken Sarkies (www.jiggerjuice.net)
@date 18 February 2015

This is a control GUI for a water flow testbench to test and calibrate the
Xerofill watermeter.

@note
Compiler: gcc (Ubuntu 4.8.2-19ubuntu1) 4.8.2
@note
Uses: Qt version 4.8.6
*/

/****************************************************************************
 *   Copyright (C) 2015 by Ken Sarkies                                      *
 *   ksarkies@internode.on.net                                              *
 *                                                                          *
 *   This file is part of Testbench                                         *
 *                                                                          *
 *   Testbench is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU General Public License as         *
 *   published by the Free Software Foundation; either version 2 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   Testbench is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with testbench.                                                  *
 *   If not, write to the Free Software Foundation, Inc.,                   *
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.              *
 ***************************************************************************/

#include "testbench-main.h"
#include <QApplication>
#include <QMessageBox>

//-----------------------------------------------------------------------------
/** @brief Testbench GUI Main Program

*/

int main(int argc,char ** argv)
{
    QString inPort = SERIAL_PORT;
    uint initialBaudrate = BAUDRATE;

    QApplication application(argc,argv);
    TestbenchGui testbenchGui(inPort,initialBaudrate);
    if (testbenchGui.success())
    {
        testbenchGui.show();
        return application.exec();
    }
    else
        QMessageBox::critical(0,"Unable to connect to remote system",
              QString("%1").arg(testbenchGui.error()));
    return false;
}
