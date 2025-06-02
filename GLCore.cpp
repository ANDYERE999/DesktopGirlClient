#include "LAppDelegate.hpp" // 必须要放在第一个，不然包老实的
#include "LAppView.hpp"
#include "LAppPal.hpp"
#include "LAppLive2DManager.hpp"
#include "LAppModel.hpp"
#include "LAppDefine.hpp"
#include "GLCore.h"
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QDebug>
#include <QTextStream>
#include <QDateTime>
#include <QPushButton>
#include <QTextEdit>
#include <QFontMetrics>
#include <QTextDocument>
#include <QLabel>
#include <QThread>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QMediaRecorder>
#include <QAudioInput>
#include <QMediaCaptureSession>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QPainterPath>
#include <QProcess>
#include <QRandomGenerator>
#include <QMediaPlayer>
#include <QAudioOutput>

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#endif

// 添加Live2D命名空间引用
using namespace Live2D::Cubism::Framework;

GLCore::GLCore(int w, int h, QWidget *parent)
    : QOpenGLWidget(parent),
      isLeftPressed(false),
      isRightPressed(false),
      isFrameless(true),  // 默认无边框
      initialWidth(w),
      initialHeight(h),
      currentFavorability(0),
      favorabilityBar(nullptr),
      favorabilityTimer(nullptr),
      isHotkeySystemReady(false),  // 初始化热键系统状态
      currentModelDirectory(""),  // 初始化模型目录
      chatContainer(nullptr),     // 初始化对话框容器
      inputTextEdit(nullptr),     // 初始化文本输入框
      sendButton(nullptr),        // 初始化发送按钮
      chatLayout(nullptr),        // 初始化对话框布局
      isReacted(true),            // 初始化为已反应状态
      debugChatPosition(true),    // 启用对话框位置调试（可根据需要调整）
      timeoutTimer(nullptr),      // 初始化超时定时器
      timeoutLabel(nullptr),      // 初始化超时标签
      isWaitingForResponse(false), // 初始化响应等待状态
      audioPlayer(nullptr),       // 初始化音频播放器
      // 新增的成员变量初始化
      buttonContainer(nullptr),   // 初始化按钮容器
      buttonLayout(nullptr),      // 初始化按钮布局
      button1(nullptr),           // 初始化按钮1
      button2(nullptr),           // 初始化按钮2
      button3(nullptr),           // 初始化按钮3
      button1State(Button1State::Hide), // 初始化按钮1状态
      button2State(Button2State::ChatMode), // 初始化按钮2状态
      componentsVisible(true),    // 初始化组件可见性
      microphoneContainer(nullptr), // 初始化麦克风容器
      microphoneButton(nullptr),  // 初始化麦克风按钮
      microphoneState(MicrophoneState::Start), // 初始化麦克风状态
      mediaRecorder(nullptr),     // 初始化媒体录音器
      audioInput(nullptr),        // 初始化音频输入
      captureSession(nullptr),    // 初始化捕获会话
      recordingTimer(nullptr),    // 初始化录音计时器
      isRecording(false),         // 初始化录音状态
      // 对话气泡相关变量初始化
      speechBubbleWidget(nullptr), // 初始化对话气泡Widget
      bubbleDisplayTimer(nullptr), // 初始化对话气泡显示计时器
      bubbleFadeTimer(nullptr),    // 初始化对话气泡淡入淡出计时器
      isBubbleVisible(false),      // 初始化对话气泡可见性
      bubbleFadeStep(0),           // 初始化淡入淡出步骤
      isFadingIn(false)            // 初始化淡入状态
{
    this->resize(w, h);
    setFocusPolicy(Qt::StrongFocus);  // 确保可以接收键盘事件
    
    qDebug() << "GLCore constructor: initial size" << w << "x" << h;
    qDebug() << "GLCore constructor: frameless=" << isFrameless;
    
    // 初始化音频播放器
    audioPlayer = AudioOutput::getInstance();
    qDebug() << "AudioOutput实例已初始化";
    
    // 清理上次运行遗留的临时音频文件
    cleanupTemporaryAudioFiles();
    
    // 设置OpenGL格式以支持透明度
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setAlphaBufferSize(8);  // 关键：设置alpha缓冲区
    format.setSamples(4);          // 抗锯齿
    setFormat(format);
    
    //this->setAttribute(Qt::WA_DeleteOnClose);       // 窗口关闭时自动释放内存
    this->setWindowFlag(Qt::FramelessWindowHint); // 设置无边框窗口
    this->setWindowFlag(Qt::WindowStaysOnTopHint); // 设置窗口始终在顶层
    //this->setWindowFlag(Qt::Tool); // 不在应用程序图标
    this->setAttribute(Qt::WA_TranslucentBackground); // 设置窗口背景透明
    
    // 设置窗口标题和图标
    this->setWindowTitle("布兰妮-v1");
    this->setWindowIcon(QIcon("asset/icon/app.ico"));
    
    // 初始化好感度UI
    setupFavorabilityUI();
    
    // 初始化对话框UI
    setupChatUI();
    
    // 初始化超时UI
    setupTimeoutUI();
    
    // 初始化按钮UI
    setupButtonUI();
    
    // 初始化麦克风UI
    setupMicrophoneUI();
    
    // 设置录音系统
    setupRecording();
    
    // 初始化对话气泡UI
    setupSpeechBubbleUI();
    
    // 不要在构造函数中初始化热键系统，等Live2D准备好再初始化
    qDebug() << "GLCore constructor: 延迟初始化热键系统，等待Live2D准备完成";
    
    // 程序启动时默认设置isReacted为true
    updateConfigIsReacted(true);
    isReacted = true;
    qDebug() << "程序启动：设置默认isReacted=true";

    // 显示窗口以获取有效的窗口句柄
    this->show();
    
    // 使用Windows DWM API设置透明背景
#ifdef _WIN32
    auto viewId = this->winId();
    DWM_BLURBEHIND bb = { 0 };
    HRGN hRgn = CreateRectRgn(0, 0, -1, -1); // 应用透明的矩形范围，整个窗口客户区
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = hRgn;
    bb.fEnable = TRUE;
    DwmEnableBlurBehindWindow((HWND)viewId, &bb);
    DeleteObject(hRgn); // 清理资源
#endif

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, [=]() {
        update();
        });
    timer->start((1.0 / 60) * 1000);    // 60FPS

    this->setMouseTracking(true);
}

GLCore::~GLCore()
{
    // 清理录音相关资源
    if (isRecording) {
        stopRecording();
    }
    
    if (mediaRecorder) {
        mediaRecorder->stop();
    }
    
    if (recordingTimer) {
        recordingTimer->stop();
    }
    
    // 清理音乐播放器
    stopAllMusic();
    for (QMediaPlayer* player : musicPlayers) {
        if (player) {
            player->deleteLater();
        }
    }
    musicPlayers.clear();
    
    for (QAudioOutput* output : audioOutputs) {
        if (output) {
            output->deleteLater();
        }
    }
    audioOutputs.clear();
    
    // 清理好感度相关资源
    if (favorabilityTimer) {
        favorabilityTimer->stop();
    }
    
    // 清理对话气泡相关资源
    if (bubbleDisplayTimer) {
        bubbleDisplayTimer->stop();
    }
    if (bubbleFadeTimer) {
        bubbleFadeTimer->stop();
    }
    // speechBubbleWidget会通过Qt的父子关系自动清理
    
    // Qt的父子关系会自动清理QProgressBar和QTimer等组件
}

void GLCore::mouseMoveEvent(QMouseEvent* event)
{
    LAppDelegate::GetInstance()->GetView()->OnTouchesMoved(event->position().x(), event->position().y());

    if (isLeftPressed) {
        this->move(event->pos() - this->currentPos + this->pos());
    }

    

    // 允许事件继续传递，将鼠标事件传递给主窗口，实现鼠标拖动无边框窗口
    event->ignore();
}

void GLCore::mousePressEvent(QMouseEvent* event)
{
    LAppDelegate::GetInstance()->GetView()->OnTouchesBegan(event->position().x(), event->position().y());

    if (event->button() == Qt::LeftButton) {
        this->isLeftPressed = true;
        this->currentPos = event->pos();
    }
    // 右键切换边框显示
    if (event->button() == Qt::RightButton) {
        // 切换边框状态
        isFrameless = !isFrameless;
        
        // 调试输出
        qDebug() << "Switching to frameless:" << isFrameless;
        
        if (isFrameless) {
            // 设置为无边框
            this->setWindowFlag(Qt::FramelessWindowHint, true);
            // 使用当前窗口大小而不是初始大小
            QSize currentSize = this->size();
            this->setFixedSize(currentSize.width(), currentSize.height());
        } else {
            // 显示边框
            this->setWindowFlag(Qt::FramelessWindowHint, false);
            // 移除固定大小限制，允许调整大小
            this->setMinimumSize(200, 150);  // 设置最小大小
            this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);  // 移除最大大小限制
        }
        
        // 重新显示窗口以应用更改
        this->show();
        
        // 在show()之后更新所有组件的可见性
        updateComponentsVisibility();
        
        this->isRightPressed = true;
    }

    // 允许事件继续传递
    event->ignore();
}

void GLCore::mouseReleaseEvent(QMouseEvent* event)
{
    LAppDelegate::GetInstance()->GetView()->OnTouchesEnded(event->position().x(), event->position().y());

    if (event->button() == Qt::LeftButton) {
        isLeftPressed = false;
    }
    if (event->button() == Qt::RightButton) {
        isRightPressed = false;
    }

    
    // 允许事件继续传递
    event->ignore();
}

