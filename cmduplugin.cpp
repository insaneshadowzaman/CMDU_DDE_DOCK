#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>

#include "cmduplugin.h"
#include "sysinfo.h"

CMDUPlugin::CMDUPlugin(QObject *parent)
    : QObject(parent),
      m_tipsLabel(new QLabel),
      m_refershTimer(new QTimer(this)),
      m_settings("deepin", "dde-dock-cmdu")
{
    m_tipsLabel->setObjectName("cmdu");
    m_tipsLabel->setStyleSheet("color:white; padding:0px 3px;");
    m_refershTimer->setInterval(1000);
    m_refershTimer->start();
    m_centralWidget = new CMDUWidget;
    connect(m_centralWidget, &CMDUWidget::requestContextMenu, [this] { m_proxyInter->requestContextMenu(this, QString()); });
    connect(m_centralWidget, &CMDUWidget::requestUpdateGeometry, [this] { m_proxyInter->itemUpdate(this, QString()); });
    connect(m_refershTimer, &QTimer::timeout, this, &CMDUPlugin::updateCMDU);

    m_sysinfo = SysInfo();
}

const QString CMDUPlugin::pluginName() const
{
    return "cmdu";
}

const QString CMDUPlugin::pluginDisplayName() const
{
    return "网速";
}

void CMDUPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;
    if (m_centralWidget->enabled())
        m_proxyInter->itemAdded(this, QString());
}

void CMDUPlugin::pluginStateSwitched()
{
    m_centralWidget->setEnabled(!m_centralWidget->enabled());
    if (m_centralWidget->enabled())
        m_proxyInter->itemAdded(this, QString());
    else
        m_proxyInter->itemRemoved(this, QString());
}

bool CMDUPlugin::pluginIsDisable()
{
    return !m_centralWidget->enabled();
}

int CMDUPlugin::itemSortKey(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1").arg(displayMode());
    return m_settings.value(key, 0).toInt();
}

void CMDUPlugin::setSortKey(const QString &itemKey, const int order)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1").arg(displayMode());
    m_settings.setValue(key, order);
}

QWidget *CMDUPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_centralWidget;
}

QWidget *CMDUPlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_tipsLabel;
}

const QString CMDUPlugin::itemCommand(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return "";
}

const QString CMDUPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;
    items.reserve(1);

    QMap<QString, QVariant> about;
    about["itemId"] = "about";
    about["itemText"] = "关于";
    about["isActive"] = true;
    items.push_back(about);

    QMap<QString, QVariant> changelog;
    changelog["itemId"] = "changelog";
    changelog["itemText"] = "更新日志";
    changelog["isActive"] = true;
    items.push_back(changelog);

    QMap<QString, QVariant> boot_analyze;
    boot_analyze["itemId"] = "boot_analyze";
    boot_analyze["itemText"] = "启动分析";
    boot_analyze["isActive"] = true;
    items.push_back(boot_analyze);

    QMap<QString, QVariant> boot_record;
    boot_record["itemId"] = "boot_record";
    boot_record["itemText"] = "开机记录";
    boot_record["isActive"] = true;
    items.push_back(boot_record);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;
    return QJsonDocument::fromVariant(menu).toJson();
}

void CMDUPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey);
    Q_UNUSED(checked);
    if(menuId == "about"){
        about();
    }else if(menuId == "changelog"){
        changeLog();
    }else if(menuId == "boot_analyze"){
        bootAnalyze();
    }else if(menuId == "boot_record"){
        bootRecord();
    }
}

void CMDUPlugin::about()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "系统信息 3.7", "关于\n\n深度Linux系统上一款在任务栏显示网速，鼠标悬浮显示开机时间、CPU占用、内存占用、下载字节、上传字节的插件。\n作者：黄颖\nE-mail: sonichy@163.com\n源码：https://github.com/sonichy/CMDU_DDE_DOCK");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}

