#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTime>
#include <QByteArray>
#include <QStringList>
#include <QDebug>
#include <QAudioFormat>
#include <QAudioOutput>

/////////////////// Function Prototypes ///////////////////
typedef const bool (__stdcall *CHECK_HALACFunc)(const char*); //__stdcall
typedef const char* (__stdcall *GET_VERSIONFunc)(const char*);
typedef const unsigned char (__stdcall *GET_MODEFunc)(const char*);
typedef const unsigned int (__stdcall *GET_INPUT_FILE_SIZEFunc)(const char*);
typedef const unsigned short (__stdcall *GET_CHANNEL_COUNTFunc)(const char*);
typedef const unsigned int (__stdcall *GET_SAMPLE_RATEFunc)(const char*);
typedef const unsigned short (__stdcall *GET_BIT_COUNTFunc)(const char*);
typedef const unsigned int (__stdcall *GET_INPUT_DATA_SIZEFunc)(const char*);
typedef const int (__stdcall *GET_METADATA_SIZEFunc)(const char*);
typedef const char* (__stdcall *GET_METADATAFunc)(const char*, int);
typedef const unsigned int (__stdcall *GET_FRAME_COUNTFunc)(const char*);
typedef const unsigned int* (__stdcall *GET_FRAME_SIZESFunc)(const char*);
typedef const unsigned int* (__stdcall *GET_COMPRESSED_FRAME_SIZESFunc)(const char*);
typedef const unsigned long long (__stdcall *GET_FIRST_FRAME_START_POSITIONFunc)(const char*, unsigned int);
typedef const unsigned long long (__stdcall *GET_SELECTED_COMPRESSED_FRAMES_SIZEFunc)(const char*, unsigned int, unsigned int);
typedef const unsigned long long (__stdcall *GET_SELECTED_FRAMES_SIZEFunc)(const char*, unsigned int, unsigned int);

typedef char* (__stdcall *GET_FRAME_DATASFunc)(const char*, unsigned int, unsigned int, unsigned short);
typedef char* (__stdcall *CREATE_WAV_HEADERFunc)(const char*);

