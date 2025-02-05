#include "stdafx.h"
#include "CvvImage.h"

// Construction/Destruction

//For normalize the rectangle 
CV_INLINE RECT NormalizeRect( RECT r );
CV_INLINE RECT NormalizeRect( RECT r )
{
   int t;

   //Swap the values to ensure the positive width
   if( r.left > r.right )
   {
      t = r.left;
      r.left = r.right;
      r.right = t;
   }

   //Swap the values to ensure the positive height
   if( r.top > r.bottom )
   {
      t = r.top;
      r.top = r.bottom;
      r.bottom = t;
   }

   return r;
}

//Converts a Windows RECT structure to an OpenCV CvRect.
CV_INLINE CvRect RectToCvRect( RECT sr );
CV_INLINE CvRect RectToCvRect( RECT sr )
{
   sr = NormalizeRect( sr ); //For ensuring the valid dimensions
   return cvRect( sr.left, sr.top, sr.right - sr.left, sr.bottom - sr.top ); //Returns into OpenCV CvRect with correct width and height
}

//Converts an OpenCV CvRect back to a Windows RECT structure.
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

//Converts a RECT into an OpenCV IplROI structure.
CV_INLINE IplROI RectToROI( RECT r );
CV_INLINE IplROI RectToROI( RECT r )
{
   IplROI roi;
   r = NormalizeRect( r ); //Ensure the valid dimensions
   roi.xOffset = r.left;
   roi.yOffset = r.top;
   roi.width = r.right - r.left;
   roi.height = r.bottom - r.top;
   roi.coi = 0; //Channel of Intrest is set to 0, means all channels of image are considred, default setting for full-colour image operations.

   return roi;
}

//Fill BITMAPINFO based on input params
void  FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin )
//origin: Indicates whether the image data is stored top-down or bottom-up

{
    //Returns to null if no meet the assumption
   assert( bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));

   BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

   memset( bmih, 0, sizeof(*bmih));
   bmih->biSize = sizeof(BITMAPINFOHEADER);
   bmih->biWidth = width;
   bmih->biHeight = origin ? abs(height)  : -abs(height); 

   /*
   
    A positive height (abs(height))-> bottom-up (first scan line is the bottom row)
    A negative height (-abs(height)) -> top-down (first scan line is the top row)

   */

   bmih->biPlanes = 1; //Number of color planes in the bit map / Number of planes for the target device , always be 1 

   /*
   
   Why is it always 1?
    - The color planes concept comes from old graphics hardware that 
    supported multiple planes for storing image data (e.g., one plane for red, one for green, one for blue).
    - Modern bitmap formats store all color information in a single plane, so biPlanes is always set to 1.

   */

   bmih->biBitCount = (unsigned short)bpp; //bpp from input

   /*
   
    8 → Grayscale (requires a palette).
    24 → RGB (8 bits per channel, no alpha).
    32 → RGBA (8 bits per channel + Alpha).

   */

   bmih->biCompression = BI_RGB;  //compression type.

   if (bpp == 8)  // Only for 8-bit grayscale images
   {
       RGBQUAD* palette = bmi->bmiColors;  // Access the palette array
       int i;
       for (i = 0; i < 256; i++)  // Loop through all 256 possible grayscale values
       {
           palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i; // Set R=G=B=i (grayscale)
           palette[i].rgbReserved = 0;  // Reserved field, set to 0
       }
   }

}

//Constructor
CvvImage::CvvImage()
{
   m_img = 0;
}

//Memory Cleanup
void CvvImage::Destroy()
{
   cvReleaseImage( &m_img );
}

//Destructor
CvvImage::~CvvImage()
{
   Destroy();
}

