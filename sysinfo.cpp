#include "sysinfo.h"
#include <QProcess>
#include <QFile>
#include <QTime>
#include <sys/time.h>
#include <QDir>
#include <QDirIterator>
#include <vector>

SysInfo::SysInfo()
{
    m_startupfinishedtime = QString();
    m_beforecputotaltime = 0;
    m_beforeidle = 0;
    m_cuppercent = 0;
    m_mempercent = 0;
    m_beforetotalup = 0;
    m_beforetotaldown = 0;
    m_bootrecord = QString();
    m_bootanalyze = QString();

    getCPUString();
    getMemoryString();
    long int upspeed, downspeed;
    m_beforeclock = (double)clock();
    getNetSpeed(upspeed, downspeed);
}

const QString SysInfo::getStartupFinishedTime()
{
    if(m_startupfinishedtime == QString())
    {
        QProcess *process = new QProcess;
        process->start("systemd-analyze");
        process->waitForFinished();
        QString pread = process->readAllStandardOutput();

        m_startupfinishedtime = pread.mid(pread.indexOf("=") + 1, pread.indexOf("\n") - pread.indexOf("=") - 1);
        m_startupfinishedtime.replace("min"," 分");
        m_startupfinishedtime.replace("ms"," 毫秒");
        m_startupfinishedtime.replace("s"," 秒");
    }
    return m_startupfinishedtime;
}

const QString SysInfo::getUptime()
{
    // 开机到现在运行了多久
    QFile rfile("/proc/uptime");
    rfile.open(QIODevice::ReadOnly);
    QString line = rfile.readLine();
    rfile.close();

    QTime qtime(0,0,0);
    qtime = qtime.addSecs(line.left(line.indexOf(".")).toInt());
    return qtime.toString("hh:mm:ss");
}

int SysInfo::getCPUPercent()
{
    return m_cuppercent;
}

const QString SysInfo::getCPUString()
{
    // CPU
    QFile rfile("/proc/stat");
    rfile.open(QIODevice::ReadOnly);
    QString line = rfile.readLine();
    rfile.close();

    char cpu[5];
    long user, nice, sys, idle, iowait, irq, softirq;
    QByteArray cpudata = line.toLatin1();
    sscanf(cpudata.constData(), "%s%ld%ld%ld%ld%ld%ld%ld", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);

    long currentcputotaltime = user + nice + sys + idle + iowait + irq + softirq;
    int cpupercent = ((currentcputotaltime - m_beforecputotaltime) - (idle - m_beforeidle)) * 100 / (currentcputotaltime - m_beforecputotaltime);
    m_beforeidle = idle;
    m_beforecputotaltime = currentcputotaltime;
    m_cuppercent = cpupercent;
    return QString::number(cpupercent) + "%";
}

int SysInfo::getMemoryPercent()
{
    return m_mempercent;
}

const QString SysInfo::getMemoryString()
{
    // 内存
    QFile rfile("/proc/meminfo");
    rfile.open(QIODevice::ReadOnly);

    //总内存
    QString line = rfile.readLine();
    long memtotal = line.replace("MemTotal:", "").replace("kB", "").replace(" ", "").toLong();

    //剩余内存
    line = rfile.readLine();
    line = rfile.readLine();
    long memavilable = line.replace("MemAvailable:","").replace("kB","").replace(" ","").toLong();

    rfile.close();

    long memused = memtotal - memavilable;
    int memusedpercent = memused * 100 / memtotal;
    m_mempercent = memusedpercent;
    return QString("%1 / %2 = %3").arg(bytetoKBMBGB(memused<<10)).arg(bytetoKBMBGB(memtotal<<10)).arg(QString::number(memusedpercent) + "%");
}