void GLCore::initializeGL()
{
    // 设置OpenGL状态以支持透明度渲染
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);  // 禁用深度测试以确保透明度正常工作
    
    LAppDelegate::GetInstance()->Initialize(this);
    
    // Live2D系统初始化完成后，延迟一段时间再初始化热键系统
    QTimer::singleShot(1000, this, [this]() {
        qDebug() << "Live2D系统已准备好，开始初始化热键系统...";
        initKeyMapping();
    });
}

void GLCore::paintGL()
{
    // 设置完全透明背景以配合DWM透明效果
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // RGBA，A=0表示完全透明
    glClear(GL_COLOR_BUFFER_BIT);
    
    LAppDelegate::GetInstance()->update();
    
}

void GLCore::resizeGL(int w, int h)
{
    LAppDelegate::GetInstance()->resize(w, h);
}

void GLCore::resizeEvent(QResizeEvent* event)
{
    QOpenGLWidget::resizeEvent(event);
    
    // 调整进度条位置和大小
    if (favorabilityBar) {
        int margin = 5;
        favorabilityBar->setGeometry(margin, margin, width() - 2 * margin, 20);
    }
    
    // 调整按钮容器位置
    updateButtonContainer();
    
    // 调整对话框位置和大小
    if (chatContainer) {
        int margin = 10;
        int containerHeight = chatContainer->height();
        // 将对话框放置在窗口底部
        chatContainer->setGeometry(margin, height() - containerHeight - margin, 
                                   width() - 2 * margin, containerHeight);
        qDebug() << "对话框位置调整:" << chatContainer->geometry();
    }
    
    // 调整麦克风容器位置
    updateMicrophoneContainer();
    
    // 调整超时标签位置
    if (timeoutLabel && timeoutLabel->isVisible()) {
        int labelWidth = width() - 40;
        int labelX = 20;
        int labelY = height() / 4; // 屏幕中上部
        timeoutLabel->setGeometry(labelX, labelY, labelWidth, 40);
        qDebug() << "超时标签位置调整:" << timeoutLabel->geometry();
    }
    
    // 调整对话气泡位置
    if (speechBubbleWidget && speechBubbleWidget->isVisible()) {
        QSize bubbleSize = speechBubbleWidget->size();
        // 计算位置：左右居中，上下3/4处
        int x = (width() - bubbleSize.width()) / 2;
        int y = height() * 3 / 4 - bubbleSize.height() / 2;
        speechBubbleWidget->move(x, y);
        qDebug() << "对话气泡位置调整:" << x << y;
    }
}

void GLCore::wheelEvent(QWheelEvent* event)
{
    // 不再处理缩放，直接接受事件防止传递
    event->accept();
}

// 从配置文件读取好感度
int GLCore::readFavorabilityFromConfig() {
    QString configPath = QDir::currentPath() + "/contact/config.json";
    QFile file(configPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open config file:" << configPath;
        return currentFavorability; // 如果读取失败，返回当前值
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString();
        return currentFavorability; // 如果解析失败，返回当前值
    }
    
    QJsonObject obj = doc.object();
    int rawFavorability = obj["favorability"].toInt(0);
    
    // 强制限制在0-100范围内，提高鲁棒性
    int boundedFavorability = qBound(0, rawFavorability, 100);
    
    // 如果值被修正了，输出调试信息
    if (rawFavorability != boundedFavorability) {
        qDebug() << "Favorability value corrected: raw=" << rawFavorability 
                 << "corrected=" << boundedFavorability;
    }
    
    return boundedFavorability;
}

// 设置好感度UI
void GLCore::setupFavorabilityUI() {
    // 创建好感度进度条作为GLCore的子widget
    favorabilityBar = new QProgressBar(this);  // 重要：设置this作为父对象
    favorabilityBar->setRange(0, 100);
    favorabilityBar->setValue(0);
    favorabilityBar->setFixedHeight(20);
    favorabilityBar->setTextVisible(true);
    favorabilityBar->setFormat("好感度: %p%");
    
    // 设置进度条样式
    updateProgressBarStyle();
    
    // 初始状态下隐藏进度条（因为默认是无边框）
    favorabilityBar->setVisible(false);
    qDebug() << "Initial setup: frameless=" << isFrameless << ", progress bar visible=" << favorabilityBar->isVisible();
    
    // 创建并启动定时器
    favorabilityTimer = new QTimer(this);
    connect(favorabilityTimer, &QTimer::timeout, this, &GLCore::updateFavorability);
    favorabilityTimer->start(1000); // 每1秒更新一次
    
    // 立即读取一次好感度
    updateFavorability();
    
    // 设置进度条的初始位置
    int margin = 5;
    favorabilityBar->setGeometry(margin, margin, width() - 2 * margin, 20);
}

// 更新进度条样式
void GLCore::updateProgressBarStyle() {
    if (!favorabilityBar) return;
    
    QString style = R"(
        QProgressBar {
            border: 2px solid #555555;
            border-radius: 10px;
            background-color: #CC4444;
            text-align: center;
            color: white;
            font-weight: bold;
            font-size: 12px;
            padding: 0px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                                       stop:0 #88DD88, stop:1 #66BB66);
            border-radius: 8px;
            margin: 0px;
            border: none;
        }
    )";
    
    favorabilityBar->setStyleSheet(style);
}

// 更新好感度槽函数
void GLCore::updateFavorability() {
    int newFavorability = readFavorabilityFromConfig();
    if (newFavorability != currentFavorability) {
        currentFavorability = newFavorability;
        if (favorabilityBar) {
            favorabilityBar->setValue(currentFavorability);
        }
    }
    
    // 检查isReacted状态并控制对话框显示
    bool newIsReacted = readIsReactedFromConfig();
    if (newIsReacted != isReacted) {
        isReacted = newIsReacted;
        qDebug() << "isReacted状态变化:" << isReacted;
        
        // 如果从等待响应状态变为已响应，停止超时计时
        if (isWaitingForResponse && isReacted) {
            if (timeoutTimer && timeoutTimer->isActive()) {
                timeoutTimer->stop();
                qDebug() << "收到AI响应，停止超时计时";
            }
            isWaitingForResponse = false;
        }
        
        // 更新组件可见性
        updateComponentsVisibility();
    }
    
    // 持续监控并调整对话框位置 - 每秒更新一次
    updateChatPosition();
    
    // 检查并播放音频文件 - 每秒监测一次
    checkAndPlayAudioFile();
    
    // 检查并显示对话气泡 - 每秒监测一次
    checkAndShowSpeechBubble();
}

// 新增方法：更新对话框位置
void GLCore::updateChatPosition() {
    if (!chatContainer) return;
    
    int margin = 10;
    int containerHeight = chatContainer->height();
    QRect newGeometry(margin, height() - containerHeight - margin, 
                     width() - 2 * margin, containerHeight);
    
    // 检查是否需要调整位置
    if (chatContainer->geometry() != newGeometry) {
        chatContainer->setGeometry(newGeometry);
        if (debugChatPosition) {
            qDebug() << "对话框位置调整:" << newGeometry;
        }
    }
    
    // 提供详细的状态调试信息（仅当启用调试时）
    if (debugChatPosition) {
        static int debugCounter = 0;
        debugCounter++;
        
        // 每10秒输出一次详细状态（因为定时器是每秒触发）
        if (debugCounter % 10 == 0) {
            qDebug() << "=== 对话框状态监控 ===";
            qDebug() << "窗口大小:" << width() << "x" << height();
            qDebug() << "对话框位置:" << chatContainer->geometry();
            qDebug() << "对话框可见:" << chatContainer->isVisible();
            qDebug() << "边框模式:" << (!isFrameless ? "有边框" : "无边框");
            qDebug() << "isReacted:" << isReacted;
            qDebug() << "输入框高度:" << (inputTextEdit ? inputTextEdit->height() : 0);
            qDebug() << "容器高度:" << containerHeight;
            qDebug() << "========================";
        }
    }
}

// 检测并初始化模型 - 增加配置文件检测
void GLCore::detectAndInitializeModel() {
    // 首先检查config.json中指定的模型
    QString configPath = QDir::currentPath() + "/contact/config.json";
    QFile file(configPath);
    QString primaryModel = "Nahida_1080"; // 默认值
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            primaryModel = obj["modelName"].toString("Nahida_1080");
        }
    }
    
    // 优先检测配置文件中指定的模型
    QString primaryDir = "Resources/" + primaryModel;
    QString primaryVtubeConfig = QDir::currentPath() + "/" + primaryDir + "/" + primaryModel + ".vtube.json";
    
    // 设置当前模型目录为指定的模型
    currentModelDirectory = primaryDir;
    
    if (QFile::exists(primaryVtubeConfig)) {
        qDebug() << "发现配置指定的模型配置:" << primaryVtubeConfig;
        
        if (loadVTubeHotkeyConfig(primaryDir)) {
            isHotkeySystemReady = true;
            printHotkeyMappings();
            qDebug() << "热键系统初始化成功!";
            return;
        }
    }
    
    // 如果指定模型没有热键配置，输出无配置信息，不再回退到备用模型
    qDebug() << "指定模型" << primaryModel << "没有热键配置";
    isHotkeySystemReady = false;
    hotkeyConfigs.clear(); // 确保清空配置
    printHotkeyMappings(); // 输出"无热键配置"信息
}