//Image Allocation
bool  CvvImage::Create( int w, int h, int bpp, int origin )
{
   const unsigned max_img_size = 10000;

   //Validation Check
   if( (bpp != 8 && bpp != 24 && bpp != 32) || (unsigned)w >=  max_img_size || (unsigned)h >= max_img_size || (origin != IPL_ORIGIN_TL && origin != IPL_ORIGIN_BL))

        //IPL_ORIGIN_TL → Top-Left
        //IPL_ORIGIN_BL → Bottom - Left

   {
      assert(0); // most probably, it is a programming error
      return false;
   }

   //Checks if an image already exists
   if( !m_img || Bpp() != bpp || m_img->width != w || m_img->height != h )
   {
      //If has exisiting image and different size, it is destroyed and recreated.
      if( m_img && m_img->nSize == sizeof(IplImage))
         Destroy();

      //Create a new image
      m_img = cvCreateImage( cvSize( w, h ), IPL_DEPTH_8U, bpp/8 ); //(bpp/8) use for converts bpp into num of channels
   }

   //Set image origin
   if( m_img )
      m_img->origin = origin == 0 ? IPL_ORIGIN_TL : IPL_ORIGIN_BL;

   return m_img != 0; //returns true
}

//Copies an image from another CvvImage object
void  CvvImage::CopyOf( CvvImage& image, int desired_color )
{
   IplImage* img = image.GetImage(); 
   if( img )
   {
      CopyOf( img, desired_color );
   }
}

//Validation Check for valid IpImage
#define HG_IS_IMAGE(img)\
   ((img) != 0 && ((const IplImage*)(img))->nSize == sizeof(IplImage) &&\
   ((IplImage*)img)->imageData != 0)

//Copies an image from an IpImage
void  CvvImage::CopyOf( IplImage* img, int desired_color )
{
   if( HG_IS_IMAGE(img) ) // Validate input image
   {
      int color = desired_color;
      CvSize size = cvGetSize( img );

      if( color < 0 ) //Colour not specified
         color = img->nChannels > 1; // Auto-detect color format

      if (Create(size.width, size.height, (!color ? 1 : img->nChannels > 1 ? img->nChannels : 3) * 8, img->origin))

          // Explanation for bit depth calculation:
          // Condition 1: If color == 0 → Grayscale (1 channel, 8-bit)
          // Condition 2: If color == 1 and img has multiple channels → Keep img->nChannels
          // Condition 3: If color == 1 but img is single-channel → Force 3 channels (RGB)

      {
         cvConvertImage( img, m_img, 0 ); 
      }
   }
}

//Copies an image from an IpImage but allowing resizing
void  CvvImage::CopyOf(IplImage* img, int width, int height, int desired_color)
{
	if (HG_IS_IMAGE(img)) // Validate input image
	{
		int color = desired_color;

		if (color < 0)
			color = img->nChannels > 1; // Auto-detect color format

		if (Create(width, height, (!color ? 1 : img->nChannels > 1 ? img->nChannels : 3) * 8, img->origin))
		{
			cvConvertImage(img, m_img, 0);
		}
	}
}

//Load Image from file
bool  CvvImage::Load( const char* filename, int desired_color )
{
   IplImage* img = cvLoadImage( filename, desired_color ); 

   if( !img ) //If fails
      return false;

   CopyOf( img, desired_color ); //Copy Image into CvvImage
   cvReleaseImage( &img );

   return true;
}

//Load & Crop a region
bool  CvvImage::LoadRect( const char* filename,
                   int desired_color, CvRect r )
{
   if( r.width < 0 || r.height < 0 ) return false; //Validate Dimensions

   IplImage* img = cvLoadImage( filename, desired_color );

   if( !img ) //If load fails
      return false;

   if( r.width == 0 || r.height == 0 ) //If region is empty 
   {
      r.width = img->width; //Defaults to entire image width
      r.height = img->height; //Defaults to entire image height
      r.x = r.y = 0;
   }

   if( r.x > img->width || r.y > img->height || r.x + r.width < 0 || r.y + r.height < 0 ) //If region out of bounds
   {
      cvReleaseImage( &img ); //Release the image memory

      return false; 
   }

   //Adjust r to Source Image, ensures r stays within the image bounds

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


   cvSetImageROI( img, r ); //Crop the selected region
   CopyOf( img, desired_color ); //Copies the cropped image
   cvReleaseImage( &img ); //Release memory

   return true;
}

//Save Image
bool  CvvImage::Save( const char* filename )
{
   if( !m_img )
      return false;

   cvSaveImage( filename, m_img );
   return true;
}

//Display Image in an OpenCV Window
void  CvvImage::Show( const char* window )
{
   if( m_img )
      cvShowImage( window, m_img );
}

