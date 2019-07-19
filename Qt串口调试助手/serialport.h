#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>
#include <QTimer>

namespace Ui {
class SerialPort;
}

class SerialPort : public QMainWindow
{
    Q_OBJECT

public:
    explicit SerialPort(QWidget *parent = 0);
    ~SerialPort();

    /* 初始状态栏 */
    void InitStatusBar();   // 初始化状态栏
    void SetSerState();     // 设置状态栏串口状态
    void SetRecvNum();      // 设置接收字节数
    void SetSendNum();      // 设置发送字节数

    /* 初始化CombBox控件 */
    void InitCommCmb();         // 初始化CombBox控件
    void SetPortNumCmb();       // 设置串口号
    QStringList GetEnableCommPortQt();      // 获取计算机可用串口 Qt方式

    void SetBaudCmb();          // 设置波特率
    void SetDPaityCmb();        // 设置校验位
    void SetDataBCmb();         // 设置数据位
    void SetStopBCmb();         // 设置停止位
    void SetStreamCmb();        // 设置流控制

    bool SetSerialPortParam(QSerialPort *serial);   // 设置串口参数，失败返回false，成功返回true

    void StringToHex(QString str, QByteArray &senddata);
    char ConvertHexChar(char ch);
    QTimer *timer;

    /* 父类函数重写 */
    void closeEvent(QCloseEvent *event);

private slots:
    void on_baudRCmb_currentIndexChanged(int index);    // 自定义波特率
    void on_OnOffBtn_clicked();                         // 打开/断开串口
    void on_ReflushSerPortBtn_clicked();    // 刷新串口，重新扫描计算机可用串口
    void slot_RecvPortData();               // 接收串口数据
    void on_SendBtn_clicked();              // 发送数据
    void on_ClearRecvBtn_clicked();         // 清空接收区
    void slot_ResetNumBtn_clicked();          // 复位计数
    void slot_DingshiSend(int state);         //定时发送
    void Timerupdate();

private:
    Ui::SerialPort *ui;

    /* 状态栏控件 */
    QLabel *m_SerStateLbl;        // 串口状态
    QLabel *m_RecvNumLbl;         // 接收字节数
    QLabel *m_SendNumLbl;         // 发送字节数
    QPushButton *m_ResetNumBtn;   // 复位计数按钮

    /* 发送、接收字节数 */
    uint m_nRecvNum;              // 接收字节数
    uint m_nSendNum;              // 发送字节数
    bool m_bOpen;                 // 标识串口状态

    QSerialPort *m_serial;        // 串口通信类对象
    qint64 m_nReadBuffSize;       // 串口缓冲区大小
};

#endif // SERIALPORT_H
