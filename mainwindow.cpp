#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QPrinter>
#include <QDebug>
#include <QShortcut>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScreen>
#include "xlsxdocument.h"
#include "xlsxcell.h"
#include "qrcodegen.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Создаём область графики и привязываем её к GraphicsView
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    curWindowSize = this->size();

    defaultFontName = ui->fontComboBox->currentText();

    // Создаем горячую клавишу Del, привязанную к listWidget
    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), ui->listWidget);

    connect(deleteShortcut, &QShortcut::activated, this, &MainWindow::deleteSelectedListItem);// Удаление строки по нажатию кнопки Del
    connect(ui->buttonNext, &QPushButton::clicked, this, &MainWindow::onButtonNextClick);
    connect(ui->buttonPrev, &QPushButton::clicked, this, &MainWindow::onButtonPrevClick);
    connect(ui->buttonWide, &QPushButton::clicked, this, &MainWindow::onButtonWideClick);
    connect(ui->buttonNarrow, &QPushButton::clicked, this, &MainWindow::onButtonNarrowClick);
    connect(ui->buttonUnderline, &QPushButton::clicked, this, &MainWindow::onButtonUnderlineClick);
    connect(ui->buttonItalic, &QPushButton::clicked, this, &MainWindow::onButtonItalicClick);
    connect(ui->buttonBold, &QPushButton::clicked, this, &MainWindow::onButtonBoldClick);
    connect(ui->fontComboBox, &QFontComboBox::currentFontChanged, this, &MainWindow::onFontChanged);
    connect(ui->buttonOpen, &QPushButton::clicked, this, &MainWindow::onButtonOpenClick);
    connect(ui->buttonSave, &QPushButton::clicked, this, &MainWindow::onButtonSaveClick);
    connect(ui->buttonUp, &QPushButton::clicked, this, &MainWindow::onButtonUpClick);
    connect(ui->buttonDown, &QPushButton::clicked, this, &MainWindow::onButtonDownClick);
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::onListRowChanged);
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &MainWindow::onComboTextChanged);
    connect(ui->checkBox, &QCheckBox::clicked, this, &MainWindow::onCheckBoxClick);
    connect(ui->radioDown, &QRadioButton::clicked, this, &MainWindow::onRadioDownClick);
    connect(ui->radioRight, &QRadioButton::clicked, this, &MainWindow::onRadioRightClick);
    connect(ui->spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onSpinBoxChange);
}

void MainWindow::onRadioDownClick()
{
    ui->radioRight->setChecked(!ui->radioDown->isChecked());
    Redraw();
}

void MainWindow::onRadioRightClick()
{
    ui->radioDown->setChecked(!ui->radioRight->isChecked());
    Redraw();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    // 1. Вызываем реализацию базового класса,
    QMainWindow::resizeEvent(event);

    // 2. Получаем новые размеры
    QSize newWindowSize = event->size();
    int newWidth = newWindowSize.width();

    // 3. Изменение положения и размеров элементов окна
    ui->buttonNext->move(newWidth - 40, ui->buttonNext->y());
    ui->buttonPrev->move(newWidth - 40, ui->buttonPrev->y());
    QSize curGraphicsSize = ui->graphicsView->size();
    ui->graphicsView->resize(curGraphicsSize + newWindowSize - curWindowSize);

    curWindowSize = newWindowSize;
}

void MainWindow::onButtonNextClick()
{
    if(curValueRow < lastValueRow){
        curValueRow++;
        if(curValueRow == lastValueRow)
            ui->buttonNext->setEnabled(false);
        ui->buttonPrev->setEnabled(true);
        Redraw();
    }
}

void MainWindow::onButtonPrevClick()
{
    if(curValueRow > firstValueRow){
        curValueRow--;
        if(curValueRow == firstValueRow)
            ui->buttonPrev->setEnabled(false);
        ui->buttonNext->setEnabled(true);
        Redraw();
    }
}

void MainWindow::onSpinBoxChange()
{
    Redraw();
}

