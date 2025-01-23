#include "stdafx.h"
#include "CvvImage.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CV_INLINE RECT NormalizeRect( RECT r );
CV_INLINE RECT NormalizeRect( RECT r )
{
   int t;
   if( r.left > r.right )
   {
      t = r.left;
      r.left = r.right;
      r.right = t;
   }
   if( r.top > r.bottom )
   {
      t = r.top;
      r.top = r.bottom;
      r.bottom = t;
   }

   return r;
}
CV_INLINE CvRect RectToCvRect( RECT sr );
CV_INLINE CvRect RectToCvRect( RECT sr )
{
   sr = NormalizeRect( sr );
   return cvRect( sr.left, sr.top, sr.right - sr.left, sr.bottom - sr.top );
}
CV_INLINE RECT CvRectToRect( CvRect sr );
CV_INLINE RECT CvRectToRect( CvRect sr )
{
   RECT dr;
   dr.left = sr.x;
   dr.top = sr.y;
   dr.right = sr.x + sr.width;
   dr.bottom = sr.y + sr.height;

   return dr;
}
CV_INLINE IplROI RectToROI( RECT r );
CV_INLINE IplROI RectToROI( RECT r )
{
   IplROI roi;
   r = NormalizeRect( r );
   roi.xOffset = r.left;
   roi.yOffset = r.top;
   roi.width = r.right - r.left;
   roi.height = r.bottom - r.top;
   roi.coi = 0;

   return roi;
}
void  FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin )
{
   assert( bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));

   BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

   memset( bmih, 0, sizeof(*bmih));
   bmih->biSize = sizeof(BITMAPINFOHEADER);
   bmih->biWidth = width;
   bmih->biHeight = origin ? abs(height) : -abs(height);
   bmih->biPlanes = 1;
   bmih->biBitCount = (unsigned short)bpp;
   bmih->biCompression = BI_RGB;
   if( bpp == 8 )
   {
      RGBQUAD* palette = bmi->bmiColors;
      int i;
      for( i = 0; i < 256; i++ )
      {
         palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
         palette[i].rgbReserved = 0;
      }
   }
}
CvvImage::CvvImage()
{
   m_img = 0;
}
void CvvImage::Destroy()
{
   cvReleaseImage( &m_img );
}
CvvImage::~CvvImage()
{
   Destroy();
}
bool  CvvImage::Create( int w, int h, int bpp, int origin )
{
   const unsigned max_img_size = 10000;

   if( (bpp != 8 && bpp != 24 && bpp != 32) ||
      (unsigned)w >=  max_img_size || (unsigned)h >= max_img_size ||
      (origin != IPL_ORIGIN_TL && origin != IPL_ORIGIN_BL))
   {
      assert(0); // most probably, it is a programming error
      return false;
   }
   if( !m_img || Bpp() != bpp || m_img->width != w || m_img->height != h )
   {
      if( m_img && m_img->nSize == sizeof(IplImage))
         Destroy();
      /* prepare IPL header */
      m_img = cvCreateImage( cvSize( w, h ), IPL_DEPTH_8U, bpp/8 );
   }
   if( m_img )
      m_img->origin = origin == 0 ? IPL_ORIGIN_TL : IPL_ORIGIN_BL;
   return m_img != 0;
}
void  CvvImage::CopyOf( CvvImage& image, int desired_color )
{
   IplImage* img = image.GetImage();
   if( img )
   {
      CopyOf( img, desired_color );
   }
}
#define HG_IS_IMAGE(img)\
   ((img) != 0 && ((const IplImage*)(img))->nSize == sizeof(IplImage) &&\
   ((IplImage*)img)->imageData != 0)
void  CvvImage::CopyOf( IplImage* img, int desired_color )
{
   if( HG_IS_IMAGE(img) )
   {
      int color = desired_color;
      CvSize size = cvGetSize( img );
      if( color < 0 )
         color = img->nChannels > 1;
      if( Create( size.width, size.height,
         (!color ? 1 : img->nChannels > 1 ? img->nChannels : 3)*8,
         img->origin ))
      {
         cvConvertImage( img, m_img, 0 );
      }
   }
}

void  CvvImage::CopyOf(IplImage* img, int width, int height, int desired_color)
{
	if (HG_IS_IMAGE(img))
	{
		int color = desired_color;
		if (color < 0)
			color = img->nChannels > 1;
		if (Create(width, height,
			(!color ? 1 : img->nChannels > 1 ? img->nChannels : 3) * 8,
			img->origin))
		{
			cvConvertImage(img, m_img, 0);
		}
	}
}

