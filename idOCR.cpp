#include "idOCR.h"
#include <QFile>
#include <QUrlQuery>
#include <QDebug>

idOCR::idOCR(QString apiKey, QString secretKey)
    : bai_apiKey(apiKey)
    , bai_secretKey(secretKey)
{
    bai_token = getIdOCR_token(bai_apiKey, bai_secretKey);
    qDebug() << "Got Access Token: " << bai_token;

    manager = new QNetworkAccessManager;
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

idOCR::IdCard *idOCR::getIdCard() const
{
    return (IdCard*)(&mIdCard);
}

void idOCR::Access_to_http(QString Url, QByteArray data)
{
    // 构造请求
    QNetworkRequest request((QUrl(Url)));
    manager = new QNetworkAccessManager(this);

    // 设置头信息
    request.setHeader(QNetworkRequest::ContentTypeHeader, "Content-Type:application/json");

    // 发送请求
    manager->post(request, data);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(getHttpComReply(QNetworkReply*)));
}

QString idOCR::getIdOCR_token(QString ApiKey, QString SecretKey)
{
    QString Url = "http://openapi.baidu.com/oauth/2.0/token?";
    QByteArray append = QString("grant_type=client_credentials&client_id=%1&client_secret=%2&").arg(ApiKey).arg(SecretKey).toLatin1();

    // 请求百度服务
    Access_to_http(Url, append);

    auto c1 = connect(this, SIGNAL(httpFinish(QByteArray)), this, SLOT(slothttpFinish(QByteArray)));

    eventLoop.exec();                                               // 事件循环等待finish信号的发生，得到数据
    disconnect(c1);                                                 // 注销信号与槽

    const auto &&data = QJsonDocument::fromJson(target).object();   // 使用json解析百度语音识别服务器返回的token
    if(data.isEmpty() || !data.contains("access_token"))
    {
        return "error";
    }
    return data["access_token"].toString();

}

bool idOCR::parseJsonIdCard(QJsonObject &subObj)
{
    mIdCard.clear();
    QJsonObject secObj =subObj.value("住址").toObject();
    mIdCard.address=secObj["words"].toString();

    secObj =subObj.value("住址").toObject();
    mIdCard.address=secObj["words"].toString();
    secObj =subObj.value("公民身份号码").toObject();
    mIdCard.id=secObj["words"].toString();
    secObj =subObj.value("出生").toObject();
    mIdCard.birth=secObj["words"].toString();
    secObj =subObj.value("姓名").toObject();
    mIdCard.name=secObj["words"].toString();
    secObj =subObj.value("性别").toObject();
    mIdCard.gender=secObj["words"].toString();
    secObj =subObj.value("民族").toObject();
    mIdCard.nation=secObj["words"].toString();

    qDebug().noquote()<<mIdCard.toString();

    emit ocrResult(OCR_SUCCESS, OCR_ID_CARD);
    return true;
}

void idOCR::slothttpFinish(QByteArray data)
{
    target = data;
    eventLoop.exit(1);
}

bool idOCR::recgIdCard(QString fpath)
{
    if(fpath.isNull() || fpath.isEmpty())
    {
        return false;
    }
    mOcrType = OCR_ID_CARD;
    recgImage("https://aip.baidubce.com/rest/2.0/ocr/v1/idcard", fpath);
    return true;
}

void idOCR::recgImage(QString baiUrl, QString fpath)
{
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly) || file.size() == 0)
    {
        file.close();
        qDebug()<<"Open file failed! imgPath="<<fpath;
        return ;
    }
    QByteArray fdata = file.readAll();
    file.close();

    // 配置请求参数（要根据百度ai的文档要求，正确填写参数）
    QUrlQuery params;
    params.addQueryItem("image", fdata.toBase64().toPercentEncoding());

    if(mOcrType == OCR_ID_CARD)
    {
        // 身份证特有的参数
        params.addQueryItem("id_card_side", "front");
    }

    //post request
    QUrl url(baiUrl + "?access_token=" + bai_token);
    QNetworkRequest request(url);

    // 设置数据提交格式
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded;charset=utf-8"));
    manager->post(request, params.toString().toUtf8());

}

void idOCR::getHttpComReply(QNetworkReply *pReplay)
{
    // 获取响应信息
    QByteArray httpReplyData = pReplay->readAll();
    emit httpFinish(httpReplyData);
}

void idOCR::replyFinished(QNetworkReply *reply)
{
    // 反馈数据分析
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(200 != statusCode)
    {
        qDebug() << "idOCR::replyFinished: Status code is error:" << statusCode;
        emit ocrResult(OCR_ERROR_COMMU,mOcrType);
        return;
    }

    QByteArray replyData = reply->readAll();
    qDebug() << "idOCR::replyFinished: reply data = \n" << QString(replyData);

    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(replyData, &json_error);

    if(json_error.error == QJsonParseError::NoError)
    {
        // 判断是否是对象,然后开始解析数据
        QJsonObject rootObj = document.object();

        // 因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if(rootObj.contains("words_result"))
        {
            QJsonObject subObj = rootObj.value("words_result").toObject();

            bool ret = false;
            switch (mOcrType) {
            case OCR_ID_CARD:
                ret = parseJsonIdCard(subObj);
                break;
                //可以扩展其他识别
            default:
                break;
            }

            if(!ret)
            {
                qDebug()<<"idOCR::replyFinished: unknown reply data.";
                emit ocrResult(OCR_ERROR_NOTHING,mOcrType);
            }

        }
        else
        {
            qDebug()<<"idOCR::replyFinished: error reply data.";
            if(rootObj.contains("error_code"))
            {
                qDebug()<<"idOCR::replyFinished: error_code = "<<rootObj["error_code"].toInt();
                qDebug()<<"idOCR::replyFinished: error_msg = "<<rootObj["error_msg"].toString();
            }
            emit ocrResult(OCR_ERROR_RECG,mOcrType);
        }
    }
    else
    {
        qDebug()<<"idOCR::replyFinished: json parse error.";
        emit ocrResult(OCR_ERROR_RECG,mOcrType);
    }
    reply->deleteLater();
}