void MainWindow::onCheckBoxClick()
{
    if(ui->checkBox->isChecked()){
        ui->spinBox->setEnabled(true);
        ui->radioDown->setEnabled(true);
        ui->radioRight->setEnabled(true);
    }
    else{
        ui->spinBox->setEnabled(false);
        ui->radioDown->setEnabled(false);
        ui->radioRight->setEnabled(false);
    }
    Redraw();
}

void MainWindow::onButtonWideClick()
{
    int curRow = ui->listWidget->currentRow();
    if(curRow >= 0){
        FieldProperties& selProp = fieldProp[curRow];
        if(selProp.offset < 40)
            selProp.offset += 5;
        Redraw();
    }
}

void MainWindow::onButtonNarrowClick()
{
    int curRow = ui->listWidget->currentRow();
    if(curRow >= 0){
        FieldProperties& selProp = fieldProp[curRow];
        if(selProp.offset >= 10)
            selProp.offset -= 5;
        Redraw();
    }
}

void MainWindow::changePropFont(int fontAttrType)
{
    QString selText;
    int curRow = ui->listWidget->currentRow();
    if(curRow >= 0){
        FieldProperties& selProp = fieldProp[curRow];
        switch(fontAttrType){
        case ATTR_BOLD: selProp.font.setBold(ui->buttonBold->isChecked()); break;
        case ATTR_ITALIC: selProp.font.setItalic(ui->buttonItalic->isChecked()); break;
        case ATTR_UNDERLINE: selProp.font.setUnderline(ui->buttonUnderline->isChecked()); break;
        case ATTR_SIZE:
            selText = ui->comboBox->currentText();
            selProp.font.setPointSize(selText.toInt());
            break;
        case ATTR_FONT:
            selProp.font.setFamily(ui->fontComboBox->currentFont().family());
        }

        selProp.textItem->setFont(selProp.font);
        if(selProp.textValueItem != nullptr)
            selProp.textValueItem->setFont(selProp.font);
        Redraw();
    }
}

void MainWindow::onFontChanged()
{
    changePropFont(ATTR_FONT);
}

void MainWindow::onButtonUnderlineClick()
{
    changePropFont(ATTR_UNDERLINE);
}

void MainWindow::onButtonItalicClick()
{
    changePropFont(ATTR_ITALIC);
}

void MainWindow::onButtonBoldClick()
{
    changePropFont(ATTR_BOLD);
}

void MainWindow::onComboTextChanged()
{
    changePropFont(ATTR_SIZE);
}

//Выделение строки в listWidget
void MainWindow::onListRowChanged()
{
    int curRow = ui->listWidget->currentRow();
    if(curRow >= 0){
        FieldProperties curProp = fieldProp.at(curRow);
        int indexToComboBox = ui->comboBox->findText(QString::number(curProp.font.pointSize()));
        //ui->fontComboBox->
        ui->comboBox->setCurrentIndex(indexToComboBox);
        ui->buttonBold->setChecked(curProp.font.bold());
        ui->buttonItalic->setChecked(curProp.font.italic());
        ui->buttonUnderline->setChecked(curProp.font.underline());
    }
}

void MainWindow::deleteSelectedListItem()
{
    // 1. Получаем индекс текущей (выделенной) строки
    int currentRow = ui->listWidget->currentRow();

    // Если ни одна строка не выбрана, currentRow будет равен -1
    if (currentRow != -1) {
        // 2. Извлекаем элемент из списка (он удаляется из интерфейса)
        QListWidgetItem *item = ui->listWidget->takeItem(currentRow);

        // 3. Обязательно удаляем объект из памяти, чтобы избежать утечки
        delete item;

        fieldProp.removeAt(currentRow);

        Redraw();
    }
}

void MainWindow::onButtonUpClick()
{
    int curRow = ui->listWidget->currentRow();
    if(curRow > 0){
        int prevRow = curRow - 1;
        //Блокируем посылку сигналов, чтобы onListRowChanged() не вызвался раньше времени
        ui->listWidget->blockSignals(true);
        // Извлекаем элемент из списка
        QListWidgetItem *itemCur = ui->listWidget->takeItem(curRow);
        // Вставляем его обратно на строку выше
        ui->listWidget->insertItem(prevRow, itemCur);
        //Ставим выделение на ту же строку, где оно было
        ui->listWidget->setCurrentRow(prevRow);

        FieldProperties curProp = fieldProp.at(curRow);
        fieldProp.removeAt(curRow);
        fieldProp.insert(prevRow, curProp);

        ui->listWidget->blockSignals(false);

        Redraw();
    }
}

