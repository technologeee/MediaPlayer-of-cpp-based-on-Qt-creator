#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QObject>
#include <QMediaContent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //初始化数据库
    initDatabase();

    autoindex = 0;
    ui->setupUi(this);
    //初始化音乐播放器指针
    m_player = new QMediaPlayer();
    m_list = new QMediaPlaylist(this);

    m_player->setPlaylist(m_list);

    //设置播放列表播放方式为循环播放
    m_list->setPlaybackMode(QMediaPlaylist::Loop);

    //设置空歌词显示文本和字体



    //歌词定时器初始化
    m_timer = new QTimer(this);
    //连接m_timer的timeout信号和handletimeout槽函数
    QObject::connect(m_timer,&QTimer::timeout,this,&MainWindow::handleTimeout);
    m_timer->start(100);//传入时间参数（毫秒），比如100毫秒，定时器将以100毫秒为间隔发 送超时信号

    ui->playpushButton->setIcon(QPixmap(":/pauseIcon.png"));//设置playpushbutton初始显示图标
    ui->lastmusicpushButton->setIcon(QPixmap(":/lastmusicIcon.png"));//设置上一首图标
    ui->nextmusicpushButton->setIcon(QPixmap(":/nextmusicIcon.png"));//设置下一首图标
    ui->playBackMode_pushButton->setIcon(QPixmap(":/LoopIcon.png"));//设置播放模式初始图标  列表循环
//    //窗口背景
//    QPalette pal = this->palette();//获取mainwindow的调色板
//    pal.setBrush(QPalette::Background,QBrush(QPixmap(":/C:/Users/Administrator/Pictures/musicPlayerImageSources/backgroundJpg.jpg")));//设置调色板
//    setPalette(pal);//应用到背景


    //连接m_player的positionChanged的信号函数和mainwindow中定义的slot进度条位置改变函数
    QObject::connect(m_player,&QMediaPlayer::positionChanged,this,&MainWindow::progress_positionchanged);
    QObject::connect(m_list,&QMediaPlaylist::currentMediaChanged,this,&MainWindow::musicChanged);
    //连接空格和pausebutton信号槽
//    QObject::connect(this,&MainWindow::key_spacepushed,this,&MainWindow::on_pausepushButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
    for (auto eachValue : m_currentPlaylist) //释放m_currentPlayList
    {
        delete eachValue;
    }
    m_currentPlaylist.clear();
    destoryDatabase();//释放数据库
//    for (auto eachValue : m_lyrics)
//    {
//        delete eachValue;
//    }
//    m_lyrics.clear();
}

