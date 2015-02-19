/*       Testbench GUI Main Window

Here the data stream from the test microcontroller is received and saved to a
file

@date 18 February 2015
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

#include "testbench.h"
#include "testbench-main.h"
#include "serialport.h"
#include <QApplication>
#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

//-----------------------------------------------------------------------------
/** Testbench Main Window Constructor

@param[in] parent Parent widget.
*/

TestbenchGui::TestbenchGui(QString inPort, uint baudrate,
                                       QWidget* parent) : QDialog(parent)
{
    TestbenchMainUi.setupUi(this);
    socket = new SerialPort(inPort);
    connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
    if (socket->initPort(baudrate,100))
        synchronized = true;
    else
        errorMessage = QString("Unable to access the serial port\n"
                            "Check the connections and power.\n"
                            "You may need root privileges?");
}

TestbenchGui::~TestbenchGui()
{
}

//-----------------------------------------------------------------------------
/** @brief Handle incoming serial data

This is called when data appears in the serial buffer. Data is pulled in until
a newline occurs, at which point the assembled data record QString is processed.

*/

void TestbenchGui::onDataAvailable()
{
    QByteArray data = socket->readAll();
    int n=0;
    while (n < data.size())
    {
        if ((data.at(n) != '\r') && (data.at(n) != '\n')) response += data.at(n);
        if (data.at(n) == '\n')
        {
// The current time is saved to ms precision followed by the data record.
            tick.restart();
            processResponse(response);
            response.clear();
        }
        n++;
    }
}

//-----------------------------------------------------------------------------
/** @brief Process the incoming serial data

Parse the line as csv and display each field.
*/

void TestbenchGui::processResponse(const QString response)
{
    QStringList breakdown = response.split(",");
    int size = breakdown.size();

// Time from ISO formatted date-time.
    QString timeStamp = breakdown[0].simplified().mid(11,8);
    TestbenchMainUi.timeDisplay->setText(timeStamp);

/* Flow meter count and period. These are processed further to derive a
flow rate by selecting the most accurate measure. 
Datasheet for the FS200A gives 450 counts per litre.
Count is number of pulses in 10ms, period is in ms. */
    int flowMeterCount = breakdown[1].simplified().toInt();
    long flowMeterPeriod = breakdown[2].simplified().toLong();
    float flowMeterRate;
    if (flowMeterCount < 10)
        flowMeterRate = 133.3/flowMeterPeriod;
    else
        flowMeterRate = 1.333*flowMeterCount;
    TestbenchMainUi.flowRate->setText(QString("%1").arg(flowMeterRate,0,'f',1));

/* Pressure scaled by datasheet specification 0.5V to 4.5V for 0 to 1.2 MPa.
Results given in ATM */
    int pressval = breakdown[3].simplified().toInt();
    float pressure = 3.0*(5.0*(float)pressval/1024.0-0.5);
    TestbenchMainUi.pressure->setText(QString("%1").arg(pressure,0,'f',3));

// Temperature scaled by rough calibration.
    int tempval = breakdown[4].simplified().toInt();
    float temperature = 26.0+((float)tempval-716.0)/15.07;
    TestbenchMainUi.temperature->setText(QString("%1").arg(temperature,0,'f',1));
}

//-----------------------------------------------------------------------------
/** @brief Error Message

@returns a message when the device didn't respond.
*/
QString TestbenchGui::error()
{
    return errorMessage;
}

//-----------------------------------------------------------------------------
/** @brief Successful establishment of serial port setup

@returns TRUE if successful.
*/
bool TestbenchGui::success()
{
    return true;
}


