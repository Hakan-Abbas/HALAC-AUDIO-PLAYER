#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTime>
#include <QByteArray>
#include <QStringList>
#include <QDebug>

/////////////////// Function Prototypes ///////////////////
typedef char* (*EXPORT_WAVFunc)(const char*, unsigned short);
typedef unsigned int (*EXPORT_SIZEFunc)(const char*);
typedef void (*EXPORT_DELETEFunc)();
///////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QFontDatabase::addApplicationFont(":/resource/fonts/digital-7__mono_.ttf");
    QFont fontDigital = QFont("Digital-7 Mono",24);
    ui->digital->setFont(fontDigital);

    ui->buttonLoad->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnBack->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->btnNext->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->btnLoop->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->buttonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    ui->buttonRemove->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    ui->buttonRemoveAll->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    ui->buttonSeekNext->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->buttonSeekBack->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));

    ui->tableWidget->setColumnWidth(0,300); ui->tableWidget->setColumnWidth(1,80); ui->tableWidget->setColumnWidth(2,80); ui->tableWidget->setColumnWidth(3,80);
    ui->tableWidget->setColumnWidth(4,80); ui->tableWidget->setColumnWidth(5,90); ui->tableWidget->setColumnWidth(6,90); //ui->tableWidget->setColumnWidth(7,200);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(20);
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);

    player = new QMediaPlayer();
    player->setVolume(70);

	connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
	connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonLoad_clicked()
{    
    QVector <QString> Name, Format, Channel, SampleRate, BitDepth, Size, Duration, Date;

    QStringList temp_file_names = QFileDialog::getOpenFileNames(Q_NULLPTR, tr("Open Audio Files"), defaultPath, tr("Audio Files (*.halac *.wav)"));
    //qDebug() << temp_file_names;
    if (temp_file_names.isEmpty()) return;

    defaultPath = temp_file_names.at(0).left(temp_file_names.at(0).lastIndexOf("/"));
    //qDebug() << default_path;

    bool state = true;
    for (int i = 0; i < temp_file_names.size(); i++) {
        for (int j = 0; j < filenames.size(); j++) {
            if (temp_file_names.at(i) == filenames.at(j)) {
                state = false;
                break;
            }
        }
        if (state) filenames.append(temp_file_names.at(i));
    }
    //qDebug() << file_names;
    ///////////////////////////////////////////////
    file_count = filenames.size();

    for (int i=0; i<file_count; i++) {
        QFile file(filenames.at(i));
        file.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(filenames.at(i));
        quint64 size = fileInfo.size();
        Size.append(QString("%L1").arg(size));        
        Name.append (fileInfo.fileName());
        //////////////////////////////////

        QByteArray data;
        quint32 durationSize;

        if (fileInfo.suffix() == "halac") {
            Format.append("HALAC");
            //////////////////////////////////
            file.seek(12);
            data = file.read(4);
            quint8 b0 = data.at(0), b1 = data.at(1), b2 = data.at(2), b3 = data.at(3);
            durationSize = (((b0 | (b1 << 8)) | (b2 << 0x10)) | (b3 << 0x18));
            //////////////////////////////////
            file.seek(30);
            data = file.read(2);
            Channel.append(QString::number(data.at(1) << 8 | (data.at(0) & 0xFF)));
            //////////////////////////////////
            data = file.read(4);
            b0 = data.at(0), b1 = data.at(1), b2 = data.at(2), b3 = data.at(3);
            quint32 sample_rate = (((b0 | (b1 << 8)) | (b2 << 0x10)) | (b3 << 0x18));
            SampleRate.append(QString::number(sample_rate));
            //////////////////////////////////
            file.seek(42);
            data = file.read(2);
            BitDepth.append(QString::number(data.at(1) << 8 | (data.at(0) & 0xFF)));
            //////////////////////////////////
        }
        else if (fileInfo.suffix() == "wav") {
            Format.append("WAV");
            //////////////////////////////////
            file.seek(4);
            data = file.read(4);
            quint8 b0 = data.at(0), b1 = data.at(1), b2 = data.at(2), b3 = data.at(3);
            durationSize = (((b0 | (b1 << 8)) | (b2 << 0x10)) | (b3 << 0x18));
            //////////////////////////////////
            file.seek(22);
            data = file.read(2);
            Channel.append(QString::number(data.at(1) << 8 | (data.at(0) & 0xFF)));
            //////////////////////////////////
            data = file.read(4);
            b0 = data.at(0), b1 = data.at(1), b2 = data.at(2), b3 = data.at(3);
            quint32 sample_rate = (((b0 | (b1 << 8)) | (b2 << 0x10)) | (b3 << 0x18));
            SampleRate.append(QString::number(sample_rate));
            //////////////////////////////////
            file.seek(34);
            data = file.read(2);
            BitDepth.append(QString::number(data.at(1) << 8 | (data.at(0) & 0xFF)));
            //////////////////////////////////
            //qDebug() << durationSize << Channel.at(i) << SampleRate.at(i) << BitDepth.at(i);
        }

        quint32 hours, minutes, seconds;
        quint32 totalSecs = (durationSize) / ( Channel.at(i).toInt() * SampleRate.at(i).toInt() * BitDepth.at(i).toInt() / 8 );
        hours = totalSecs / 3600;
        minutes = (totalSecs % 3600) / 60;
        seconds = totalSecs % 60;
        Duration.append(QString("%1:%2:%3").arg(hours, 2, 10, QLatin1Char('0')).arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));

        QDateTime dateTime = fileInfo.lastModified();
        Date.append(dateTime.toString("yyyy-MM-dd hh:mm:ss"));

        file.close();
    }

    ///////////////////////////////////////////////
    if (file_count > 22) ui->tableWidget->setRowCount(file_count);

    for (int i=0; i<file_count; i++) {
        QTableWidgetItem *itemName = new QTableWidgetItem();
        QTableWidgetItem *itemFormat = new QTableWidgetItem();
        QTableWidgetItem *itemChannel = new QTableWidgetItem();
        QTableWidgetItem *itemSampleRate = new QTableWidgetItem();
        QTableWidgetItem *itemBitDepth = new QTableWidgetItem();
        QTableWidgetItem *itemSize = new QTableWidgetItem();
        QTableWidgetItem *itemDuration = new QTableWidgetItem();
        QTableWidgetItem *itemDate = new QTableWidgetItem();

        itemFormat->setTextAlignment(Qt::AlignCenter);
        itemChannel->setTextAlignment(Qt::AlignCenter);
        itemSampleRate->setTextAlignment(Qt::AlignCenter);
        itemBitDepth->setTextAlignment(Qt::AlignCenter);
        itemSize->setTextAlignment(Qt::AlignRight); //Qt::AlignRight | Qt::AlignVCenter
        itemDuration->setTextAlignment(Qt::AlignCenter);
        itemDate->setTextAlignment(Qt::AlignCenter);

        itemName->setText(Name.at(i));
        itemFormat->setText(Format.at(i));
        itemChannel->setText(Channel.at(i));
        itemSampleRate->setText(SampleRate.at(i));
        itemBitDepth->setText(BitDepth.at(i));
        itemSize->setText(Size.at(i));
        itemDuration->setText(Duration.at(i));
        itemDate->setText(Date.at(i));

        ui->tableWidget->setItem(i,0,itemName);
        ui->tableWidget->setItem(i,1,itemFormat);
        ui->tableWidget->setItem(i,2,itemChannel);
        ui->tableWidget->setItem(i,3,itemSampleRate);
        ui->tableWidget->setItem(i,4,itemBitDepth);
        ui->tableWidget->setItem(i,5,itemSize);
        ui->tableWidget->setItem(i,6,itemDuration);
        ui->tableWidget->setItem(i,7,itemDate);
    }
    ui->tableWidget->setFocus();
    ui->tableWidget->selectRow(0);
    ui->statusBar->clearMessage();
}

