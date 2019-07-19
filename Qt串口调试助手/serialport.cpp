#include "SerialPort.h"
#include "ui_SerialPort.h"

#include <QSettings>
#include <qt_windows.h>
#include <QMessageBox>
#include <QDebug>
#include <QString>

SerialPort::SerialPort(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SerialPort)
{
    ui->setupUi(this);

    this->setWindowTitle(QString::fromLocal8Bit("串口调试Test2019-07-16 动漫技术部"));
    this->setFixedSize(this->width(), this->height());

    m_nRecvNum = 0;
    m_nSendNum = 0;
    m_bOpen = false;

    InitStatusBar();
    InitCommCmb();

    ui->OnOffBtn->setIcon(QIcon(":/pic/res/OFF.png"));
    ui->RecvDataEdt->setReadOnly(true);

    m_serial = new QSerialPort;
    connect(m_serial, SIGNAL(readyRead()), this, SLOT(slot_RecvPortData()));
    m_nReadBuffSize = 64;

    timer = new QTimer(this);
    ui->spinBox->setValue(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(Timerupdate()));
    connect(ui->checkBox_3, SIGNAL(stateChanged(int)), this, SLOT(slot_DingshiSend(int)));

}

SerialPort::~SerialPort()
{
    delete ui;
    delete m_SerStateLbl;
    delete m_RecvNumLbl;
    delete m_SendNumLbl;
    delete m_ResetNumBtn;
}

/***************************************************************************/
/*                              初始化状态栏                                 */
/***************************************************************************/

void SerialPort::InitStatusBar()
{
    m_SerStateLbl = new QLabel();
    m_RecvNumLbl = new QLabel();
    m_SendNumLbl = new QLabel();
    m_ResetNumBtn = new QPushButton();
    connect(m_ResetNumBtn, SIGNAL(clicked()), this, SLOT(slot_ResetNumBtn_clicked()));

    m_SerStateLbl->setMinimumSize(150, 20);
    m_RecvNumLbl->setMinimumSize(150, 20); // 标签最小尺寸
    m_SendNumLbl->setMinimumSize(150, 20);

    ui->statusBar->addWidget(m_SerStateLbl);
    ui->statusBar->addWidget(m_RecvNumLbl);
    ui->statusBar->addWidget(m_SendNumLbl);
    ui->statusBar->addWidget(m_ResetNumBtn);

    SetSerState();
    SetRecvNum();
    SetSendNum();
    m_ResetNumBtn->setText(QString::fromLocal8Bit("复位计数"));
}

void SerialPort::SetSerState()
{
    QString strState;
    if ( m_bOpen )
       {
        strState = QString::fromLocal8Bit("打开");

        }
    else
        strState = QString::fromLocal8Bit("关闭");

    m_SerStateLbl->setText(QString::fromLocal8Bit("串口状态%1").arg(strState));
}

void SerialPort::SetRecvNum()
{
    QString strRecvNum = QString::number(m_nRecvNum);
    m_RecvNumLbl->setText(QString::fromLocal8Bit("接收字节数%1").arg(strRecvNum));
}

void SerialPort::SetSendNum()
{
    QString strSendNum = QString::number(m_nSendNum);
    m_SendNumLbl->setText(QString::fromLocal8Bit("发送字节数%1").arg(strSendNum));
}

/***************************************************************************/
/*                            初始化CombBox                                 */
/***************************************************************************/

void SerialPort::InitCommCmb()
{
    SetPortNumCmb();    // 串口号
    SetBaudCmb();       // 波特率
    SetDPaityCmb();     // 校验位
    SetDataBCmb();      // 数据位
    SetStopBCmb();      // 停止位
    SetStreamCmb();     // 流控制
}

// 设置串口号
void SerialPort::SetPortNumCmb()
{
    QStringList commPortList = GetEnableCommPortQt();
    if ( !commPortList.isEmpty() )
        ui->PortNumCmb->addItems(commPortList);
}