// 加载VTube热键配置
bool GLCore::loadVTubeHotkeyConfig(const QString& modelDirectory) {
    QString modelName = QFileInfo(modelDirectory).baseName();
    QString vtubeConfigPath = QDir::currentPath() + "/" + modelDirectory + "/" + modelName + ".vtube.json";
    
    QFile file(vtubeConfigPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开VTube配置文件:" << vtubeConfigPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "VTube配置文件JSON解析错误:" << error.errorString();
        return false;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray hotkeys = rootObj["Hotkeys"].toArray();
    
    if (hotkeys.isEmpty()) {
        qDebug() << "VTube配置文件中没有找到热键配置";
        return false;
    }
    
    // 清除现有配置
    hotkeyConfigs.clear();
    
    // 解析热键配置
    int successCount = 0;
    for (const QJsonValue& value : hotkeys) {
        QJsonObject hotkey = value.toObject();
        
        // 只处理表情热键
        if (hotkey["Action"].toString() != "ToggleExpression") {
            continue;
        }
        
        QString expressionFile = hotkey["File"].toString();
        if (expressionFile.isEmpty()) {
            continue;
        }
        
        QJsonObject triggers = hotkey["Triggers"].toObject();
        QString trigger1 = triggers["Trigger1"].toString();
        QString trigger2 = triggers["Trigger2"].toString();
        QString trigger3 = triggers["Trigger3"].toString();
        bool isActive = hotkey["IsActive"].toBool(true);
        
        if (!trigger1.isEmpty() && isActive) {
            HotkeyConfig config;
            config.expressionFile = expressionFile;
            config.trigger1 = trigger1;
            config.trigger2 = trigger2;
            config.trigger3 = trigger3;
            config.isActive = isActive;
            
            hotkeyConfigs[trigger1] = config;
            
            // 加载对应的表情文件
            QString expressionName = QFileInfo(expressionFile).baseName();
            if (loadExpressionFile(modelDirectory, expressionFile, expressionName)) {
                successCount++;
            }
        }
    }
    
    qDebug() << "成功加载" << successCount << "个表情热键配置";
    return successCount > 0;
}

// 手动加载表情文件 - 使用AddExpressionManually正确存储表情
bool GLCore::loadExpressionFile(const QString& modelDirectory, const QString& expressionFile, const QString& expressionName) {
    QString expressionPath = QDir::currentPath() + "/" + modelDirectory + "/" + expressionFile;
    
    if (!QFile::exists(expressionPath)) {
        qDebug() << "表情文件不存在:" << expressionPath;
        return false;
    }
    
    QFile file(expressionPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开表情文件:" << expressionPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    // 获取Live2D模型并手动加载表情 - 增加安全检查
    auto* manager = LAppLive2DManager::GetInstance();
    if (!manager) {
        qDebug() << "Live2D管理器不可用";
        return false;
    }
    
    if (manager->GetModelNum() == 0) {
        qDebug() << "Live2D模型未加载，模型数量:" << manager->GetModelNum();
        return false;
    }
    
    auto* model = manager->GetModel(0);
    if (!model) {
        qDebug() << "Live2D模型指针为空";
        return false;
    }
    
    try {
        // 使用Live2D框架直接加载表情，正确使用命名空间
        ACubismMotion* motion = model->LoadExpression(reinterpret_cast<const csmByte*>(data.data()), 
                                                      data.size(), 
                                                      expressionName.toStdString().c_str());
        
        if (!motion) {
            qDebug() << "表情Motion创建失败:" << expressionName;
            return false;
        }
        
        // 关键修复：使用新的AddExpressionManually方法正确存储表情
        model->AddExpressionManually(expressionName.toStdString().c_str(), motion);
        
        qDebug() << "表情加载并存储成功:" << expressionName << "来自文件:" << expressionFile;
        return true;
    } catch (const std::exception& e) {
        qDebug() << "表情加载时发生异常:" << e.what();
        return false;
    } catch (...) {
        qDebug() << "表情加载时发生未知异常";
        return false;
    }
}

// Qt按键映射到VTube按键格式
QString GLCore::mapQtKeyToVTubeKey(int qtKey) {
    switch (qtKey) {
        case Qt::Key_A: return "A";
        case Qt::Key_B: return "B";
        case Qt::Key_C: return "C";
        case Qt::Key_D: return "D";
        case Qt::Key_E: return "E";
        case Qt::Key_F: return "F";
        case Qt::Key_G: return "G";
        case Qt::Key_H: return "H";
        case Qt::Key_I: return "I";
        case Qt::Key_J: return "J";
        case Qt::Key_K: return "K";
        case Qt::Key_L: return "L";
        case Qt::Key_M: return "M";
        case Qt::Key_N: return "N";
        case Qt::Key_O: return "O";
        case Qt::Key_P: return "P";
        case Qt::Key_Q: return "Q";
        case Qt::Key_R: return "R";
        case Qt::Key_S: return "S";
        case Qt::Key_T: return "T";
        case Qt::Key_U: return "U";
        case Qt::Key_V: return "V";
        case Qt::Key_W: return "W";
        case Qt::Key_X: return "X";
        case Qt::Key_Y: return "Y";
        case Qt::Key_Z: return "Z";
        
        case Qt::Key_1: return "N1";
        case Qt::Key_2: return "N2";
        case Qt::Key_3: return "N3";
        case Qt::Key_4: return "N4";
        case Qt::Key_5: return "N5";
        case Qt::Key_6: return "N6";
        case Qt::Key_7: return "N7";
        case Qt::Key_8: return "N8";
        case Qt::Key_9: return "N9";
        case Qt::Key_0: return "N0";
        
        default: return "";
    }
}

// 打印热键映射信息并保存到文件
void GLCore::printHotkeyMappings() {
    QString modelName = QFileInfo(currentModelDirectory).baseName();
    QString headerText = "========== " + modelName + " 热键映射 ==========";
    QString footerText = "==============================================";
    
    qDebug() << headerText;
    
    // 准备写入文件的内容
    QStringList fileContent;
    fileContent << headerText;
    fileContent << "";  // 空行
    
    if (hotkeyConfigs.empty()) {
        QString emptyMsg = "当前模型 " + modelName + " 没有热键配置";
        qDebug() << emptyMsg;
        fileContent << emptyMsg;
        fileContent << "";
        fileContent << "可能的原因:";
        fileContent << "  1. 模型目录下没有对应的 .vtube.json 文件";
        fileContent << "  2. .vtube.json 文件中没有有效的热键配置";
        fileContent << "  3. 热键配置文件格式不正确";
        fileContent << "";
        fileContent << "如需添加热键支持，请:";
        fileContent << "  1. 确保模型目录下有 " + modelName + ".vtube.json 文件";
        fileContent << "  2. 文件中包含有效的 Hotkeys 配置项";
        fileContent << "  3. 确保表情文件(.exp3.json)存在于模型目录中";
    } else {
        // 按键名排序以获得一致的输出
        QStringList sortedKeys;
        for (const auto& pair : hotkeyConfigs) {
            sortedKeys << pair.first;
        }
        sortedKeys.sort();
        
        for (const QString& key : sortedKeys) {
            const HotkeyConfig& config = hotkeyConfigs[key];
            QString expressionName = QFileInfo(config.expressionFile).baseName();
            
            QString mappingLine = "  " + key + " -> " + expressionName + " (" + config.expressionFile + ")";
            qDebug() << mappingLine;
            fileContent << mappingLine;
        }
    }
    
    fileContent << "";  // 空行
    fileContent << footerText;
    
    qDebug() << footerText;
    
    // 写入到文件
    QString keymapFilePath = QDir::currentPath() + "/contact/keymap.txt";
    
    // 确保 contact 目录存在
    QDir contactDir(QDir::currentPath() + "/contact");
    if (!contactDir.exists()) {
        contactDir.mkpath(".");
        qDebug() << "创建 contact 目录:" << contactDir.absolutePath();
    }
    
    QFile keymapFile(keymapFilePath);
    if (keymapFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&keymapFile);
        out.setEncoding(QStringConverter::Utf8); // 确保支持中文
        
        // 添加文件头信息
        out << "# Live2D Desktop Girl - 热键映射配置文件" << Qt::endl;
        out << "# 自动生成于: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << Qt::endl;
        out << "# 当前模型: " << currentModelDirectory << Qt::endl;
        out << Qt::endl;
        
        // 写入映射内容
        for (const QString& line : fileContent) {
            out << line << Qt::endl;
        }
        
        // 添加使用说明
        out << Qt::endl;
        out << "# 使用说明:" << Qt::endl;
        if (hotkeyConfigs.empty()) {
            out << "# - 当前模型无热键配置，只能使用以下基本功能:" << Qt::endl;
            out << "# - 按 ESC 键触发随机表情" << Qt::endl;
        } else {
            out << "# - 按对应的键即可触发表情动作" << Qt::endl;
            out << "# - 按 ESC 键触发随机表情" << Qt::endl;
        }
        out << "# - 右键点击模型可切换窗口边框显示/隐藏" << Qt::endl;
        out << "# - 修改 config.json 中的 modelName 可切换不同模型" << Qt::endl;
        
        keymapFile.close();
        qDebug() << "热键映射已保存到:" << keymapFilePath;
    } else {
        qDebug() << "无法创建热键映射文件:" << keymapFilePath << "错误:" << keymapFile.errorString();
    }
}

// 修改键盘事件处理
void GLCore::keyPressEvent(QKeyEvent* event) {
    if (!isHotkeySystemReady) {
        qDebug() << "热键系统未准备就绪，尝试重新初始化...";
        detectAndInitializeModel();
        if (!isHotkeySystemReady) {
            QOpenGLWidget::keyPressEvent(event);
            return;
        }
    }
    
    // 将Qt按键映射到VTube格式
    QString vtubeKey = mapQtKeyToVTubeKey(event->key());
    
    if (vtubeKey.isEmpty()) {
        QOpenGLWidget::keyPressEvent(event);
        return;
    }
    
    // 查找热键配置
    auto it = hotkeyConfigs.find(vtubeKey);
    if (it != hotkeyConfigs.end()) {
        const HotkeyConfig& config = it->second;
        QString expressionName = QFileInfo(config.expressionFile).baseName();
        
        triggerExpression(expressionName);
        qDebug() << "热键触发: " << vtubeKey << " -> " << expressionName;
    } else {
        // ESC键重置表情（随机表情）
        if (event->key() == Qt::Key_Escape) {
            LAppLive2DManager::GetInstance()->GetModel(0)->SetRandomExpression();
            qDebug() << "触发随机表情";
        }
        // F12键切换对话框位置调试模式
        else if (event->key() == Qt::Key_F12) {
            debugChatPosition = !debugChatPosition;
            qDebug() << "对话框位置调试模式:" << (debugChatPosition ? "开启" : "关闭");
        }
    }
    
    QOpenGLWidget::keyPressEvent(event);
}

// 修改triggerExpression方法 
void GLCore::triggerExpression(const QString& expressionName) {
    auto* manager = LAppLive2DManager::GetInstance();
    if (manager && manager->GetModelNum() > 0) {
        auto* model = manager->GetModel(0);
        if (model) {
            model->SetExpression(expressionName.toStdString().c_str());
            qDebug() << "表情触发:" << expressionName;
        }
    }
}

// 修改initKeyMapping方法，增加模型切换检测
void GLCore::initKeyMapping() {
    qDebug() << "初始化热键系统...";
    
    // 检查当前配置的模型名称
    QString configPath = QDir::currentPath() + "/contact/config.json";
    QFile file(configPath);
    QString configuredModel = "Nahida_1080"; // 默认值
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            configuredModel = obj["modelName"].toString("Nahida_1080");
            qDebug() << "从配置文件读取模型名称:" << configuredModel;
        }
    }
    
    // 检查是否需要重新初始化（模型发生变化）
    QString configuredModelDir = "Resources/" + configuredModel;
    if (configuredModelDir != currentModelDirectory) {
        qDebug() << "检测到模型切换: " << currentModelDirectory << " -> " << configuredModelDir;
        isHotkeySystemReady = false; // 重置热键系统状态
        hotkeyConfigs.clear(); // 清除现有配置
    }
    
    detectAndInitializeModel();
}