void CMDUPlugin::changeLog()
{
    QString s = "更新日志\n\n3.7 (2018-10-12)\n1.内存竖进度条绿色改白色，内存超过90%红色背景改内存竖进度条红色。\n2.补位+等宽字体，解决对齐问题。\n3.网速固定为 KB/s 单位。\n\n3.6 (2018-07-30)\n1.增加内存和CPU竖线。\n\n3.5 (2018-06-25)\n1.增加启动分析和开机记录。\n\n3.4 (2018-06-03)\n1.支持新版dock的排序功能。\n\n3.3 (2018-05-17)\n1.内存超过90%变红预警。\n2.网速小于 999 字节显示为 0.00 KB\n3.使用安全的 QString.right() 替代 QStringList.at()，增加：ms 替换为 毫秒。\n\n3.2 (2018-05-08)\n网速全部计算，不会再出现为0的情况。\n取消启动时间浮窗。\n\n3.1 (2018-03-17)\n修改空余内存计算范围。\n\n3.0 (2018-02-25)\n在新版本时间插件源码基础上修改，解决右键崩溃问题，并支持右键开关。\n\n2.4 (2017-11-11)\n增加开机时间。\n\n2.3 (2017-09-05)\n自动判断网速所在行。\n\n2.２ (2017-07-08)\n1.设置网速所在行。\n\n2.1 (2017-02-01)\n1.上传下载增加GB单位换算，且参数int改long，修复字节单位换算溢出BUG。\n\n2.0 (2016-12-07)\n1.增加右键菜单。\n\n1.0 (2016-11-01)\n1.把做好的Qt程序移植到DDE-DOCK。";
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("系统信息");
    dialog->setFixedSize(400,300);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *pushbutton_confirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(pushbutton_confirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(pushbutton_confirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}

void CMDUPlugin::updateCMDU()
{
    static bool first = true;
    // 网速
    long int upspeed, downspeed;
    long int totalup, totaldown;
    m_sysinfo.getNetSpeed(upspeed, downspeed);
    m_sysinfo.getNetTotalUpDown(totalup, totaldown);
    QString upspeedstring = m_sysinfo.bytetoKBMBGBforSpeed(upspeed) + "/s";
    QString downspeedstring = m_sysinfo.bytetoKBMBGBforSpeed(downspeed) + "/s";

    QString netspeed = upspeedstring + "↑\n" + downspeedstring + "↓";
 
    // QLabel显示信息
    if ((m_centralWidget->getMouseEnter() == true) || first){
        first = false;
        QString startup = QString() + "<tr><td colspan='3'>" + "启动: " + m_sysinfo.getStartupFinishedTime() + "</td></tr>";
        QString uptime = QString() + "<tr><td colspan='3'>" + "开机: " + m_sysinfo.getUptime() + "</td></tr>";
        QString cpuusage = QString() + "<tr><td colspan='3'>" + "CPU: " + m_sysinfo.getCPUString() + "</td></tr>";
        QString memusage = QString() + "<tr><td colspan='3'>" + "内存: " + m_sysinfo.getMemoryString() + "</td></tr>";
        // 流量及网速
        QString tipnet = QString() + "<tr><td colspan='3'>" + "上传: " + m_sysinfo.bytetoKBMBGB(totalup) + "  " + upspeedstring + "</td></tr>" + "<tr><td colspan='3'>下载: " + m_sysinfo.bytetoKBMBGB(totaldown) + "  " + downspeedstring + "</td></tr>";

        QString busyprocesses = QString() + "<tr><td colspan='3'>" + "CPU占用前三：" + "</td></tr>" + m_sysinfo.getBusyProcesses();
        m_tipsLabel->setText("<table border='0'>" + startup + uptime + cpuusage + memusage + tipnet + busyprocesses + "</table>");
    } else {
        m_sysinfo.getCPUString();
        m_sysinfo.getMemoryString();
    }

    m_centralWidget->text = netspeed;
    m_centralWidget->setCPUPercent(m_sysinfo.getCPUPercent());
    m_centralWidget->setMemPercent(m_sysinfo.getMemoryPercent());

    m_centralWidget->update();
}

void CMDUPlugin::bootRecord()
{
    QString bootrecord = m_sysinfo.getBootRecord();

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("开机记录");
    dialog->setFixedSize(500,400);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(bootrecord);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}

void CMDUPlugin::bootAnalyze()
{
    QString bootanalyze = m_sysinfo.getBootAnalyze();

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("启动进程耗时");
    dialog->setFixedSize(500,400);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(bootanalyze);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}
