#include "databasemanager.h"
#include <QDebug>
#include <QSqlError>
DataBaseManager::DataBaseManager()
{
    m_databaseType = "QSQLITE";
    //数据库类型
    m_databaseName = "player.db";
    //数据库名
    m_sqlDatabase = QSqlDatabase::addDatabase(m_databaseType);
    init();

}

DataBaseManager::~DataBaseManager()
{
   destroy();
}
DataBaseManager* DataBaseManager::m_instance = new DataBaseManager();
DataBaseManager* DataBaseManager::getInstance(){
    return m_instance;
}

bool DataBaseManager::initSongs()//初始化\创建歌曲表
{
    bool ret = m_sqlDatabase.isOpen();
    if(!ret){
        qDebug()<<"database is not open";
        return ret;
    }

    QString sql = "create table if not exists songs("//创建表如果不存在 表名songs
            "url text unique not null,"
            "name text);";
    QSqlQuery query;
    ret = query.exec(sql);
    if(!ret){
        qDebug()<<"initsongs query error"<<query.lastError().text();
        return ret;
    }else{
        qDebug()<<"initsongs query success";
        return ret;
    }
}

bool DataBaseManager::destroy() //释放接口
{
    bool ret = m_sqlDatabase.isOpen();
    if(ret){
        m_sqlDatabase.close();//关闭
        QSqlDatabase::removeDatabase(m_databaseName);//移除数据库
        qDebug()<<"m_sqldatabase destory";
    }
    return ret;
}

bool DataBaseManager::addsong(const Song& song)//往数据库中添加音乐信息的接口 存入信息为url name
{
    bool ret = m_sqlDatabase.isOpen();
    if (!ret) {
        qDebug() << "DatabaseManager addSong, database is not open";
        return ret;
    }//qDebug() << song.url() << " " << song.name()
    QSqlQuery query;
    query.prepare("insert into songs(url, name)"
                  "values(:url, :name);" );
    query.bindValue(":url",song.url());
    query.bindValue(":name",song.name());
    qDebug()<<song.url();
    ret = query.exec();
    if (!ret) {
        qDebug() << "DatabaseManager addSong, query error: " << query.lastError().text();
        return ret;
    }
    qDebug() << "DatabaseManager addSong success";
    return ret;
}

bool DataBaseManager::querySongs(QList<Song *> &songsResult)
{
    bool ret = m_sqlDatabase.isOpen();
    if (!ret) { qDebug() << "DatabaseManager querySongs, database is not open";
        return ret; }QString sql = "select url, name, artist, album from songs";
    QSqlQuery query; ret = query.exec(sql);
    if (!ret) {
        qDebug() << "DatabaseManager querySongs, query error: " << query.lastError().text();
        return ret;
    }
    qDebug() << "DatabaseManager querySongs, query success";
    QString url, name;
    while (query.next()) { url = query.value("url").toString();
        name = query.value("name").toString();
        songsResult.push_back(new Song(url, name));
        qDebug() << "DatabaseManager querySongs, query: " << url << " " << name;
    }
    return ret;
}

bool DataBaseManager::clearSongs()
{
    //定义清空歌曲接口
    //首先判断数据库状态是否打开，如果没打开，打印错误返回
    //编写delete语句，删除表中歌曲数据
    //判断语句执行是否正常，错误的话打印

        bool ret = m_sqlDatabase.isOpen();
        if (!ret) {
            qDebug() << "DatabaseManager clearSongs, database is not open";
            return ret; }QString sql = "delete from songs;";
        QSqlQuery query; ret = query.exec(sql);
        if (!ret) {

            qDebug() << "DatabaseManager clearSongs, query error: " << query.lastError().text();
            return ret;
        }qDebug() << "DatabaseManager clearSongs success";
        return ret;
}
//删除数据库所保存的歌曲


bool DataBaseManager::init()
{

        m_sqlDatabase.setDatabaseName(m_databaseName);
        bool ret = m_sqlDatabase.open();
        if (!ret) {
            qDebug() << "DatabaseManager init, open failed: " << m_databaseName;
            return ret;
        }
        ret = initSongs();//初始化歌曲表
        if (!ret) {
            qDebug() << "DatabaseManager init, init songs failed";
            destroy(); //初始化表失败，释放数据库资源
            return ret;
            }
        qDebug() << "DatabaseManager init success";
        return ret;
}