typedef void (__stdcall *DELETE_MEMORYFunc)();
///////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QFontDatabase::addApplicationFont(":/resource/fonts/digital-7__mono_.ttf");
    QFont fontDigital = QFont("Digital-7 Mono",24);
    ui->digital->setFont(fontDigital);
    ui->digital_volume->setFont(fontDigital);
    ui->digital_speed->setFont(fontDigital);

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

    ui->tableWidget->setColumnWidth(0,300); ui->tableWidget->setColumnWidth(1,70);
    ui->tableWidget->setColumnWidth(2,70); ui->tableWidget->setColumnWidth(3,70);
    ui->tableWidget->setColumnWidth(4,70); ui->tableWidget->setColumnWidth(5,70);
    ui->tableWidget->setColumnWidth(6,100); ui->tableWidget->setColumnWidth(7,80);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(22);
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
    QVector <QString> Name, Format, Version, Channel, SampleRate, BitDepth, Size, Duration, Date;

    QStringList temp_file_names = QFileDialog::getOpenFileNames(Q_NULLPTR, tr("Open Audio Files"), defaultPath, tr("Audio Files (*.halac *.wav)"));
    //qDebug() << temp_file_names;
    if (temp_file_names.isEmpty()) return;

    defaultPath = temp_file_names.at(0).left(temp_file_names.at(0).lastIndexOf("/"));
    //qDebug() << default_path;

    bool state, HALAC_STATE;
    for (int i = 0; i < temp_file_names.size(); i++) {
        state = true;
        for (int j = 0; j < filenames.size(); j++) {
            if (temp_file_names.at(i) == filenames.at(j)) {
                state = false;
                continue;
            }
        }

        if (state) {
            if (temp_file_names.at(i).right(5) == "halac") {
                HALAC_STATE = true;
                QFile file(temp_file_names.at(i));
                file.open(QIODevice::ReadOnly);
                QByteArray data;
                data = file.read(11);

                if (data[0] != 'H' || data[1] != 'A' || data[2] != 'L' || data[3] != 'A' || data[4] != 'C') {
                    qDebug() << "Only HALAC file type is supported for input!";
                    HALAC_STATE = false;
                }

                const char* VERSION_FIRST = "HALAC 0.2.9";
                if (data[6] != '0' || data[8] != '2' || data[10] != '9') {
                    qDebug() << "Only" << VERSION_FIRST << "is supported for input!";
                    HALAC_STATE = false;
                }

                file.seek(49);
                data = file.read(2);
                quint16 channel_count = data.at(1) << 8 | (data.at(0) & 0xFF); //qDebug() << channel_count;
                if (channel_count != 2) {
                    qDebug() << "Channel count must be 2!";
                    HALAC_STATE = false;
                }

                file.seek(61);
                data = file.read(2);
                quint16 bps = data.at(1) << 8 | (data.at(0) & 0xFF); //qDebug() << bps;
                if (bps != 16) {
                    qDebug() << "Bits per sample must be 16!";
                    HALAC_STATE = false;
                }

                if (HALAC_STATE) filenames.append(temp_file_names.at(i));
                file.close();
            }
            else filenames.append(temp_file_names.at(i)); // wav
        }
    }

    ///////////////////////////////////////////////
    file_count = filenames.size();

    for (int i = 0; i < file_count; i++) {
        QFile file(filenames.at(i));
        file.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(filenames.at(i));
        quint64 size = fileInfo.size();
        Size.append(QString("%L1").arg(size));
        Name.append (fileInfo.fileName());
        //////////////////////////////////

        QByteArray data;
        quint64 durationSize;

        if (fileInfo.suffix() == "halac") {
            Format.append("HALAC");
            //////////////////////////////////
            file.seek(6);
            data = file.read(5);
            Version.append(data);
            //////////////////////////////////
            file.seek(35);
            data = file.read(8);
            quint8 b0 = data.at(0), b1 = data.at(1), b2 = data.at(2), b3 = data.at(3);
            quint8 b4 = data.at(4), b5 = data.at(5), b6 = data.at(6), b7 = data.at(7);
            durationSize = (((b0 | (b1 << 8)) | (b2 << 16)) | (b3 << 24) | (b4 << 32) | (b5 << 40) | (b6 << 48) | (b7 << 56));
            //////////////////////////////////
            file.seek(49);
            data = file.read(2);
            Channel.append(QString::number(data.at(1) << 8 | (data.at(0) & 0xFF)));
            //////////////////////////////////
            data = file.read(4);
            b0 = data.at(0), b1 = data.at(1), b2 = data.at(2), b3 = data.at(3);
            quint32 sample_rate = (((b0 | (b1 << 8)) | (b2 << 0x10)) | (b3 << 0x18));
            SampleRate.append(QString::number(sample_rate));
            //////////////////////////////////
            file.seek(61);
            data = file.read(2);
            BitDepth.append(QString::number(data.at(1) << 8 | (data.at(0) & 0xFF)));
            //////////////////////////////////
        }
        else if (fileInfo.suffix() == "wav") {
            Format.append("WAV");
            Version.append("");
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
    if (file_count > 20) ui->tableWidget->setRowCount(file_count);

    for (int i = 0; i < file_count; i++) {
        QTableWidgetItem *itemName = new QTableWidgetItem();
        QTableWidgetItem *itemFormat = new QTableWidgetItem();
        QTableWidgetItem *itemVersion = new QTableWidgetItem();
        QTableWidgetItem *itemChannel = new QTableWidgetItem();
        QTableWidgetItem *itemSampleRate = new QTableWidgetItem();
        QTableWidgetItem *itemBitDepth = new QTableWidgetItem();
        QTableWidgetItem *itemSize = new QTableWidgetItem();
        QTableWidgetItem *itemDuration = new QTableWidgetItem();
        QTableWidgetItem *itemDate = new QTableWidgetItem();

        itemName->setTextAlignment(Qt::AlignLeft);
        itemFormat->setTextAlignment(Qt::AlignCenter);
        itemVersion->setTextAlignment(Qt::AlignCenter);
        itemChannel->setTextAlignment(Qt::AlignCenter);
        itemSampleRate->setTextAlignment(Qt::AlignCenter);
        itemBitDepth->setTextAlignment(Qt::AlignCenter);
        itemSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemDuration->setTextAlignment(Qt::AlignCenter);
        itemDate->setTextAlignment(Qt::AlignCenter);

        itemName->setText(Name.at(i));
        itemFormat->setText(Format.at(i));
        itemVersion->setText(Version.at(i));
        itemChannel->setText(Channel.at(i));
        itemSampleRate->setText(SampleRate.at(i));
        itemBitDepth->setText(BitDepth.at(i));
        itemSize->setText(Size.at(i));
        itemDuration->setText(Duration.at(i));
        itemDate->setText(Date.at(i));

        ui->tableWidget->setItem(i,0,itemName);
        ui->tableWidget->setItem(i,1,itemFormat);
        ui->tableWidget->setItem(i,2,itemVersion);
        ui->tableWidget->setItem(i,3,itemChannel);
        ui->tableWidget->setItem(i,4,itemSampleRate);
        ui->tableWidget->setItem(i,5,itemBitDepth);
        ui->tableWidget->setItem(i,6,itemSize);
        ui->tableWidget->setItem(i,7,itemDuration);
        ui->tableWidget->setItem(i,8,itemDate);
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

    if (seconds == 60) { seconds = 0; minutes++; }
    if (minutes == 60) { minutes = 0; hours++; }
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
            ACTIVE_FORMAT = "HALAC";
            LOAD_HALAC(filenames.at(ui->tableWidget->currentRow()));
            if (!ERROR_STATE) player->play();
        }
        else if (ui->tableWidget->item(ui->tableWidget->currentRow(), 1)->text() == "WAV") {
            ACTIVE_FORMAT = "WAV";
            player->setMedia(QUrl::fromLocalFile(filenames.at(ui->tableWidget->currentRow())));
            player->play();
        }        
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

    if (ACTIVE_FORMAT == "HALAC") {
        if (byte_array.size() > 0) {
            player->stop();
            byte_array.clear();
            delete buffer;
        }
    }
    else player->stop();

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
    //ui->tableWidget->insertRow(20);
}

void MainWindow::on_buttonRemoveAll_clicked()
{
    on_btnStop_clicked();
    ui->tableWidget->setRowCount(0);
    filenames.clear();
    file_count = 0;
    ui->tableWidget->setRowCount(20);
    ui->statusBar->clearMessage();
}

void MainWindow::on_horizontalSlider_Audio_Timeline_actionTriggered(int action)
{
    //player->setPosition(ui->horizontalSlider_Audio_Timeline->value());
    Qt::MouseButtons buttons = QApplication::mouseButtons();
    QPoint pos = ui->horizontalSlider_Audio_Timeline->mapFromGlobal(QCursor::pos());
    bool click = (buttons & Qt::LeftButton) && (pos.x() >= 0 && pos.y() >= 0 && pos.x() < ui->horizontalSlider_Audio_Timeline->size().width() && pos.y() < ui->horizontalSlider_Audio_Timeline->size().height());

    if (click) {
        float ratio = pos.x() / (float)ui->horizontalSlider_Audio_Timeline->size().width();
        int range = ui->horizontalSlider_Audio_Timeline->maximum() - ui->horizontalSlider_Audio_Timeline->minimum();
        int slider_pos = ui->horizontalSlider_Audio_Timeline->minimum() + range * ratio;

        ui->horizontalSlider_Audio_Timeline->setValue(slider_pos);
        player->setPosition(slider_pos);
    }
}

void MainWindow::LOAD_HALAC(QString filename)
{
    ///////////////////// Loading DLL ///////////////////
    HMODULE hMod = LoadLibrary(TEXT("HALAC_PLAYER_DLL.dll"));
    if (hMod == NULL) {
        qDebug() << "DLL not loaded!";
        return;
    }
    ///////////// Getting Function Adresses /////////////
    CHECK_HALACFunc CHECK_HALAC = (CHECK_HALACFunc)GetProcAddress(hMod, "CHECK_HALAC");
    GET_VERSIONFunc GET_VERSION = (GET_VERSIONFunc)GetProcAddress(hMod, "GET_VERSION");
    GET_MODEFunc GET_MODE = (GET_MODEFunc)GetProcAddress(hMod, "GET_MODE");
    GET_INPUT_FILE_SIZEFunc GET_INPUT_FILE_SIZE = (GET_INPUT_FILE_SIZEFunc)GetProcAddress(hMod, "GET_INPUT_FILE_SIZE");
    GET_CHANNEL_COUNTFunc GET_CHANNEL_COUNT = (GET_CHANNEL_COUNTFunc)GetProcAddress(hMod, "GET_CHANNEL_COUNT");
    GET_SAMPLE_RATEFunc GET_SAMPLE_RATE = (GET_SAMPLE_RATEFunc)GetProcAddress(hMod, "GET_SAMPLE_RATE");
    GET_BIT_COUNTFunc GET_BIT_COUNT = (GET_BIT_COUNTFunc)GetProcAddress(hMod, "GET_BIT_COUNT");
    GET_INPUT_DATA_SIZEFunc GET_INPUT_DATA_SIZE = (GET_INPUT_DATA_SIZEFunc)GetProcAddress(hMod, "GET_INPUT_DATA_SIZE");
    GET_METADATA_SIZEFunc GET_METADATA_SIZE = (GET_METADATA_SIZEFunc)GetProcAddress(hMod, "GET_METADATA_SIZE");
    GET_METADATAFunc GET_METADATA = (GET_METADATAFunc)GetProcAddress(hMod, "GET_METADATA");
    GET_FRAME_COUNTFunc GET_FRAME_COUNT = (GET_FRAME_COUNTFunc)GetProcAddress(hMod, "GET_FRAME_COUNT");
    GET_FRAME_SIZESFunc GET_FRAME_SIZES = (GET_FRAME_SIZESFunc)GetProcAddress(hMod, "GET_FRAME_SIZES");
    GET_COMPRESSED_FRAME_SIZESFunc GET_COMPRESSED_FRAME_SIZES = (GET_COMPRESSED_FRAME_SIZESFunc)GetProcAddress(hMod, "GET_COMPRESSED_FRAME_SIZES");

    GET_FIRST_FRAME_START_POSITIONFunc GET_FIRST_FRAME_START_POSITION = (GET_FIRST_FRAME_START_POSITIONFunc)GetProcAddress(hMod, "GET_FIRST_FRAME_START_POSITION");
    GET_SELECTED_COMPRESSED_FRAMES_SIZEFunc GET_SELECTED_COMPRESSED_FRAMES_SIZE = (GET_SELECTED_COMPRESSED_FRAMES_SIZEFunc)GetProcAddress(hMod, "GET_SELECTED_COMPRESSED_FRAMES_SIZE");
    GET_SELECTED_FRAMES_SIZEFunc GET_SELECTED_FRAMES_SIZE = (GET_SELECTED_FRAMES_SIZEFunc)GetProcAddress(hMod, "GET_SELECTED_FRAMES_SIZE");

    GET_FRAME_DATASFunc GET_FRAME_DATAS = (GET_FRAME_DATASFunc)GetProcAddress(hMod, "GET_FRAME_DATAS");
    CREATE_WAV_HEADERFunc CREATE_WAV_HEADER = (CREATE_WAV_HEADERFunc)GetProcAddress(hMod, "CREATE_WAV_HEADER");

    DELETE_MEMORYFunc DELETE_MEMORY = (DELETE_MEMORYFunc)GetProcAddress(hMod, "DELETE_MEMORY");

    ////////////////////////////////////////
    if (CHECK_HALAC == NULL) {
        qDebug() << "The CHECK_HALAC function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_VERSION == NULL) {
        qDebug() << "The GET_VERSION function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_MODE == NULL) {
        qDebug() << "The GET_MODE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_INPUT_FILE_SIZE == NULL) {
        qDebug() << "The GET_INPUT_FILE_SIZE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_CHANNEL_COUNT == NULL) {
        qDebug() << "The GET_CHANNEL_COUNT function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_SAMPLE_RATE == NULL) {
        qDebug() << "The GET_SAMPLE_RATE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_BIT_COUNT == NULL) {
        qDebug() << "The GET_BIT_COUNT function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_INPUT_DATA_SIZE == NULL) {
        qDebug() << "The GET_INPUT_DATA_SIZE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_METADATA_SIZE == NULL) {
        qDebug() << "The GET_METADATA_SIZE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_METADATA == NULL) {
        qDebug() << "The GET_METADATA function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_FRAME_COUNT == NULL) {
        qDebug() << "The GET_FRAME_COUNT function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_COMPRESSED_FRAME_SIZES == NULL) {
        qDebug() << "The GET_COMPRESSED_FRAME_SIZES function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_FRAME_SIZES == NULL) {
        qDebug() << "The GET_FRAME_SIZES function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_FIRST_FRAME_START_POSITION == NULL) {
        qDebug() << "The GET_FIRST_FRAME_START_POSITION function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_SELECTED_COMPRESSED_FRAMES_SIZE == NULL) {
        qDebug() << "The GET_SELECTED_COMPRESSED_FRAMES_SIZE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_SELECTED_FRAMES_SIZE == NULL) {
        qDebug() << "The GET_SELECTED_FRAMES_SIZE function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (GET_FRAME_DATAS == NULL) {
        qDebug() << "The GET_FRAME_DATAS function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (CREATE_WAV_HEADER == NULL) {
        qDebug() << "The CREATE_WAV_HEADER function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    if (DELETE_MEMORY == NULL) {
        qDebug() << "The DELETE_MEMORY function address could not be obtained!";
        FreeLibrary(hMod);
        return;
    }

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.read(1024 * 64);

    if (!CHECK_HALAC(data)) {
        qDebug() << "HALAC FORMAT ERROR!";
        data.clear();
        file.close();
        FreeLibrary(hMod);

        ERROR_STATE = true;
        return;
    }

    ERROR_STATE = false;
    //////////////////////////////////////

/*
    const char* version = GET_VERSION(data); for (int i = 0; i < 11; i++) qDebug() << "v " << version[i];
    qDebug() << GET_MODE(data) << GET_INPUT_FILE_SIZE(data) << GET_CHANNEL_COUNT(data) <<
    GET_BIT_COUNT(data) << GET_SAMPLE_RATE(data) << GET_INPUT_DATA_SIZE(data);

    qDebug() << GET_METADATA_SIZE(data);
    for (int i = 0; i < GET_METADATA_SIZE(data); i++) {
        qDebug() << GET_METADATA(data, GET_METADATA_SIZE(data))[i];
    }

    unsigned int frame_count = GET_FRAME_COUNT(data);
    qDebug() << frame_count;

    int frame_comp_sizes[frame_count];
    memcpy(frame_comp_sizes, GET_COMPRESSED_FRAME_SIZES(data), 4 * frame_count);
    for (int i = 0; i < frame_count; i++) qDebug() << i << frame_comp_sizes[i];
    qDebug() << "----------";
    unsigned int frame_sizes[frame_count];
    memcpy(frame_sizes, GET_FRAME_SIZES(data), 4 * frame_count);
    for (int i = 0; i < frame_count; i++) qDebug() << i << frame_sizes[i];
    qDebug() << "----------";
*/

    /////////////////////////////////////////////////////
    quint16 thread_count = 16;
    quint32 frame_count = GET_FRAME_COUNT(data);
    /////////////////////////////////////////////////////

    ui->spinBox_total_frame->setValue(frame_count);
    ui->spinBox_last_frame->setMaximum(frame_count - 1);

    if (INDEX != ui->tableWidget->currentRow()) {
        ui->spinBox_first_frame->setValue(0);
        ui->spinBox_last_frame->setValue(frame_count - 1);
    }
    quint32 first_frame = ui->spinBox_first_frame->value();
    quint32 last_frame = ui->spinBox_last_frame->value();
    /////////////////////////////////////////////////////

    int start_pos = GET_FIRST_FRAME_START_POSITION(data, first_frame);
    int compressed_frames_size = GET_SELECTED_COMPRESSED_FRAMES_SIZE(data, first_frame, last_frame);
    int frames_size = GET_SELECTED_FRAMES_SIZE(data, first_frame, last_frame);

    //////////////////////////////////
    char header_data[44];
    memcpy(header_data, CREATE_WAV_HEADER(data), 44);
    memcpy(header_data + 40, &frames_size, 4); // update DATA_SIZE

    byte_array.clear();
    byte_array.append(header_data, 44);
    //////////////////////////////////

    file.seek(start_pos);
    data = file.read(compressed_frames_size);
    char* wav_data = GET_FRAME_DATAS(data.data(), first_frame, last_frame, thread_count);

    byte_array.append(wav_data, frames_size);
    DELETE_MEMORY();

    buffer = new QBuffer(&byte_array);
    buffer->open(QIODevice::ReadOnly);
    player->setMedia(QMediaContent(), buffer);
    /////////////////////////////////////////////////

    INDEX = ui->tableWidget->currentRow();

    file.close();
    FreeLibrary(hMod);
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    on_btnStop_clicked();
    on_btnPlay_clicked();
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
        "About HALAC Audio Player Version 0.1.4",
        "HALAC (High Availability Lossless Audio Compression)<br /><br />"
        "It is a free and open source player who can play audio data in the .halac and .wav format. "
        "This program uses HALAC 0.2.9 codec version.");
}