void MainWindow::musicChanged(const QMediaContent &content){
    ui->playlist_listWidget->setCurrentRow(m_list->currentIndex()); //设置当前歌曲为焦点
    qDebug()<<content.request().url().fileName();

    //截取文件名.前字符串
    QString musicname = content.request().url().fileName();
    QString filename = content.request().url().toString();
    qDebug()<<filename;
    QString lrcfilename = filename.replace(".mp3",".lrc");
    qDebug()<<"lrcname is"<< lrcfilename;
    QString basename;
    int pointsite = musicname.indexOf(".");
//    qDebug()<<musicname.indexOf(".");
    basename = musicname.left(pointsite);

    ui->musicLabel->setText(basename);

//    updateAllLyrics();//调用显示歌词函数
    readLyricsFromFile(lrcfilename);//存入歌词到m_lyrics中
    updateAllLyrics();//读取歌词
}
void MainWindow::on_addMusicButton_clicked()
{

    //使用文件对话框，选择歌曲文件

    QStringList files = QFileDialog::getOpenFileNames(this,//父对象
                                                QString::fromLocal8Bit("addmusic"),//对话框标题
                                                QString(),//对话框打开的默认路径
                                                QString::fromLocal8Bit("musicfile (*.mp3)")
                                                 );//对话框显示文件的过滤规则， 此处显示.mp3后缀的文件

    for(auto file : files){
    if(file.isEmpty()){//如果为空
        qDebug()<<"add music failed";
    }else{

        auto iter = m_currentPlaylist.find(file);//定义迭代器在每次循环中查找file 返回的自动推导出的类型为iterator
        if(iter != m_currentPlaylist.end()){
            //find函数返回查找到的file迭代器，如果不是.end() 则表示找到了该file 并返回了其迭代器   说明file存在于m_currentPlaylist的键中
            qDebug()<<"已含有该歌曲";
            continue;   //跳过后面步骤 并继续循环
        }

        //解码MP3文件获取歌曲信息 并存入map（m_currentPlaylist）中
//        getSongInfoFromMp3File(file); 解码信息不全  此处更改为按文件路径和文件名解析 存入键和键


        qDebug() << "add music success: " << file;
        QUrl url(file);

        QString musicname = QFileInfo(file).baseName();

        //    Song * song = new Song(QUrl(filePath), name);
        //    m_currentPlaylist.insert(filePath, song);
            //解码获取的歌曲信息不全，时有时无，所以不在此存储歌曲信息到m_currentPlaylist中
            //在添加歌曲按钮槽函数中实现

        Song* song = new Song(url,musicname);

        m_currentPlaylist.insert(file,song);

        addsongToSql(*song);
        //m_player->setMedia(QMediaContent(url)); //添加到播放器，取消添加到播放器

        m_list->addMedia(QMediaContent(url));   //添加到播放列表

        int item_index = autoindex++;//存储播放列表当前文件下标
        qDebug()<<"添加文件下标为："<<item_index;

        //ui->musicLabel->setText(QFileInfo(file).baseName());


        QListWidgetItem* item = new QListWidgetItem();//新建


        qDebug()<<"name:"<<song->name();
        item->setText(QFileInfo(file).baseName());//设置文本



        //设置item的备注信息为文件的QString形式的index
        item->setWhatsThis(QString("%1").arg(item_index));

        //控件添加item项目
        ui->playlist_listWidget->addItem(item);

        QString mediaPath = file;
        QString lyricsFilePath = mediaPath.replace(".mp3", ".lrc");
        qDebug() << "lyricsFilePath: " << lyricsFilePath;





        if (QFileInfo(lyricsFilePath).isFile())
        {
            qDebug() << "it is a file";
            //3.调用读取歌词文件函数
            readLyricsFromFile(lyricsFilePath);
        }else{
            qDebug() << "it is not a file";
            //未读取到歌词的情况
            lyricsFilePath.clear();//清空字符串
            //显示无歌词

             }

        }
    }
}

void MainWindow::readLyricsFromFile(const QString& filePath) {

    //0.清空歌词存储容器
    m_lyrics.clear();

    //1.判断参数是否是一个文件
    bool ret = QFileInfo(filePath).isFile();
    if (!ret){
        qDebug() << "MainWindow readLyricsFromFile, it is not file: " << filePath;
        return;
    }
    qDebug() << "MainWindow readLyricsFromFile read lyrics file: " << filePath;

    //2.读取文件
    //2.1 使用QFile打开文件
    QFile file(filePath);

    //使用歌词文件路径构造一个文件对象
    ret = file.open(QIODevice::ReadOnly | QIODevice::Text);

    //使用只读模式和文本 模式打开歌词文件
    if (!ret){
        qDebug() << "MainWindow readLyricsFromFile, open failed: " << filePath;
        return;
    }

    //2.2 使用QTextStream按行读取文件内容
    //声明一个字符串line，保存每行内容: {"[时间标签]文本"}
    QString line;

    //声明一个字符串列表保存第一次行分割后的内容: {"[时间标签", "文本"}
    QStringList lineContents;

    //声明一个字符串列表保存第二次分割后的时间内容: {"[分钟数", "秒数.毫秒数"}
    QStringList timeContents;

    //构造文本流对象
    QTextStream ts(&file);
    ts.setCodec("utf-8");   //解决读取中文乱码问题

    //如果文本流没读到文件末尾，就循环读取
    while (!ts.atEnd()){
        //读取一行内容
        line = ts.readLine();
        qDebug() << "MainWindow readLyricsFromFile line: " << line;

        //判断行内容是否为空,不为空则将这行内容进行解析，并存储到歌词存储容器中
        if (!line.isEmpty()){
            //根据']'第一次分割行内容得: {"[时间标签", "文本"}
            lineContents = line.split(']');

            //lineContents第一部分是时间标签，根据':'第二次分割得时间内容: {"[分钟 数", "秒数.毫秒数"}
            timeContents = lineContents[0].split(':');

            //时间内容timeContents的第一部分去掉'['，转换成整型的分钟数
            int minutes = timeContents[0].mid(1).toInt();

            //时间内容timeContents的第二部分，转换成整型的秒数
            double seconds = timeContents[1].toDouble();

            //(分钟数 * 60 + 秒数) * 1000 得这一行对应的播放时间进度
            //lineContents第二部分是歌曲信息或歌词文本,直接存储
            //将时间戳和文本存储到歌词存储容器中
            m_lyrics.insert((minutes * 60 + seconds) * 1000, lineContents[1]);

            lineContents.clear();
            timeContents.clear();


        }
    }
}

