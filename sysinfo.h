#ifndef SYSINFO_H
#define SYSINFO_H

#include <QTimer>

// Can be opened when you need to smoothly calculate the network speed
#define __OPEN_NET_SPEED_SMOOTH__    1
// The number of data to be saved when the network speed is smoothed
#define __NET_SPEED_COUNT__          3

struct strproc {
    QString *pid;
    QString *name;
    // The process will always change, need to be culled and added, set all exist to 0 before each use, after counting the CPU usage, then retrieve, delete 0
    bool exist;
    // previous utime + stime
    long int bticks;
    // CPU usage
    int percent;
};

class SysInfo
{
public:
    SysInfo();

    const QString getStartupFinishedTime();
    const QString getUptime();
    int getCPUPercent();
    const QString getCPUString();
    int getMemoryPercent();
    const QString getMemoryString();
    void getNetSpeed(long int &upspeed, long int &downspeed);
    void getNetTotalUpDown(long int &totalup, long int &totaldown);

    QString bytetoKB(long byte);
    QString bytetoKBMBGB(long byte);
    QString bytetoKBMBGBforSpeed(long byte);

    QString getBootRecord();
    QString getBootAnalyze();
    QString getBusyProcesses();

private:
    QString m_startupfinishedtime;
    long int m_beforecputotaltime, m_beforeidle;
    int m_cuppercent, m_mempercent;
    double m_beforeclock;
    long int m_beforetotalup, m_beforetotaldown;
    QString m_bootrecord;
    QString m_bootanalyze;

    // Need space?
    std::vector<struct strproc *> pvstrproc;
};

#endif // SYSINFO_H