// 初始化对话框UI
void GLCore::setupChatUI() {
    // 创建对话框容器
    chatContainer = new QFrame(this);
    chatContainer->setFixedHeight(72); // 初始高度
    
    // 创建水平布局
    chatLayout = new QHBoxLayout(chatContainer);
    chatLayout->setContentsMargins(10, 10, 10, 10);  // 确保上下边距相等
    chatLayout->setSpacing(10);
    chatLayout->setAlignment(Qt::AlignVCenter);  // 垂直居中对齐
    
    // 创建文本输入框
    inputTextEdit = new QTextEdit(chatContainer);
    inputTextEdit->setPlaceholderText("请输入文字");
    inputTextEdit->setFixedHeight(52);
    inputTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    inputTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 创建发送按钮
    sendButton = new QPushButton("发送", chatContainer);
    sendButton->setFixedSize(80, 52);
    
    // 添加到布局，确保垂直居中
    chatLayout->addWidget(inputTextEdit, 1, Qt::AlignVCenter);  // 输入框可伸缩，垂直居中
    chatLayout->addWidget(sendButton, 0, Qt::AlignVCenter);     // 按钮固定大小，垂直居中
    
    // 设置样式
    updateChatUIStyle();
    
    // 连接信号
    connect(sendButton, &QPushButton::clicked, this, &GLCore::onSendButtonClicked);
    connect(inputTextEdit, &QTextEdit::textChanged, this, &GLCore::onTextChanged);
    
    // 初始状态下隐藏对话框（因为默认是无边框）
    chatContainer->setVisible(false);
    qDebug() << "对话框UI初始化完成，初始状态隐藏";
    
    // 从配置文件读取初始isReacted状态
    isReacted = readIsReactedFromConfig();
    qDebug() << "初始isReacted状态:" << isReacted;
}

// 更新对话框样式
void GLCore::updateChatUIStyle() {
    if (!chatContainer || !inputTextEdit || !sendButton) return;
    
    // 设置容器样式
    QString containerStyle = R"(
        QFrame {
            background-color: rgba(255, 255, 255, 240);
            border: 2px solid #CCCCCC;
            border-radius: 15px;
            padding: 5px;
        }
    )";
    chatContainer->setStyleSheet(containerStyle);
    
    // 设置输入框样式
    QString inputStyle = R"(
        QTextEdit {
            border: 2px solid #DDDDDD;
            border-radius: 10px;
            background-color: rgba(255, 255, 255, 250);
            font-size: 14px;
            padding: 8px;
            color: #333333;
        }
        QTextEdit:focus {
            border: 2px solid #4CAF50;
        }
    )";
    inputTextEdit->setStyleSheet(inputStyle);
    
    // 设置发送按钮样式
    QString buttonStyle = R"(
        QPushButton {
            background-color: #4CAF50;
            border: none;
            border-radius: 10px;
            color: white;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
    )";
    sendButton->setStyleSheet(buttonStyle);
}

// 调整输入框高度
void GLCore::adjustInputHeight() {
    if (!inputTextEdit || !chatContainer) return;
    
    QTextDocument* doc = inputTextEdit->document();
    QFontMetrics fm(inputTextEdit->font());
    
    // 计算文本高度
    int docHeight = static_cast<int>(doc->size().height());
    int lineHeight = fm.lineSpacing();
    int lines = qMax(1, (docHeight + lineHeight - 1) / lineHeight); // 向上取整
    
    // 限制最大行数为5行
    lines = qMin(lines, 5);
    
    // 计算新的高度
    int newInputHeight = lines * lineHeight + 16; // 16是padding
    int newContainerHeight = newInputHeight + 20; // 20是容器的padding
    
    // 设置新高度
    inputTextEdit->setFixedHeight(newInputHeight);
    chatContainer->setFixedHeight(newContainerHeight);
    sendButton->setFixedHeight(newInputHeight); // 按钮高度与输入框保持一致
    
    qDebug() << "调整输入框高度: 行数=" << lines << "输入框高度=" << newInputHeight;
}

// 发送消息
void GLCore::sendMessage() {
    if (!inputTextEdit) return;
    
    QString message = inputTextEdit->toPlainText().trimmed();
    if (message.isEmpty()) {
        qDebug() << "消息为空，不发送";
        return;
    }
    
    // 写入消息到 ./contact/in.txt
    QString inFilePath = QDir::currentPath() + "/contact/in.txt";
    
    // 确保 contact 目录存在
    QDir contactDir(QDir::currentPath() + "/contact");
    if (!contactDir.exists()) {
        contactDir.mkpath(".");
        qDebug() << "创建 contact 目录:" << contactDir.absolutePath();
    }
    
    QFile inFile(inFilePath);
    if (inFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&inFile);
        out.setEncoding(QStringConverter::Utf8); // 确保支持中文
        out << message << Qt::endl;
        inFile.close();
        qDebug() << "消息已写入 in.txt:" << message;
    } else {
        qDebug() << "无法写入消息到 in.txt:" << inFile.errorString();
        return;
    }
    
    // 设置 config.json 的 isReacted 为 false
    updateConfigIsReacted(false);
    isReacted = false;
    
    // 启动超时计时
    startResponseTimeout();
    
    // 清空输入框
    inputTextEdit->clear();
    
    // 隐藏对话框（等待AI回复）
    if (chatContainer) {
        chatContainer->setVisible(false);
        qDebug() << "消息发送完成，隐藏对话框等待AI回复";
    }
    
    qDebug() << "消息发送流程完成，开始60秒超时计时";
}

// 从配置文件读取isReacted状态
bool GLCore::readIsReactedFromConfig() {
    QString configPath = QDir::currentPath() + "/contact/config.json";
    QFile file(configPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open config file for isReacted:" << configPath;
        return true; // 默认返回true
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error for isReacted:" << error.errorString();
        return true; // 默认返回true
    }
    
    QJsonObject obj = doc.object();
    bool reacted = obj["isReacted"].toBool(true);
    
    return reacted;
}

// 更新配置文件的isReacted值
void GLCore::updateConfigIsReacted(bool reacted) {
    QString configPath = QDir::currentPath() + "/contact/config.json";
    QFile file(configPath);
    
    // 读取现有配置
    QJsonObject config;
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError) {
            config = doc.object();
        }
    }
    
    // 更新isReacted值
    config["isReacted"] = reacted;
    
    // 写回文件
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(config);
        file.write(doc.toJson());
        file.close();
        qDebug() << "更新config.json中的isReacted为:" << reacted;
    } else {
        qDebug() << "无法写入config.json:" << file.errorString();
    }
}

