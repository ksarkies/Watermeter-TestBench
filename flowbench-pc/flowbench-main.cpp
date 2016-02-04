/*       FlowBench GUI Main Window

Here the data stream from the test microcontroller is received and saved to a
file

@date 18 February 2015
*/

/****************************************************************************
 *   Copyright (C) 2015 by Ken Sarkies                                      *
 *   ksarkies@internode.on.net                                              *
 *                                                                          *
 *   This file is part of FlowBench                                         *
 *                                                                          *
 *   FlowBench is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU General Public License as         *
 *   published by the Free Software Foundation; either version 2 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FlowBench is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with testbench.                                                  *
 *   If not, write to the Free Software Foundation, Inc.,                   *
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.              *
 ***************************************************************************/

#include "flowbench.h"
#include "flowbench-main.h"
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
#include <QFileInfo>
#include <QDebug>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

//-----------------------------------------------------------------------------
/** FlowBench Main Window Constructor

@param[in] parent Parent widget.
*/

FlowBenchGui::FlowBenchGui(QWidget* parent) : QDialog(parent)
{
    FlowBenchMainUi.setupUi(this);
    setComboBoxes();

    FlowBenchMainUi.startPushButton->setText("Start");
    FlowBenchMainUi.startPushButton->setStyleSheet("background-color:lightgreen;");
    recordingActive = false;
    socket1 = NULL;
    socket2 = NULL;
}

FlowBenchGui::~FlowBenchGui()
{
}

//-----------------------------------------------------------------------------
/** @brief Setup Serial ComboBoxes

Test existence of serial port (ACM and USB) and build both combobox entries.
Different ports are checked on Windows and Linux.
*/

void FlowBenchGui::setComboBoxes()
{
    QString port;
    FlowBenchMainUi.input1ComboBox->clear();
    FlowBenchMainUi.input2ComboBox->clear();
#ifdef Q_OS_LINUX
    port = "/dev/ttyS0";
    FlowBenchMainUi.input1ComboBox->insertItem(0,port);
    for (int i=3; i>=0; i--)
    {
        port = "/dev/ttyUSB"+QString::number(i);
        QFileInfo checkUSBFile1(port);
        if (checkUSBFile1.exists())
            FlowBenchMainUi.input1ComboBox->insertItem(0,port);
    }
    for (int i=3; i>=0; i--)
    {
        port = "/dev/ttyACM"+QString::number(i);
        QFileInfo checkACMFile1(port);
        if (checkACMFile1.exists())
            FlowBenchMainUi.input1ComboBox->insertItem(0,port);
    }
    port = "/dev/ttyS0";
    FlowBenchMainUi.input2ComboBox->insertItem(0,port);
    for (int i=3; i>=0; i--)
    {
        port = "/dev/ttyACM"+QString::number(i);
        QFileInfo checkACMFile2(port);
        if (checkACMFile2.exists())
            FlowBenchMainUi.input2ComboBox->insertItem(0,port);
    }
    for (int i=3; i>=0; i--)
    {
        port = "/dev/ttyUSB"+QString::number(i);
        QFileInfo checkUSBFile2(port);
        if (checkUSBFile2.exists())
            FlowBenchMainUi.input2ComboBox->insertItem(0,port);
    }
#else
    for (int i=3; i>=0; i--)
    {
        port = "COM"+QString::number(i+3);
        QFileInfo checkACMFile1(port);
        if (checkACMFile1.exists())
            FlowBenchMainUi.input1ComboBox->insertItem(0,port);
    }
    for (int i=3; i>=0; i--)
    {
        port = "COM"+QString::number(i+3);
        QFileInfo checkUSBFile2(port);
        if (checkUSBFile2.exists())
            FlowBenchMainUi.input2ComboBox->insertItem(0,port);
    }
#endif
    FlowBenchMainUi.input1ComboBox->setCurrentIndex(0);
    FlowBenchMainUi.input2ComboBox->setCurrentIndex(0);

    QStringList baudrates;
    baudrates << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    FlowBenchMainUi.baudrate1ComboBox->addItems(baudrates);
    FlowBenchMainUi.baudrate1ComboBox->setCurrentIndex(BAUDRATE1);
    FlowBenchMainUi.baudrate2ComboBox->addItems(baudrates);
    FlowBenchMainUi.baudrate2ComboBox->setCurrentIndex(BAUDRATE2);
}

//-----------------------------------------------------------------------------
/** @brief Connect to selected serial port 1

*/

void FlowBenchGui::on_connect1_clicked()
{
    QString inPort1 = FlowBenchMainUi.input1ComboBox->currentText();
    baudrate1 = BAUDRATE1;

    if (socket1 != NULL)
    {
        disconnect(socket1, SIGNAL(readyRead()), this, SLOT(onData1Available()));
        delete socket1;
        socket1 = NULL;
        FlowBenchMainUi.connect1->setText("Connect");
    }
    else
    {
        socket1 = new SerialPort(inPort1);
        connect(socket1, SIGNAL(readyRead()), this, SLOT(onData1Available()));
        socket1->initPort(baudrate1,100);
        FlowBenchMainUi.connect1->setText("Disconnect");
    }
    setComboBoxes();        // Rebuild combobox entries
}