bool MainWindow::clearSongs()
{
    return m_database->getInstance()->clearSongs();
}
void MainWindow::on_playpushButton_clicked()
{

    if (m_player->state() != m_player->PlayingState)//不等于播放状态，即暂停或停 止状态
    {
        m_player->play();
//        ui->playpushButton->setText("pause");//设置playlabel文本
        ui->playpushButton->setIcon(QPixmap(":/playIcon.png"));
    }else {
        m_player->pause();
//        ui->playpushButton->setText("play");
        ui->playpushButton->setIcon(QPixmap(":/pauseIcon.png"));
    }
}

//void MainWindow::on_pausepushButton_clicked()
//{
//    qDebug()<<"pausepushbuttonclicked EVENT";
//    m_player->pause();
//}

//void MainWindow::on_stoppushButton_clicked()
//{
//    m_player->stop();
//}
void MainWindow::progress_positionchanged(qint64 position)    //进度条位置变化槽函数
{
    //转换为以秒为单位的当前播放位置
    qint64 sPosition = position / 1000;
    qDebug()<<sPosition;
    //转换为以秒为单位的歌曲总时长
    qint64 songDuration =  m_player->duration() / 1000;
    qDebug()<<songDuration;
    //设置musictimeLabel的显示文本
    int sposition_min = sPosition / 60;
    int sposition_sec = sPosition % 60;
    int sduration_min = songDuration / 60;
    int sduration_sec = songDuration % 60;

    QString labelText = tr("%1").arg(sposition_min);
    labelText += ":";
    labelText += tr("%1").arg(sposition_sec);
    labelText += "/";
    labelText += tr("%1").arg(sduration_min);
    labelText += ":";
    labelText += tr("%1").arg(sduration_sec);

    ui->musictimeLabel->setText(labelText); //设置标签文本 用来显示时间

    //根据position设置滑动条位置
    //获取滑动条最大长度
    int sliderMax = ui->musicpositionSlider->maximum();

    //设置位置为position/duration *maxlenth
    ui->musicpositionSlider->setValue(((double)sPosition/(double)songDuration)*sliderMax);  //需要将sPosition进行强制类型转换，否则会导致程序异常崩溃，可能是小数点后数值过多

}







void MainWindow::on_musicpositionSlider_sliderReleased()
{
    //根据位置设定媒体播放进度
    //进度条的值
    int sliderValue = ui->musicpositionSlider->value();
    //进度条的最大值
    int sliderValueMax = ui->musicpositionSlider->maximum();
    //计算所占百分比
    double processpercent = (double)sliderValue/(double)sliderValueMax;
    //根据所占百分比设定音乐播放位置
    m_player->setPosition(m_player->duration()*processpercent);

}





void MainWindow::on_playBackMode_pushButton_clicked()
{
    //CurrentItemOnce, CurrentItemInLoop, Sequential, Loop, Random
    qDebug()<<m_list->playbackMode();
    if(m_list->playbackMode() == QMediaPlaylist::Loop){//循环-》随机
//        ui->playBackMode_pushButton->setText("随机播放");
        ui->playBackMode_pushButton->setIcon(QPixmap(":/randomIcon.png"));

        m_list->setPlaybackMode(QMediaPlaylist::Random);

        qDebug()<<"设置播放模式为"<<m_list->playbackMode();

    }else if(m_list->playbackMode() == QMediaPlaylist::Random){//随机-》单曲
//        ui->playBackMode_pushButton->setText("单曲循环");
        ui->playBackMode_pushButton->setIcon(QPixmap(":/thisLoopIcon.png"));

        m_list->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);

        qDebug()<<"设置播放模式为"<<m_list->playbackMode();
    }else if(m_list->playbackMode() == QMediaPlaylist::CurrentItemInLoop){//单曲-》循环
//        ui->playBackMode_pushButton->setText("列表循环");
        ui->playBackMode_pushButton->setIcon(QPixmap(":/LoopIcon.png"));
        m_list->setPlaybackMode(QMediaPlaylist::Loop);

        qDebug()<<"设置播放模式为"<<m_list->playbackMode();
    }
}

void MainWindow::on_lastmusicpushButton_clicked()
{
    m_list->previous();

}

void MainWindow::on_nextmusicpushButton_clicked()
{
    m_list->next();

}