// 获取计算机可用串口 QSerialPort QSerialPortInfo类
QStringList SerialPort::GetEnableCommPortQt()
{
    QStringList CommPortList;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if (serial.open(QIODevice::ReadWrite))
        {
            CommPortList.append(serial.portName());
            serial.close();
        }
    }

    return CommPortList;
}



 // 设置波特率
void SerialPort::SetBaudCmb()
{
    QStringList baudRList;
    baudRList.append(tr("110"));
    baudRList.append(tr("300"));
    baudRList.append(tr("600"));
    baudRList.append(tr("1200"));
    baudRList.append(tr("2400"));
    baudRList.append(tr("4800"));
    baudRList.append(tr("9600"));
    baudRList.append(tr("14400"));
    baudRList.append(tr("19200"));
    baudRList.append(tr("38400"));
    baudRList.append(tr("56000"));
    baudRList.append(tr("57600"));
    baudRList.append(tr("115200"));
    baudRList.append(tr("128000"));
    baudRList.append(tr("256000"));

    baudRList.append(   QString::fromLocal8Bit("自定义"));

    ui->baudRCmb->addItems(baudRList);
    ui->baudRCmb->setCurrentIndex(6);
}

// 设置校验位
void SerialPort::SetDPaityCmb()
{
    QStringList DPaityList;
    DPaityList.append(tr("NONE"));
    DPaityList.append(tr("ODD"));
    DPaityList.append(tr("EVEN"));
    DPaityList.append(tr("MARK"));
    DPaityList.append(tr("SPACE"));

    ui->DPaityCmb->addItems(DPaityList);
}

 // 设置数据位
void SerialPort::SetDataBCmb()
{
    for (int i = 5; i <= 8; i++)
    {
        QString strDataB = QString::number(i);
        ui->DataBCmb->addItem(strDataB);
    }
    ui->DataBCmb->setCurrentIndex(3);
}

// 设置停止位
void SerialPort::SetStopBCmb()
{
    ui->StopBCmb->addItem(tr("1"));
    ui->StopBCmb->addItem(tr("1.5"));
    ui->StopBCmb->addItem(tr("2"));
}

// 流控制
void SerialPort::SetStreamCmb()
{
    ui->FlowCtrlCmb->addItem(tr("NO"));
    ui->FlowCtrlCmb->addItem(tr("RTS/CTS"));
    ui->FlowCtrlCmb->addItem(tr("XON/XOFF"));
}

/***************************************************************************/
/*                                 槽函数                                   */
/***************************************************************************/
// 波特率自定义
void SerialPort::on_baudRCmb_currentIndexChanged(int index)
{
    uint nCount = ui->baudRCmb->count();
    if ((unsigned)index == nCount - 1)
    {
        ui->baudRCmb->setEditable(TRUE);
        ui->baudRCmb->setItemText(index, tr(""));
    }
    else
    {
        ui->baudRCmb->setEditable(FALSE);

        ui->baudRCmb->setItemText(nCount-1,    QString::fromLocal8Bit("自定义"));
    }
}

// 刷新串口
void SerialPort::on_ReflushSerPortBtn_clicked()
{
    ui->PortNumCmb->clear();
    SetPortNumCmb();
}

// 复位计数
void SerialPort::slot_ResetNumBtn_clicked()
{
    m_nSendNum = 0;
    m_nRecvNum = 0;
    SetSendNum();
    SetRecvNum();
}

// 清空接受区
void SerialPort::on_ClearRecvBtn_clicked()
{
    ui->RecvDataEdt->setText(tr(""));
}
void SerialPort::slot_DingshiSend(int state)
{
    if (state == Qt::Checked) // "选中"
        {
            int timenum=ui->spinBox->text().toInt();
            if(timenum==0)
            {

                ui->checkBox_3->setCheckState(Qt::Unchecked);
                ui->spinBox->setValue(1000);
                timer->stop();
            }
            else timer->start(timenum);
        }

    else // 未选中 - Qt::Unchecked
        {
            timer->stop();
        }

}