void MainWindow::on_positionChanged(quint64 position)
{
    ui->horizontalSlider_Audio_Timeline->setValue(position);

    if (position != 0 && position == player->duration()) {
        if (ui->btnLoop->isChecked()) on_btnNext_clicked();
        else on_btnStop_clicked();
        return;
    }

    quint32 hours, minutes, seconds, ms;
    quint32 totalSecs = position / 1000;

    hours = totalSecs / 3600;
    minutes = (totalSecs % 3600) / 60;
    seconds = totalSecs % 60;
    if (position > 100) seconds++;

    //QDateTime::fromTime_t(totalSecs).toString("hh:mm:ss");
    ui->digital->setText(QString("%1:%2:%3").arg(hours, 2, 10, QLatin1Char('0')).arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));

    //ui->lcdNumber_2->display(QString::number(position/1000));
}

void MainWindow::on_durationChanged(quint64 duration)
{
    if (duration == 0) return;
    ui->horizontalSlider_Audio_Timeline->setRange(0, duration);
    ui->statusBar->showMessage(filenames.at(ui->tableWidget->currentRow()));
}

void MainWindow::on_btnPlay_clicked()
{
    //if (file_count == 0) return;
    if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1) == 0x0) return;

    if (player->state() == QMediaPlayer::StoppedState) {
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

        if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1)->text() == "HALAC") {
            LOAD_HALAC(filenames.at(ui->tableWidget->currentRow()));
        }
        else if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1)->text() == "WAV") {
            player->setMedia(QUrl::fromLocalFile(filenames.at(ui->tableWidget->currentRow())));
        }
        player->play();
    }
    else if (player->state() == QMediaPlayer::PlayingState) {
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        player->pause();
    }
    else if (player->state() == QMediaPlayer::PausedState) {
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        player->play();
    }
}

