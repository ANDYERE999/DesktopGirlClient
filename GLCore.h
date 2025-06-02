#pragma once

#include <QtWidgets/QWidget>
#include <QOpenGLWidget>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QMediaRecorder>
#include <QAudioInput>
#include <QMediaCaptureSession>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QProcess>
#include <map>
#include <string>
#include "ProjectSources/Inc/AudioOutput.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include "SpeechBubbleWidget.h"

// 热键配置结构体
struct HotkeyConfig {
    QString expressionFile;   // 表情文件名（如 "Happy1.exp3.json"）
    QString trigger1;         // 主要触发键
    QString trigger2;         // 次要触发键
    QString trigger3;         // 第三触发键
    bool isActive;           // 是否激活
};

// 按钮状态枚举
enum class Button1State {
    Hide,  // 1_hide.ico
    Show   // 1_show.ico
};

enum class Button2State {
    ChatMode,       // 2_chat_mode.ico
    MicrophoneMode, // 2_microphone_mode.ico
    CommandMode     // 2_command_mode.ico
};

enum class MicrophoneState {
    Start,  // microphone_Start.png
    Stop    // microphone_Stop.png
};

class GLCore : public QOpenGLWidget
{
    Q_OBJECT

public:
    GLCore(int width, int height, QWidget* parent = nullptr);
    ~GLCore();

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    // 重写函数
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void resizeEvent(QResizeEvent* event) override;  // 添加resize事件处理
    void keyPressEvent(QKeyEvent* event) override;   // 添加键盘事件处理

private slots:
    void updateFavorability(); // 更新好感度的槽函数
    void onSendButtonClicked(); // 发送按钮点击槽函数
    void onTextChanged();       // 文本改变槽函数
    void onResponseTimeout();   // 响应超时槽函数
    
    // 新增的按钮和麦克风相关槽函数
    void onButton1Clicked();    // 按钮1点击槽函数
    void onButton2Clicked();    // 按钮2点击槽函数
    void onButton3Clicked();    // 按钮3点击槽函数
    void onMicrophoneClicked(); // 麦克风按钮点击槽函数
    void onRecordingTimeout();  // 录音超时槽函数
    
    // 对话气泡相关槽函数
    void onBubbleDisplayTimeout(); // 对话气泡显示超时槽函数
    void onBubbleFadeUpdate();     // 对话气泡淡入淡出更新槽函数

private:
    bool isLeftPressed; // 鼠标左键是否按下
    bool isRightPressed;// 鼠标右键是否按下
    QPoint currentPos;  // 当前鼠标位置

    // 边框显示状态
    bool isFrameless;   // 是否无边框状态

    // 初始窗口大小
    int initialWidth;
    int initialHeight;
    
    // 好感度相关
    QProgressBar* favorabilityBar;  // 好感度进度条
    QTimer* favorabilityTimer;     // 定时器
    int currentFavorability;       // 当前好感度值
    
    // 对话框相关
    QFrame* chatContainer;         // 对话框容器
    QTextEdit* inputTextEdit;      // 文本输入框
    QPushButton* sendButton;       // 发送按钮
    QHBoxLayout* chatLayout;       // 对话框布局
    bool isReacted;                // 是否已反应（控制对话框显示）
    bool debugChatPosition;        // 是否启用对话框位置调试
    
    // 超时机制相关
    QTimer* timeoutTimer;          // 超时定时器
    QLabel* timeoutLabel;          // 超时提示标签
    bool isWaitingForResponse;     // 是否正在等待响应
    
    // 左侧按钮栏相关
    QFrame* buttonContainer;       // 按钮容器
    QVBoxLayout* buttonLayout;     // 按钮布局
    QPushButton* button1;          // 按钮1（隐藏/显示）
    QPushButton* button2;          // 按钮2（模式切换）
    QPushButton* button3;          // 按钮3（指南）
    Button1State button1State;     // 按钮1状态
    Button2State button2State;     // 按钮2状态
    bool componentsVisible;        // 其他组件是否可见
    
    // 麦克风组件相关
    QFrame* microphoneContainer;   // 麦克风容器
    QPushButton* microphoneButton; // 麦克风按钮
    MicrophoneState microphoneState; // 麦克风状态
    QMediaRecorder* mediaRecorder; // 媒体录音器
    QAudioInput* audioInput;       // 音频输入
    QMediaCaptureSession* captureSession; // 捕获会话
    QTimer* recordingTimer;        // 录音计时器
    bool isRecording;              // 是否正在录音
    
    // 热键系统相关
    std::map<QString, HotkeyConfig> hotkeyConfigs;  // 键盘映射配置
    QString currentModelDirectory;                   // 当前模型目录
    bool isHotkeySystemReady;                       // 热键系统是否准备就绪
    