void MainWindow::onButtonDownClick()
{
    int curRow = ui->listWidget->currentRow();
    if(curRow != -1 && curRow < ui->listWidget->count()-1){
        int nextRow = curRow + 1;
        //Блокируем посылку сигналов, чтобы onListRowChanged() не вызвался раньше времени
        ui->listWidget->blockSignals(true);
        // Извлекаем элемент из списка
        QListWidgetItem *itemCur = ui->listWidget->takeItem(curRow);
        // Вставляем его обратно на строку выше
        ui->listWidget->insertItem(nextRow, itemCur);
        //Ставим выделение на ту же строку, где оно было
        ui->listWidget->setCurrentRow(nextRow);

        FieldProperties curProp = fieldProp.at(curRow);
        fieldProp.removeAt(curRow);
        fieldProp.insert(nextRow, curProp);

        ui->listWidget->blockSignals(false);

        Redraw();
    }
}

void MainWindow::DrawValues(int row, QGraphicsScene * scene)
{
    QMap<QString, QString> typeAndData;

    QVariant v;
    QString type, data;
    for(int excelCol = firstCol; excelCol <= lastCol; excelCol++) {
        type = (this->*ptrReadCell)(firstRow, excelCol);
        data = (this->*ptrReadCell)(row, excelCol);
        typeAndData[type] = data;
    }
    //Перебираем все уже расположенные на сцене прямоугольники текста и определяем самый широкий
    qreal maxWidth = 0;
    for(int i = 0; i < ui->listWidget->count(); i++){
        FieldProperties& Prop = fieldProp[i];
        qreal rightX = Prop.textItem->boundingRect().bottomRight().x();
        if(rightX > maxWidth)
            maxWidth = rightX;
    }

    int vertOffset = 0;
    for(int i = 0; i < ui->listWidget->count(); i++){
        FieldProperties& Prop = fieldProp[i];
        type = Prop.name;
        Prop.textValueItem = scene->addText(typeAndData[type], Prop.font);
        Prop.textValueItem->setPos(maxWidth + 10, vertOffset);
        vertOffset += Prop.offset;
    }
}

void MainWindow::DrawQRCode(QGraphicsScene * scene)
{
    //Если установлена галочка "Генерировать QR-код"
    if(ui->checkBox->isChecked()){
        QString QRData = genStrToQRCode(curValueRow);// Собираем данные в одну строку
        QPixmap QRPixmap = genQRPixmap(QRData, ui->spinBox->value());
        QGraphicsPixmapItem *pixmapItem = scene->addPixmap(QRPixmap);
        int numListElem = ui->listWidget->count();
        int vertOffset = 0, horizOffset = 0;
        if(ui->radioDown->isChecked()){
            for(int i = 0; i < numListElem; i++)
                vertOffset += fieldProp.at(i).offset;
            vertOffset += 20;
        }
        else{
            for(int i = 0; i < fieldProp.count(); i++){
                FieldProperties prop = fieldProp.at(i);
                QPointF topRightOwn = prop.textValueItem->boundingRect().topRight();
                QPointF topRightSc = prop.textValueItem->mapToScene(topRightOwn);
                int rectValueRightX = (int)topRightSc.x();
                if(rectValueRightX > horizOffset)
                    horizOffset = rectValueRightX;
            }
            horizOffset += 20;
        }
        pixmapItem->setPos(horizOffset, vertOffset);
    }
}