void SysInfo::getNetSpeed(long int &upspeed, long int &downspeed)
{
    // 为了网速不变来变去，精确计算时间
    double currentclock = (double)clock();
    long int mselapsed = (long int)(currentclock - m_beforeclock);
    m_beforeclock = currentclock;

    // 网速
    QFile rfile("/proc/net/dev");
    rfile.open(QIODevice::ReadOnly);

    // 跳过头两行
    QString line = rfile.readLine();
    line = rfile.readLine();

    long int currenttotalup = 0;
    long int currenttotaldown = 0;
    while(!rfile.atEnd()){
        line = rfile.readLine();
        QStringList list = line.split(QRegExp("\\s{1,}"));
        currenttotalup += list.at(9).toLong();
        currenttotaldown += list.at(1).toLong();;
    }
    rfile.close();

    // 计算网速
    upspeed = (currenttotalup - m_beforetotalup) * 1000 / mselapsed;
    downspeed = (currenttotaldown - m_beforetotaldown) * 1000 / mselapsed;

    // 保存当前总上传下载
    m_beforetotalup = currenttotalup;
    m_beforetotaldown = currenttotaldown;

#if __OPEN_NET_SPEED_SMOOTH__
    // 网速有时候变化很大，为了平滑网速，需要求一段时间的平均值

    static long int upspeedarray[__NET_SPEED_COUNT__] = {0};
    static long int downspeedarray[__NET_SPEED_COUNT__] = {0};
    static int arrayhead = 0;
    
    upspeedarray[arrayhead] = upspeed;
    downspeedarray[arrayhead] = downspeed;
    arrayhead ++;
    if (arrayhead >= __NET_SPEED_COUNT__)
        arrayhead = 0;
    
    upspeed = 0;
    downspeed = 0;
    for (int i = 0; i < __NET_SPEED_COUNT__; i++)
    {
        upspeed += upspeedarray[i];
        downspeed += downspeedarray[i];
    }
    // 头9秒的网速不准确
    upspeed = upspeed / __NET_SPEED_COUNT__;
    downspeed = downspeed / __NET_SPEED_COUNT__;
#endif // __OPEN_NET_SPEED_SMOOTH__
}

void SysInfo::getNetTotalUpDown(long int &totalup, long int &totaldown)
{
    totalup = m_beforetotalup;
    totaldown = m_beforetotaldown;
}

// 之前的网速显示固定位KB，可以删除了
QString SysInfo::bytetoKB(long byte)
{
    QString s = "";
    if(byte > 999) {
        return QString("%1").arg(byte/1024, 5, 'f', 0, QLatin1Char(' ')) + "KB";
    } else {
        return QString("%1").arg(0, 5, 'f', 0, QLatin1Char(' ')) + "KB";
    }
}

QString SysInfo::bytetoKBMBGB(long byte)
{
    QString s = "";
    if(byte > 999999999) {
        s = QString::number(byte / (1024 * 1024 * 1024.0), 'f', 2) + "GB";
    } else if(byte > 999999) {
        s = QString::number(byte / (1024 * 1024.0), 'f', 2) + "MB";
    } else if(byte > 999) {
        s = QString::number(byte / 1024.0, 'f', 2) + "KB";
    } else {
        s = byte + "B";
    }
    return s;
}

// 网速显示专用，最多显示1.23宽，最大值999，最小值0.01KB，要不动来动去的
QString SysInfo::bytetoKBMBGBforSpeed(long byte)
{
    QString s = "";
    long int result;
    QString unit;

    if(byte > 999999999) {
        result = (long int)(byte / (1024 * 1024 * 1024.0) * 100);
        unit = "GB";
    } else if(byte > 999999) {
        result = (long int)(byte / (1024 * 1024.0) * 100);
        unit = "MB";
    } else if(byte > 999) {
        result = (long int)(byte / 1024.0 * 100);
        unit = "KB";
    } else {
        //return QString::number(byte) + "B";
        result = (long int)(byte / 1024.0 * 100);
        unit = "KB";
    }

    // 小于等于999
    if (result < 1000)
        s = QString::number(result / 100.0, 'f', 2) + unit;
    // 小于等于9999
    else if (result < 10000)
        s = QString::number(result / 100.0, 'f', 1) + unit;
    // 小于等于99999
    else if (result < 100000)
        s = QString::number(result / 100.0, 'f', 0) + unit;

    return s;
}

QString SysInfo::getBootRecord()
{
    if(m_bootrecord == QString()) {
        QProcess *process = new QProcess;
        process->start("last reboot");
        process->waitForFinished();
        m_bootrecord = process->readAllStandardOutput();
    }
    return m_bootrecord;
}

QString SysInfo::getBootAnalyze()
{
    if(m_bootanalyze == QString()) {
        QProcess *process = new QProcess;
        process->start("systemd-analyze blame");
        process->waitForFinished();
        m_bootanalyze = process->readAllStandardOutput();
    }
    return m_bootanalyze;
}

