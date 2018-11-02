#ifndef SYSINFO_H
#define SYSINFO_H

#include <QTimer>

// 需要平滑计算网速时可以打开
#define __OPEN_NET_SPEED_SMOOTH__    1
// 网速平滑处理时，需要保存的数据数
#define __NET_SPEED_COUNT__          3

struct strproc {
    QString *pid;
    QString *name;
    // 进程总会变动，需要剔除和加入，每次使用前把所有exist置0，统计完CPU占用后，再检索，为0删除
    bool exist;
    // 之前的 utime + stime
    long int bticks;
    // CPU占用
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

    // 需要是否空间
    std::vector<struct strproc *> pvstrproc;
};

#endif // SYSINFO_H
