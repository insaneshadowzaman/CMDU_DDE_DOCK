#include "cmduwidget.h"
#include "dde-dock/constants.h"
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <QMouseEvent>

#define PLUGIN_STATE_KEY    "enable"

CMDUWidget::CMDUWidget(QWidget *parent)
    : QWidget(parent),
      m_settings("deepin", "dde-dock-cmdu")
{
    font.setFamily("Noto Mono");
    m_textup = "9.99KB/s↑   ";
    m_textdown = "9.99KB/s↓   ";
    // "9.99KB/s↑   \n9.99KB/s↓   "
    text = m_textup + "\n" + m_textdown;
    m_cpupercent = 0;
    m_mempercent = 0;
    m_mouseenter = false;
}

void CMDUWidget::setCPUPercent(int cpupercent)
{
    m_cpupercent = cpupercent;
}

void CMDUWidget::setMemPercent(int mempercent)
{
    m_mempercent = mempercent;
}

bool CMDUWidget::enabled()
{
    return m_settings.value(PLUGIN_STATE_KEY, true).toBool();
}

void CMDUWidget::setEnabled(const bool b)
{
    m_settings.setValue(PLUGIN_STATE_KEY, b);
}

QSize CMDUWidget::sizeHint() const
{
    QFontMetrics FM(font);
    return FM.boundingRect(m_textup).size() + QSize(0, FM.boundingRect(m_textdown).height());
}

void CMDUWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

void CMDUWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);

    // 显示网速    
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    painter.setFont(font);
    //painter.drawText(rect().adjusted(2,0,0,0), Qt::AlignLeft | Qt::AlignVCenter, text);
    painter.drawText( 2, 0, width() - 12, height(), Qt::AlignRight | Qt::AlignVCenter, text); 

    // 显示CPU和内存占用彩条
    QLinearGradient linearGradient(0, height(), 0, 0);
    linearGradient.setColorAt(0,Qt::green);
    linearGradient.setColorAt(1,Qt::red);
    painter.setBrush(linearGradient);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.drawRect(width() - 8, height()*(100-m_cpupercent)/100, 3, height()*m_cpupercent/100);
    painter.drawRect(width() - 3, height()*(100-m_mempercent)/100, 3, height()*m_mempercent/100);    
}

void CMDUWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::RightButton)
        return QWidget::mousePressEvent(e);

    const QPoint p(e->pos() - rect().center());
    if (p.manhattanLength() < std::min(width(), height()) * 0.8 * 0.5)
    {
        emit requestContextMenu();
        return;
    }

    QWidget::mousePressEvent(e);
}

void CMDUWidget::enterEvent(QEvent *event)
{
    m_mouseenter = true;
    QWidget::enterEvent(event);
}

void CMDUWidget::leaveEvent(QEvent *event)
{
    m_mouseenter = false;
    QWidget::leaveEvent(event);
}

bool CMDUWidget::getMouseEnter()
{
    return m_mouseenter;
}