void MainWindow::Redraw(QGraphicsScene * scene)
{
    if(xlsx_ptr != nullptr || xls_ptr != nullptr){
        if(scene == nullptr)
            scene = ui->graphicsView->scene();
        scene->clear();

        int vertOffset = 0;
        QGraphicsTextItem * newTextItem;
        for(int i = 0; i < fieldProp.count(); i++){
            FieldProperties & Prop = fieldProp[i];
            newTextItem = scene->addText(Prop.name, Prop.font);
            newTextItem-> setPos(0, vertOffset);
            if(scene == ui->graphicsView->scene())
                Prop.textItem = newTextItem;

            vertOffset += Prop.offset;
        }
        DrawValues(curValueRow, scene);

        DrawQRCode(scene);
    }
}

QString MainWindow::genStrToQRCode(int row)
{
    QString ret;

    QVariant v;
    QString type, data;
    for(int excelCol = firstCol; excelCol <= lastCol; excelCol++) {
        type = (this->*ptrReadCell)(firstRow, excelCol);
        data = (this->*ptrReadCell)(row, excelCol);
        QString tempStr = QString::asprintf("%s %s\n", (char*)type.data(), (char*)data.data());
        ret += tempStr;
    }

    return ret;
}

QPixmap MainWindow::genQRPixmap(QString QRData, int pixelSize)
{
    // 2. Формируем символ QR-кода как двумерный массив битов
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(QRData.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
    // 3. Рассчитываем размеры итогового изображения
    int qrSize = qr.getSize();
    int border = 4; // Стандартная белая рамка безопасности для QR-кодов
    int totalSize = (qrSize + border * 2) * pixelSize;

    // 4. Создаем пустое белое изображение
    QPixmap pixmap(totalSize, totalSize);
    pixmap.fill(Qt::white);

    // 5. Отрисовываем черные квадраты (модули) QR-кода с помощью QPainter
    QPainter painter(&pixmap);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);

    for (int y = 0; y < qrSize; ++y) {
        for (int x = 0; x < qrSize; ++x) {
            // Если модуль (пиксель) QR-кода должен быть черным
            if (qr.getModule(x, y)) {
                int posX = (x + border) * pixelSize;
                int posY = (y + border) * pixelSize;
                painter.drawRect(posX, posY, pixelSize, pixelSize);
            }
        }
    }

    return pixmap;
}

int MainWindow::openXLSXDocument(QString fileName)
{
    try{
        if(xlsx_ptr != nullptr)
            delete xlsx_ptr;

        // Сначала читаем названия листов, используя QXlsx::Document
        xlsx_ptr = new QXlsx::Document(fileName);
        if (!xlsx_ptr->isLoadPackage()) {
            QMessageBox::warning(this, "Error", "Не получилось загрузить файл xlsx.");
            throw 1;
        }

        const QStringList sheetNames = xlsx_ptr->sheetNames();
        if (sheetNames.isEmpty()) {
            QMessageBox::information(this, "Info", "Excel файл не содержит ни одного листа.");
            throw 1;
        }

        QString sheetName = sheetNames.first();
        if(!xlsx_ptr->selectSheet(sheetName)){
            qWarning() << "Не удалось выбрать лист:" << sheetName;
            throw 1;
        }

        QXlsx::CellRange range = xlsx_ptr->dimension();
        firstRow = range.firstRow();
        lastRow = range.lastRow();
        firstCol = range.firstColumn();
        lastCol = range.lastColumn();

        if (firstRow == 0 && lastRow == 0 && firstCol == 0 && lastCol == 0) {
            QMessageBox::information(this, "Info", "Первый лист файла пустой.");
            throw 1;
        }

        if(lastRow - firstRow >= 1) {
            curValueRow = firstRow+1;
            firstValueRow = firstRow+1;
            lastValueRow = lastRow;
        }
        else{
            curValueRow = -1;
            firstValueRow = -1;
            lastValueRow = -1;
        }
        ptrReadCell = &MainWindow::readXlsxCell;
    }
    catch(int except){
        delete xlsx_ptr;
        xlsx_ptr = nullptr;
        return ERROR;
    }
    return SUCCESS;
}