void MainWindow::on_playlist_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
   //将item中保存的QSting类型的url输出出来
   int index = item->whatsThis().toInt();
   qDebug()<<index;
   m_list->setCurrentIndex(index);
   MainWindow::handleTimeout();
}
void MainWindow::updateAllLyrics(){
    /*
    * 描述：显示全部歌词函数
    * 逻辑：
    * 1.读取歌词容器中的文本部分
    * QMap m_lyrics存储内容形式: {(key1, value1), (key2, value2), ...}
    * 即{(第一行的时间部分0, 第一行的歌曲信息或歌词文本: 光辉岁月 - BEYOND), ...}
    * 2.清空歌词列表控件的旧内容
    * 3.写入到界面的歌词列表控件中
    * 循环遍历歌词容器中的文本部分
    * 构造一个QListWidgetItem元素
    * 设置他的文本为行歌词文本
    * 添加到歌词列表控件
*/
    qDebug() << "MainWindow updateAllLyrics";
    //清空歌词列表控件的旧内容
    ui->lyrics_listWidget->clear();

    //判断歌词容器是否为空
    if (m_lyrics.isEmpty())//为空，界面显示当前歌曲无歌词
        //无歌词时listwidget显示
    {
        ui->lyrics_listWidget->addItem(lycrisEmpty);    //显示无歌词
        return;
    }else{  //有歌词时
        //循环遍历歌词容器中的文本部分
        //m_lyrics.values(): {value1, value2, ...}
        //即{第一行文本, 第二行文本, ...}
        for (auto text : m_lyrics.values()) {

            QListWidgetItem * item = new QListWidgetItem();
            QFont fontitem = QFont();
            fontitem.setPointSize(10);
            item->setFont(fontitem);
            item->setText(text);
            item->setTextAlignment(Qt::AlignCenter);
            //设置文本水平居中
            ui->lyrics_listWidget->addItem(item);

        }
        ui->lyrics_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏歌词列表和歌曲列表滚动条
        ui->lyrics_listWidget->setStyleSheet("background-color: transparent");// 设置背景透明
        ui->lyrics_listWidget->setFrameShape(QListWidget::NoFrame);//无边框
        ui->playlist_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏歌词列表和歌曲列表滚动条
        ui->playlist_listWidget->setStyleSheet("background-color: transparent");//设置背景透明
    }




}

void MainWindow::handleTimeout(){
    //封装一个实现刷新歌词函数，在此调用
    updateLyricsOnTime();

}

void MainWindow::displayNolyrics(){
    lycrisEmpty = new QListWidgetItem();
    lycrisEmpty->setTextAlignment(Qt::AlignCenter);
    lycrisEmpty->setText("无歌词");

    QFont emptyfont;
    emptyfont.setBold(true);//加粗
    //emptyfont.setItalic(true);//斜体
    emptyfont.setPointSize(20);//字体大小

    lycrisEmpty->setFont(emptyfont);
}
void MainWindow::updateLyricsOnTime(){
    //1.判断是否有歌词，为空则返回
    if (m_lyrics.isEmpty()) {
        displayNolyrics();
        return;
    }
    //如果还没有开始同步显示当前行，也就是刚开始播放显示了所有歌词，就将歌词列表控件的第一 行设置为当前行
    if (ui->lyrics_listWidget->currentRow() == -1)
        //同步显示前，当前行 currentRow的初始值是-1
    {
        ui->lyrics_listWidget->setCurrentRow(0);
        //设置第一行为当前行，会自动高亮 显示, 完成第一次同步
        return;
    }
    //3.读取媒体播放器的当前播放时间进度
    qint64 position = m_player->position();
    //获取界面所同步的当前行的行数
    int currentRow = ui->lyrics_listWidget->currentRow();
    //界面列表控件当前行和歌词容器中的时间部分的元素序号是对应的
    //获取歌词容器中的时间戳部分
    //m_lyrics.keys(): {key1, key2, ...}
    QList<qint64> lyricsTimeStamp = m_lyrics.keys();
    //比较媒体播放器的当前播放时间和界面列表当前行的时间戳（开始时间）
    if (position < lyricsTimeStamp[currentRow]){
        //往前找匹配的歌词行，找到当前播放时间大于或等于某一行的时间戳，
        //即这一行就是当前应该同步显示的行，记录这个行的索引，即行数
        while (currentRow > 0) {
            //递减一行
            --currentRow;
            //判断当前播放时间是否大于或等于某一行的时间戳
            if (position >= lyricsTimeStamp[currentRow]) {
                //这一行就是当前应该同步显示的行，记录这个行的索引，即行数
                break;
            }
        }
    }else if (position > lyricsTimeStamp[currentRow]) {
        //往后找匹配的歌词行，找到播放时间小于某一行的时间戳，
        //即它的上一行是所要同步显示的行，记录这个行的索引，即行数
        while (currentRow < lyricsTimeStamp.size() - 1) {
            //递增一行
            ++currentRow;
            //找到播放时间小于某一行的时间戳
            if (position < lyricsTimeStamp[currentRow]) {
                //它的上一行是所要同步显示的行，记录这个行的索引，即行数
                //递减一行
                --currentRow;
                break;
            }
        }
    }else//不小于，也不大于，就是等于
    {
        //就是这行，已同步，啥也不干
    }
    //5.将所要同步显示的行元素进行高亮和滚动垂直居中显示
    //获取列表控件的当前行元素
    QListWidgetItem * item = ui->lyrics_listWidget->item(currentRow);
    ui->lyrics_listWidget->setCurrentItem(item);
    //高亮显示当前所同步的行元素
    ui->lyrics_listWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    //当前行元素滚动到垂直居中位置
}

