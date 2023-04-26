#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->addWidget(&_statusLabel);

    ui->endString->addItem(QStringLiteral("нет"), "");
    ui->endString->addItem(QStringLiteral("\\n"), "\n");
    ui->endString->addItem(QStringLiteral("\\r\\n"), "\r\n");

    connect(ui->outData, &QPlainTextEdit::textChanged, this, &MainWindow::outDataTextChanged);
    connect(ui->sendData, &QPushButton::clicked, this, &MainWindow::write);

    connect(&_serialport, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(&_serialport, &QSerialPort::readyRead, this, &MainWindow::readData);

    ui->outData->installEventFilter(this);

    disableAction(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionAboutQt_triggered()
{
    QApplication::aboutQt();
}


void MainWindow::on_actionQuit_triggered()
{
    QApplication::closeAllWindows();
}


void MainWindow::on_actionUART_triggered()
{
    _settingDialog.show();
}

void MainWindow::on_actionConnect_triggered()
{
    const SettingsDialog::Settings p = _settingDialog.settings();
    _serialport.setPortName(p.name);
    _serialport.setBaudRate(p.baudRate);
    _serialport.setDataBits(p.dataBits);
    _serialport.setParity(p.parity);
    _serialport.setStopBits(p.stopBits);
    _serialport.setFlowControl(p.flowControl);

    if (_serialport.open(QIODevice::ReadWrite))
    {
        disableAction(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), _serialport.errorString());
        showStatusMessage(tr("Open error"));
    }
}

void MainWindow::showStatusMessage(const QString &message)
{
    _statusLabel.setText(message);
}

void MainWindow::on_actionDisconnect_triggered()
{
    if (_serialport.isOpen())
        _serialport.close();

    disableAction(true);

    showStatusMessage(tr("Disconnected"));
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->outData && ui->hex->isChecked() && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int keyCode = keyEvent->key();
        char ch = keyCode;
        if (std::isxdigit(ch)
                || keyCode == Qt::Key_Up || keyCode == Qt::Key_Down
                || keyCode == Qt::Key_Left || keyCode == Qt::Key_Right
                || keyCode == Qt::Key_Return
                || keyCode == Qt::Key_Backspace || keyCode == Qt::Key_Delete)
            return QMainWindow::eventFilter(watched, event);
        else if ((keyCode == Qt::Key_C || keyCode == Qt::Key_V) && keyEvent->modifiers() == Qt::ControlModifier)
            return QMainWindow::eventFilter(watched, event);

        return true;
    }
    else
    {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(watched, event);
    }
}

QString MainWindow::byteToHexString(uint8_t ch) const
{
    QString byte = QString::number(ch, 16);
    if (ch < 0x10)
        byte = '0' + byte;

    return byte.toUpper();
}

QString MainWindow::textToHexText(const QString &str, QString delim) const
{
    QString result;

    QByteArray rawData;

    for (int i = 0; i < str.length(); ++i)
    {
        if (str[i] > QChar(0xff))
            rawData.push_back(str[i].unicode() >> 8);

        rawData.push_back(str[i].unicode());
    }

    for (int i = 0; i < rawData.length(); ++i)
    {
        result += byteToHexString(rawData[i]);
        if ((i != (rawData.length() - 1)))
            result += delim;
    }
    return result;
}

QString MainWindow::hexTextToText(const QString &str) const
{
    QString result, byte;
    for (const auto &el : str)
    {
        byte += el;
        if (byte.size() == 2)
        {
            bool ok;
            int code = byte.toInt(&ok, 16);
            if (ok)
            {
                result += QChar(code);
                byte.clear();
            }
        }
    }

    return result;
}

void MainWindow::disableAction(bool state)
{
    ui->actionConnect->setEnabled(state);
    ui->actionUART->setEnabled(state);
    ui->outData->setEnabled(!state);
    ui->sendData->setEnabled(!state);
    ui->actionDisconnect->setEnabled(!state);
}

void MainWindow::outDataTextChanged()
{
    QString result;

    if (ui->hex->isChecked())
    {
        QString rawString = ui->outData->toPlainText().remove('\n').remove(' ');

        // Добавить в старший разряд 0 для полного байта
        if (rawString.length() % 2 > 0)
            rawString.insert(rawString.length() - 1, '0');

        // конец строки
        QByteArray endString = ui->endString->currentData().toString().toLatin1();
        for( int i = 0; i < endString.length(); ++i)
            rawString += byteToHexString(endString[i]);

        // вставка двоеточий
        for (int i = 0; i < rawString.length(); ++i)
        {
            result += rawString[i];
            if ((i % 2 == 1) && (i != (rawString.length() - 1)))
                result += ':';
        }
        ui->outDataRaw->setPlainText(result.toUpper());
    }
    else if (ui->ascii->isChecked())
    {
        QString str = ui->outData->toPlainText() + ui->endString->currentData().toString();
        ui->outDataRaw->setPlainText(textToHexText(str, ":"));
    }
}

void MainWindow::on_endString_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    outDataTextChanged();
}


void MainWindow::write()
{
    QByteArray dataSend;
    for (const auto &byte : ui->outDataRaw->toPlainText().split(':'))
    {
        dataSend.append(byte.toInt(nullptr, 16));
    }
    _serialport.write(dataSend);
}

void MainWindow::readData()
{
//    _serialport.waitForReadyRead(500);
    QByteArray data = _serialport.readAll();
    QString timeMarker = QTime::currentTime().toString("hh:mm:ss.z") + " -> ";
    ui->inData->appendPlainText(timeMarker + data);

    QString result;
    for (int i = 0; i < data.length(); ++i)
    {
        result += byteToHexString(data[i]);;
        if ((i != (data.length() - 1)))
            result += ':';
    }
    ui->inDataRaw->appendPlainText(timeMarker + result.toUpper());
}
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        QMessageBox::critical(this, tr("Critical Error"), _serialport.errorString());
        on_actionDisconnect_triggered();
    }
}

void MainWindow::on_clear_clicked()
{
    ui->inData->clear();
    ui->inDataRaw->clear();
}


void MainWindow::on_hex_toggled(bool checked)
{
    if (checked)
        ui->outData->setPlainText(textToHexText(ui->outData->toPlainText()));
}

void MainWindow::on_ascii_toggled(bool checked)
{
    if (checked)
        ui->outData->setPlainText(hexTextToText(ui->outData->toPlainText()));
}

void MainWindow::on_actionDF_Player_triggered()
{
    disconnect(&_serialport, &QSerialPort::readyRead, this, &MainWindow::readData);

    DF_Player *dfPlayer = new DF_Player(_serialport);
    dfPlayer->exec();
    delete dfPlayer;

    connect(&_serialport, &QSerialPort::readyRead, this, &MainWindow::readData);
}