void MainWindow::on_btnStop_clicked()
{
    if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1) == 0x0) return;

    if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1)->text() == "HALAC") {
        if (byte_array.size() > 0) {
            player->stop();
            byte_array.clear();
            delete buffer;
        }
    }
    else if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1)->text() == "WAV") {
        player->stop();
    }
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void MainWindow::on_btnBack_clicked()
{
    if (file_count == 0) return;

    int index = ui->tableWidget->currentRow();
    if (index == 0) index = file_count - 1;
    else index--;

    ui->tableWidget->selectRow(index);
    on_tableWidget_cellDoubleClicked(index, 0);
}

void MainWindow::on_btnNext_clicked()
{
    if (file_count == 0) return;

    int index = ui->tableWidget->currentRow();
    if (index == file_count - 1) index = 0;
    else index++;

    ui->tableWidget->selectRow(index);
    on_tableWidget_cellDoubleClicked(index, 0);
}

void MainWindow::on_btnLoop_clicked()
{
    //
}

void MainWindow::on_buttonMute_clicked()
{
    if (ui->buttonMute->isChecked()) {
        player->setMuted(true);
        ui->buttonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
    else {
        player->setMuted(false);
        ui->buttonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }

}

void MainWindow::on_dial_valueChanged(int value)
{
    player->setVolume(value);
}

void MainWindow::on_buttonRemove_clicked()
{
    QList<QTableWidgetItem*> selected_rows = ui->tableWidget->selectedItems();
    while( !selected_rows.isEmpty() )
    {
        QTableWidgetItem *itm = selected_rows.at(0);
        filenames.removeAt(itm->row());
        file_count--;
        ui->tableWidget->removeRow(itm->row());
        selected_rows = ui->tableWidget->selectedItems();        
    }
    //ui->tableWidget->insertRow(22);
}

void MainWindow::on_buttonRemoveAll_clicked()
{
    on_btnStop_clicked();
    ui->tableWidget->setRowCount(0);
    filenames.clear();
    file_count = 0;
    ui->tableWidget->setRowCount(22);
    ui->statusBar->clearMessage();
}

void MainWindow::on_horizontalSlider_Audio_Timeline_actionTriggered(int action)
{
    player->setPosition(ui->horizontalSlider_Audio_Timeline->value());
}

void MainWindow::LOAD_HALAC(QString filename)
{
    ///////////////////// Loading DLL ///////////////////
    HMODULE hMod = LoadLibrary(TEXT("HALAC_0.2.7_AVX.dll"));
    if (hMod == NULL) {
        qDebug() << "DLL not loaded!";
        return;
    }
    ///////////// Getting Function Adresses /////////////
    EXPORT_WAVFunc EXPORT_WAV = (EXPORT_WAVFunc)GetProcAddress(hMod, "EXPORT_WAV");
    EXPORT_SIZEFunc EXPORT_SIZE = (EXPORT_SIZEFunc)GetProcAddress(hMod, "EXPORT_SIZE");
    EXPORT_DELETEFunc EXPORT_DELETE = (EXPORT_DELETEFunc)GetProcAddress(hMod, "EXPORT_DELETE");
    ////////////////////////////////////////
    if (EXPORT_WAV == NULL) {
        qDebug() << "The EXPORT_WAV function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (EXPORT_SIZE == NULL) {
        qDebug() << "The EXPORT_SIZE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (EXPORT_DELETE == NULL) {
        qDebug() << "The EXPORT_DELETE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }
    /////////////////////////////////////////////////////
    unsigned int data_size = EXPORT_SIZE(filename.toLatin1());
    //char* out = new char[data_size];
    char* out = EXPORT_WAV(filename.toLatin1(), 16);
    /////////////////////////////////////////////////////
    byte_array.clear();
    byte_array.append(out, data_size);
    EXPORT_DELETE();

    buffer = new QBuffer(&byte_array);
    buffer->setBuffer(&byte_array);
    buffer->open(QIODevice::ReadOnly);

    player->setMedia(QMediaContent(), buffer);

    FreeLibrary(hMod);
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1) == 0x0) return;

    if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1)->text() == "HALAC") {
        if (byte_array.size() > 0) {
            player->stop();
            byte_array.clear();
            delete buffer;
        }
        LOAD_HALAC(filenames.at(ui->tableWidget->currentRow()));
    }
    else if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1)->text() == "WAV") {
        player->setMedia(QUrl::fromLocalFile(filenames.at(ui->tableWidget->currentRow())));
    }
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    player->play();
}

