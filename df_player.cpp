#include "df_player.h"
#include "ui_df_player.h"

DF_Player::DF_Player(QSerialPort &serial, QDialog *parent) :
    QDialog(parent),
    _serial(serial),
    ui(new Ui::DF_Player)
{
    ui->setupUi(this);

    _threshold = 100;
//    timoutTimer.setInterval(_threshold);
//    timoutTimer.setSingleShot(true);
    _debug = false;

    sendStack.start_byte = dfplayer::SB;
    sendStack.version    = dfplayer::VER;
    sendStack.length     = dfplayer::LEN;
    sendStack.end_byte   = dfplayer::EB;

    recStack.start_byte  = dfplayer::SB;
    recStack.version     = dfplayer::VER;
    recStack.length      = dfplayer::LEN;
    recStack.end_byte    = dfplayer::EB;

    connect(ui->play_pause, &QPushButton::clicked, this, &DF_Player::play);
    connect(ui->stop, &QPushButton::clicked, this, &DF_Player::stop);
    connect(ui->next, &QPushButton::clicked, this, &DF_Player::playNext);
    connect(ui->prev, &QPushButton::clicked, this, &DF_Player::playPrevious);

    connect(ui->vol_inc, &QPushButton::clicked, this, &DF_Player::incVolume);
    connect(ui->vol_dec, &QPushButton::clicked, this, &DF_Player::decVolume);

    updateData();
}

DF_Player::~DF_Player()
{
    delete ui;
}

/**************************************************************************/
 /*!
     @brief  Play the next song in chronological order.
 */
 /**************************************************************************/
