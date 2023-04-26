#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>
#include <QSerialPort>
#include <QTime>
#include <df_player.h>

#include "settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAboutQt_triggered();
    void on_actionQuit_triggered();
    void on_actionUART_triggered();
    void on_actionConnect_triggered();

    void on_actionDisconnect_triggered();

    void outDataTextChanged();

    void on_endString_currentIndexChanged(int index);

    void write();
    void readData();
    void handleError(QSerialPort::SerialPortError error);

    void on_clear_clicked();

    void on_hex_toggled(bool checked);

    void on_ascii_toggled(bool checked);

    void on_actionDF_Player_triggered();

private:
    Ui::MainWindow *ui;

    SettingsDialog _settingDialog;
    QSerialPort _serialport;
    QLabel _statusLabel;

    void showStatusMessage(const QString &message);

    QString byteToHexString(uint8_t ch) const;
    QString textToHexText(const QString &str, QString delim = "") const;
    QString hexTextToText(const QString &str) const;

    void disableAction(bool state);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H
