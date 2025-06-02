#include "SpeechBubbleWidget.h"
#include <QDebug>
#include <QApplication>
#include <cmath>

SpeechBubbleWidget::SpeechBubbleWidget(QWidget* parent)
    : QWidget(parent), bubbleOpacity(1.0), recommendedSize(200, 100), opacityEffect(nullptr)
{
    // 设置窗口属性 - 移除独立窗口设置，只保留透明背景
    setAttribute(Qt::WA_TranslucentBackground);
    // 不要设置FramelessWindowHint，让它作为子widget正常显示
    
    // 创建透明度效果
    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(1.0);
    setGraphicsEffect(opacityEffect);
    
    // 初始化字体
    initializeFont();
    
    qDebug() << "SpeechBubbleWidget创建成功";
}

SpeechBubbleWidget::~SpeechBubbleWidget()
{
    qDebug() << "SpeechBubbleWidget已销毁";
}

void SpeechBubbleWidget::setText(const QString& text)
{
    bubbleText = text;
    updateRecommendedSize();
    update(); // 触发重绘
    qDebug() << "设置对话气泡文本:" << text;
}

void SpeechBubbleWidget::setOpacity(qreal opacity)
{
    bubbleOpacity = qBound(0.0, opacity, 1.0);
    if (opacityEffect) {
        opacityEffect->setOpacity(bubbleOpacity);
    }
    update(); // 触发重绘
}

QSize SpeechBubbleWidget::getRecommendedSize() const
{
    return recommendedSize;
}

void SpeechBubbleWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 设置整体透明度
    painter.setOpacity(bubbleOpacity);
    
    // 绘制对话气泡
    drawSpeechBubble(painter);
}

void SpeechBubbleWidget::initializeFont()
{
    // 从嵌入的资源中加载字体
    int fontId = QFontDatabase::addApplicationFont(":/fonts/adorable.ttf");
    
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QString fontFamily = fontFamilies.first();
            bubbleFont = QFont(fontFamily, 14, QFont::Normal);
            qDebug() << "成功加载嵌入字体:" << fontFamily;
        } else {
            qDebug() << "警告：无法获取嵌入字体的字体族，使用备用字体";
            bubbleFont = QFont("SimSun", 14, QFont::Normal); // 备用字体
        }
    } else {
        qDebug() << "警告：无法加载嵌入字体资源，使用备用字体";
        bubbleFont = QFont("SimSun", 14, QFont::Normal); // 备用字体
    }
    
    // 确保字体可读性
    bubbleFont.setStyleHint(QFont::SansSerif);
    bubbleFont.setPixelSize(16);
    
    qDebug() << "字体初始化完成 - 从嵌入资源加载";
}

void SpeechBubbleWidget::updateRecommendedSize()
{
    if (bubbleText.isEmpty()) {
        recommendedSize = QSize(200, 100);
        return;
    }
    
    QFontMetrics fm(bubbleFont);
    
    // 计算文本显示所需的尺寸
    QRect textRect = fm.boundingRect(QRect(0, 0, 800, 600), Qt::TextWordWrap, bubbleText);
    
    // 添加内边距和箭头空间
    int padding = 20; // 内边距
    int arrowHeight = 30; // 箭头高度
    int minWidth = 120; // 最小宽度
    int maxWidth = 600; // 最大宽度
    
    int bubbleWidth = qBound(minWidth, textRect.width() + padding * 2, maxWidth)+20;
    int bubbleHeight = textRect.height() + padding * 2 + arrowHeight +20;
    
    recommendedSize = QSize(bubbleWidth, bubbleHeight);
    
    qDebug() << "计算推荐气泡大小:" << recommendedSize << "文本:" << bubbleText.left(20) + "...";
}

void SpeechBubbleWidget::drawSpeechBubble(QPainter& painter)
{
    if (bubbleText.isEmpty()) return;
    
    // 计算绘制区域
    QRect fullRect = rect();
    int arrowHeight = 30;
    QRect bubbleRect = fullRect.adjusted(0, 0, 0, -arrowHeight);
    
    // 创建气泡路径
    QPainterPath bubblePath = createBubblePath(bubbleRect);
    
    // 绘制气泡背景（白色）
    painter.fillPath(bubblePath, QBrush(QColor(255, 255, 255)));
    
    // 绘制气泡边框（绿色 #22B14C）
    QPen borderPen(QColor(0x22, 0xB1, 0x4C), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(borderPen);
    painter.drawPath(bubblePath);
    
    // 绘制气泡箭头
    drawArrow(painter, bubbleRect);
    
    // 绘制文本（黑色）
    painter.setPen(QColor(0, 0, 0));
    painter.setFont(bubbleFont);
    
    // 计算文本绘制区域（去除内边距）
    QRect textRect = bubbleRect.adjusted(15, 15, -15, -15);
    painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, bubbleText);
}

QPainterPath SpeechBubbleWidget::createBubblePath(const QRect& rect) const
{
    QPainterPath path;
    int radius = 15; // 圆角半径
    
    // 创建圆角矩形
    path.addRoundedRect(rect, radius, radius);
    
    return path;
}

void SpeechBubbleWidget::drawArrow(QPainter& painter, const QRect& bubbleRect)
{
    // 计算箭头位置（下方靠左，黄金分割比例）
    int bubbleWidth = bubbleRect.width();
    int arrowX = bubbleRect.left() + bubbleWidth * 0.382; // 黄金分割比例
    int arrowY = bubbleRect.bottom();
    
    int arrowWidth = 30;
    int arrowHeight = 25;
    
    // 创建箭头三角形路径
    QPainterPath arrowPath;
    arrowPath.moveTo(arrowX - arrowWidth/2, arrowY);
    arrowPath.lineTo(arrowX + arrowWidth/2, arrowY);
    arrowPath.lineTo(arrowX, arrowY + arrowHeight);
    arrowPath.closeSubpath();
    
    // 绘制箭头背景（白色）
    painter.fillPath(arrowPath, QBrush(QColor(255, 255, 255)));
    
    // 绘制箭头边框（绿色）
    QPen borderPen(QColor(0x22, 0xB1, 0x4C), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(borderPen);
    painter.drawPath(arrowPath);
} 