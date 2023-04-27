#ifndef DF_PLAYER_H
#define DF_PLAYER_H

#include "qdialog.h"
#include <QDialog>
#include <QSerialPort>
#include <QTime>
#include <QThread>

namespace Ui {
class DF_Player;
}

/* Packet Values */
#define STACK_SIZE              10   // total number of bytes in a stack/packet (same for cmds and queries)
#define SB                      0x7E // start byte
#define VER                     0xFF // version
#define LEN                     0x06 // number of bytes after "LEN" (except for checksum data and EB)
#define FEEDBACK                1    // feedback requested
#define NO_FEEDBACK             0    // no feedback requested
#define EB                      0xEF // end byte

/* Control Commands */
#define CTRL_NEXT               0x01
#define CTRL_PREV               0x02
#define CTRL_SPEC_PLAY          0x03 // Specify playback of a track(in the root directory of a storage device)
#define CTRL_INC_VOL            0x04
#define CTRL_DEC_VOL            0x05
#define CTRL_VOLUME             0x06
#define CTRL_EQ                 0x07
#define CTRL_PLAYBACK_MODE      0x08 // Specify single repeat playback
#define CTRL_PLAYBACK_SRC       0x09 // Specify playback of a device(USB/SD)
#define CTRL_SLEEP              0x0A
//#define NORMAL           0x0B
#define CTRL_RESET              0x0C
#define CTRL_PLAY               0x0D
#define CTRL_PAUSE              0x0E
#define CTRL_SPEC_FOLDER        0x0F // Specify playback a track in a folder
#define CTRL_AUDIO_AMPL         0x10 // Audio amplification setting
#define CTRL_REPEAT_PLAY        0x11 // Set all repeat playback
#define CTRL_SPEC_PLAY_MP3      0x12 // Specify playback of folder named “MP3”
#define CTRL_INSERT_ADVERT      0x13 // Insert an advertisement
#define CTRL_SPEC_TRACK_3000    0x14 // Specify playback a track in a folder that supports 3000 tracks
#define CTRL_STOP_ADVERT        0x15
#define CTRL_STOP               0x16
#define CTRL_REPEAT_FOLDER      0x17 // Specify repeat playback of a folder
#define CTRL_RANDOM_ALL         0x18
#define CTRL_REPEAT_CURRENT     0x19
#define CTRL_SET_DAC            0x1A

/* Query Command */
#define CMD_CUR_DIVE_ONLINE     0x3F // Query current online storage device
#define CMD_ERROR               0x40 // Module returns an error data with this command
#define CMD_FEEDBACK            0x41 // Module reports a feedback with this command
#define CMD_STATUS              0x42 // Query current status
#define CMD_VOLUME              0x43 // Query current volume
#define CMD_EQ                  0x44 // Query current EQ
//#define GET_MODE         0x45 // !!!
//#define GET_VERSION      0x46 // !!!
#define CMD_USB_FILES           0x47 // Query number of tracks in the root of USB flash drive
#define CMD_SD_FILES            0x48 // Query number of tracks in the root of micro SD card
//#define GET_FLASH_FILES  0x49 // !!!
//#define KEEP_ON          0x4A // !!!
#define CMD_USB_TRACK           0x4B // Query current track in the USB flash drive
#define CMD_SD_TRACK            0x4C // Query current track in the micro SD Card
//#define GET_FLASH_TRACK   0x4D //!!!
#define CMD_FOLDER_FILES        0x4E // Query number of tracks in a folder
#define CMD_FOLDERS             0x4F // Query number of folders in the current storage device

/** EQ Values */
#define EQ_NORMAL        0
#define EQ_POP           1
#define EQ_ROCK          2
#define EQ_JAZZ          3
#define EQ_CLASSIC       4
#define EQ_BASE          5

/** Mode Values */
//#define REPEAT           0
//#define FOLDER_REPEAT    1
//#define SINGLE_REPEAT    2
//#define RANDOM           3

///** Playback Source Values */
//#define U                1
//#define TF               2
//#define AUX              3
//#define SLEEP            4
//#define FLASH            5

/** Base Volume Adjust Value */
//#define VOL_ADJUST       0x10

/** Repeat Play Values */
//#define STOP_REPEAT      0
//#define START_REPEAT     1

#define start_byte      0
#define version         1
#define length          2
#define commandValue    3
#define feedbackValue   4
#define paramMSB        5
#define paramLSB        6
#define checksumMSB     7
#define checksumLSB     8
#define end_byte        9


class DF_Player : public QDialog
{
    Q_OBJECT

public:
    explicit DF_Player(QSerialPort &serial, QDialog *parent = nullptr);
    ~DF_Player();

    uint8_t sendDataArr, recDataArr;

    bool _debug;

    void playNext();
    void playPrevious();
    void play(uint16_t trackNum);
    void stop();
    void playFromMP3Folder(uint16_t trackNum);
    void playAdvertisement(uint16_t trackNum);
    void stopAdvertisement();
    void incVolume();
    void decVolume();
    void volume(uint8_t volume);
    void EQSelect(uint8_t setting);
    void loop(uint16_t trackNum);
    void playbackSource(uint8_t source);
    void standbyMode();
    void normalMode();
    void reset();
    void resume();
    void pause();
    void playFolder(uint8_t folderNum, uint8_t trackNum);
    void playLargeFolder(uint8_t folderNum, uint16_t trackNum);
    void volumeAdjustSet(uint8_t gain);
    void startRepeatPlay();
    void stopRepeatPlay();
    void repeatFolder(uint16_t folder);
    void randomAll();
    void startRepeat();
    void stopRepeat();
    void startDAC();
    void stopDAC();
    void sleep();
    void wakeUp();

    bool isPlaying();
    int16_t currentVolume();
    int16_t currentEQ();
    int16_t currentMode();
    int16_t currentVersion();
    int16_t numUsbTracks();
    int16_t numSdTracks();
    int16_t numFlashTracks();
    int16_t currentUsbTrack();
    int16_t currentSdTrack();
    int16_t currentFlashTrack();
    int16_t numTracksInFolder(uint8_t folder);
    int16_t numFolders();

    void setTimeout(unsigned long threshold);
    void findChecksum(stack& _stack);
    void sendData();
    void flush();
    int16_t query(uint8_t cmd, uint8_t msb=0, uint8_t lsb=0);
    bool parseFeedback();

    void printError();

    void updateData();


private slots:
    void on_update_clicked();

    void on_volume_valueChanged(int value);

    void on_volume_sliderReleased();

    void on_reset_clicked();

private:
    QTime timeDot;
    int _threshold;

    /** MP3 response packet parsing states */
    enum fsm {
        find_start_byte,
        find_ver_byte,
        find_len_byte,
        find_command_byte,
        find_feedback_byte,
        find_param_MSB,
        find_param_LSB,
        find_checksum_MSB,
        find_checksum_LSB,
        find_end_byte
    };
    fsm state = find_start_byte;

    QSerialPort &_serial;
    Ui::DF_Player *ui;
};

#endif // DF_PLAYER_H