QString MainWindow::readXlsxCell(int row, int col)
{
    QString ret;
    QVariant v = xlsx_ptr->read(row, col);
    if(v.userType() == QMetaType::QDate) {
        // Явно преобразуем в QDate и задаем маску формата
        QDate date = v.toDate();
        ret = date.toString("dd.MM.yyyy");
    } else if(v.userType() == QMetaType::QTime) {
        // Явно преобразуем в QTime и задаем маску формата
        QTime time = v.toTime();
        ret = time.toString("hh:mm:ss");
    }
    else {
        // Если в ячейке оказалась не дата или время, а обычный текст
        ret = v.toString();
    }
    return ret;
}

int MainWindow::openXLSDocument(QString name)
{
    try{
        if(xls_ptr != nullptr)
            delete xls_ptr;

        // Сначала читаем названия листов, используя QXlsx::Document
        xls_ptr = new xls::WorkBook(name.toStdString());

        int numSheets = xls_ptr->GetSheetCount();
        if (numSheets == 0) {
            QMessageBox::information(this, "Info", "Excel файл не содержит ни одного листа.");
            throw 1;
        }

        xls_ptr->OpenSheet(0);
        firstRow = -1;
        firstCol = -1;
        int numRows = xls_ptr->GetNumRows();
        int numCols = xls_ptr->GetNumCols();
        lastRow = 0;
        lastCol = 0;
        for(int row = 0; row < numRows; row++){
            for(int col = 0; col < numCols; col++){
                xls::cellContent cont = xls_ptr->GetCell(0, row, col);
                if(cont.str != ""){
                    if(firstRow == -1){
                        firstRow = row;
                        firstCol = col;
                    }
                    if(row > lastRow)
                        lastRow = row;
                    if(col > lastCol)
                        lastCol = col;
                }
            }
        }

        if (firstRow == -1) {
            QMessageBox::information(this, "Info", "Первый лист файла пустой.");
            throw 1;
        }

        if(lastRow - firstRow >= 1) {
            curValueRow = firstRow+1;
            firstValueRow = firstRow+1;
            lastValueRow = lastRow;
        }
        else{
            curValueRow = -1;
            firstValueRow = -1;
            lastValueRow = -1;
        }
        ptrReadCell = &MainWindow::readXlsCell;

    }
    catch(int except){
        delete xls_ptr;
        xls_ptr = nullptr;
        return ERROR;
    }
    catch(xls::XlsException e){
        QMessageBox::information(this, "Info", e.what());
        return ERROR;
    }

    return SUCCESS;

}

QString MainWindow::readXlsCell(int row, int col)
{
    xls::cellContent cont = xls_ptr->GetCell(0, row, col);
    QString ret(cont.str.data());
    return ret;
}

void MainWindow::onButtonOpenClick()
{
    // Открываем диалоговое окно
    QString fileName = QFileDialog::getOpenFileName(
        this,                           // Родительское окно (this)
        tr("Открыть файл"),             // Заголовок окна
        QDir::homePath(),               // Стартовая папка (например, домашняя директория)
        tr("Файлы Excel (*.xlsx *.xls);;Все файлы (*.*)") // Фильтр расширений
    );

    // Проверяем, выбрал ли пользователь файл (а не нажал "Отмена")
    if (fileName.isEmpty())
        return;

    QFileInfo fileInfo(fileName);
    QString ext = fileInfo.suffix();

    if(ext.toLower() == "xlsx")
        if(openXLSXDocument(fileName) != SUCCESS)
            return;

    if(ext.toLower() == "xls")
        if(openXLSDocument(fileName) != SUCCESS)
            return;

    fieldProp.clear();
    ui->listWidget->clear();
    QGraphicsScene * scene = ui->graphicsView->scene();
    scene->clear();

    ui->fontComboBox->setCurrentFont(defaultFontName);
    ui->comboBox->setCurrentIndex(0);
    ui->buttonBold->setChecked(false);
    ui->buttonItalic->setChecked(false);
    ui->buttonUnderline->setChecked(false);
    int i = 0;
    FieldProperties newProp;
    newProp.font.setFamily(defaultFontName);
    newProp.font.setPointSize((ui->comboBox->currentText()).toInt());
    newProp.font.setBold(ui->buttonBold->isChecked());
    newProp.font.setItalic(ui->buttonItalic->isChecked());
    newProp.font.setUnderline(ui->buttonUnderline->isChecked());
    newProp.offset = 20;
    for(int excelCol = firstCol; excelCol <= lastCol; excelCol++) {
        QVariant v = (this->*ptrReadCell)(firstRow, excelCol);
        QString s = v.toString();
        ui->listWidget->addItem(s);
        newProp.name = s;
        newProp.textItem = scene->addText(s, newProp.font);
        newProp.textItem->setPos(0, 0 + newProp.offset*i);
        fieldProp.append(newProp);
        i++;
    }
    //Если есть вторая строка со значениями полей, добавляем прямоугольники с текстом в graphicsView
    if(lastRow - firstRow >= 1)
        DrawValues(curValueRow, scene);

    //Если строк данных в Excel-файле больше 1, активируем кнопки перебора этикеток с разными данными
    if(lastRow - firstRow >= 2){
        ui->buttonNext->setEnabled(true);
        ui->buttonPrev->setEnabled(false);
    }
    //Рисуем QR-код
    DrawQRCode(scene);

    ui->buttonSave->setEnabled(true);
}

