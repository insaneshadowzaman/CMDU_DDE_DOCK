#ifndef SYSINFO_H
#define SYSINFO_H

#include <QTimer>

// 需要平滑计算网速时可以打开
#define __OPEN_NET_SPEED_SMOOTH__    0
// 网速平滑处理时，需要保存的数据数
#define __NET_SPEED_COUNT__          3

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

private:
    QString m_startupfinishedtime;
    long int m_beforecputotaltime, m_beforeidle;
    int m_cuppercent, m_mempercent;
    double m_beforeclock;
    long int m_beforetotalup, m_beforetotaldown;
    QString m_bootrecord;
    QString m_bootanalyze;
};

#endif // SYSINFO_H
