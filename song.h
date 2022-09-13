#ifndef SONG_H
#define SONG_H
#include <QUrl>
#include <QString>
//歌曲类
//1.封装媒体文件路径url、歌曲名name、歌手artist、专辑名album等歌曲信息作为数据成员
//2.提供相关接口访问这些数据成员
class Song
{
public:
    Song();

    Song(const QUrl& url,
         const QString& name = 0
         )
        :m_url(url),
        m_name(name)

        {}
public:
    QUrl url()const{return m_url;}
    QString name()const{return m_name;}

private:
     QUrl m_url;
     QString m_name;

};


#endif // SONG_H
