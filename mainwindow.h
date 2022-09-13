#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QString>
#include <QMap>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QTimer>
#include "song.h"
#include "databasemanager.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QMap<QString,Song*>m_currentPlaylist;//声明当前播放列表存储容器

    QMap<qint64, QString>m_lyrics; //用键值对存储歌曲时间和歌词

    QTimer * m_timer;//定时器





private:
    void getSongInfoFromMp3File(const QString& filepath);//从歌曲文件中获取信息存入Song中

    bool initDatabase();//初始化数据库

    void destoryDatabase();//释放数据库

    bool addsongToSql(const Song& song);//添加歌曲信息到数据库

    bool getSongsFromSql();//从数据库中查询所有歌曲

public:
    void updateAllLyrics(); //显示全部歌词函数

    void updateLyricsOnTime();//实时刷新歌词函数

    void displayNolyrics();//显示无歌词

    void readLyricsFromFile(const QString&);//从歌词文件中读取并解析歌词内容

    bool clearSongs();//删除数据库所保存的歌曲

public:
    int autoindex;//自定义歌曲序号

private slots:  //槽函数
    void on_addMusicButton_clicked();//添加歌曲按钮



    void progress_positionchanged(qint64 position);    //进度条位置变化槽函数
    void on_playpushButton_clicked();

    //void on_pausepushButton_clicked();

    void on_musicpositionSlider_sliderReleased();       //进度条松开槽函数

    void musicChanged(const QMediaContent &);       //歌曲改变槽函数



    void on_playBackMode_pushButton_clicked();      //播放模式更改槽函数

    void on_lastmusicpushButton_clicked();

    void on_nextmusicpushButton_clicked();



    void on_playlist_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void handleTimeout();

    void on_clearList_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QMediaPlayer* m_player;
    QMediaPlaylist* m_list;
    QListWidget* m_listwidget;
    QListWidgetItem* lycrisEmpty;
    DataBaseManager* m_database;//调用数据库管理单例类指针



};
#endif // MAINWINDOW_H
