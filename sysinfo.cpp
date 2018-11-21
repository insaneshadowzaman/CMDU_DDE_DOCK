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
        m_startupfinishedtime.replace("min"," Minute");
        m_startupfinishedtime.replace("ms"," millisecond");
        m_startupfinishedtime.replace("s"," second");
    }
    return m_startupfinishedtime;
}

const QString SysInfo::getUptime()
{
    // How long has it been running since it was turned on?
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
    // RAM
    QFile rfile("/proc/meminfo");
    rfile.open(QIODevice::ReadOnly);

    //Total memory
    QString line = rfile.readLine();
    long memtotal = line.replace("MemTotal:", "").replace("kB", "").replace(" ", "").toLong();

    //Remaining memory
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
    // In order to change the network speed, calculate the time accurately.
    double currentclock = (double)clock();
    long int mselapsed = (long int)(currentclock - m_beforeclock);
    m_beforeclock = currentclock;

    // Speed
    QFile rfile("/proc/net/dev");
    rfile.open(QIODevice::ReadOnly);

    // Skip the first two lines
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

    // Calculating network speed
    upspeed = (currenttotalup - m_beforetotalup) * 1000 / mselapsed;
    downspeed = (currenttotaldown - m_beforetotaldown) * 1000 / mselapsed;

    // Save current total upload download
    m_beforetotalup = currenttotalup;
    m_beforetotaldown = currenttotaldown;

#if __OPEN_NET_SPEED_SMOOTH__
    // The speed of the network sometimes varies greatly. In order to smooth the speed of the network, it is required to average the time.

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
    // The first 9 seconds of network speed is not accurate
    upspeed = upspeed / __NET_SPEED_COUNT__;
    downspeed = downspeed / __NET_SPEED_COUNT__;
#endif // __OPEN_NET_SPEED_SMOOTH__
}

void SysInfo::getNetTotalUpDown(long int &totalup, long int &totaldown)
{
    totalup = m_beforetotalup;
    totaldown = m_beforetotaldown;
}

// The previous network speed shows a fixed bit KB, which can be deleted.
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

// Dedicated to the network speed display, up to 1.23 wide, maximum 999, minimum 0.01 KB, or not to move
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

    // Less than or equal to 999
    if (result < 1000)
        s = QString::number(result / 100.0, 'f', 2) + unit;
    // Less than or equal to 9999
    else if (result < 10000)
        s = QString::number(result / 100.0, 'f', 1) + unit;
    // Less than or equal to 99999
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

    // Empty exist
    for(int i = 0; i < pvstrproc.size(); i++) {
        pvstrproc[i]->exist = false;
    }

    for (int i = 0; i < pidlist.size(); i++) {
        bool ok;
        pidlist[i].toInt(&ok, 10);
        if (ok) {
            // Find pid, create if not found
            int curri = 0;
            for(curri = 0; curri < pvstrproc.size(); curri++) {
                if (pvstrproc[curri]->pid == pidlist[i]) {
                    pvstrproc[curri]->exist = true;
                    break;
                }
            }
            // Create a new record for the new process
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
            // 0 is PID, 1 is the name, 14 is utime, 15 is stime
            // Special circumstances that need to be addressed(Web Content)
            long int cticks;
            if (array[1].endsWith(")")) {
                if (pvstrproc[curri]->name == QString()) {
                    pvstrproc[curri]->name = new QString(array[1].mid(1, array[1].count() - 2));
                }
                cticks = array[14].toLong() + array[15].toLong();
            } else {
                if (pvstrproc[curri]->name == QString()) {                    
                    pvstrproc[curri]->name = new QString((array[1] + " " + array[2]).mid(1, array[1].count() + array[2].count() - 1));
                }
                cticks = array[15].toLong() + array[16].toLong();
            }
            pvstrproc[curri]->percent = (int)(cticks - pvstrproc[curri]->bticks);
            pvstrproc[curri]->bticks = cticks;
        }
    }

    // Delete records of processes that have been closed
    for(int i = pvstrproc.size() - 1; i >= 0; i--) {
        if (pvstrproc[i]->exist == false) {
            delete pvstrproc[i]->pid;
            delete pvstrproc[i]->name;
            delete pvstrproc[i];
            pvstrproc.erase(pvstrproc.begin() + i);
        }
    }

    // Find the three processes that account for the highest CPU
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

    return QString("<tr><td>%1</td><td>%2</td><td align='right'>%3% </td></tr>").arg(t[0].pid->toLocal8Bit().data()).arg(t[0].name->toLocal8Bit().data()).arg(t[0].percent) + \
            QString("<tr><td>%1</td><td>%2</td><td align='right'>%3% </td></tr>").arg(t[1].pid->toLocal8Bit().data()).arg(t[1].name->toLocal8Bit().data()).arg(t[1].percent) + \
            QString("<tr><td>%1</td><td>%2</td><td align='right'>%3% </td></tr>").arg(t[2].pid->toLocal8Bit().data()).arg(t[2].name->toLocal8Bit().data()).arg(t[2].percent);
}
