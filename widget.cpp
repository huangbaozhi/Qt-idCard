#include "widget.h"
#include "idOCR.h"
#include <QFileDialog>
#include <QScrollArea>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QGroupBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->initUI();

    this->sigAndSlot();
}

Widget::~Widget()
{
}

void Widget::initUI()
{
    //创建百度识别类的对象
    //百度AI初始化
    QString apiKey = "Fm4osZSoIBatkH50GSkCNxMn";
    QString secretKey = "BlaLL3ubOKTbd650Sx5U9IMyl6wzTrE7";

    m_pIdOCR = new idOCR(apiKey, secretKey);

    QHBoxLayout *pHShowLayout = new QHBoxLayout(this);
    QVBoxLayout *pVShowLayout = new QVBoxLayout(this);
    // 显示身份证
    QGroupBox *pImageBox = new QGroupBox("图片预览");
    QHBoxLayout *pHLayout = new QHBoxLayout(this);

    QLineEdit *pSelectPathEdit = new QLineEdit(this);
    m_pSelectPathEdit = pSelectPathEdit;

    QPushButton *pSelectPathBtn = new QPushButton("选择图片");
    m_pSelectPathBtn = pSelectPathBtn;

    pHLayout->addWidget(pSelectPathEdit);
    pHLayout->addWidget(pSelectPathBtn);

    QHBoxLayout *pImgLyt = new QHBoxLayout(this);
    QLabel *pImageLbl = new QLabel();
    m_pImageLbl = pImageLbl;
    pImageLbl->setFixedSize(400, 300);

    pImgLyt->addWidget(pImageLbl);
    pVShowLayout->addLayout(pHLayout);
    pVShowLayout->addLayout(pImgLyt);
    pImageBox->setLayout(pVShowLayout);

    // 显示身份证信息
    QGroupBox *pResultBox = new QGroupBox("识别结果");

    QVBoxLayout *pVIdocrLyt = new QVBoxLayout(this);
    QHBoxLayout *pHIdOcrLyt = new QHBoxLayout(this);

    QPushButton *pIdOCRBtn = new QPushButton("身份证识别");
    m_pIdOCRBtn = pIdOCRBtn;

    pHIdOcrLyt->addStretch();
    pHIdOcrLyt->addWidget(pIdOCRBtn);
    pHIdOcrLyt->addStretch();

    QHBoxLayout *pResultLyt = new QHBoxLayout(this);
    QTextBrowser *pShowInfBrowser = new QTextBrowser();
    m_pShowInfBrowser = pShowInfBrowser;
    pShowInfBrowser->setFixedSize(280, 300);
    pResultLyt->addWidget(pShowInfBrowser);

    pVIdocrLyt->addLayout(pHIdOcrLyt);
    pVIdocrLyt->addLayout(pResultLyt);
    pResultBox->setLayout(pVIdocrLyt);

    pHShowLayout->addWidget(pImageBox);
    pHShowLayout->addWidget(pResultBox);
}

void Widget::sigAndSlot()
{
    connect(m_pIdOCR, &idOCR::ocrResult,this, &Widget::on_OcrResult);
    connect(m_pSelectPathBtn, &QPushButton::clicked, this, &Widget::sltShowImage);
    connect(m_pIdOCRBtn, &QPushButton::clicked, this, &Widget::sltIdCardResult);
}

void Widget::on_OcrResult(idOCR::OCR_RET ret, idOCR::OCR_TYPE mOcrType)
{
    if(ret == idOCR::OCR_SUCCESS)
    {
        switch (mOcrType)
        {
            case idOCR::OCR_ID_CARD:
                m_pShowInfBrowser->setText(m_pIdOCR->getIdCard()->toString());
                break;
                // 这里可以扩展其他类型识别
            default:
                break;
        }
    }
    else
    {
        m_pShowInfBrowser->setText("识别失败");
    }
}

void Widget::sltShowImage()
{
    QString filename = QFileDialog::getOpenFileName(this, "请选择图片", nullptr,"*.jpg");
    m_pSelectPathEdit->setText(filename);
    m_pImageLbl->setPixmap(filename);
    m_pImageLbl->setScaledContents(true);
}

void Widget::sltIdCardResult()
{
    m_pIdOCR->recgIdCard(m_pSelectPathEdit->text());
}