//-----------------------------------------------------------------------------
/** @brief Connect to selected serial port 2

*/

void FlowBenchGui::on_connect2_clicked()
{
    QString inPort2 = FlowBenchMainUi.input2ComboBox->currentText();
    baudrate2 = BAUDRATE2;

    if (socket2 != NULL)
    {
        disconnect(socket2, SIGNAL(readyRead()), this, SLOT(onData2Available()));
        delete socket2;
        socket2 = NULL;
        FlowBenchMainUi.connect2->setText("Connect");
    }
    else
    {
        socket2 = new SerialPort(inPort2);
        connect(socket2, SIGNAL(readyRead()), this, SLOT(onData2Available()));
        socket2->initPort(baudrate2,100);
        FlowBenchMainUi.connect2->setText("Disconnect");
    }
    setComboBoxes();        // Rebuild combobox entries
}

//-----------------------------------------------------------------------------
/** @brief Handle incoming serial data on the Arduino

This is called when data appears in the serial buffer. Data is pulled in until
a newline occurs, at which point the assembled data record QString is processed.
*/

void FlowBenchGui::onData1Available()
{
    QByteArray data = socket1->readAll();
    int n=0;
    while (n < data.size())
    {
        if ((data.at(n) != '\r') && (data.at(n) != '\n')) response1 += data.at(n);
        if (data.at(n) == '\n')
        {
// The current time is saved to ms precision followed by the data record.
            tick.restart();
            processResponse(response1);
/* Get local time in ISO 8601 format. */
            QString timeString = QDateTime::currentDateTime().time().toString(Qt::ISODate);
            QString msString = QString("%1")
                    .arg(QDateTime::currentDateTime().time().msec()/10,2,10,QLatin1Char('0'));
            FlowBenchMainUi.timeDisplay->setText(timeString+"."+msString);
/* Save the response with timestamp stripped and replaced with local time. */
            QString line = timeString+','+msString+','+response1.mid(response1.indexOf(",")+1);
            if ((! saveFile.isEmpty()) && recordingActive) saveLine(line);
            response1.clear();
        }
        n++;
    }
}

//-----------------------------------------------------------------------------
/** @brief Handle incoming serial data on the alternate device

This is called when data appears in the serial buffer. Data is pulled in until
a newline occurs, at which point the assembled data record QString is processed.
*/

void FlowBenchGui::onData2Available()
{
    QByteArray data = socket2->readAll();
    int n=0;
    while (n < data.size())
    {
        if ((data.at(n) != '\r') && (data.at(n) != '\n')) response2 += data.at(n);
        if (data.at(n) == '\n')
        {
// The current time is saved to ms precision followed by the data record.
            tick.restart();
/* Get local time in ISO 8601 format. */
            QString timeString = QDateTime::currentDateTime().time().toString(Qt::ISODate);
            QString msString = QString("%1")
                    .arg(QDateTime::currentDateTime().time().msec()/10,2,10,QLatin1Char('0'));
/* Save the response with timestamp stripped and replaced wth local time. */
            QString line = timeString+','+msString+','+response2.mid(response2.indexOf(",")+1);
            if ((! saveFile.isEmpty()) && recordingActive) saveLine(line);
            response2.clear();
        }
        n++;
    }
}

//-----------------------------------------------------------------------------
/** @brief Process the incoming serial data for the Arduino only

Parse the line as csv and display each field.
*/

void FlowBenchGui::processResponse(const QString response)
{
    QString line;
    QStringList breakdown = response.split(",");
    int size = breakdown.size();

    if (size < 1) return;
// Extract time from ISO 8601 formatted date-time.
    QString timeStamp = breakdown[0].simplified();
    line = QString("%1").arg(timeStamp,30);
//    FlowBenchMainUi.timeDisplay->setText(timeStamp.mid(11,8));

    if (size < 3) return;
/* Flow meter count and period. These are processed further to derive a
flow rate in litres per minute by selecting the most accurate measure.
Datasheet for the FS200A specifies 450 counts per litre.
Count is number of pulses in 1s, period is in ms. */
    int flowMeterCount = breakdown[1].simplified().toInt();
    long flowMeterPeriod = breakdown[2].simplified().toLong();
    float flowMeterRate;
    if (flowMeterCount < 10)
        flowMeterRate = 133.3/flowMeterPeriod;
    else
        flowMeterRate = 0.1333*flowMeterCount;
    FlowBenchMainUi.flowRate->setText(QString("%1").arg(flowMeterRate,0,'f',1));
    line += QString("%1").arg(flowMeterRate,10,'f',1);

    if (size < 4) return;
/* Pressure scaled by datasheet specification 0.5V to 4.5V for 0 to 1.2 MPa.
Results given in ATM */
    int pressval = breakdown[3].simplified().toInt();
    float pressure = 3.0*(5.0*(float)pressval/1024.0-0.5);
    FlowBenchMainUi.pressure->setText(QString("%1").arg(pressure,0,'f',3));
    line += QString("%1").arg(pressure,10,'f',1);

    if (size < 5) return;
// Temperature scaled by rough calibration.
//    int tempval = breakdown[4].simplified().toInt();
//    float temperature = 26.0+((float)tempval-716.0)/15.07;
    float temperature = breakdown[4].simplified().toFloat();
    FlowBenchMainUi.temperature->setText(QString("%1").arg(temperature,0,'f',2));
    line += QString("%1").arg(temperature,10,'f',1);

    if (size < 7) return;
/* Water meter count and period. These are processed further to display a
count value with the best accuracy. */
    int waterMeterCount = breakdown[5].simplified().toInt();
    long waterMeterPeriod = breakdown[6].simplified().toLong();
    float waterMeterRate;
    if (waterMeterCount < 10)
        waterMeterRate = 102.9/waterMeterPeriod;
    else
        waterMeterRate = 0.1029*waterMeterCount;
    FlowBenchMainUi.meterFlow->setText(QString("%1").arg(waterMeterRate,0,'f',1));
    line += QString("%1").arg(waterMeterRate,10,'f',1);
qDebug() << line;
    FlowBenchMainUi.displayListWidget->addItem(line);
    FlowBenchMainUi.displayListWidget->scrollToBottom();
}