// 发送按钮点击槽函数
void GLCore::onSendButtonClicked() {
    qDebug() << "发送按钮被点击";
    
    // 根据按钮2的状态选择不同的处理方式
    if (button2State == Button2State::CommandMode) {
        // 指令模式：执行指令解析
        sendCommand();
    } else {
        // 聊天模式：发送消息
        sendMessage();
    }
}

// 文本改变槽函数
void GLCore::onTextChanged() {
    adjustInputHeight();
}

// 设置超时UI
void GLCore::setupTimeoutUI() {
    // 创建超时提示标签
    timeoutLabel = new QLabel(this);
    timeoutLabel->setText("AI后端响应超时！");
    timeoutLabel->setAlignment(Qt::AlignCenter);
    timeoutLabel->setFixedHeight(40);
    timeoutLabel->setVisible(false); // 初始隐藏
    
    // 设置超时标签样式
    QString timeoutStyle = R"(
        QLabel {
            background-color: rgba(255, 100, 100, 240);
            border: 2px solid #FF4444;
            border-radius: 10px;
            color: white;
            font-weight: bold;
            font-size: 16px;
            padding: 8px;
        }
    )";
    timeoutLabel->setStyleSheet(timeoutStyle);
    
    // 创建超时定时器
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true); // 单次触发
    connect(timeoutTimer, &QTimer::timeout, this, &GLCore::onResponseTimeout);
    
    qDebug() << "超时UI初始化完成";
}

// 开始响应超时计时
void GLCore::startResponseTimeout() {
    if (timeoutTimer) {
        isWaitingForResponse = true;
        timeoutTimer->start(60000); // 60秒 = 60000毫秒
        qDebug() << "开始超时计时，60秒后如果没有响应将触发超时";
    }
}

// 处理响应超时
void GLCore::handleResponseTimeout() {
    qDebug() << "AI后端响应超时！强制设置isReacted=true";
    
    // 强制设置isReacted为true
    updateConfigIsReacted(true);
    isReacted = true;
    isWaitingForResponse = false;
    
    // 根据当前按钮状态更新组件可见性，而不是硬编码显示对话框
    updateComponentsVisibility();
    qDebug() << "超时后根据当前模式恢复组件显示";
    
    // 显示超时消息
    if (timeoutLabel) {
        // 计算超时标签位置（屏幕中上部）
        int labelWidth = width() - 40;
        int labelX = 20;
        int labelY = height() / 4; // 屏幕中上部
        
        timeoutLabel->setGeometry(labelX, labelY, labelWidth, 40);
        timeoutLabel->setVisible(true);
        
        qDebug() << "显示超时提示标签:" << timeoutLabel->geometry();
        
        // 5秒后自动隐藏超时消息
        QTimer::singleShot(5000, this, &GLCore::hideTimeoutMessage);
    }
}

// 隐藏超时消息
void GLCore::hideTimeoutMessage() {
    if (timeoutLabel) {
        timeoutLabel->setVisible(false);
        qDebug() << "隐藏超时提示标签";
    }
}

// 响应超时槽函数
void GLCore::onResponseTimeout() {
    qDebug() << "触发响应超时事件";
    handleResponseTimeout();
}

// 检查并播放音频文件 - 每秒监测一次
void GLCore::checkAndPlayAudioFile() {
    // 构建音频文件路径
    QString audioFilePath = QDir::currentPath() + "/contact/1.wav";
    
    // 检查文件是否存在
    if (QFile::exists(audioFilePath)) {
        qDebug() << "发现音频文件:" << audioFilePath;
        
        // 使用AudioOutput播放音频
        if (audioPlayer) {
            // 检查是否正在播放其他音频，避免重复播放
            if (audioPlayer->getState() == QMediaPlayer::PlayingState) {
                qDebug() << "正在播放其他音频，跳过当前文件";
                return;
            }
            
            // 使用带时间戳的临时文件名，避免文件名冲突
            QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
            QString tempAudioPath = QDir::currentPath() + "/contact/temp_" + timestamp + ".wav";
            
            // 复制文件到临时位置
            if (QFile::copy(audioFilePath, tempAudioPath)) {
                // 立即删除原始文件，避免循环播放
                if (QFile::remove(audioFilePath)) {
                    qDebug() << "原始音频文件已删除:" << audioFilePath;
                } else {
                    qDebug() << "删除原始音频文件失败:" << audioFilePath;
                }
                
                // 使用临时文件播放音频
                audioPlayer->setAudioPath(tempAudioPath);
                audioPlayer->playAudio();
                qDebug() << "开始播放音频文件:" << tempAudioPath;
                
                // 播放完成后删除临时文件
                QObject::connect(audioPlayer, &AudioOutput::playbackFinished, 
                               this, [this, tempAudioPath]() {
                    // 删除临时音频文件，增加重试机制
                    int retryCount = 0;
                    bool deleted = false;
                    
                    while (retryCount < 3 && !deleted) {
                        if (QFile::remove(tempAudioPath)) {
                            qDebug() << "临时音频文件播放完成并已删除:" << tempAudioPath;
                            deleted = true;
                        } else {
                            retryCount++;
                            qDebug() << "删除临时音频文件失败，重试" << retryCount << "/3:" << tempAudioPath;
                            QThread::msleep(100); // 等待100毫秒后重试
                        }
                    }
                    
                    if (!deleted) {
                        qDebug() << "警告：临时音频文件删除失败，可能需要手动清理:" << tempAudioPath;
                    }
                    
                    // 断开这个连接，避免后续重复触发
                    QObject::disconnect(audioPlayer, &AudioOutput::playbackFinished, this, nullptr);
                }, Qt::QueuedConnection);
            } else {
                qDebug() << "复制音频文件到临时位置失败，源文件:" << audioFilePath << "目标文件:" << tempAudioPath;
            }
        } else {
            qDebug() << "AudioOutput实例不可用，无法播放音频";
        }
    }
}

// 清理上次运行遗留的临时音频文件
void GLCore::cleanupTemporaryAudioFiles() {
    // 构建临时音频文件的搜索路径
    QString tempAudioPattern = QDir::currentPath() + "/contact/temp_*.wav";
    
    // 获取所有匹配的临时音频文件
    QFileInfoList tempAudioFiles = QDir(QDir::currentPath() + "/contact").entryInfoList(QStringList() << "temp_*.wav", QDir::Files);
    
    // 遍历所有临时音频文件并删除
    for (const QFileInfo& fileInfo : tempAudioFiles) {
        QString tempAudioPath = fileInfo.absoluteFilePath();
        if (QFile::remove(tempAudioPath)) {
            qDebug() << "清理临时音频文件:" << tempAudioPath;
        } else {
            qDebug() << "清理临时音频文件失败:" << tempAudioPath;
        }
    }
}

// ========== 新增方法实现 ==========

// 设置按钮UI
void GLCore::setupButtonUI() {
    // 创建按钮容器
    buttonContainer = new QFrame(this);
    buttonContainer->setFixedWidth(80); // 固定宽度
    buttonContainer->setVisible(false); // 初始隐藏，只在有边框模式显示
    
    // 创建垂直布局
    buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(10, 20, 10, 10);
    buttonLayout->setSpacing(15);
    buttonLayout->setAlignment(Qt::AlignTop);
    
    // 创建按钮1（隐藏/显示）
    button1 = new QPushButton(buttonContainer);
    button1->setFixedSize(60, 60);
    button1->setIconSize(QSize(50, 50));
    updateButton1Icon();
    
    // 创建按钮2（模式切换）
    button2 = new QPushButton(buttonContainer);
    button2->setFixedSize(60, 60);
    button2->setIconSize(QSize(50, 50));
    updateButton2Icon();
    
    // 创建按钮3（指南）
    button3 = new QPushButton(buttonContainer);
    button3->setFixedSize(60, 60);
    button3->setIconSize(QSize(50, 50));
    QString button3IconPath = QDir::currentPath() + "/asset/Buttons/3_guidebook.ico";
    QIcon button3CircularIcon = createCircularIcon(button3IconPath, 50);
    button3->setIcon(button3CircularIcon);
    
    // 设置按钮样式为圆形
    QString buttonStyle = R"(
        QPushButton {
            border-radius: 30px;
            background-color: rgba(255, 255, 255, 240);
            border: 2px solid #CCCCCC;
            padding: 0px;
        }
        QPushButton:hover {
            background-color: rgba(230, 230, 230, 250);
            border: 2px solid #999999;
        }
        QPushButton:pressed {
            background-color: rgba(200, 200, 200, 250);
            border: 2px solid #666666;
        }
        QPushButton::icon {
            border-radius: 25px;
            padding: 5px;
        }
    )";
    
    button1->setStyleSheet(buttonStyle);
    button2->setStyleSheet(buttonStyle);
    button3->setStyleSheet(buttonStyle);
    
    // 添加按钮到布局
    buttonLayout->addWidget(button1);
    buttonLayout->addWidget(button2);
    buttonLayout->addWidget(button3);
    buttonLayout->addStretch(); // 添加伸缩空间
    
    // 连接信号
    connect(button1, &QPushButton::clicked, this, &GLCore::onButton1Clicked);
    connect(button2, &QPushButton::clicked, this, &GLCore::onButton2Clicked);
    connect(button3, &QPushButton::clicked, this, &GLCore::onButton3Clicked);
    
    qDebug() << "按钮UI初始化完成";
}

