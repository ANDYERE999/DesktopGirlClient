#pragma once

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>
#include <QString>
#include <QFontDatabase>
#include <QDir>
#include <QGraphicsOpacityEffect>

class SpeechBubbleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpeechBubbleWidget(QWidget* parent = nullptr);
    ~SpeechBubbleWidget() override;
    
    // 设置对话内容
    void setText(const QString& text);
    
    // 设置透明度（用于淡入淡出效果）
    void setOpacity(qreal opacity);
    
    // 获取推荐的气泡大小
    QSize getRecommendedSize() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString bubbleText;          // 对话内容
    qreal bubbleOpacity;         // 透明度
    QFont bubbleFont;            // 字体
    QSize recommendedSize;       // 推荐大小
    QGraphicsOpacityEffect* opacityEffect; // 透明度效果
    
    // 私有方法
    void initializeFont();       // 初始化字体
    void updateRecommendedSize(); // 更新推荐大小
    void drawSpeechBubble(QPainter& painter); // 绘制对话气泡
    QPainterPath createBubblePath(const QRect& rect) const; // 创建气泡路径
    void drawArrow(QPainter& painter, const QRect& bubbleRect); // 绘制气泡箭头
}; 