//-----------------------------------------------------------------------------
/** @brief Activate the Start/Stop Recording button.

Only works on socket 1, which should be set to the Arduino.
*/

void FlowBenchGui::on_startPushButton_clicked()
{
    SerialPort* socket = NULL;
    if (socket1 != NULL) socket = socket1;
    else if (socket2 != NULL) socket = socket2;
    if (socket != NULL)
    {
        recordingActive = (FlowBenchMainUi.startPushButton->text() == "Start");
        if (recordingActive)
        {
            FlowBenchMainUi.startPushButton->setText("Stop");
            FlowBenchMainUi.startPushButton->setStyleSheet("background-color:pink;");
            socket->write("As+\n\r");
        }
        else
        {
            FlowBenchMainUi.startPushButton->setText("Start");
            FlowBenchMainUi.startPushButton->setStyleSheet("background-color:lightgreen;");
            socket->write("As-\n\r");
        }
    }
}

//-----------------------------------------------------------------------------
/** @brief Obtain a save file name and path and attempt to open it.

The files are csv but the ending can be arbitrary to allow compatibility
with the data processing application.
*/

void FlowBenchGui::on_saveFileButton_clicked()
{
//! Make sure there is no file already open.
    if (! saveFile.isEmpty())
    {
        displayErrorMessage("A file is already open - close first");
        return;
    }
    FlowBenchMainUi.errorLabel->clear();
    QString filename = QFileDialog::getSaveFileName(this,
                        "Save Acquired Data",
                        QString(),
                        "Comma Separated Variables (*.csv *.txt)");
    if (filename.isEmpty()) return;
//    if (! filename.endsWith(".csv")) filename.append(".csv");
    QFileInfo fileInfo(filename);
    saveDirectory = fileInfo.absolutePath();
    saveFile = saveDirectory.filePath(filename);
    outFile = new QFile(saveFile);             // Open file for output
    if (! outFile->open(QIODevice::WriteOnly))
    {
        displayErrorMessage("Could not open the output file");
        return;
    }
}

//-----------------------------------------------------------------------------
/** @brief Save a line to the opened save file.

*/
void FlowBenchGui::saveLine(QString line)
{
//! Check that the save file has been defined and is open.
    if (saveFile.isEmpty())
    {
        displayErrorMessage("Output File not defined");
        return;
    }
    if (! outFile->isOpen())
    {
        displayErrorMessage("Output File not open");
        return;
    }
/* If either of the last two characters are \n or \r, strip them out. */
    if (line.endsWith("\n") || line.endsWith("\r"))
        line = line.remove(1);
    if (line.endsWith("\n") || line.endsWith("\r"))
        line = line.remove(1);

    QTextStream out(outFile);
    out << line << "\n\r";
}

//-----------------------------------------------------------------------------
/** @brief Close the save file.

*/
void FlowBenchGui::on_closeFileButton_clicked()
{
    FlowBenchMainUi.errorLabel->clear();
    if (saveFile.isEmpty())
        displayErrorMessage("File already closed");
    else
    {
        outFile->close();
        delete outFile;
//! Save the name to prevent the same file being used.
        saveFile = QString();
    }
}

//-----------------------------------------------------------------------------
/** @brief Show an error condition in the Error label.

@param[in] message: Message to be displayed.
*/

void FlowBenchGui::displayErrorMessage(const QString message)
{
    FlowBenchMainUi.errorLabel->setText(message);
}

//-----------------------------------------------------------------------------
/** @brief Error Message

@returns a message when the device didn't respond.
*/
QString FlowBenchGui::error()
{
    return errorMessage;
}

//-----------------------------------------------------------------------------
/** @brief Successful establishment of serial port setup

@returns TRUE if successful.
*/
bool FlowBenchGui::success()
{
    return true;
}