void MainWindow::onButtonSaveClick()
{
    // 1. Запрашиваем у пользователя путь для сохранения файла
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Сохранить в PDF"), "", tr("PDF Files (*.pdf)"));

    if (fileName.isEmpty()) return;

    // 2. Инициализируем QPrinter в режиме вывода в PDF
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    // 3. Настраиваем вид страницы
    QPageSize pageSize(QPageSize::A4);
    QMarginsF margins(10, 10, 10, 10); //поля (в миллиметрах)
    QPageLayout layout(pageSize, QPageLayout::Portrait, margins, QPageLayout::Millimeter);//ориентация и поля
    printer.setPageLayout(layout);

    // 3. Настраиваем QPainter для рисования на принтере
    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    double screenDpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    double scale = printer.resolution() / screenDpi;
    qreal curOffsetY = 0.0, lineOffsetY = 20;
    int savedCurValueRow = curValueRow;
    QRectF sceneRect, extSceneRect, scaledSceneRect, paintRect;
    QGraphicsScene * scene = new QGraphicsScene(this);
    for(curValueRow = firstValueRow; curValueRow <= lastValueRow; curValueRow++){
        Redraw(scene);
        //Проверяем, что этикетка помещается на лист A4 по ширине
        sceneRect = scene->sceneRect();
        //Для первого прохода цикла смещение линии относительно текста 20, для остальных проходов - 0,
        //т.к. размер ограничивающего прямоугольника не сбрасывается после scene->clear() (вызывается в Redraw())
        scene->addLine(0, sceneRect.height() + lineOffsetY, sceneRect.width(), sceneRect.height() + lineOffsetY);
        lineOffsetY = 0;
        extSceneRect = scene->sceneRect();
        scaledSceneRect = QRectF(extSceneRect.x()*scale, extSceneRect.y()*scale, extSceneRect.width()*scale, extSceneRect.height()*scale);
        paintRect = printer.pageLayout().paintRectPixels(printer.resolution());

        if(scaledSceneRect.x() > paintRect.x()){
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          "Подтверждение",
                                          "Этикетка по ширине не помещается на формате А4 вертикальный. Продолжить?",
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply != QMessageBox::Yes)
                break;
        }

        if(paintRect.height() < (curOffsetY + scaledSceneRect.height() + 20*scale)){
            printer.newPage();
            curOffsetY = 0.0;
        }

        // 4. Рендерим сцену в PDF-документ
        scene->render(&painter, QRectF(0, curOffsetY, scaledSceneRect.width(), scaledSceneRect.height()));
        curOffsetY = curOffsetY + scaledSceneRect.height() + 20*scale;
    }
    delete scene;
    curValueRow = savedCurValueRow;
}

MainWindow::~MainWindow()
{
    if(xlsx_ptr != nullptr){
        delete xlsx_ptr;
        xlsx_ptr = nullptr;
    }
    if(xls_ptr != nullptr){
        delete xls_ptr;
        xls_ptr = nullptr;
    }
    delete ui;
}