void MainWindow::on_actionAbout_Author_triggered()
{
    QMessageBox::about(
        this,
        "About The Author",
        "Hakan ABBAS<br />Computer Science Researcher &amp; Software Specialist<br />"
        "<table>"
        "<tr>"
        "<td align = right>E-Mail&nbsp;&nbsp;&nbsp;&nbsp; :</td>"
        "<td width = 230>&nbsp;<a "
        "href='mailto:abbas.hakan@gmail.com'>abbas.hakan@gmail.com</a></td>"
        "</tr>"
        "<tr>"
        "<td align = right>LinkedIn :</td>"
        "<td>&nbsp;<a "
        "href='https://linkedin.com/in/hakan-abbas'>https://www.linkedin.com/in/hakan-abbas</a></td>"
        "</tr>"
        "<tr>"
        "<td align = right>Github&nbsp;&nbsp;&nbsp; :</td>"
        "<td>&nbsp;<a "
        "href='https://github.com/Hakan-Abbas/'>https://github.com/Hakan-Abbas/</a></td>"
        "</tr>"
        "</table>");
}

void MainWindow::on_dial_speed_valueChanged(int value)
{
    if (value % 10 == 0) ui->digital_speed->setText(QString::number(value / 10) + ".0");
    else {
        float new_value = value / 10.0;
        ui->digital_speed->setText(QString::number(new_value));
        player->setPlaybackRate(new_value);
    }
}

void MainWindow::on_dial_volume_valueChanged(int value)
{
    player->setVolume(value);
    if (value == 0) {
        ui->buttonMute->setChecked(true);
        ui->buttonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
    else {
        ui->buttonMute->setChecked(false);
        ui->buttonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }
}

void MainWindow::on_spinBox_first_frame_valueChanged(int arg1)
{
    int value = ui->spinBox_last_frame->value();
    if (arg1 > value) ui->spinBox_first_frame->setValue(value);
}

void MainWindow::on_spinBox_last_frame_valueChanged(int arg1)
{
    int value = ui->spinBox_first_frame->value();
    if (arg1 < value) ui->spinBox_last_frame->setValue(value);
}

