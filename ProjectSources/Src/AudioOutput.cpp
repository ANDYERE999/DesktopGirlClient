//
// Created by 666 on 25-6-1.
//

#include "AudioOutput.h"
#include <QDebug>

AudioOutput *AudioOutput::instance = nullptr;

AudioOutput *AudioOutput::getInstance()
{
    // 懒汉式
    if (instance == nullptr) {
        instance = new AudioOutput();
    }
    return instance;
}

AudioOutput::AudioOutput(QObject *parent) : QMediaPlayer(parent)
{
    audioBuffer = new QBuffer(this); // 初始化缓冲区
    
    // 创建并设置音频输出设备 (Qt6新要求)
    audioOutput = new QAudioOutput(this);
    audioOutput->setDevice(QMediaDevices::defaultAudioOutput());
    this->setAudioOutput(audioOutput);

    // 监听媒体状态变化 (Qt6中信号名称没有变化)
    connect(this, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            emit playbackFinished(); // 触发播放完成信号
        }
    });
}


AudioOutput::~AudioOutput()
{
    // 停止播放
    this->stopAudio();
    delete audioBuffer; // 释放缓冲区
}

/**
 * 设置音频文件路径
 * @param path
 */
void AudioOutput::setAudioPath(const QString &path)
{
    this->url = QUrl::fromLocalFile(path);
}

/**
 * 播放音频
 */
void AudioOutput::playAudio()
{
    if(this->url.isEmpty()){
        return;
    }
    this->setSource(this->url);  // Qt6: setMedia -> setSource
    this->play();
}

/**
 * 从字节数组中播放音频
 * @param wavData
 */
void AudioOutput::playFromByteArray(const QByteArray &wavData) {
    // 确保数据有效
    if (wavData.isEmpty()) {
        qWarning() << "尝试播放空音频数据";
        return;
    }

    // 重置缓冲区
    audioBuffer->close();
    audioBuffer->setData(wavData);
    audioBuffer->open(QIODevice::ReadOnly);

    // Qt6: 使用setSourceDevice代替setMedia
    this->setSourceDevice(audioBuffer);
    this->play();
}

/**
 * 暂停播放
 */
void AudioOutput::pauseAudio()
{
    this->pause();
}

/**
 * 停止播放
 */
void AudioOutput::stopAudio()
{
    this->stop();
}

/**
 * 设置播放速度
 * @param double
 */
void AudioOutput::setPlaySpeed(double speed)
{
    this->setPlaybackRate(speed);
}


/**
 * 获取播放速度
 * @return double
 */
double AudioOutput::getPlaySpeed()
{
    return this->playbackRate();
}


/**
 * 设置播放音量 (Qt6: 通过QAudioOutput设置)
 * @param int
 */
void AudioOutput::setPlayVolume(int volume)
{
    if (audioOutput) {
        // Qt6中音量范围是0.0-1.0，而不是0-100
        float normalizedVolume = static_cast<float>(volume) / 100.0f;
        audioOutput->setVolume(normalizedVolume);
    }
}

/**
 * 获取播放音量 (Qt6: 从QAudioOutput获取)
 * @return int
 */
int AudioOutput::getPlayVolume()
{
    if (audioOutput) {
        // 将0.0-1.0范围转换回0-100
        return static_cast<int>(audioOutput->volume() * 100.0f);
    }
    return 0;
}

/**
 * 获取当前播放位置（毫秒）
 * @return long long
 */
qint64 AudioOutput::getPlayPosition()
{
    // 首先要确定当前音频是否是正在播放
    if(this->playbackState() != QMediaPlayer::PlayingState){  // Qt6: state() -> playbackState()
        return 0;
    }
    return this->position();
}

/**
 * 获取音频时长（毫秒）
 * @return long long
 */
qint64 AudioOutput::getMediaDuration()
{
    if(this->playbackState() != QMediaPlayer::PlayingState){  // Qt6: state() -> playbackState()
        return 0;
    }
    return this->duration();
}

/**
 * 根据当前播放位置，返回播放进度百分比
 * @return double
 */
double AudioOutput::getPlayProgress()
{
    if(this->playbackState() != QMediaPlayer::PlayingState){  // Qt6: state() -> playbackState()
        return 0;
    }
    qint64 duration = this->getMediaDuration();
    if (duration <= 0) {
        return 0;
    }
    return static_cast<double>(this->getPlayPosition()) / static_cast<double>(duration);
}


/**
 * 获取播放状态 (Qt6: State -> PlaybackState)
 * QMediaPlayer::StoppedState、QMediaPlayer::PlayingState、QMediaPlayer::PausedState
 * @return QMediaPlayer::PlaybackState
 */
QMediaPlayer::PlaybackState AudioOutput::getState()
{
    return this->playbackState();  // Qt6: state() -> playbackState()
}

/**
 * 获取错误类型
 *  NoError,
    ResourceError,
    FormatError,
    NetworkError,
    AccessDeniedError
 * @return QMediaPlayer::Error
 */
QMediaPlayer::Error AudioOutput::getError()
{
    return this->error();
}

/**
 * 获取错误描述
 * @return QString
 */
QString AudioOutput::getErrorString()
{
    return this->errorString();
}


/**
 * 子线程创建时调用二次初始化方法
 */
void AudioOutput::initialize()
{
    // 进行一些设置之类的
}