QString SysInfo::getBusyProcesses()
{
    QDir dir("/proc");
    QStringList pidlist = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 清空exist
    for(int i = 0; i < pvstrproc.size(); i++) {
        pvstrproc[i]->exist = false;
    }

    for (int i = 0; i < pidlist.size(); i++) {
        bool ok;
        pidlist[i].toInt(&ok, 10);
        if (ok) {
            // 查找pid，未找到则创建
            int curri = 0;
            for(curri = 0; curri < pvstrproc.size(); curri++) {
                if (pvstrproc[curri]->pid == pidlist[i]) {
                    pvstrproc[curri]->exist = true;
                    break;
                }
            }
            // 为新进程新建记录
            if (curri == pvstrproc.size()) {
                pvstrproc.push_back(new struct strproc);
                pvstrproc[curri]->pid = new QString(pidlist[i]);
                pvstrproc[curri]->name = NULL;
                pvstrproc[curri]->exist = true;
                pvstrproc[curri]->bticks = 0;
                pvstrproc[curri]->percent = 0;
            }

            // CPU：/proc/15128/stat
            QFile rfile("/proc/" + pidlist[i] + "/" + "stat");
            rfile.open(QIODevice::ReadOnly);
            QString line = rfile.readLine();
            rfile.close();
            QStringList array = line.split(" ");
            // 0为PID，1为名字，14为utime，15为stime
            if (pvstrproc[curri]->name == QString()) {
                pvstrproc[curri]->name = new QString(array[1].mid(1, array[1].count() - 2));
            }
            long int cticks = array[14].toLong() + array[15].toLong();
            pvstrproc[curri]->percent = (int)(cticks - pvstrproc[curri]->bticks);
            pvstrproc[curri]->bticks = cticks;
        }
    }

    // 删除已经被关闭进程的记录
    for(int i = pvstrproc.size() - 1; i >= 0; i--) {
        if (pvstrproc[i]->exist == false) {
            delete pvstrproc[i]->pid;
            delete pvstrproc[i]->name;
            delete pvstrproc[i];
            pvstrproc.erase(pvstrproc.begin() + i);
        }
    }

    // 找出占CPU最高的三个进程
    struct strproc t[3] = {0};
    for(int i = 0; i < pvstrproc.size(); i++) {
        for (int j = 0; j < 3; j++) {
            if (pvstrproc[i]->percent > t[j].percent) {
                for (int k = 2; k > j; k--) {
                    t[k] = t[k - 1];
                }
                t[j] = *pvstrproc[i];
                break;
            }
        }
    }

    /*return t[0].pid + QString("\t") + t[0].name + "\t\t" + QString().number(t[0].percent) + "%\n" + \
            t[1].pid + "\t" + t[1].name + "\t\t" + QString().number(t[1].percent) + "%\n" + \
            t[2].pid + "\t" + t[2].name + "\t\t" + QString().number(t[2].percent) + "%";*/
    /*return QString("%1 %2 %3%\n").arg(t[0].pid->toLocal8Bit().data(), 5, QChar(' ')).arg(t[0].name->toLocal8Bit().data(), 16, QChar(' ')).arg(t[0].percent, 3, 10, QChar(' ')) + \
            QString("%1 %2 %3%\n").arg(t[1].pid->toLocal8Bit().data(), 5, QChar(' ')).arg(t[1].name->toLocal8Bit().data(), 16, QChar(' ')).arg(t[1].percent, 3, 10, QChar(' ')) + \
            QString("%1 %2 %3%").arg(t[2].pid->toLocal8Bit().data(), 5, QChar(' ')).arg(t[2].name->toLocal8Bit().data(), 16, QChar(' ')).arg(t[2].percent, 3, 10, QChar(' '));*/
    return QString("<tr><td>%1</td><td>%2</td><td align='right'>%3% </td></tr>").arg(t[0].pid->toLocal8Bit().data()).arg(t[0].name->toLocal8Bit().data()).arg(t[0].percent) + \
            QString("<tr><td>%1</td><td>%2</td><td align='right'>%3% </td></tr>").arg(t[1].pid->toLocal8Bit().data()).arg(t[1].name->toLocal8Bit().data()).arg(t[1].percent) + \
            QString("<tr><td>%1</td><td>%2</td><td align='right'>%3% </td></tr>").arg(t[2].pid->toLocal8Bit().data()).arg(t[2].name->toLocal8Bit().data()).arg(t[2].percent);
}