void MainWindow::getSongInfoFromMp3File(const QString& filePath){
    qDebug() << "get song info from file : " << filePath;
    QString name, artist, album;
    //1.打开文件，跳转到倒数第128个字节处
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        int length = file.size();
        //获取文件长度
        file.seek(length - 128);
        //跳转到倒数第128个字节处
        char infoArray[128];
        if (file.read(infoArray, sizeof(infoArray)) > 0) {
            //读取TAG信息，固定为字符串"TAG"，不是则说明格式有错

            //从infoArray的位置读取三个字节
            QString tag = QString::fromLocal8Bit(infoArray, 3);

            //从数组 infoArray中取三个字节
            qDebug() << "tag: " << tag;

            //获取歌曲名，从infoArray[3]位置开始读取30个字节
            name = QString::fromLocal8Bit(infoArray + 3, 30);

            //获取歌手名，从infoArray[33]位置开始读取30个字节
            artist = QString::fromLocal8Bit(infoArray + 33, 30);

            //获取专辑名，从infoArray[63]位置开始读取30个字节
            album = QString::fromLocal8Bit(infoArray + 63, 30);

            //去掉多余的空字符
            name.truncate(name.indexOf(QChar::Null));
            artist.truncate(name.indexOf(QChar::Null));
            album.truncate(name.indexOf(QChar::Null));

            qDebug() << "get info success, song: " << name << " " << artist << " " << album;
        }else {
            qDebug() << "get song info, read file failed";
        }
        file.close();
    }else {
        qDebug() << "get song info, open file failed: " << filePath;
    }
//    //3.构造一个歌曲对象，保存到歌曲对象容器中
//    Song * song = new Song(QUrl(filePath), name);
//    m_currentPlaylist.insert(filePath, song);
    //解码获取的歌曲信息不全，时有时无，所以不在此存储歌曲信息到m_currentPlaylist中
    //在添加歌曲按钮槽函数中实现
}

bool MainWindow::initDatabase()
{
    bool ret = (m_database->getInstance()->init());
    return ret;
}

void MainWindow::destoryDatabase()
{
    m_database->getInstance()->destroy();
}

bool MainWindow::addsongToSql(const Song &song)
{
    bool ret = m_database->getInstance()->addsong(song);
    if(ret){
        qDebug()<<"save song:"<<song.name()<<"to sql success";
    }
    if(!ret){
        qDebug()<<"save song:"<<song.name()<<"to sql failed";
    }
    return ret;

}

bool MainWindow::getSongsFromSql()
{
    qDebug() << "MainWindow getSongsFromSql";
    QList<Song*> songsResult;
    bool ret = m_database->getInstance()->querySongs(songsResult);
    for (int i = 0; i < songsResult.size(); ++i) {
        qDebug() << "MainWindow getSongsFromSql, song: " << songsResult[i]->name();
        m_currentPlaylist.insert(songsResult[i]->url().toString(), songsResult[i]);
        m_list->addMedia(songsResult[i]->url());
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(songsResult[i]->name());
        ui->playlist_listWidget->addItem(item);
    }
    return ret;
}





void MainWindow::on_clearList_pushButton_clicked()
{
    m_list->clear();
    ui->playlist_listWidget->clear();
    for (auto eachValue : m_currentPlaylist) {
        delete eachValue;
    }
    m_currentPlaylist.clear();
    clearSongs();
}
