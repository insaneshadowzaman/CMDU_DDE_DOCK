#include "cmduplugin.h"
#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>

CMDUPlugin::CMDUPlugin(QObject *parent)
    : QObject(parent),
      m_tipsLabel(new QLabel),
      m_refershTimer(new QTimer(this)),
      m_settings("deepin", "dde-dock-cmdu")
{
    i=db=ub=dbt=ubt=dbt1=ubt1=dbt0=ubt0=0;
    m_tipsLabel->setObjectName("cmdu");
    m_tipsLabel->setStyleSheet("color:white; padding:0px 3px;");
    m_refershTimer->setInterval(1000);
    m_refershTimer->start();
    m_centralWidget = new CMDUWidget;
    connect(m_centralWidget, &CMDUWidget::requestContextMenu, [this] { m_proxyInter->requestContextMenu(this, QString()); });
    connect(m_centralWidget, &CMDUWidget::requestUpdateGeometry, [this] { m_proxyInter->itemUpdate(this, QString()); });
    connect(m_refershTimer, &QTimer::timeout, this, &CMDUPlugin::updateCMDU);

    // Boot time
    QProcess *process = new QProcess;
    process->start("systemd-analyze");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QString SD = PO.mid(PO.indexOf("=") + 1, PO.indexOf("\n") - PO.indexOf("=") - 1);
    SD.replace("min"," minute");
    SD.replace("ms"," milisecond");
    SD.replace("s"," second");
    startup = "start up: " + SD;
}

const QString CMDUPlugin::pluginName() const
{
    return "cmdu";
}

const QString CMDUPlugin::pluginDisplayName() const
{
    return "Speed";
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
    about["itemText"] = "on";
    about["isActive"] = true;
    items.push_back(about);

    QMap<QString, QVariant> changelog;
    changelog["itemId"] = "changelog";
    changelog["itemText"] = "update log";
    changelog["isActive"] = true;
    items.push_back(changelog);

    QMap<QString, QVariant> boot_analyze;
    boot_analyze["itemId"] = "boot_analyze";
    boot_analyze["itemText"] = "start analysis";
    boot_analyze["isActive"] = true;
    items.push_back(boot_analyze);

    QMap<QString, QVariant> boot_record;
    boot_record["itemId"] = "boot_record";
    boot_record["itemText"] = "boot record";
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
    QMessageBox aboutMB(QMessageBox::NoIcon, "System Information 3.5", "About \n\n Deep Linux system shows a plug-in that displays the network speed in the taskbar, the mouse hover shows the boot time, CPU usage, memory usage, download bytes, and upload bytes. \n Author: Huang Ying \nE-mail: sonichy@163.com \n Translator: Rahat Zaman \nE-mail: insaneshadowzaman@gmail.com \nSource:https: //github.com/insaneshadowzaman/CMDU_DDE_DOCK \nSource: https: //github.com/sonichy/CMDU_DDE_DOCK \n Acknowledgments: \nlinux028@deepin.org");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}

void CMDUPlugin::changeLog()
{
    QString s = "Too lazy to add update log.";
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("system message");
    dialog->setFixedSize(400,300);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *pushbutton_confirm = new QPushButton("determine");
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

QString CMDUPlugin::KB(long k)
{
    QString s = "";
    if(k > 999999){
        s = QString::number(k/(1024*1024.0),'f',2) + "GB";
    }else{
        if(k > 999){
            s = QString::number(k/1024.0,'f',2) + "MB";
        }else{
            s = QString::number(k/1.0,'f',2) + "KB";
        }
    }
    return s;
}

QString CMDUPlugin::BS(long b)
{
    QString s = "";
    if(b > 999999999){
        s = QString::number(b/(1024*1024*1024.0),'f',2) + "GB";
    }else{
        if(b > 999999){
            s = QString::number(b/(1024*1024.0),'f',2) + "MB";
        }else{
            if(b>999){
                s = QString::number(b/1024.0,'f',2) + "KB";
            }else{
                s = QString::number(0,'f',2) + "KB";
            }
        }
    }
    return s;
}

void CMDUPlugin::updateCMDU()
{
    // Boot
    QFile file("/proc/uptime");
    file.open(QIODevice::ReadOnly);
    QString l = file.readLine();
    file.close();
    QTime t(0,0,0);
    t = t.addSecs(l.left(l.indexOf(".")).toInt());
    QString uptime = "Boot: " + t.toString("hh:mm:ss");

    // RAM
    file.setFileName("/proc/meminfo");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    long mt = l.replace("MemTotal:","").replace("kB","").replace(" ","").toLong();
    l = file.readLine();
    l = file.readLine();
    long ma = l.replace("MemAvailable:","").replace("kB","").replace(" ","").toLong();
    l = file.readLine();
    l = file.readLine();
    file.close();
    long mu = mt - ma;
    int mp = mu*100/mt;
    QString mem = "RAM: " + QString("%1/%2=%3").arg(KB(mu)).arg(KB(mt)).arg(QString::number(mp) + "%");

    // CPU
    file.setFileName("/proc/stat");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    QByteArray ba;
    ba = l.toLatin1();
    const char *ch;
    ch = ba.constData();
    char cpu[5];
    long user,nice,sys,idle,iowait,irq,softirq,tt;
    sscanf(ch,"%s%ld%ld%ld%ld%ld%ld%ld",cpu,&user,&nice,&sys,&idle,&iowait,&irq,&softirq);
    tt = user + nice + sys + idle + iowait + irq + softirq;
    file.close();
    QString cusage = "";
    if(i>0) cusage = "CPU: " + QString::number(((tt-tt0)-(idle-idle0))*100/(tt-tt0)) + "%";
    idle0 = idle;
    tt0 = tt;

    // Speed
    file.setFileName("/proc/net/dev");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    l = file.readLine();
    dbt1=ubt1=0;
    while(!file.atEnd()){
        l = file.readLine();
        QStringList list = l.split(QRegExp("\\s{1,}"));
        db = list.at(1).toLong();
        ub = list.at(9).toLong();
        dbt1 += db;
        ubt1 += ub;
    }
    file.close();
    QString dss = "";
    QString uss = "";
    if(i > 0){
        long ds = dbt1 - dbt0;
        long us = ubt1 - ubt0;
        dss = BS(ds) + "/s";
        uss = BS(us) + "/s";
        dbt0 = dbt1;
        ubt0 = ubt1;
    }
    QString netspeed = "↑ " + uss + "\n↓ " + dss;
    QString net = "Upload: " + BS(ubt1) + "  " + uss + "\nDownload: " + BS(dbt1) + "  " + dss;

    i++;
    if(i>2)i=2;

    // Draw
    m_tipsLabel->setText(startup + "\n" + uptime + "\n" + cusage + "\n" + mem + "\n" + net);
    m_centralWidget->text = netspeed;
    m_centralWidget->mp = mp;
    m_centralWidget->update();

}

void CMDUPlugin::bootRecord()
{
    QProcess *process = new QProcess;
    process->start("last reboot");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("Boot record");
    dialog->setFixedSize(500,400);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(PO);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("determine");
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
    QProcess *process = new QProcess;
    process->start("systemd-analyze blame");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("Startup process takes time");
    dialog->setFixedSize(500,400);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(PO);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("determine");
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