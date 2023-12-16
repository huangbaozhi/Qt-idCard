#ifndef WIDGET_H
#define WIDGET_H

#include "idOCR.h"
#include <QWidget>

class QTextBrowser;
class QLineEdit;
class QLabel;
class QPushButton;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    void initUI();
    void sigAndSlot();

private slots:
    void on_OcrResult(idOCR::OCR_RET ret, idOCR::OCR_TYPE mOcrType);
    void sltShowImage();            // 预览图片
    void sltIdCardResult();         // 身份证

private:
    idOCR           *m_pIdOCR;
    QTextBrowser    *m_pShowInfBrowser;
    QLineEdit       *m_pSelectPathEdit;
    QLabel          *m_pImageLbl;

    QPushButton     *m_pSelectPathBtn;
    QPushButton     *m_pIdOCRBtn;

};
#endif // WIDGET_H