//Display Using Windows GDI
void  CvvImage::Show( HDC dc, int x, int y, int w, int h, int from_x, int from_y )
{
   if( m_img && m_img->depth == IPL_DEPTH_8U ) //Check Image valids and is 8 bit depth)
   {
      uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
      BITMAPINFO* bmi = (BITMAPINFO*)buffer;

      int bmp_w = m_img->width, bmp_h = m_img->height;
      FillBitmapInfo( bmi, bmp_w, bmp_h, Bpp(), m_img->origin );

      // Ensure from_x and from_y are within valid bounds
      from_x = MIN( MAX( from_x, 0 ), bmp_w - 1 );
      from_y = MIN( MAX( from_y, 0 ), bmp_h - 1 );

      // Calculate valid width and height to display
      int sw = MAX( MIN( bmp_w - from_x, w ), 0 );
      int sh = MAX( MIN( bmp_h - from_y, h ), 0 );

      // Display the image on the given device context (HDC)
      SetDIBitsToDevice(
         dc, x, y, sw, sh, from_x, from_y, from_y, sh,
         m_img->imageData + from_y*m_img->widthStep,
         bmi, DIB_RGB_COLORS );
   }
}

// Resize the image while maintaining aspect ratio
void CvvImage::ResizeImage(IplImage* srcimg, RECT* resizerect)
{
    CRect m_rect;
    m_rect.CopyRect(resizerect);

    float cw = (float)m_rect.Width();  // Client area width
    float ch = (float)m_rect.Height(); // Client area height
    float pw = (float)srcimg->width;   // Original image width
    float ph = (float)srcimg->height;  // Original image height

    float cs = cw / ch;  // Aspect ratio of client area
    float ps = pw / ph;  // Aspect ratio of the image

    // Determine scaling factor based on aspect ratio
    float scale = (cs > ps) ? (ch / ph) : (cw / pw);

    int rw = (int)(pw * scale); // Scaled width
    int rh = (int)(ph * scale); // Scaled height

    // Create a new resized image
    m_resimage = cvCreateImage(cvSize(rw, rh), srcimg->depth, srcimg->nChannels);
    cvResize(srcimg, m_resimage);
}


// Draw the image on the given device context (HDC) with resizing
void CvvImage::DrawToHDC(HDC hDCDst, RECT* pDstRect)
{
    if (pDstRect && m_img && m_img->depth == IPL_DEPTH_8U && m_img->imageData)
    {
        // Resize the image based on destination rectangle
        ResizeImage(m_img, pDstRect);

        uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
        BITMAPINFO* bmi = (BITMAPINFO*)buffer;

        int bmp_w = m_resimage->width, bmp_h = m_resimage->height;
        CvRect roi = cvGetImageROI(m_resimage);
        CvRect dst = RectToCvRect(*pDstRect);

        // Calculate centering offsets to maintain aspect ratio
        int add_w = (int)((dst.width - roi.width) / 2);
        int add_h = (int)((dst.height - roi.height) / 2);

        if (roi.width == dst.width && roi.height == dst.height)
        {
            Show(hDCDst, dst.x + add_w, dst.y + add_h, bmp_w, bmp_h, roi.x, roi.y);
            return;
        }

        // Choose appropriate image scaling mode
        if (roi.width > bmp_w)
        {
            SetStretchBltMode(hDCDst, HALFTONE); // High-quality scaling
        }
        else
        {
            SetStretchBltMode(hDCDst, COLORONCOLOR); // Faster scaling
        }

        FillBitmapInfo(bmi, bmp_w, bmp_h, Bpp(), m_resimage->origin);

        // Stretch and display the resized image
        ::StretchDIBits(
            hDCDst,
            dst.x + add_w, dst.y + add_h, bmp_w, bmp_h,
            roi.x, roi.y, roi.width, roi.height,
            m_resimage->imageData, bmi, DIB_RGB_COLORS, SRCCOPY);
    }
}

// Fill the entire image with a specified color
void CvvImage::Fill(int color)
{
    cvSet(m_img, cvScalar(
        color & 255,         // Blue channel
        (color >> 8) & 255,  // Green channel
        (color >> 16) & 255, // Red channel
        (color >> 24) & 255  // Alpha channel (if applicable)
    ));
}