void SerialPort::Timerupdate()
{
   on_SendBtn_clicked();
}


/***************************************************************************/
/*                             串口通信                                     */
/***************************************************************************/
// 打开/关闭串口
void SerialPort::on_OnOffBtn_clicked()
{
    if (m_serial->isOpen()) // 已经处于打开状态，则关闭串口
    {
        m_serial->close();
        ui->OnOffBtn->setText(QString::fromLocal8Bit("打开"));
        ui->OnOffBtn->setIcon(QIcon(":/pic/res/OFF.png"));
        m_bOpen = false;
        SetSerState();
    }
    else // 串口处于关闭状态，打开串口
    {
        if ( !SetSerialPortParam(m_serial) )
        {
            QMessageBox::critical(this, tr("Error"),   QString::fromLocal8Bit("串口错误"), QMessageBox::Ok);
            return;
        }

        // 打开串口
        if ( !m_serial->open(QIODevice::ReadWrite) ) // 打开失败
        {
            QMessageBox::critical(this, tr("Error"),   QString::fromLocal8Bit("串口不存在或者被其它程序占用！"), QMessageBox::Ok);
            // QString strRecv = ui->RecvDataEdt->toPlainText();
            // strRecv += tr("\n【Error】Can't Open COM Port!");
            ui->RecvDataEdt->append(QString::fromLocal8Bit("\n【Error】Can't Open COM Port!"));
            return;
        }

        // 设置串口缓冲区大小
        m_serial->setReadBufferSize(m_nReadBuffSize);

        ui->OnOffBtn->setText(QString::fromLocal8Bit("断开"));

        //ui->OnOffBtn->setIcon(QIcon(":/pic/res/ON.png"));
        m_bOpen = true;
        SetSerState();
    }
}

// 设置串口参数，失败返回false，成功返回true
bool SerialPort::SetSerialPortParam(QSerialPort *serial)
{
    // 设置串口号
    QString strPortNum = ui->PortNumCmb->currentText();
    if (strPortNum == tr(""))
        return false;
    serial->setPortName(strPortNum);

    // 设置波特率
    qint32 nBaudRate = ui->baudRCmb->currentText().toInt();
    serial->setBaudRate(nBaudRate);

    // 设置奇偶校验
    int nParityType = ui->DPaityCmb->currentIndex();
    switch (nParityType)
    {
    case 0:
        serial->setParity(QSerialPort::NoParity);
        break;
    case 1:
        serial->setParity(QSerialPort::OddParity);
        break;
    case 2:
        serial->setParity(QSerialPort::EvenParity);
        break;
    case 3:
        serial->setParity(QSerialPort::MarkParity);
        break;
    case 4:
        serial->setParity(QSerialPort::SpaceParity);
        break;
    default:
        serial->setParity(QSerialPort::UnknownParity);
        break;
    }

    // 设置数据位
    int nDataBits = ui->DataBCmb->currentIndex();
    switch (nDataBits)
    {
    case 0:
        serial->setDataBits(QSerialPort::Data5);
        break;
    case 1:
        serial->setDataBits(QSerialPort::Data6);
        break;
    case 2:
        serial->setDataBits(QSerialPort::Data7);
        break;
    case 3:
        serial->setDataBits(QSerialPort::Data8);
        break;
    default:
        serial->setDataBits(QSerialPort::UnknownDataBits);
        break;
    }

    // 设置停止位
    int nStopBits = ui->StopBCmb->currentIndex();
    switch (nStopBits)
    {
    case 0:
        serial->setStopBits(QSerialPort::OneStop);
        break;
    case 1:
        serial->setStopBits(QSerialPort::OneAndHalfStop);
        break;
    case 2:
        serial->setStopBits(QSerialPort::TwoStop);
        break;
    default:
        serial->setStopBits(QSerialPort::UnknownStopBits);
        break;
    }

    // 流控制
    int nFlowCtrl = ui->FlowCtrlCmb->currentIndex();
    switch (nFlowCtrl)
    {
    case 0:
        serial->setFlowControl(QSerialPort::NoFlowControl);
        break;
    case 1:
        serial->setFlowControl(QSerialPort::HardwareControl);
        break;
    case 2:
        serial->setFlowControl(QSerialPort::SoftwareControl);
        break;
    default:
        serial->setFlowControl(QSerialPort::UnknownFlowControl);
        break;
    }

    return true;
}