void DF_Player::playNext()
{
    sendStack.commandValue  = dfplayer::NEXT;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Play the previous song in chronological order.
 */
 /**************************************************************************/
void DF_Player::playPrevious()
{
    sendStack.commandValue  = dfplayer::PREV;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
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
    sendStack.commandValue  = dfplayer::PLAY;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = (trackNum >> 8) & 0xFF;
    sendStack.paramLSB = trackNum & 0xFF;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Stop the current playback
 */
 /**************************************************************************/
void DF_Player::stop()
{
    sendStack.commandValue = dfplayer::STOP;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 0;

    findChecksum(sendStack);
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
    sendStack.commandValue  = dfplayer::USE_MP3_FOLDER;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = (trackNum >> 8) & 0xFF;
    sendStack.paramLSB = trackNum & 0xFF;

    findChecksum(sendStack);
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
    sendStack.commandValue  = dfplayer::INSERT_ADVERT;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = (trackNum >> 8) & 0xFF;
    sendStack.paramLSB = trackNum & 0xFF;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Stop the interrupting track.
 */
 /**************************************************************************/
void DF_Player::stopAdvertisement()
{
    sendStack.commandValue  = dfplayer::STOP_ADVERT;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 0;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Increment the volume by 1 out of 30.
 */
 /**************************************************************************/
void DF_Player::incVolume()
{
    sendStack.commandValue  = dfplayer::INC_VOL;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Decrement the volume by 1 out of 30.
 */
 /**************************************************************************/
void DF_Player::decVolume()
{
    sendStack.commandValue  = dfplayer::DEC_VOL;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Set the volume to a specific value out of 30.
     @param    volume
               The volume level (0 - 30).
 */
 /**************************************************************************/
void DF_Player::volume(uint8_t volume)
{
    if (volume <= 30)
    {
        sendStack.commandValue  = dfplayer::VOLUME;
        sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
        sendStack.paramMSB = 0;
        sendStack.paramLSB = volume;

        findChecksum(sendStack);
        sendData();
    }
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
    if (setting <= 5)
    {
        sendStack.commandValue  = dfplayer::EQ;
        sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
        sendStack.paramMSB = 0;
        sendStack.paramLSB = setting;

        findChecksum(sendStack);
        sendData();
    }
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
  sendStack.commandValue  = dfplayer::PLAYBACK_MODE;
  sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
  sendStack.paramMSB = (trackNum >> 8) & 0xFF;
  sendStack.paramLSB = trackNum & 0xFF;

  findChecksum(sendStack);
  sendData();
}

/**************************************************************************/
 /*!
     @brief  Specify the playback source.
     @param    source
               The playback source ID.
 */
 /**************************************************************************/
void DF_Player::playbackSource(uint8_t source)
{
    if ((source > 0) && (source <= 5))
    {
        sendStack.commandValue  = dfplayer::PLAYBACK_SRC;
        sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
        sendStack.paramMSB = 0;
        sendStack.paramLSB = source;

        findChecksum(sendStack);
        sendData();
    }
}

/**************************************************************************/
 /*!
     @brief  Put the MP3 player in standby mode (this is NOT sleep mode).
 */
 /**************************************************************************/
void DF_Player::standbyMode()
{
    sendStack.commandValue  = dfplayer::STANDBY;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Pull the MP3 player out of standby mode.
 */
 /**************************************************************************/
void DF_Player::normalMode()
{
    sendStack.commandValue  = dfplayer::NORMAL;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Reset all settings to factory default.
 */
 /**************************************************************************/
void DF_Player::reset()
{
    sendStack.commandValue  = dfplayer::RESET;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Resume playing current track.
 */
 /**************************************************************************/
void DF_Player::resume()
{
    sendStack.commandValue  = dfplayer::PLAYBACK;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Pause playing current track.
 */
 /**************************************************************************/
void DF_Player::pause()
{
    sendStack.commandValue  = dfplayer::PAUSE;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
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
    sendStack.commandValue  = dfplayer::SPEC_FOLDER;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = folderNum;
    sendStack.paramLSB = trackNum;

    findChecksum(sendStack);
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
    const uint16_t arg = (((uint16_t)folderNum) << 12) | (trackNum & 0xfff);

    sendStack.commandValue	= dfplayer::SPEC_TRACK_3000;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = arg >> 8;
    sendStack.paramLSB = arg & 0xff;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Specify volume gain.
     @param    gain
               The specified volume gain.
 */
 /**************************************************************************/
void DF_Player::volumeAdjustSet(uint8_t gain)
{
    if (gain <= 31)
    {
        sendStack.commandValue  = dfplayer::VOL_ADJ;
        sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
        sendStack.paramMSB = 0;
        sendStack.paramLSB = dfplayer::VOL_ADJUST + gain;

        findChecksum(sendStack);
        sendData();
    }
}

/**************************************************************************/
 /*!
     @brief  Play all tracks.
 */
 /**************************************************************************/
void DF_Player::startRepeatPlay()
{
    sendStack.commandValue  = dfplayer::REPEAT_PLAY;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = dfplayer::START_REPEAT;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Stop repeat play.
 */
 /**************************************************************************/
void DF_Player::stopRepeatPlay()
{
    sendStack.commandValue  = dfplayer::REPEAT_PLAY;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = dfplayer::STOP_REPEAT;

    findChecksum(sendStack);
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
    sendStack.commandValue  = dfplayer::REPEAT_FOLDER;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = (folder >> 8) & 0xFF;
    sendStack.paramLSB = folder & 0xFF;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Play all tracks in a random order.
 */
 /**************************************************************************/
void DF_Player::randomAll()
{
    sendStack.commandValue  = dfplayer::RANDOM_ALL;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 0;

    findChecksum(sendStack);
    sendData();
}


/**************************************************************************/
 /*!
     @brief  Repeat the current track.
 */
 /**************************************************************************/
void DF_Player::startRepeat()
{
    sendStack.commandValue  = dfplayer::REPEAT_CURRENT;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 0;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Stop repeat play of the current track.
 */
 /**************************************************************************/
void DF_Player::stopRepeat()
{
    sendStack.commandValue  = dfplayer::REPEAT_CURRENT;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Turn on DAC.
 */
 /**************************************************************************/
void DF_Player::startDAC()
{
    sendStack.commandValue  = dfplayer::SET_DAC;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 0;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Turn off DAC.
 */
 /**************************************************************************/
void DF_Player::stopDAC()
{
    sendStack.commandValue  = dfplayer::SET_DAC;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = 0;
    sendStack.paramLSB = 1;

    findChecksum(sendStack);
    sendData();
}

/**************************************************************************/
 /*!
     @brief  Put the MP3 player into sleep mode.
 */
 /**************************************************************************/
void DF_Player::sleep()
{
    playbackSource(dfplayer::SLEEP);
}


/**************************************************************************/
 /*!
     @brief  Pull the MP3 player out of sleep mode.
 */
 /**************************************************************************/
void DF_Player::wakeUp()
{
    playbackSource(dfplayer::TF);
}

/**************************************************************************/
 /*!
     @brief  Determine if a track is currently playing.
     @return True if a track is currently playing, false if not, -1 if error.
 */
 /**************************************************************************/
bool DF_Player::isPlaying()
{
    int16_t result = query(dfplayer::GET_STATUS_);

    if (result != -1)
        return (result & 1);

    return 0;
}

/**************************************************************************/
 /*!
     @brief  Determine the current volume setting.
     @return Volume level (0-30), -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::currentVolume()
{
    return query(dfplayer::GET_VOL);
}

/**************************************************************************/
 /*!
     @brief  Determine the current EQ setting.
     @return EQ setting, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::currentEQ()
{
    return query(dfplayer::GET_EQ);
}

/**************************************************************************/
 /*!
     @brief  Determine the current mode.
     @return Mode, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::currentMode()
{
    return query(dfplayer::GET_MODE);
}


/**************************************************************************/
 /*!
     @brief  Determine the current firmware version.
     @return Firmware version, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::currentVersion()
{
    return query(dfplayer::GET_VERSION);
}

/**************************************************************************/
 /*!
     @brief  Determine the number of tracks accessible via USB.
     @return Number of tracks accessible via USB, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::numUsbTracks()
{
    return query(dfplayer::GET_TF_FILES);
}

/**************************************************************************/
 /*!
     @brief  Determine the number of tracks accessible via SD card.
     @return Number of tracks accessible via SD card, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::numSdTracks()
{
    return query(dfplayer::GET_U_FILES);
}

/**************************************************************************/
 /*!
     @brief  Determine the number of tracks accessible via flash.
     @return Number of tracks accessible via flash, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::numFlashTracks()
{
    return query(dfplayer::GET_FLASH_FILES);
}

/**************************************************************************/
 /*!
     @brief  Determine the current track played via USB.
     @return Current track played via USB, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::currentUsbTrack()
{
    return query(dfplayer::GET_TF_TRACK);
}

/**************************************************************************/
 /*!
     @brief  Determine the current track played via SD card.
     @return Current track played via SD card, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::currentSdTrack()
{
    return query(dfplayer::GET_U_TRACK);
}

/**************************************************************************/
 /*!
     @brief  Determine the current track played via flash.
     @return Current track played via flash, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::currentFlashTrack()
{
    return query(dfplayer::GET_FLASH_TRACK);
}

/**************************************************************************/
 /*!
     @brief  Determine the number of tracks in the specified folder.
     @param    folder
               The folder number.
     @return Number of tracks in the specified folder, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::numTracksInFolder(uint8_t folder)
{
    return query(dfplayer::GET_FOLDER_FILES, (folder >> 8) & 0xFF, folder & 0xFF);
}

/**************************************************************************/
 /*!
     @brief  Determine the number of folders available.
     @return Number of folders available, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::numFolders()
{
    return query(dfplayer::GET_FOLDERS);
}

/**************************************************************************/
 /*!
     @brief  Set the timout value for MP3 player query responses.
     @param    threshold
               Number of ms allowed for the MP3 player to respond (timeout)
               to a query.
 */
 /**************************************************************************/
void DF_Player::setTimeout(unsigned long threshold)
{
    _threshold = threshold;
//    timoutTimer.setInterval(_threshold);
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
void DF_Player::findChecksum(stack& _stack)
{
    uint16_t checksum = (~(_stack.version + _stack.length + _stack.commandValue + _stack.feedbackValue + _stack.paramMSB + _stack.paramLSB)) + 1;

    _stack.checksumMSB = checksum >> 8;
    _stack.checksumLSB = checksum & 0xFF;
}

/**************************************************************************/
 /*!
     @brief  Send a config/command packet to the MP3 player.
 */
 /**************************************************************************/
void DF_Player::sendData()
{
    QByteArray data;
    data.push_back(sendStack.start_byte);
    data.push_back(sendStack.version);
    data.push_back(sendStack.length);
    data.push_back(sendStack.commandValue);
    data.push_back(sendStack.feedbackValue);
    data.push_back(sendStack.paramMSB);
    data.push_back(sendStack.paramLSB);
    data.push_back(sendStack.checksumMSB);
    data.push_back(sendStack.checksumLSB);
    data.push_back(sendStack.end_byte);

    _serial.write(data);

    if (_debug)
        qDebug() << data.toHex(':');
}

/**************************************************************************/
 /*!
     @brief  Clear out the serial input buffer connected to the MP3 player.
 */
 /**************************************************************************/
void DF_Player::flush()
{
    _serial.flush();
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
     @return Query response, -1 if error.
 */
 /**************************************************************************/
int16_t DF_Player::query(uint8_t cmd, uint8_t msb, uint8_t lsb)
{
    flush();

    sendStack.commandValue  = cmd;
    sendStack.feedbackValue = dfplayer::NO_FEEDBACK;
    sendStack.paramMSB = msb;
    sendStack.paramLSB = lsb;

    findChecksum(sendStack);
    sendData();
//    timeDot = QTime::currentTime();

    if (parseFeedback())
        if (recStack.commandValue != 0x40)
            return (recStack.paramMSB << 8) | recStack.paramLSB;

    return -1;
}

/**************************************************************************/
 /*!
     @brief  Parse MP3 player query responses.
     @return True if success, false if error.
 */
 /**************************************************************************/
bool DF_Player::parseFeedback()
{
    while (_serial.bytesAvailable())
    {
//        if (_serial.bytesAvailable())
//        {
            uint8_t recChar = _serial.read(1).at(0);

            if (_debug)
            {
                qDebug() << "Rec: " << Qt::hex << recChar;
                qDebug() << "State: ";
            }

            switch (state)
            {
            case find_start_byte:
            {
                if (_debug)
                    qDebug() << "find_start_byte";

                if (recChar == dfplayer::SB)
                {
                    recStack.start_byte = recChar;
                    state = find_ver_byte;
                }
                break;
            }
            case find_ver_byte:
            {
                if (_debug)
                    qDebug() << "find_ver_byte";

                if (recChar != dfplayer::VER)
                {
                    if (_debug)
                        qDebug() << "ver error";

                    state = find_start_byte;
                    return false;
                }

                recStack.version = recChar;
                state = find_len_byte;
                break;
            }
            case find_len_byte:
            {
                if (_debug)
                    qDebug() << "find_len_byte";

                if (recChar != dfplayer::LEN)
                {
                    if (_debug)
                        qDebug() << "len error";

                    state = find_start_byte;
                    return false;
                }

                recStack.length = recChar;
                state = find_command_byte;
                break;
            }
            case find_command_byte:
            {
                if (_debug)
                    qDebug() << "find_command_byte";

                recStack.commandValue = recChar;
                state = find_feedback_byte;
                break;
            }
            case find_feedback_byte:
            {
                if (_debug)
                    qDebug() << "find_feedback_byte";

                recStack.feedbackValue = recChar;
                state = find_param_MSB;
                break;
            }
            case find_param_MSB:
            {
                if (_debug)
                    qDebug() << "find_param_MSB";

                recStack.paramMSB = recChar;
                state = find_param_LSB;
                break;
            }
            case find_param_LSB:
            {
                if (_debug)
                    qDebug() << "find_param_LSB";

                recStack.paramLSB = recChar;
                state = find_checksum_MSB;
                break;
            }
            case find_checksum_MSB:
            {
                if (_debug)
                    qDebug() << "find_checksum_MSB";

                recStack.checksumMSB = recChar;
                state = find_checksum_LSB;
                break;
            }
            case find_checksum_LSB:
            {
                if (_debug)
                    qDebug() << "find_checksum_LSB";

                recStack.checksumLSB = recChar;

                int recChecksum  = (recStack.checksumMSB << 8) | recStack.checksumLSB;
                findChecksum(recStack);
                int calcChecksum = (recStack.checksumMSB << 8) | recStack.checksumLSB;

                if (recChecksum != calcChecksum)
                {
                    if (_debug)
                    {
                        qDebug() << "checksum error";
                        qDebug() << "recChecksum: 0x" << Qt::hex << recChecksum;
                        qDebug() << "calcChecksum: 0x" << Qt::hex << calcChecksum;
                    }

                    state = find_start_byte;
                    return false;
                }
                else
                    state = find_end_byte;
                break;
            }
            case find_end_byte:
            {
                if (_debug)
                    qDebug() << "find_end_byte";

                if (recChar != dfplayer::EB)
                {
                    if (_debug)
                        qDebug() << "eb error";

                    state = find_start_byte;
                    return false;
                }

                recStack.end_byte = recChar;
                state = find_start_byte;
                return true;
                break;
            }
            default:
                break;
            }
//        }

//        if ((QTime::currentTime().msecsSinceStartOfDay() - timeDot.msecsSinceStartOfDay()) > _threshold)
//        {
//            if (_debug)
//                qDebug() << "timeout error";

//            state = find_start_byte;
//            return false;
//        }
    }
}

/**************************************************************************/
 /*!
     @brief  Print the error description if an error has been received
 */
 /**************************************************************************/
void DF_Player::printError()
{
    if (recStack.commandValue == 0x40)
    {
        switch (recStack.paramLSB)
        {
        case 0x1:
        {
            qDebug() << "Module busy (this info is returned when the initialization is not done)";
            break;
        }
        case 0x2:
        {
            qDebug() << "Currently sleep mode(supports only specified device in sleep mode)";
            break;
        }
        case 0x3:
        {
            qDebug() << "Serial receiving error(a frame has not been received completely yet)";
            break;
        }
        case 0x4:
        {
            qDebug() << "Checksum incorrect";
            break;
        }
        case 0x5:
        {
            qDebug() << "Specified track is out of current track scope";
            break;
        }
        case 0x6:
        {
            qDebug() << "Specified track is not found";
            break;
        }
        case 0x7:
        {
            qDebug() << "Insertion error(an inserting operation only can be done when a track is being played)";
            break;
        }
        case 0x8:
        {
            qDebug() << "SD card reading failed(SD card pulled out or damaged)";
            break;
        }
        case 0xA:
        {
            qDebug() << "Entered into sleep mode";
            break;
        }
        default:
        {
            qDebug() << "Unknown error: " << recStack.paramLSB;
            break;
        }
        }
    }
    else
        qDebug() << "No error";
}

void DF_Player::updateData()
{
    int ms = 500;

    QThread::usleep(ms);
    ui->volume->setValue(currentVolume());
    QThread::usleep(ms);
    qDebug() << "currentVolume " << currentVolume();
    QThread::usleep(ms);
    qDebug() << "isPlaying " << isPlaying();
    QThread::usleep(ms);
    qDebug() << "current mode " << currentMode();
    QThread::usleep(ms);
    qDebug() << "current version " << currentVersion();
    QThread::usleep(ms);
    qDebug() << "numUsbTracks " << numUsbTracks();
    QThread::usleep(ms);
    qDebug() << "numSdTracks " << numSdTracks();
    QThread::usleep(ms);
    qDebug() << "numFlashTracks " << numFlashTracks();
    QThread::usleep(ms);
    qDebug() << "currentUsbTrack " << currentUsbTrack();
    QThread::usleep(ms);
    qDebug() << "currentSdTrack " << currentSdTrack();
    QThread::usleep(ms);
    qDebug() << "currentFlashTrack " << currentFlashTrack();
    QThread::usleep(ms);
    qDebug() << "numTracksInFolder 1" << numTracksInFolder(1);
    QThread::usleep(ms);
    qDebug() << "numFolders " << numFolders();
    QThread::usleep(ms);
}

void DF_Player::on_update_clicked()
{
    updateData();
}