// 设置麦克风UI
void GLCore::setupMicrophoneUI() {
    // 创建麦克风容器
    microphoneContainer = new QFrame(this);
    microphoneContainer->setFixedSize(120, 120);
    microphoneContainer->setVisible(false); // 初始隐藏
    
    // 创建麦克风按钮
    microphoneButton = new QPushButton(microphoneContainer);
    microphoneButton->setFixedSize(100, 100);
    microphoneButton->setIconSize(QSize(80, 80));
    microphoneButton->move(10, 10); // 居中放置
    updateMicrophoneIcon();
    
    // 设置麦克风按钮样式为圆形
    QString micButtonStyle = R"(
        QPushButton {
            border-radius: 50px;
            background-color: rgba(255, 255, 255, 240);
            border: 3px solid #4CAF50;
            padding: 0px;
        }
        QPushButton:hover {
            background-color: rgba(230, 255, 230, 250);
            border: 3px solid #45a049;
        }
        QPushButton:pressed {
            background-color: rgba(200, 255, 200, 250);
            border: 3px solid #3d8b40;
        }
        QPushButton::icon {
            border-radius: 40px;
            padding: 10px;
        }
    )";
    microphoneButton->setStyleSheet(micButtonStyle);
    
    // 连接信号
    connect(microphoneButton, &QPushButton::clicked, this, &GLCore::onMicrophoneClicked);
    
    qDebug() << "麦克风UI初始化完成";
}

// 更新按钮容器位置
void GLCore::updateButtonContainer() {
    if (!buttonContainer) return;
    
    // 将按钮容器放置在窗口左侧
    int containerHeight = buttonContainer->sizeHint().height();
    buttonContainer->setGeometry(10, 30, 80, height() - 60);
    
    qDebug() << "按钮容器位置调整:" << buttonContainer->geometry();
}

// 更新麦克风容器位置
void GLCore::updateMicrophoneContainer() {
    if (!microphoneContainer) return;
    
    // 将麦克风容器放置在窗口底部中央（与对话框同位置）
    int margin = 10;
    int containerWidth = microphoneContainer->width();
    int containerHeight = microphoneContainer->height();
    int x = (width() - containerWidth) / 2;
    int y = height() - containerHeight - margin;
    
    microphoneContainer->setGeometry(x, y, containerWidth, containerHeight);
    
    qDebug() << "麦克风容器位置调整:" << microphoneContainer->geometry();
}

// 更新组件可见性
void GLCore::updateComponentsVisibility() {
    if (isFrameless) {
        // 无边框模式：隐藏所有UI组件
        if (buttonContainer) buttonContainer->setVisible(false);
        if (favorabilityBar) favorabilityBar->setVisible(false);
        if (chatContainer) chatContainer->setVisible(false);
        if (microphoneContainer) microphoneContainer->setVisible(false);
        return;
    }
    
    // 有边框模式：根据按钮1状态决定其他组件的可见性
    bool shouldShowComponents = componentsVisible && (button1State == Button1State::Show);
    
    // 按钮容器始终显示（在有边框模式下）
    if (buttonContainer) buttonContainer->setVisible(true);
    
    // 好感度条的可见性
    if (favorabilityBar) favorabilityBar->setVisible(shouldShowComponents);
    
    // 底部组件的可见性（根据按钮2的状态）
    bool shouldShowBottomComponent = shouldShowComponents && isReacted;
    
    if (button2State == Button2State::ChatMode || button2State == Button2State::CommandMode) {
        // 显示对话框
        if (chatContainer) chatContainer->setVisible(shouldShowBottomComponent);
        if (microphoneContainer) microphoneContainer->setVisible(false);
        // 更新输入框提示文字
        updateInputPlaceholder();
    } else if (button2State == Button2State::MicrophoneMode) {
        // 显示麦克风
        if (chatContainer) chatContainer->setVisible(false);
        if (microphoneContainer) microphoneContainer->setVisible(shouldShowBottomComponent);
    }
    
    qDebug() << "组件可见性更新 - 边框:" << !isFrameless 
             << "组件可见:" << shouldShowComponents 
             << "底部组件可见:" << shouldShowBottomComponent
             << "按钮2状态:" << static_cast<int>(button2State);
}

// 播放点击音效
void GLCore::playClickSound() {
    if (audioPlayer) {
        QString clickSoundPath = QDir::currentPath() + "/asset/Buttons/click_effect.mp3";
        if (QFile::exists(clickSoundPath)) {
            audioPlayer->setAudioPath(clickSoundPath);
            audioPlayer->playAudio();
            qDebug() << "播放点击音效:" << clickSoundPath;
        } else {
            qDebug() << "点击音效文件不存在:" << clickSoundPath;
        }
    }
}

// 更新按钮1图标
void GLCore::updateButton1Icon() {
    if (!button1) return;
    
    QString iconPath;
    if (button1State == Button1State::Hide) {
        iconPath = QDir::currentPath() + "/asset/Buttons/1_hide.ico";
    } else {
        iconPath = QDir::currentPath() + "/asset/Buttons/1_show.ico";
    }
    
    // 创建圆形蒙版图标
    QIcon circularIcon = createCircularIcon(iconPath, 50);
    button1->setIcon(circularIcon);
    qDebug() << "更新按钮1图标:" << iconPath;
}

// 更新按钮2图标
void GLCore::updateButton2Icon() {
    if (!button2) return;
    
    QString iconPath;
    switch (button2State) {
        case Button2State::ChatMode:
            iconPath = QDir::currentPath() + "/asset/Buttons/2_chat_mode.ico";
            break;
        case Button2State::MicrophoneMode:
            iconPath = QDir::currentPath() + "/asset/Buttons/2_microphone_mode.ico";
            break;
        case Button2State::CommandMode:
            iconPath = QDir::currentPath() + "/asset/Buttons/2_command_mode.ico";
            break;
    }
    
    // 创建圆形蒙版图标
    QIcon circularIcon = createCircularIcon(iconPath, 50);
    button2->setIcon(circularIcon);
    qDebug() << "更新按钮2图标:" << iconPath;
}

// 更新麦克风图标
void GLCore::updateMicrophoneIcon() {
    if (!microphoneButton) return;
    
    QString iconPath;
    if (microphoneState == MicrophoneState::Start) {
        iconPath = QDir::currentPath() + "/asset/microphone_widget/microphone_Start.png";
    } else {
        iconPath = QDir::currentPath() + "/asset/microphone_widget/microphone_Stop.png";
    }
    
    // 创建圆形蒙版图标
    QIcon circularIcon = createCircularIcon(iconPath, 80);
    microphoneButton->setIcon(circularIcon);
    qDebug() << "更新麦克风图标:" << iconPath;
}

// 设置录音
void GLCore::setupRecording() {
    // 创建录音相关组件
    captureSession = new QMediaCaptureSession(this);
    mediaRecorder = new QMediaRecorder(this);
    
    // 获取默认音频输入设备
    QAudioDevice defaultAudioInput = QMediaDevices::defaultAudioInput();
    if (defaultAudioInput.isNull()) {
        qDebug() << "警告：未找到音频输入设备";
        return;
    }
    
    audioInput = new QAudioInput(defaultAudioInput, this);
    
    // 设置捕获会话
    captureSession->setAudioInput(audioInput);
    captureSession->setRecorder(mediaRecorder);
    
    // 创建录音计时器（最长1分钟）
    recordingTimer = new QTimer(this);
    recordingTimer->setSingleShot(true);
    connect(recordingTimer, &QTimer::timeout, this, &GLCore::onRecordingTimeout);
    
    // 连接录音完成信号
    connect(mediaRecorder, &QMediaRecorder::recorderStateChanged, 
            this, [this](QMediaRecorder::RecorderState state) {
        qDebug() << "录音状态变化:" << state;
        if (state == QMediaRecorder::StoppedState && isRecording) {
            finishRecording();
        }
    });
    
    qDebug() << "录音系统初始化完成";
}

// 开始录音
void GLCore::startRecording() {
    if (!mediaRecorder || isRecording) {
        qDebug() << "无法开始录音：录音器不可用或已在录音中";
        return;
    }
    
    // 确保输出目录存在
    QDir contactDir(QDir::currentPath() + "/contact");
    if (!contactDir.exists()) {
        contactDir.mkpath(".");
    }
    
    QString outputPath = QDir::currentPath() + "/contact/voice.mp3";
    
    // 如果文件已存在，先删除
    if (QFile::exists(outputPath)) {
        QFile::remove(outputPath);
    }
    
    // 设置录音格式和输出位置
    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(outputPath));
    
    // 设置音频格式
    QMediaFormat format;
    format.setFileFormat(QMediaFormat::MP3);
    format.setAudioCodec(QMediaFormat::AudioCodec::MP3);
    mediaRecorder->setMediaFormat(format);
    
    // 设置音频质量
    mediaRecorder->setAudioSampleRate(44100);
    mediaRecorder->setAudioBitRate(128000);
    
    // 开始录音
    mediaRecorder->record();
    isRecording = true;
    
    // 启动超时计时器（60秒）
    recordingTimer->start(60000);
    
    // 更新麦克风状态和图标
    microphoneState = MicrophoneState::Stop;
    updateMicrophoneIcon();
    
    qDebug() << "开始录音，输出到:" << outputPath;
}

