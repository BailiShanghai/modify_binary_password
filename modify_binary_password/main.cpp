#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QByteArray>
#include <QIODevice>
#include <QDataStream>

static quint32 passwordCoef0 = 0;
static quint32 passwordCoef1 = 0;
static quint32 passwordCoef3 = 0;
static quint32 passwordCoef2 = 0;
static quint32 startAddress = 0x44;
static quint32 isVerboseFlag = 0;
static QString bootloaderPath;

int binaryFileInfo(void)
{
    QFileInfo fileInfo(bootloaderPath);

    qDebug() << QDir().currentPath();
    if(!fileInfo.exists())
    {
        return false;
    }

    qDebug() << "binary file information:";
    qDebug() << "              file path:" << fileInfo.filePath();
    qDebug() << "              file name:" << fileInfo.fileName();
    qDebug() << "              file size:" << fileInfo.size() << "bytes";

    return true;
}

QString getRandomString(size_t length)
{
    size_t i;
    int randomx = 0;
    qsrand((quint32)QDateTime::currentMSecsSinceEpoch());//为随机值设定一个seed

    const char chrs[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int chrs_size = sizeof(chrs);

    char* ch = new char[length + 1];
    memset(ch, 0, length + 1);
    for (i = 0; i < length; ++i)
    {
        randomx= rand() % (chrs_size - 1);
        ch[i] = chrs[randomx];
    }

    QString ret(ch);
    delete[] ch;
    return ret;
}

int binaryFileHandleAdv(void)
{
    qint64 fileSize;
    QByteArray binaryContent;
    QString newBinaryFileName;
    QFile bFile(bootloaderPath);
    QFileInfo fileInfo(bootloaderPath);

    /*prepare a huge buffer*/
    fileSize = fileInfo.size();
    binaryContent.resize((int)fileSize);

    newBinaryFileName = "boot_" + getRandomString(8) + "loader.bin";
    bFile.open(QIODevice::ReadWrite);
    QDataStream binaryStream(&bFile);
    bFile.close();

    return 0;
}

int binaryFileHandle(void)
{
    bool ok;
    qint64 fileSize;
    QByteArray binaryContent;
    QByteArray passcodeContent;
    QByteArray tailContent;
    QString newBinaryFileName;
    QFile bFile(bootloaderPath);
    QFileInfo fileInfo(bootloaderPath);

    newBinaryFileName = "boot_" + getRandomString(8) + "_loader.bin";
#ifdef CFG_COPY_ORIGNAL_BINARY_FILE
    ok = QFile::copy(bootloaderPath, newBinaryFileName);
    if(false == ok)
    {
        qDebug() << "copy file failed";
    }
#endif

    fileSize = fileInfo.size();

    bFile.open(QIODevice::ReadWrite);
    QDataStream binaryStream(&bFile);

    binaryContent.resize((int)startAddress);
    binaryStream.readRawData(binaryContent.data(), (quint32)startAddress);

    passcodeContent.append((const char*)&passwordCoef0, (int)sizeof (passwordCoef0));
    passcodeContent.append((const char*)&passwordCoef1, (int)sizeof (passwordCoef1));
    passcodeContent.append((const char*)&passwordCoef2, (int)sizeof (passwordCoef2));
    passcodeContent.append((const char*)&passwordCoef3, (int)sizeof (passwordCoef3));

    tailContent.resize((int)fileSize - 16 - startAddress);
    binaryStream.skipRawData(16);
    binaryStream.readRawData(tailContent.data(), (quint32)fileSize - 16 - startAddress);

    binaryContent.append(passcodeContent);
    binaryContent.append(tailContent);

    do
    {
        QFile nFile(newBinaryFileName);

        nFile.open(QIODevice::WriteOnly);
        nFile.write(binaryContent);
        nFile.close();
    }while(0);

    bFile.close();

    QFileInfo tmpFileInfo("bootloader_orignal.bin");
    if(tmpFileInfo.exists("bootloader_orignal.bin"))
    {
        QFile tmp0File("bootloader_orignal.bin");
        ok = tmp0File.remove();
    }

    QFile tmp1File(bootloaderPath);
    ok = tmp1File.rename("bootloader_orignal.bin");

    QFile tmp2File(newBinaryFileName);
    ok = tmp2File.rename("bootloader.bin");

    return 0;
}

int main(int argc, char *argv[])
{
    bool ok;

    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("modify_binary_passcode");
    QCoreApplication::setApplicationVersion("ver1.0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("parser_helper");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("coef0",QCoreApplication::translate("main", "password1:coef0"));
    parser.addPositionalArgument("coef1",QCoreApplication::translate("main", "password1:coef1"));
    parser.addPositionalArgument("coef2",QCoreApplication::translate("main", "password1:coef2"));
    parser.addPositionalArgument("coef3",QCoreApplication::translate("main", "password1:coef3"));

    QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
                                  QCoreApplication::translate("main", "verbose"));
    parser.addOption(verboseOption);

    QCommandLineOption fileOption(QStringList() << "f" << "file",
                                  QCoreApplication::translate("main", "binaryFile"));
    fileOption.setValueName("bootFile");
    fileOption.setDefaultValue("bootloader.bin");
    parser.addOption(fileOption);

    QCommandLineOption addressOption("a");
    addressOption.setValueName("startAddressValue");
    addressOption.setDefaultValue("0x44");
    parser.addOption(addressOption);

    parser.process(a);

    /*get paramters*/
    isVerboseFlag = parser.isSet(verboseOption);
    const QStringList args = parser.positionalArguments();
    if(args.isEmpty() || (args.size() < 4))
    {
        qDebug() << "coef parameters is exceptional, and recommend to use the standard hex value format.";
        return -1;
    }

    /* coef0*/
    passwordCoef0 = args[0].toULong(&ok, 16);
    if(false == ok)
    {
        qDebug() << "coef0 param is exceptional, and recommend to use the standard hex value format.";
        return -1;
    }

    /* coef1*/
    passwordCoef1 = args[1].toULong(&ok, 16);
    if(false == ok)
    {
        qDebug() << "coef1 param is exceptional, and recommend to use the standard hex value format.";
        return -1;
    }

    /* coef2*/
    passwordCoef2 = args[2].toULong(&ok, 16);
    if(false == ok)
    {
        qDebug() << "coef2 param is exceptional, and recommend to use the standard hex value format.";
        return -1;
    }

    /* coef3*/
    passwordCoef3 = args[3].toULong(&ok, 16);
    if(false == ok)
    {
        qDebug() << "coef3 param is exceptional, and recommend to use the standard hex value format.";
        return -1;
    }

    startAddress = parser.value(addressOption).toULong(&ok, 16);
    if(false == ok)
    {
        qDebug() << "address param is exceptional, and recommend to use the standard hex value format.";
        return -1;
    }

    bootloaderPath = parser.value(fileOption);

    if(true == binaryFileInfo())
    {
        binaryFileHandle();
    }

    return 0;/*a.exec();*/
}
