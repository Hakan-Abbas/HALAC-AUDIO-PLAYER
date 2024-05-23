#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QMediaPlayer>
#include <QBuffer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    //void resizeEvent(QResizeEvent* event);

    void on_buttonLoad_clicked();

    void on_btnPlay_clicked();

    void on_btnStop_clicked();

    void on_btnBack_clicked();

    void on_btnNext_clicked();

    void on_btnLoop_clicked();

    void on_positionChanged(quint64);

    void on_durationChanged(quint64);

    void on_buttonMute_clicked();

    void on_buttonRemove_clicked();

    void on_buttonRemoveAll_clicked();

    void on_horizontalSlider_Audio_Timeline_actionTriggered(int action);

    void LOAD_HALAC(QString filename);

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_buttonSeekNext_clicked();

    void on_buttonSeekBack_clicked();

    void on_actionOpen_Files_triggered();

    void on_actionRemove_Selected_Files_triggered();

    void on_actionRemove_All_Files_triggered();

    void on_actionQuit_triggered();

    void on_actionAbout_Author_triggered();

    void on_actionAbout_HALAC_triggered();

    void on_dial_speed_valueChanged(int value);

    void on_dial_volume_valueChanged(int value);

    void on_spinBox_first_frame_valueChanged(int arg1);

    void on_spinBox_last_frame_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;

    //QString defaultPath = "C:/Users/ÖĞRETMEN/source/repos/HALAC_V.0.1/WAV/YABANCI/Busta_Rhymes";
    QString defaultPath = "C:/Users/HAKAN/source/repos/HALAC_V.0.1/WAV/test";
    QStringList filenames;
    quint32 file_count = 0;
    qint32 INDEX = -1;
    QString ACTIVE_FORMAT;

    QMediaPlayer *player;
    bool ok, ERROR_STATE = false;

    QByteArray byte_array;
    QBuffer *buffer;

};

#endif // MAINWINDOW_H