bool  CvvImage::Load( const char* filename, int desired_color )
{
   IplImage* img = cvLoadImage( filename, desired_color );
   if( !img )
      return false;

   CopyOf( img, desired_color );
   cvReleaseImage( &img );

   return true;
}
bool  CvvImage::LoadRect( const char* filename,
                   int desired_color, CvRect r )
{
   if( r.width < 0 || r.height < 0 ) return false;

   IplImage* img = cvLoadImage( filename, desired_color );
   if( !img )
      return false;
   if( r.width == 0 || r.height == 0 )
   {
      r.width = img->width;
      r.height = img->height;
      r.x = r.y = 0;
   }
   if( r.x > img->width || r.y > img->height ||
      r.x + r.width < 0 || r.y + r.height < 0 )
   {
      cvReleaseImage( &img );
      return false;
   }
   /* truncate r to source image */
   if( r.x < 0 )
   {
      r.width += r.x;
      r.x = 0;
   }
   if( r.y < 0 )
   {
      r.height += r.y;
      r.y = 0;
   }
   if( r.x + r.width > img->width )
      r.width = img->width - r.x;

   if( r.y + r.height > img->height )
      r.height = img->height - r.y;
   cvSetImageROI( img, r );
   CopyOf( img, desired_color );
   cvReleaseImage( &img );
   return true;
}
bool  CvvImage::Save( const char* filename )
{
   if( !m_img )
      return false;
   cvSaveImage( filename, m_img );
   return true;
}
void  CvvImage::Show( const char* window )
{
   if( m_img )
      cvShowImage( window, m_img );
}
void  CvvImage::Show( HDC dc, int x, int y, int w, int h, int from_x, int from_y )
{
   if( m_img && m_img->depth == IPL_DEPTH_8U )
   {
      uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
      BITMAPINFO* bmi = (BITMAPINFO*)buffer;
      int bmp_w = m_img->width, bmp_h = m_img->height;
      FillBitmapInfo( bmi, bmp_w, bmp_h, Bpp(), m_img->origin );
      from_x = MIN( MAX( from_x, 0 ), bmp_w - 1 );
      from_y = MIN( MAX( from_y, 0 ), bmp_h - 1 );
      int sw = MAX( MIN( bmp_w - from_x, w ), 0 );
      int sh = MAX( MIN( bmp_h - from_y, h ), 0 );
      SetDIBitsToDevice(
         dc, x, y, sw, sh, from_x, from_y, from_y, sh,
         m_img->imageData + from_y*m_img->widthStep,
         bmi, DIB_RGB_COLORS );
   }
}

	   ///////////////////////////////////////////////////////////////////////////////////////////////////
	   ////以下是增加缩放函数的定义
	   //////////////////////////////////////////////////////////////////////////////////////////////////
       void CvvImage::ResizeImage(IplImage *srcimg,RECT* resizerect)
	   {
		   CRect m_rect;
           m_rect.CopyRect(resizerect);
       float    cw=(float)m_rect.Width();    //客户区的宽
	   float    ch=(float)m_rect.Height();   //客户区的高
	   float    pw=(float)srcimg->width; //载入图像的宽
	   float    ph=(float)srcimg->height;//载入图像的高
	   float    cs=cw/ch;//客户区的宽高比
	   float    ps=pw/ph;//载入图像的宽高比
	   float    scale=(cs>ps)?(ch/ph):(cw/pw); //缩放比例因子
	   int      rw=(int)(pw*scale);//缩放后图片的宽
	   int      rh=(int)(ph*scale);//缩放后图片的高 
	   m_resimage=cvCreateImage(cvSize(rw,rh),srcimg->depth,srcimg->nChannels);
	   cvResize(srcimg,m_resimage);
	   }
	   ///////////////////////////////////////////////////////////////////////////////////////////////////
	   ////以上是增加缩放函数定义
	   //////////////////////////////////////////////////////////////////////////////////////////////////

void  CvvImage::DrawToHDC( HDC hDCDst, RECT* pDstRect )
{
   if( pDstRect && m_img && m_img->depth == IPL_DEPTH_8U && m_img->imageData )
   {
	  ResizeImage(m_img,pDstRect);//先把要显示的图片按比例缩放至客户区窗口

      uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
      BITMAPINFO* bmi = (BITMAPINFO*)buffer;
      int bmp_w = m_resimage->width, bmp_h = m_resimage->height;//m_img修改成了m_resimage,m_resimage是缩放后的图片
      CvRect roi = cvGetImageROI( m_resimage );                             //下面都是如此修改
      CvRect dst = RectToCvRect( *pDstRect );

	  int add_w=(int)((dst.width-roi.width)/2); //左顶点需要向右平移的距离
	  int add_h=(int)((dst.height-roi.height)/2); //左顶点需要向下平移的距离
      if( roi.width == dst.width && roi.height == dst.height )
      {
         Show( hDCDst, dst.x+add_w, dst.y+add_h, bmp_w, bmp_h, roi.x, roi.y );//原来的dst.width, dst.height分别修改为bmp_w, bmp_h
		 //Show( hDCDst, dst.x, dst.y, dst.width, dst.height, roi.x, roi.y );//这是原来的函数，上面的是修改过的，也就是原来的
		                                                                     //dst.x变成了dst.x+add_w,dst.y变成了dst.y+add_h，
                                                                             //下面都如此做了修改
		 return;
      }
      if( roi.width >  bmp_w )//原来的dst.width修改为bmp_w
      {
         SetStretchBltMode(
            hDCDst,           // handle to device context
            HALFTONE );
      }
      else
      {
         SetStretchBltMode(
            hDCDst,           // handle to device context
            COLORONCOLOR );
      }
      FillBitmapInfo( bmi, bmp_w, bmp_h, Bpp(), m_resimage->origin );
      ::StretchDIBits(
         hDCDst,
         dst.x+add_w, dst.y+add_h,  bmp_w,  bmp_h,//原来的dst.width, dst.height分别修改为bmp_w, bmp_h
         roi.x, roi.y, roi.width, roi.height,                       //dst.x变成了dst.x+add_w;    dst.y变成了dst.y+add_h，
         m_resimage->imageData, bmi, DIB_RGB_COLORS, SRCCOPY );
   }
}
void  CvvImage::Fill( int color )
{
   cvSet( m_img, cvScalar(color&255,(color>>8)&255,(color>>16)&255,(color>>24)&255) );
}