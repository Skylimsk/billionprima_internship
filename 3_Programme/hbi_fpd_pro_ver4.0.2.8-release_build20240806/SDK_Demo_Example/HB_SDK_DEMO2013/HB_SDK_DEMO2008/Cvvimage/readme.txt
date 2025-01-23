修改备注：
1.在CvvImage.h头文件中增加了#include "opencv2/imgproc/imgproc.hpp"  
因为我用到这个文件中缩放的函数cvResize()。

2.在CvvImage.h头文件中增加了缩放函数ResizeImage(IplImage *srcimg,RECT* resizerect)的声明。

3.在CvvImage.h头文件中只做了1,2修改，并有中文备注。

4.在CvvImage.cpp源文件中增加了图片缩放函数ResizeImage(IplImage *srcimg,RECT* resizerect)定义。

5.剩下的修改全部函数DrawtoHdc里面,把里面显示图片的roi进行了修改，图片显示居中等等，在文件中有备注。