// 停止录音
void GLCore::stopRecording() {
    if (!isRecording) {
        qDebug() << "当前未在录音，无需停止";
        return;
    }
    
    // 停止录音计时器
    if (recordingTimer && recordingTimer->isActive()) {
        recordingTimer->stop();
    }
    
    // 停止录音
    if (mediaRecorder) {
        mediaRecorder->stop();
    }
    
    qDebug() << "停止录音";
}

// 完成录音处理
void GLCore::finishRecording() {
    isRecording = false;
    
    // 更新麦克风状态和图标
    microphoneState = MicrophoneState::Start;
    updateMicrophoneIcon();
    
    // 设置config.json的isReacted为false
    updateConfigIsReacted(false);
    isReacted = false;
    
    // 启动AI响应超时计时
    startResponseTimeout();
    
    // 隐藏麦克风组件
    if (microphoneContainer) {
        microphoneContainer->setVisible(false);
    }
    
    qDebug() << "录音完成，设置isReacted=false，启动60秒超时计时并隐藏麦克风组件";
}

// 按钮1点击处理
void GLCore::onButton1Clicked() {
    playClickSound();
    
    // 切换按钮1状态
    if (button1State == Button1State::Hide) {
        button1State = Button1State::Show;
        componentsVisible = true;
    } else {
        button1State = Button1State::Hide;
        componentsVisible = false;
    }
    
    updateButton1Icon();
    updateComponentsVisibility();
    
    qDebug() << "按钮1点击，新状态:" << static_cast<int>(button1State) 
             << "组件可见:" << componentsVisible;
}

// 按钮2点击处理
void GLCore::onButton2Clicked() {
    playClickSound();
    
    // 循环切换按钮2状态
    switch (button2State) {
        case Button2State::ChatMode:
            button2State = Button2State::MicrophoneMode;
            break;
        case Button2State::MicrophoneMode:
            button2State = Button2State::CommandMode;
            break;
        case Button2State::CommandMode:
            button2State = Button2State::ChatMode;
            break;
    }
    
    updateButton2Icon();
    updateComponentsVisibility();
    
    qDebug() << "按钮2点击，新状态:" << static_cast<int>(button2State);
}

// 按钮3点击处理
void GLCore::onButton3Clicked() {
    playClickSound();
    
    // 打开keymap.txt文件
    QString keymapPath = QDir::currentPath() + "/contact/keymap.txt";
    
    if (QFile::exists(keymapPath)) {
        bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(keymapPath));
        if (success) {
            qDebug() << "成功打开keymap.txt文件";
        } else {
            qDebug() << "打开keymap.txt文件失败";
        }
    } else {
        qDebug() << "keymap.txt文件不存在:" << keymapPath;
    }
}

// 麦克风按钮点击处理
void GLCore::onMicrophoneClicked() {
    playClickSound();
    
    if (!isRecording) {
        // 开始录音
        startRecording();
    } else {
        // 停止录音
        stopRecording();
    }
}

// 录音超时处理
void GLCore::onRecordingTimeout() {
    qDebug() << "录音超时（60秒），强制停止录音";
    stopRecording();
}

// 创建圆形蒙版图标
QIcon GLCore::createCircularIcon(const QString& iconPath, int size) {
    // 加载原始图标
    QPixmap originalPixmap(iconPath);
    if (originalPixmap.isNull()) {
        qDebug() << "无法加载图标:" << iconPath;
        return QIcon();
    }
    
    // 缩放到指定大小
    QPixmap scaledPixmap = originalPixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // 创建圆形蒙版
    QPixmap roundedPixmap(size, size);
    roundedPixmap.fill(Qt::transparent);
    
    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 设置圆形裁剪路径
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    
    // 绘制缩放后的图标，居中放置
    int x = (size - scaledPixmap.width()) / 2;
    int y = (size - scaledPixmap.height()) / 2;
    painter.drawPixmap(x, y, scaledPixmap);
    
    painter.end();
    
    return QIcon(roundedPixmap);
}

// ========== 指令系统实现 ==========

// 更新输入框提示文字
void GLCore::updateInputPlaceholder() {
    if (!inputTextEdit) return;
    
    if (button2State == Button2State::CommandMode) {
        inputTextEdit->setPlaceholderText("请输入指令");
    } else {
        inputTextEdit->setPlaceholderText("请输入文字");
    }
    
    qDebug() << "更新输入框提示文字为:" << inputTextEdit->placeholderText();
}

// 发送指令（command模式）
void GLCore::sendCommand() {
    if (!inputTextEdit) return;
    
    QString command = inputTextEdit->toPlainText().trimmed();
    if (command.isEmpty()) {
        qDebug() << "指令为空，不执行";
        return;
    }
    
    qDebug() << "执行指令:" << command;
    
    // 解析并执行指令
    parseAndExecuteCommand(command);
    
    // 清空输入框
    inputTextEdit->clear();
}

// 解析并执行指令
void GLCore::parseAndExecuteCommand(const QString& command) {
    QStringList parts = command.split(' ', Qt::SkipEmptyParts);
    
    if (parts.isEmpty()) {
        showCommandError("错误：指令不存在");
        return;
    }
    
    QString mainCommand = parts[0].toLower();
    
    if (mainCommand == "playmusic") {
        executePlayMusicCommand(parts);
    } else if (mainCommand == "execute") {
        executeScriptCommand(parts);
    } else {
        showCommandError("错误：指令不存在");
    }
}

// 执行音乐播放指令
void GLCore::executePlayMusicCommand(const QStringList& args) {
    if (args.size() == 1) {
        // 没有参数时默认播放theme.mp3
        QString defaultPath = QDir::currentPath() + "/UserData/music/theme.mp3";
        
        if (!QFile::exists(defaultPath)) {
            showCommandError("错误：找不到默认音频文件theme.mp3");
            return;
        }
        
        QMediaPlayer* musicPlayer = createNewMusicPlayer();
        if (musicPlayer) {
            musicPlayer->setSource(QUrl::fromLocalFile(defaultPath));
            musicPlayer->play();
            qDebug() << "开始播放默认音乐:" << defaultPath;
        } else {
            showCommandError("错误：无法创建音乐播放器");
        }
        return;
    }
    
    // 检查是否有-r参数（循环播放）
    bool isLooping = false;
    QStringList processedArgs = args;
    
    // 如果第二个参数是-r，则启用循环播放
    if (args.size() >= 2 && args[1] == "-r") {
        isLooping = true;
        processedArgs.removeAt(1); // 移除-r参数
        qDebug() << "检测到循环播放参数";
    }
    
    // 如果移除-r后只剩下主命令，则报错
    if (processedArgs.size() == 1) {
        showCommandError("错误：-r参数后缺少文件名");
        return;
    }
    
    // 将除第一个参数外的所有参数重新拼接，支持包含空格的文件名
    QString action = processedArgs.mid(1).join(" ").toLower();
    
    if (action == "stop") {
        // 停止所有音乐播放
        stopAllMusic();
        qDebug() << "已停止所有音乐播放";
        return;
    }
    
    QString musicDir = QDir::currentPath() + "/UserData/music";
    QDir dir(musicDir);
    
    if (!dir.exists()) {
        showCommandError("错误：音乐目录不存在");
        return;
    }
    
    // 获取所有mp3文件
    QStringList mp3Files = dir.entryList(QStringList() << "*.mp3", QDir::Files);
    
    if (mp3Files.isEmpty()) {
        showCommandError("错误：音乐目录中没有mp3文件");
        return;
    }
    
    QString targetFile;
    
    if (action == "random") {
        // 随机选择一个mp3文件
        int randomIndex = QRandomGenerator::global()->bounded(mp3Files.size());
        targetFile = mp3Files[randomIndex];
        qDebug() << "随机选择音乐文件:" << targetFile;
    } else {
        // 指定文件名（支持包含空格的文件名）
        QString specifiedFile = action;  // 使用拼接后的完整文件名
        if (!specifiedFile.endsWith(".mp3", Qt::CaseInsensitive)) {
            specifiedFile += ".mp3";
        }
        
        // 检查文件是否存在
        if (mp3Files.contains(specifiedFile, Qt::CaseInsensitive)) {
            // 找到匹配的文件（不区分大小写）
            for (const QString& file : mp3Files) {
                if (file.compare(specifiedFile, Qt::CaseInsensitive) == 0) {
                    targetFile = file;
                    break;
                }
            }
        } else {
            showCommandError("错误：找不到对应的音频文件");
            return;
        }
    }
    
    // 播放指定的音乐文件
    QString fullPath = musicDir + "/" + targetFile;
    
    // 创建新的音乐播放器
    QMediaPlayer* musicPlayer = createNewMusicPlayer();
    if (musicPlayer) {
        musicPlayer->setSource(QUrl::fromLocalFile(fullPath));
        
        // 设置循环播放
        if (isLooping) {
            musicPlayer->setLoops(QMediaPlayer::Infinite);
            qDebug() << "设置为循环播放模式";
        }
        
        musicPlayer->play();
        qDebug() << "开始播放音乐:" << fullPath << (isLooping ? "(循环模式)" : "");
    } else {
        showCommandError("错误：无法创建音乐播放器");
    }
}

