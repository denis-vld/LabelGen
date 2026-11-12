#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsTextItem>
#include "xlsxdocument.h"
#include "XlsReader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct FieldProperties{
        QString name;
        QFont font;
        int offset;
        QGraphicsTextItem *textItem;
        QGraphicsTextItem *textValueItem = nullptr;
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    const int SUCCESS = 0;
    const int ERROR = 1;
    enum FontAttributes{
        ATTR_BOLD = 0,
        ATTR_ITALIC = 1,
        ATTR_UNDERLINE = 2,
        ATTR_SIZE = 3,
        ATTR_FONT = 4
    };

private:
    QVector<FieldProperties> fieldProp;
    QSize curWindowSize;
    QString defaultFontName;

    xls::WorkBook * xls_ptr = nullptr;
    QXlsx::Document * xlsx_ptr = nullptr;
    int curValueRow;
    int firstValueRow;
    int lastValueRow;
    int firstRow;
    int lastRow;
    int firstCol;
    int lastCol;

private:
    Ui::MainWindow *ui;
    void deleteSelectedListItem();
    void onButtonOpenClick();
    void onButtonSaveClick();
    void onButtonUpClick();
    void onButtonDownClick();
    void onListRowChanged();
    void changePropFont(int fontAttrType);
    void onComboTextChanged();
    void onButtonBoldClick();
    void onButtonItalicClick();
    void onButtonUnderlineClick();
    void onFontChanged();
    void onButtonWideClick();
    void onButtonNarrowClick();
    void onCheckBoxClick();
    void onSpinBoxChange();
    void onButtonNextClick();
    void onButtonPrevClick();
    void onRadioDownClick();
    void onRadioRightClick();

    void DrawQRCode(QGraphicsScene * scene);
    void DrawValues(int row, QGraphicsScene * scene);
    void Redraw(QGraphicsScene * scene = nullptr);
    QString genStrToQRCode(int row);
    QPixmap genQRPixmap(QString QRData, int pixelSize);
    int openXLSXDocument(QString name);
    int openXLSDocument(QString name);
    QString readXlsCell(int row, int col);
    QString readXlsxCell(int row, int col);
    QString (MainWindow::*ptrReadCell)(int row, int col);
};
#endif // MAINWINDOW_H
