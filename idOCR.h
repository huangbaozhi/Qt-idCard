#ifndef IDOCR_H
#define IDOCR_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class idOCR : public QObject
{
    Q_OBJECT
public:
    // 识别结果
    enum OCR_RET
    {
        OCR_SUCCESS,
        OCR_ERROR,
        OCR_ERROR_RECG,             // 识别错误
        OCR_ERROR_NOTHING,          // 没有要识别的信息
        OCR_ERROR_IMG,
        OCR_ERROR_PARAM,            // 参数错误
        OCR_ERROR_BUSY,             // 繁忙
        OCR_ERROR_COMMU,            // 通信错误
        OCR_ERROR_TIMEOUT,          // 执行超时
        OCR_ERROR_OTHER,            // 其余错误
    };
    Q_ENUM(OCR_RET)

    // 识别证件类型
    enum OCR_TYPE
    {
        OCR_UNKNOWN_TYPE,
        OCR_ID_CARD,                // 身份证
        //可以扩展其他类型
    };
    Q_ENUM(OCR_TYPE)

    // 身份证识别结果
    class IdCard{
    public:
        QString address;            // 住址
        QString id;                 // 身份证号
        QString birth;              // 生日
        QString name;               // 名字
        QString gender;             // 性别
        QString nation;             // 民族
        IdCard() {}
        QString toString(){
            QString str = "";
            str += "身份证识别结果: ";
            str += "\n姓名: " + name;
                       str += "\n身份证号: " + id;
            str += "\n生日: " + birth;
                       str += "\n性别: " + gender;
                       str += "\n民族: " + nation;
                       str += "\n住址: " + address;
                   return str;
        }
        void clear()
        {
            name.clear();
            id.clear();
            birth.clear();
            gender.clear();
            nation.clear();
            address.clear();
        }
    };

public:
    // 构造函数
    idOCR(QString apiKey, QString secretKey);

    // 获取识别结果的函数
    IdCard *getIdCard() const;

private:
    void Access_to_http(QString Url, QByteArray data);
    QString getIdOCR_token(QString ApiKey, QString SecretKey);

    //解析百度ai返回结果函数
    bool parseJsonIdCard(QJsonObject& subObj);  // 解析身份证识别返回结果

public slots:
    void slothttpFinish(QByteArray);
    bool recgIdCard(QString fpath);
    void recgImage(QString baiUrl, QString fpath);

private slots:
    void getHttpComReply(QNetworkReply *pReplay);
    void replyFinished(QNetworkReply *reply);

signals:
    void httpFinish(QByteArray);
    void ocrResult(OCR_RET ret,OCR_TYPE mOcrType);

private:
    QString                 bai_apiKey;
    QString                 bai_secretKey;
    QString                 bai_token;
    QString                 data[10];
    QNetworkAccessManager   *manager;
    QNetworkReply           *pReplay;

    QByteArray              target;
    QEventLoop              eventLoop;

    OCR_TYPE                mOcrType;   // 识别类型
    IdCard                  mIdCard;    // 保存身份证结果


};

#endif // IDOCR_H
