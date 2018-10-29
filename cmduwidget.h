#ifndef CMDUWIDGET_H
#define CMDUWIDGET_H

#include <QWidget>
#include <QSettings>

class CMDUWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CMDUWidget(QWidget *parent = 0);
    //bool is24HourFormat() const { return m_24HourFormat; }
    bool enabled();
    void setEnabled(const bool b);
    QString text;
    void setCPUPercent(int cpupercent);
    void setMemPercent(int mempercent);

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    bool getMouseEnter();

signals:
    void requestUpdateGeometry() const;
    void requestContextMenu() const;

public slots:
    //void toggleHourFormat();

private:
    QSize sizeHint() const;
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    QPixmap m_cachedIcon;
    QString m_cachedTime;
    QSettings m_settings;
    bool m_24HourFormat;
    QFont font;
    
    QString m_textup;
    QString m_textdown;
    int m_cpupercent;
    int m_mempercent;

    bool m_mouseenter;
};

#endif // CMDUWIDGET_H