// 槽函数，接收串口数据
void SerialPort::slot_RecvPortData()
{
    QByteArray bytes = m_serial->readAll();
    if ( !bytes.isEmpty() )
    {
        if(ui->checkBox_2->isChecked())//Hex接收
        {
            QDataStream out1(&bytes,QIODevice::ReadWrite);    //将字节数组读入
            while(!out1.atEnd())
            {
                qint8 outChar = 0;
                out1>>outChar;   //每字节填充一次，直到结束
                //十六进制的转换
                QString str = QString("%1").arg(outChar&0xFF,2,16,QLatin1Char('0'));
                ui->RecvDataEdt->insertPlainText(str);
                qDebug()<<str;
            }
            ui->RecvDataEdt->insertPlainText("\n");
        }

        else
        {
            QString strRecv = QString::fromLocal8Bit(bytes);
            ui->RecvDataEdt->append(strRecv);
        }

        m_nRecvNum += bytes.count();
        SetRecvNum();
    }
    else
        ui->RecvDataEdt->setText(    QString::fromLocal8Bit("接收数据出错！"));


}

// 发送数据，写串口
void SerialPort::on_SendBtn_clicked()
{
    // 串口未打开
    if ( !m_bOpen )
    {
        QMessageBox::warning(this, tr("Error"), QString::fromLocal8Bit("串口未打开，发送失败"), QMessageBox::Ok);
        return;
    }

    QByteArray SendBytes = ui->SendDataEdt->toPlainText().toLocal8Bit();
    if ( !SendBytes.isEmpty() )
    {

        if(ui->checkBox->isChecked())//Hex发送
        {
            QByteArray senddata1;
            StringToHex(SendBytes,senddata1);//将str字符串转换为16进制的形式
            m_serial->write(senddata1);//发送到串口
        }
        else
        {
             m_serial->write(SendBytes);
        }

        m_nSendNum += SendBytes.count();
        SetSendNum();
    }
}

void SerialPort::StringToHex(QString str, QByteArray &senddata)
{    int hexdata,lowhexdata;
     int hexdatalen = 0;
     int len = str.length();
     senddata.resize(len/2);
     char lstr,hstr;
     for(int i=0; i<len; )
     {
         //char lstr,
         hstr=str[i].toLatin1();
         if(hstr == ' ')
         {
             i++;
             continue;
         }
         i++;
         if(i >= len)            break;
         lstr = str[i].toLatin1();
         hexdata = ConvertHexChar(hstr);
         lowhexdata = ConvertHexChar(lstr);
         if((hexdata == 16) || (lowhexdata == 16))            break;
         else
             hexdata = hexdata*16+lowhexdata;
         i++;
         senddata[hexdatalen] = (char)hexdata;
         hexdatalen++;
     }
      senddata.resize(hexdatalen);
}

char SerialPort::ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}


/***************************************************************************/
/*                                函数重写                                  */
/***************************************************************************/
// 串口关闭事件，如果窗口关闭前串口未关闭，则关闭串口
void SerialPort::closeEvent(QCloseEvent *event)
{
    if (m_serial->isOpen())
        m_serial->close();

    event->accept();
}




