#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <QSqlQuery>
#include <QSqlDatabase>
#include "song.h"
class DataBaseManager
{
public:
    static DataBaseManager* getInstance(); //静态公共的单例返回方法
    static DataBaseManager* m_instance;//静态单例
    QSqlDatabase m_sqlDatabase;//数据库对象
    QString m_databaseType;//数据库类型
    QString m_databaseName;//数据库名

    bool init();//初始化数据库接口, 初始化操作主要是设置数据库名，打开数据库，初始化歌曲表。
    bool initSongs();//初始化歌曲表
    bool destroy();//释放接口
    bool addsong(const Song& song);//添加歌曲接口
    bool querySongs(QList<Song*>& songsResult);//查询所有歌曲的接口
    bool clearSongs();//定义清空歌曲接口


private:
    DataBaseManager();  //构造函数私有化
    ~DataBaseManager();

};

#endif // DATABASEMANAGER_H