    // 音频监测相关
    AudioOutput* audioPlayer;                       // 音频播放器实例
    
    // 指令模式相关
    QList<QMediaPlayer*> musicPlayers;             // 音乐播放器列表（支持多个同时播放）
    QList<QAudioOutput*> audioOutputs;             // 音频输出设备列表
    
    // 对话气泡相关
    SpeechBubbleWidget* speechBubbleWidget;         // 对话气泡Widget
    QTimer* bubbleDisplayTimer;                     // 对话气泡显示计时器
    QTimer* bubbleFadeTimer;                        // 对话气泡淡入淡出计时器
    bool isBubbleVisible;                           // 对话气泡是否可见
    int bubbleFadeStep;                             // 淡入淡出步骤（0-10）
    bool isFadingIn;                                // 是否正在淡入
    
    // 私有方法
    int readFavorabilityFromConfig(); // 从配置文件读取好感度
    void setupFavorabilityUI();      // 设置好感度UI
    void updateProgressBarStyle();   // 更新进度条样式
    void triggerExpression(const QString& expressionName); // 触发表情
    void initKeyMapping();           // 初始化键盘映射
    
    // 对话框相关方法
    void setupChatUI();              // 设置对话框UI
    void updateChatUIStyle();        // 更新对话框样式
    void adjustInputHeight();        // 调整输入框高度
    void sendMessage();              // 发送消息
    void sendCommand();              // 发送指令（command模式）
    void updateInputPlaceholder();   // 更新输入框提示文字
    bool readIsReactedFromConfig();  // 从配置文件读取isReacted状态
    void updateConfigIsReacted(bool reacted); // 更新配置文件的isReacted值
    void updateChatPosition();       // 更新对话框位置
    
    // 指令系统相关方法
    void parseAndExecuteCommand(const QString& command); // 解析并执行指令
    void executePlayMusicCommand(const QStringList& args); // 执行音乐播放指令
    void executeScriptCommand(const QStringList& args);    // 执行脚本指令
    void showCommandError(const QString& message);         // 显示指令错误信息
    void stopAllMusic();                                   // 停止所有音乐播放
    QMediaPlayer* createNewMusicPlayer();                  // 创建新的音乐播放器
    
    // 超时机制相关方法
    void setupTimeoutUI();           // 设置超时UI
    void startResponseTimeout();     // 开始响应超时计时
    void handleResponseTimeout();    // 处理响应超时
    void hideTimeoutMessage();       // 隐藏超时消息
    
    // 新的热键系统方法
    bool loadVTubeHotkeyConfig(const QString& modelDirectory);  // 加载VTube热键配置
    bool loadExpressionFile(const QString& modelDirectory, const QString& expressionFile, const QString& expressionName); // 手动加载表情文件
    QString mapQtKeyToVTubeKey(int qtKey);                       // Qt按键映射到VTube按键格式
    void printHotkeyMappings();                                  // 打印热键映射信息
    void detectAndInitializeModel();                            // 检测并初始化模型
    
    // 音频监测相关方法
    void checkAndPlayAudioFile();                               // 检查并播放音频文件
    void cleanupTemporaryAudioFiles();                          // 清理临时音频文件
    
    // 新增的UI设置方法
    void setupButtonUI();            // 设置按钮UI
    void setupMicrophoneUI();        // 设置麦克风UI
    void updateButtonContainer();    // 更新按钮容器位置
    void updateMicrophoneContainer(); // 更新麦克风容器位置
    void updateComponentsVisibility(); // 更新组件可见性
    void playClickSound();           // 播放点击音效
    
    // 对话气泡相关方法
    void setupSpeechBubbleUI();      // 设置对话气泡UI
    void checkAndShowSpeechBubble(); // 检查并显示对话气泡
    void showSpeechBubble(const QString& text, int displayTime); // 显示对话气泡
    void hideSpeechBubble();         // 隐藏对话气泡
    void updateBubbleFade();         // 更新气泡淡入淡出效果
    void deleteSpeechBubbleFile();   // 删除对话气泡文件
    
    // 按钮状态更新方法
    void updateButton1Icon();        // 更新按钮1图标
    void updateButton2Icon();        // 更新按钮2图标
    void updateMicrophoneIcon();     // 更新麦克风图标
    
    // 录音相关方法
    void setupRecording();           // 设置录音
    void startRecording();           // 开始录音
    void stopRecording();            // 停止录音
    void finishRecording();          // 完成录音处理
    
    // 圆形图标创建方法
    QIcon createCircularIcon(const QString& iconPath, int size); // 创建圆形蒙版图标
};