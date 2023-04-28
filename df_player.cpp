#include "df_player.h"
#include "ui_df_player.h"

DF_Player::DF_Player(QSerialPort &serial, QDialog *parent) :
    QDialog(parent),
    _serial(serial),
    ui(new Ui::DF_Player),
    recDataBufferSize(0),
    num_folders(0)
{
    ui->setupUi(this);

    connect(ui->play_pause, &QPushButton::clicked, this, &DF_Player::play);
    connect(ui->stop, &QPushButton::clicked, this, &DF_Player::stop);
    connect(ui->next, &QPushButton::clicked, this, &DF_Player::playNext);
    connect(ui->prev, &QPushButton::clicked, this, &DF_Player::playPrevious);

    connect(ui->vol_inc, &QPushButton::clicked, this, &DF_Player::incVolume);
    connect(ui->vol_dec, &QPushButton::clicked, this, &DF_Player::decVolume);

    connect(&_serial, &QSerialPort::readyRead, this, &DF_Player::dataRecive);

    updateData();
}

DF_Player::~DF_Player()
{
    disconnect(&_serial, &QSerialPort::readyRead, this, &DF_Player::dataRecive);

    delete ui;
}

/**************************************************************************/
/*!
     @brief  Play the next song in chronological order.
 */
/**************************************************************************/
void DF_Player::playNext()
{
    sendDataBuffer[CMD_VALUE] = CTRL_NEXT;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play the previous song in chronological order.
 */
/**************************************************************************/
void DF_Player::playPrevious()
{
    sendDataBuffer[CMD_VALUE] = CTRL_PREV;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play a specific track.
     @param    trackNum
               The track number to play.
 */
/**************************************************************************/
void DF_Player::play(uint16_t trackNum)
{
    sendDataBuffer[CMD_VALUE] = CTRL_PLAY;
    sendDataBuffer[PARAM_MSB] = (trackNum >> 8);
    sendDataBuffer[PARAM_LSB] = trackNum;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Stop the current playback
 */
/**************************************************************************/
void DF_Player::stop()
{
    sendDataBuffer[CMD_VALUE] = CTRL_STOP;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 0;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play a specific track in the folder named "MP3".
     @param    trackNum
               The track number to play.
 */
/**************************************************************************/
void DF_Player::playFromMP3Folder(uint16_t trackNum)
{
    sendDataBuffer[CMD_VALUE] = CTRL_SPEC_PLAY_MP3;
    sendDataBuffer[PARAM_MSB] = (trackNum >> 8);
    sendDataBuffer[PARAM_LSB] = trackNum;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Interrupt the current track with a new track.
     @param    trackNum
               The track number to play.
 */
/**************************************************************************/
void DF_Player::playAdvertisement(uint16_t trackNum)
{
    sendDataBuffer[CMD_VALUE] = CTRL_INSERT_ADVERT;
    sendDataBuffer[PARAM_MSB] = (trackNum >> 8);
    sendDataBuffer[PARAM_LSB] = trackNum;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Stop the interrupting track.
 */
/**************************************************************************/
void DF_Player::stopAdvertisement()
{
    sendDataBuffer[CMD_VALUE] = CTRL_STOP_ADVERT;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 0;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Increment the volume by 1 out of 30.
 */
/**************************************************************************/
void DF_Player::incVolume()
{
    sendDataBuffer[CMD_VALUE] = CTRL_INC_VOL;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Decrement the volume by 1 out of 30.
 */
/**************************************************************************/
void DF_Player::decVolume()
{
    sendDataBuffer[CMD_VALUE] = CTRL_DEC_VOL;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Set the volume to a specific value out of 30.
     @param    volume
               The volume level (0 - 30).
 */
/**************************************************************************/
void DF_Player::volume(uint8_t value)
{
    if (value > 30)
        return;

    sendDataBuffer[CMD_VALUE] = CTRL_VOLUME;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = value;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Set the EQ mode.
     @param    setting
               The desired EQ ID.
 */
/**************************************************************************/
void DF_Player::EQSelect(uint8_t setting)
{
    if (setting > 5)
        return;

    sendDataBuffer[CMD_VALUE] = CTRL_EQ;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = setting;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Loop a specific track.
     @param    trackNum
               The track number to play.
 */
/**************************************************************************/
void DF_Player::loop(uint16_t trackNum)
{
    sendDataBuffer[CMD_VALUE] = CTRL_PLAYBACK_MODE;
    sendDataBuffer[PARAM_MSB] = (trackNum >> 8);
    sendDataBuffer[PARAM_LSB] = trackNum;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Specify the playback source.
     @param    source
               The playback source ID.
               1 - USB
               2 - SD
 */
/**************************************************************************/
void DF_Player::playbackSource(uint8_t source)
{
    if (source < 1 || source > 2)
        return;

    sendDataBuffer[CMD_VALUE] = CTRL_PLAYBACK_SRC;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = source;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Reset all settings to factory default.
 */
/**************************************************************************/
void DF_Player::reset()
{
    sendDataBuffer[CMD_VALUE] = CTRL_RESET;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Pause playing current track.
 */
/**************************************************************************/
void DF_Player::pause()
{
    sendDataBuffer[CMD_VALUE] = CTRL_PAUSE;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play a specific track from a specific folder.
     @param    folderNum
               The folder number.
     @param    trackNum
               The track number to play.
 */
/**************************************************************************/
void DF_Player::playFolder(uint8_t folderNum, uint8_t trackNum)
{
    sendDataBuffer[CMD_VALUE] = CTRL_SPEC_FOLDER;
    sendDataBuffer[PARAM_MSB] = folderNum;
    sendDataBuffer[PARAM_LSB] = trackNum;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play a specific track from a specific folder, where the track
             names are numbered 4 digit (e.g. 1234-mysong.mp3) and can be
             up to 3000. Only 15 folders ("01" to "15") are supported in this
             mode.
     @param    folderNum
               The folder number.
     @param    trackNum
               The track number to play.
 */
/**************************************************************************/
void DF_Player::playLargeFolder(uint8_t folderNum, uint16_t trackNum)
{
    const uint16_t arg = (((uint16_t)folderNum) << 12) | (trackNum & 0x0FFFF);
    sendDataBuffer[CMD_VALUE] = CTRL_SPEC_TRACK_3000;
    sendDataBuffer[PARAM_MSB] = arg >> 8;
    sendDataBuffer[PARAM_LSB] = arg;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Specify volume gain.
     @param    gain
               The specified volume gain.
               on - 1
               off - 2
 */
/**************************************************************************/
void DF_Player::volumeAdjustSet(uint8_t on_off, uint8_t gain)
{
    if (gain > 31 || on_off > 1)
        return;

    sendDataBuffer[CMD_VALUE] = CTRL_AUDIO_AMPL;
    sendDataBuffer[PARAM_MSB] = on_off;
    sendDataBuffer[PARAM_LSB] = gain;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play all tracks.
 */
/**************************************************************************/
void DF_Player::startRepeatPlay()
{
    sendDataBuffer[CMD_VALUE] = CTRL_REPEAT_PLAY;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Stop repeat play.
 */
/**************************************************************************/
void DF_Player::stopRepeatPlay()
{
    sendDataBuffer[CMD_VALUE] = CTRL_REPEAT_PLAY;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 0;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play all tracks in a given folder.
     @param    folderNum
               The folder number.
 */
/**************************************************************************/
void DF_Player::repeatFolder(uint16_t folder)
{
    if (folder > 99)
        return;

    sendDataBuffer[CMD_VALUE] = CTRL_REPEAT_FOLDER;
    sendDataBuffer[PARAM_MSB] = (folder >> 8);
    sendDataBuffer[PARAM_LSB] = folder;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Play all tracks in a random order.
 */
/**************************************************************************/
void DF_Player::randomAll()
{
    sendDataBuffer[CMD_VALUE] = CTRL_RANDOM_ALL;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 0;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Repeat the current track.
 */
/**************************************************************************/
void DF_Player::startRepeat()
{
    sendDataBuffer[CMD_VALUE] = CTRL_REPEAT_CURRENT;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 0;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Stop repeat play of the current track.
 */
/**************************************************************************/
void DF_Player::stopRepeat()
{
    sendDataBuffer[CMD_VALUE] = CTRL_REPEAT_CURRENT;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Turn on DAC.
 */
/**************************************************************************/
void DF_Player::startDAC()
{
    sendDataBuffer[CMD_VALUE] = CTRL_SET_DAC;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 0;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Turn off DAC.
 */
/**************************************************************************/
void DF_Player::stopDAC()
{
    sendDataBuffer[CMD_VALUE] = CTRL_SET_DAC;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 1;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Put the MP3 player into sleep mode.
 */
/**************************************************************************/
void DF_Player::sleep()
{
    sendDataBuffer[CMD_VALUE] = CTRL_SLEEP;
    sendDataBuffer[PARAM_MSB] = 0;
    sendDataBuffer[PARAM_LSB] = 0;
    sendData();
}

/**************************************************************************/
/*!
     @brief  Determine if a track is currently playing.
 */
/**************************************************************************/
void DF_Player::query_isPlaying()
{
    query(CMD_STATUS, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine the current volume setting.
 */

/**************************************************************************/
void DF_Player::query_currentVolume()
{
    query(CMD_VOLUME, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine the current EQ setting.
 */
/**************************************************************************/
void DF_Player::query_currentEQ()
{
    query(CMD_EQ, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine the number of tracks accessible via USB.
 */
/**************************************************************************/
void DF_Player::query_numUsbTracks()
{
    query(CMD_USB_FILES, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine the number of tracks accessible via SD card.
 */
/**************************************************************************/
void DF_Player::query_numSdTracks()
{
    query(CMD_SD_FILES, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine the current track played via USB.
 */
/**************************************************************************/
void DF_Player::query_currentUsbTrack()
{
    query(CMD_USB_TRACK, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine the current track played via SD card.
 */
/**************************************************************************/
void DF_Player::query_currentSdTrack()
{
    return query(CMD_SD_TRACK, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine the number of tracks in the specified folder.
     @param    folder
               The folder number.
 */
/**************************************************************************/
void DF_Player::query_numTracksInFolder(uint16_t folder)
{
    query(CMD_FOLDER_FILES, folder >> 8, folder);
}

/**************************************************************************/
/*!
     @brief  Determine the number of folders available.
 */
/**************************************************************************/
void DF_Player::query_numFolders()
{
    query(CMD_FOLDERS, 0, 0);
}

/**************************************************************************/
/*!
     @brief  Determine and insert the checksum of a given config/command
             packet into that same packet struct.
     @param    _stack
               Reference to a struct containing the config/command packet
               to calculate the checksum over.
 */
/**************************************************************************/
void DF_Player::calculateChecksum()
{
    uint16_t checksum = (~(sendDataBuffer[VERSION] + sendDataBuffer[LENGTH] + sendDataBuffer[CMD_VALUE] + sendDataBuffer[FEEDBAC_VALUE]
                           + sendDataBuffer[PARAM_MSB] + sendDataBuffer[PARAM_LSB])) + 1;

    sendDataBuffer[CHECKSUM_MSB] = checksum >> 8;
    sendDataBuffer[CHECKSUM_LSB] = checksum;
}

/**************************************************************************/
/*!
     @brief  Send a config/command packet to the MP3 player.
 */
/**************************************************************************/
void DF_Player::sendData()
{
    _serial.flush();
    QThread::msleep(100);
    sendDataBuffer[START_BYTE] = SB;
    sendDataBuffer[VERSION] = VER;
    sendDataBuffer[LENGTH] = LEN;
    sendDataBuffer[FEEDBAC_VALUE] = NO_FEEDBACK;
    calculateChecksum();
    sendDataBuffer[END_BYTE] = EB;

    _serial.write(reinterpret_cast<char *>(sendDataBuffer), BUFFER_SIZE);

    printf("Data send -> ");
    printBuff(sendDataBuffer, BUFFER_SIZE);
    printf("\n");
    fflush(stdout);
}

/**************************************************************************/
/*!
     @brief  Query the MP3 player for specific information.
     @param    cmd
               The command/query ID.
     @param    msb
               The payload/parameter MSB.
     @param    lsb
               The payload/parameter LSB.
 */
/**************************************************************************/
void DF_Player::query(uint8_t cmd, uint8_t msb, uint8_t lsb)
{
    sendDataBuffer[START_BYTE] = SB;
    sendDataBuffer[VERSION] = VER;
    sendDataBuffer[LENGTH] = LEN;
    sendDataBuffer[CMD_VALUE] = cmd;
    sendDataBuffer[FEEDBAC_VALUE] = NO_FEEDBACK;
    sendDataBuffer[PARAM_MSB] = msb;
    sendDataBuffer[PARAM_LSB] = lsb;
    calculateChecksum();
    sendDataBuffer[END_BYTE] = EB;

    sendData();
}

/**************************************************************************/
/*!
     @brief  Parse MP3 player query responses.
 */
/**************************************************************************/

void DF_Player::parseData()
{
    if (recDataBufferSize < BUFFER_SIZE)
        return;

    if (recDataBuffer[START_BYTE] != SB && recDataBuffer[END_BYTE] != EB)
    {
        printf("Recive unknow data. Data in rec buffer: ");
        printBuff(recDataBuffer, recDataBufferSize);

        // Смещаем буфер влево, отбрасывая первый байт
        memmove(recDataBuffer, &recDataBuffer[1], BUFFER_SIZE - 1);
        recDataBufferSize--;
        return;
    }

    printf("Data recive <- ");
    printBuff(recDataBuffer, recDataBufferSize);
    printf(" - ");

#define MSB recDataBuffer[PARAM_MSB]
#define LSB recDataBuffer[PARAM_LSB]

    switch (recDataBuffer[CMD_VALUE])
    {
    case CMD_CUR_DEV_ONLINE:
        if (LSB == 0x01)
            printf("USB flash drive online\n");
        else if (LSB == 0x02)
            printf("SD card online\n");
        else if (LSB == 0x03)
            printf("PC online\n");
        else if (LSB == 0x04)
            printf("USB flash drive and SD card online\n");
        break;
    case CMD_ERROR:
        printError();
        break;
    case CMD_FEEDBACK:
        if (MSB == 0 && LSB == 0)
            printf("Module has successfully received the command\n");
        break;
    case CMD_STATUS:
        if (MSB == 0 || MSB == 1)  // 1 || 2
        {
            printf("A track in %s is %s\n",
                   (MSB == 1) ? "USB" : "SD",
                   (LSB == 0) ? "stopped" : (LSB == 1) ? "playing" : "paused");
        }
        else if (MSB == 0x10)
            printf("Module in sleep mode\n");
        break;
    case CMD_VOLUME:
        printf("Volume is %d\n", LSB);
        curr_vol = LSB;
        ui->volume->setValue(LSB);
        break;
    case CMD_EQ:
        printf("EQ is %d\n", LSB);
        curr_eq = LSB;
        break;

    case CMD_USB_FILES:
        num_USB_tracks = ((uint16_t)MSB << 8) | LSB;
        printf("Files in USB: %d\n", num_USB_tracks);
        break;
    case CMD_SD_FILES:
        num_SD_tracks = ((uint16_t)MSB << 8) | LSB;
        printf("Files in SD: %d\n", num_SD_tracks);
        break;

    case CMD_USB_TRACK:
        curr_USB_track = ((uint16_t)MSB << 8) | LSB;
        printf("The track %d in USB being played\n", curr_USB_track);
        break;
    case CMD_SD_TRACK:
        curr_SD_track = ((uint16_t)MSB << 8) | LSB;
        printf("The track %d in SD being played\n", curr_SD_track);
        break;

    case CMD_FOLDER_FILES:
        printf("%d track in folder\n", ((uint16_t)MSB << 8) | LSB);
        break;
    case CMD_FOLDERS:
        num_folders = ((uint16_t)MSB << 8) | LSB;
        printf("%d folders in current device\n", num_folders);
        break;

    case CMD_DEV_PLUGGED:
        if (LSB == 1)
            printf("USB flash drive is plugged in\n");
        else if (LSB == 2)
            printf("SD card is plugged in\n");
        else if (LSB == 4)
            printf("USB cable connected to PC is plugged in\n");
        break;
    case CMD_DEV_PULL_OUT:
        if (LSB == 1)
            printf("USB flash drive is pulled out\n");
        else if (LSB == 2)
            printf("SD card is pulled out\n");
        else if (LSB == 4)
            printf("USB cable connected to PC is pulled out\n");
        break;
    case CMD_TRACK_FINSH_USB:
        printf("%d track is finished playing in USB flash drive\n", ((uint16_t)MSB << 8) | LSB);
        break;
    case CMD_TRACK_FINSH_SD:
        printf("%d track is finished playing in SD card\n", ((uint16_t)MSB << 8) | LSB);
        break;
    default:
        printf("In recive unknow data.\n");
    }

    fflush(stdout);

    recDataBufferSize = 0;

#undef MSB
#undef LSB
}

/**************************************************************************/
/*!
     @brief  Print the error description if an error has been received
 */
/**************************************************************************/
void DF_Player::printError()
{
    if (recDataBuffer[CMD_VALUE] == CMD_ERROR)
    {
        switch (recDataBuffer[PARAM_LSB])
        {
        case 0x1:
        {
            printf("Module busy (this info is returned when the initialization is not done)");
            break;
        }
        case 0x2:
        {
            printf("Currently sleep mode(supports only specified device in sleep mode)");
            break;
        }
        case 0x3:
        {
            printf("Serial receiving error(a frame has not been received completely yet)");
            break;
        }
        case 0x4:
        {
            printf("Checksum incorrect");
            break;
        }
        case 0x5:
        {
            printf("Specified track is out of current track scope");
            break;
        }
        case 0x6:
        {
            printf("Specified track is not found");
            break;
        }
        case 0x7:
        {
            printf("Insertion error(an inserting operation only can be done when a track is being played)");
            break;
        }
        case 0x8:
        {
            printf("SD card reading failed(SD card pulled out or damaged)");
            break;
        }
        case 0xA:
        {
            printf("Entered into sleep mode");
            break;
        }
        default:
        {
            printf("Unknown error: %d", recDataBuffer[PARAM_LSB]);
            break;
        }
        }
    }
    else
        printf("No error");

    printf("\n");
    fflush(stdout);
}

void DF_Player::printBuff(uint8_t *data, uint8_t size)
{
    for (int i = 0; i < size - 1; ++i)
        printf("%02X:", data[i]);
    printf("%02X", data[size - 1]);
}

void DF_Player::updateData()
{
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    query_isPlaying();
    query_currentVolume();
    query_currentEQ();
    query_numUsbTracks();
    query_numSdTracks();
    query_currentUsbTrack();
    query_currentSdTrack();
    query_numFolders();

    for (int i = 0; i <= num_folders; ++i)
    {
        query_numTracksInFolder(i);
    }


    QGuiApplication::restoreOverrideCursor();
}

void DF_Player::dataRecive()
{
    char byte;
    while (_serial.bytesAvailable())
    {
        _serial.read(&byte, 1);

        recDataBuffer[recDataBufferSize] = byte;
        recDataBufferSize++;
        if (recDataBufferSize == BUFFER_SIZE)
            parseData();
    }
}

void DF_Player::on_update_clicked()
{
    updateData();
}

void DF_Player::on_volume_valueChanged(int value)
{
    //    volume(ui->volume->value());
}


void DF_Player::on_volume_sliderReleased()
{
    qDebug() <<"slider - " << ui->volume->value();
    //    volume(ui->volume->value());
}


void DF_Player::on_reset_clicked()
{
    reset();
}


void DF_Player::on_update_2_clicked()
{
    playbackSource(2);
}