// 执行脚本指令
void GLCore::executeScriptCommand(const QStringList& args) {
    if (args.size() < 2) {
        showCommandError("错误：execute指令格式错误");
        return;
    }
    
    QString scriptName = args[1];
    if (!scriptName.endsWith(".bat", Qt::CaseInsensitive)) {
        scriptName += ".bat";
    }
    
    QString scriptPath = QDir::currentPath() + "/UserData/script/" + scriptName;
    
    if (!QFile::exists(scriptPath)) {
        showCommandError("错误：找不到对应的脚本文件");
        return;
    }
    
    // 执行批处理文件
    QProcess* process = new QProcess(this);
    
    // 连接进程完成信号
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process, scriptName](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "脚本执行完成:" << scriptName << "退出码:" << exitCode;
        process->deleteLater(); // 自动清理进程对象
    });
    
    // 连接错误信号
    connect(process, &QProcess::errorOccurred,
            this, [this, process, scriptName](QProcess::ProcessError error) {
        qDebug() << "脚本执行错误:" << scriptName << "错误:" << error;
        showCommandError("错误：脚本执行失败");
        process->deleteLater(); // 自动清理进程对象
    });
    
    // 启动批处理文件
    process->start("cmd.exe", QStringList() << "/c" << scriptPath);
    qDebug() << "启动脚本:" << scriptPath;
}

// 显示指令错误信息
void GLCore::showCommandError(const QString& message) {
    if (timeoutLabel) {
        // 使用超时标签显示错误信息
        timeoutLabel->setText(message);
        
        // 计算标签位置（屏幕中上部）
        int labelWidth = width() - 40;
        int labelX = 20;
        int labelY = height() / 4;
        
        timeoutLabel->setGeometry(labelX, labelY, labelWidth, 40);
        timeoutLabel->setVisible(true);
        
        qDebug() << "显示指令错误信息:" << message;
        
        // 3秒后自动隐藏错误消息
        QTimer::singleShot(3000, this, [this]() {
            if (timeoutLabel) {
                timeoutLabel->setVisible(false);
                timeoutLabel->setText("AI后端响应超时！"); // 恢复原始文本
                qDebug() << "隐藏指令错误信息";
            }
        });
    }
}

// 停止所有音乐播放
void GLCore::stopAllMusic() {
    for (QMediaPlayer* player : musicPlayers) {
        if (player) {
            player->stop();
        }
    }
    
    // 清理已停止的播放器
    for (int i = musicPlayers.size() - 1; i >= 0; --i) {
        QMediaPlayer* player = musicPlayers[i];
        if (player && player->playbackState() != QMediaPlayer::PlayingState) {
            player->deleteLater();
            musicPlayers.removeAt(i);
        }
    }
    
    qDebug() << "已停止所有音乐播放，当前播放器数量:" << musicPlayers.size();
}

// 创建新的音乐播放器
QMediaPlayer* GLCore::createNewMusicPlayer() {
    QMediaPlayer* musicPlayer = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    
    if (musicPlayer && audioOutput) {
        // 设置音频输出
        musicPlayer->setAudioOutput(audioOutput);
        
        // 添加到列表
        musicPlayers.append(musicPlayer);
        audioOutputs.append(audioOutput);
        
        // 连接播放完成信号，自动清理播放器
        connect(musicPlayer, &QMediaPlayer::playbackStateChanged,
                this, [this, musicPlayer, audioOutput](QMediaPlayer::PlaybackState state) {
            if (state == QMediaPlayer::StoppedState) {
                qDebug() << "音乐播放完成，清理播放器";
                musicPlayers.removeOne(musicPlayer);
                audioOutputs.removeOne(audioOutput);
                musicPlayer->deleteLater();
                audioOutput->deleteLater();
            }
        });
        
        qDebug() << "创建新的音乐播放器，当前播放器数量:" << musicPlayers.size();
    }
    
    return musicPlayer;
}

// ========== 对话气泡系统实现 ==========

// 设置对话气泡UI
void GLCore::setupSpeechBubbleUI() {
    // 创建对话气泡Widget
    speechBubbleWidget = new SpeechBubbleWidget(this);
    speechBubbleWidget->setVisible(false); // 初始隐藏
    
    // 创建显示计时器
    bubbleDisplayTimer = new QTimer(this);
    bubbleDisplayTimer->setSingleShot(true);
    connect(bubbleDisplayTimer, &QTimer::timeout, this, &GLCore::onBubbleDisplayTimeout);
    
    // 创建淡入淡出计时器
    bubbleFadeTimer = new QTimer(this);
    connect(bubbleFadeTimer, &QTimer::timeout, this, &GLCore::onBubbleFadeUpdate);
    
    qDebug() << "对话气泡UI初始化完成";
}

// 检查并显示对话气泡
void GLCore::checkAndShowSpeechBubble() {
    // 如果当前正在显示气泡，不检查新的
    if (isBubbleVisible) {
        return;
    }
    
    QString outJsonPath = QDir::currentPath() + "/contact/out.json";
    
    // 检查文件是否存在
    if (!QFile::exists(outJsonPath)) {
        return;
    }
    
    QFile file(outJsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开out.json文件:" << outJsonPath;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "out.json解析错误:" << error.errorString();
        // 解析失败也要删除文件
        deleteSpeechBubbleFile();
        return;
    }
    
    QJsonObject obj = doc.object();
    QString text = obj["text"].toString();
    int displayTime = obj["time"].toInt(3); // 默认3秒
    
    if (text.isEmpty()) {
        qDebug() << "out.json中text为空";
        deleteSpeechBubbleFile();
        return;
    }
    
    qDebug() << "发现对话气泡文件，文本:" << text << "显示时长:" << displayTime << "秒";
    
    // 立即删除文件，避免重复显示
    deleteSpeechBubbleFile();
    
    // 显示对话气泡
    showSpeechBubble(text, displayTime);
}

// 显示对话气泡
void GLCore::showSpeechBubble(const QString& text, int displayTime) {
    if (!speechBubbleWidget) {
        qDebug() << "对话气泡Widget未初始化";
        return;
    }
    
    // 设置文本
    speechBubbleWidget->setText(text);
    
    // 获取推荐大小并调整位置
    QSize bubbleSize = speechBubbleWidget->getRecommendedSize();
    speechBubbleWidget->resize(bubbleSize);
    
    // 计算居中位置
    int x = (width() - bubbleSize.width()) / 2;
    int y = height() * 3 / 4 - bubbleSize.height() / 2;
    speechBubbleWidget->move(x, y);
    
    qDebug() << "对话气泡位置:" << x << y << "大小:" << bubbleSize;
    
    // 开始淡入效果
    isBubbleVisible = true;
    isFadingIn = true;
    bubbleFadeStep = 0;
    
    speechBubbleWidget->setOpacity(0.0);
    speechBubbleWidget->setVisible(true);
    
    // 启动淡入淡出计时器（每50毫秒更新一次，0.5秒完成淡入）
    bubbleFadeTimer->start(50);
    
    // 设置显示时长计时器（淡入0.5秒 + 显示时间 + 淡出0.5秒）
    int totalDisplayTime = 500 + displayTime * 1000 + 500;
    bubbleDisplayTimer->start(displayTime * 1000 + 500); // 淡入后开始计时
    
    qDebug() << "开始显示对话气泡，总时长:" << totalDisplayTime << "毫秒";
}

// 隐藏对话气泡
void GLCore::hideSpeechBubble() {
    if (!isBubbleVisible || !speechBubbleWidget) {
        return;
    }
    
    // 开始淡出效果
    isFadingIn = false;
    bubbleFadeStep = 10; // 从最大不透明度开始淡出
    
    // 如果淡入淡出计时器没有运行，启动它
    if (!bubbleFadeTimer->isActive()) {
        bubbleFadeTimer->start(50);
    }
    
    qDebug() << "开始隐藏对话气泡（淡出效果）";
}

// 更新气泡淡入淡出效果
void GLCore::updateBubbleFade() {
    if (!speechBubbleWidget || !isBubbleVisible) {
        bubbleFadeTimer->stop();
        return;
    }
    
    if (isFadingIn) {
        // 淡入过程
        bubbleFadeStep++;
        qreal opacity = bubbleFadeStep / 10.0;
        speechBubbleWidget->setOpacity(opacity);
        
        if (bubbleFadeStep >= 10) {
            // 淡入完成
            bubbleFadeTimer->stop();
            qDebug() << "对话气泡淡入完成";
        }
    } else {
        // 淡出过程
        bubbleFadeStep--;
        qreal opacity = bubbleFadeStep / 10.0;
        speechBubbleWidget->setOpacity(opacity);
        
        if (bubbleFadeStep <= 0) {
            // 淡出完成，隐藏气泡
            bubbleFadeTimer->stop();
            speechBubbleWidget->setVisible(false);
            isBubbleVisible = false;
            qDebug() << "对话气泡淡出完成并隐藏";
        }
    }
}

// 删除对话气泡文件
void GLCore::deleteSpeechBubbleFile() {
    QString outJsonPath = QDir::currentPath() + "/contact/out.json";
    
    if (QFile::exists(outJsonPath)) {
        if (QFile::remove(outJsonPath)) {
            qDebug() << "成功删除out.json文件";
        } else {
            qDebug() << "删除out.json文件失败:" << outJsonPath;
        }
    }
}

// 对话气泡显示超时槽函数
void GLCore::onBubbleDisplayTimeout() {
    qDebug() << "对话气泡显示时间到，开始隐藏";
    hideSpeechBubble();
}

// 对话气泡淡入淡出更新槽函数
void GLCore::onBubbleFadeUpdate() {
    updateBubbleFade();
}