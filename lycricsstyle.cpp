#include "lycricsstyle.h"
#include <QStyle>
LycricsStyle::LycricsStyle()
{

    lycrisEmptystring = QString("%1").arg("无歌词！");
    lycrisEmptyfont.setBold(true);//加粗
    lycrisEmptyfont.setPointSize(20);//字体大小

}