void MainWindow::on_buttonSeekNext_clicked()
{
    if (player->state() == QMediaPlayer::StoppedState) return;
    player->setPosition(player->position() + 10000);
}

void MainWindow::on_buttonSeekBack_clicked()
{
    if (player->state() == QMediaPlayer::StoppedState) return;

    int pos = player->position() - 10000;
    if (pos < 0) on_btnBack_clicked();
    else player->setPosition(player->position() - 10000);
}

void MainWindow::on_actionOpen_Files_triggered()
{
    on_buttonLoad_clicked();
}

void MainWindow::on_actionRemove_Selected_Files_triggered()
{
    on_buttonRemove_clicked();
}

void MainWindow::on_actionRemove_All_Files_triggered()
{
    on_buttonRemoveAll_clicked();
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionAbout_HALAC_triggered()
{
    QMessageBox::about(
        this,
        "About HALAC Audio Player Version 0.1.0",
        "<strong>HALAC (High Availability Lossless Audio Compression)</strong><br />"
        "It is a free and open source player who can play audio data in the .halac and .wav format. "
        "This program uses HALAC 0.2.7 codec version.");
}

void MainWindow::on_actionAbout_Author_triggered()
{
    QMessageBox::about(
        this,
        "About The Author",
        "<strong>Hakan ABBAS<br />Computer Science Researcher &amp; Software Specialist</strong><br />"
        "<table>"
        "<tr>"
        "<td align = right>E-Mail&nbsp;&nbsp;&nbsp;&nbsp; :</td>"
        "<td width = 230>&nbsp;<a "
        "href='mailto:abbas.hakan@gmail.com'>abbas.hakan@gmail.com</a></td>"
        "</tr>"
        "<tr>"
        "<td align = right>LinkedIn :</td>"
        "<td>&nbsp;<a "
        "href='https://www.linkedin.com/in/hakan-abbas'>https://www.linkedin.com/in/hakan-abbas</a></td>"
        "</tr>"
        "<tr>"
        "<td align = right>Github&nbsp;&nbsp;&nbsp; :</td>"
        "<td>&nbsp;<a "
        "href='https://github.com/Hakan-Abbas/'>https://github.com/Hakan-Abbas/</a></td>"
        "</tr>"
        "<tr>"
        "<td align = right>Youtube :</td>"
        "<td>&nbsp;<a href='http://www.youtube.com/hakanabbas'>youtube.com/hakanabbas</a></td>"
        "</tr>"
        "</table>");
}

