#pragma once
#ifndef CVVIMAGE_CLASS_DEF
#define CVVIMAGE_CLASS_DEF

// opencv341
#include <opencv/highgui.h>
#include <core/core.hpp>
#include <highgui/highgui.hpp>
#include <imgproc/imgproc.hpp> //Header file needed to add for use the cvResize() function

#ifdef _DEBUG
#pragma comment(lib,"opencv_world341d.lib") //In Debug Mode
#else
#pragma comment(lib,"opencv_world341.lib") //In Release Mode
#endif

using namespace cv;
using namespace std;

/* CvvImage class definition */
class  CvvImage
{
public:

    //Constructor
   CvvImage();
   
   //Destructor
   virtual ~CvvImage();

   /* Create image (BGR or grayscale) */
   virtual bool  Create( int width, int height, int bits_per_pixel, int image_origin = 0 );
   //virtual bool  Create (Dimensions, bit depth, origin)

   /* Load image from specified file */
   virtual bool  Load( const char* filename, int desired_color = 1 );

   /* Load rectangle region from the file */
   virtual bool  LoadRect( const char* filename, int desired_color, CvRect r );

//Only compiled and included when the code is built for Windows Platform
#if defined WIN32 || defined _WIN32

   //Convert RECT from Windows into OpenCV's cvRect, then calls LoadRect 
   //function with CvRect to handle the actual operation
   virtual bool  LoadRect( const char* filename, int desired_color, RECT r )
   {
      return LoadRect( filename, desired_color,
         cvRect( r.left, r.top, r.right - r.left, r.bottom - r.top ));
   }

#endif

   /* Save entire image to specified file. */
   virtual bool  Save( const char* filename );

   /* Get copy of input image Region of Intrest (ROI) 
   
   Note for desired_color:

   -1: Default value, keeps the color format of the source image.
   0: Converts into Grayscale
   1: Ensures the image remains in color format.
   
   */

   //Copying from Another CvvImage
   virtual void  CopyOf( CvvImage& image, int desired_color = -1 );
   //virtual void CopyOf (Reference to the source, Color Format of the copied image
   
   //Copying from an IpImage
   virtual void  CopyOf( IplImage* img, int desired_color = -1 );

   //Copying and Resizing from an IpImage
   virtual void  CopyOf( IplImage* img, int width, int height, int desired_color = -1); // opencv341 cvGetSize exception

   //Returns the internal IplImage* 
   IplImage* GetImage() { return m_img; };

   //Release memory allocated for the internal IpImage
   virtual void  Destroy(void);

   /* width and height of ROI */

   //Returns the width of image or ROI
   int Width() { return !m_img ? 0 : !m_img->roi ? m_img->width : m_img->roi->width; };
   //int Width() {return 0 if is nullptr : return full image width if no ROI set : return width of ROI if ROI set;}

   //Returns the height of image or ROI
   int Height() { return !m_img ? 0 : !m_img->roi ? m_img->height : m_img->roi->height;};

   //Returns the bits per pixel (BPP) of the image.
   int Bpp() { return m_img ? (m_img->depth & 255)*m_img->nChannels : 0; };

   //Fills the image with a specified colour
   virtual void  Fill( int color );

   //Displays the image in a HighGUI OpenCV window.
   virtual void  Show( const char* window );

#if defined WIN32 || defined _WIN32

   /* draw part of image to the specified DC at the specific position */
   virtual void  Show( HDC dc, int x, int y, int width, int height, int from_x = 0, int from_y = 0 );

   //Resizes the given IplImage to fit within the specified Windows RECT region.
   virtual void   ResizeImage(IplImage *srcimg,RECT* resizerect);

   //Draws the current image (or ROI) into a specific Windows DC at the specified destination rectangle.
   virtual void  DrawToHDC( HDC hDCDst, RECT* pDstRect );

#endif

protected:

   IplImage*  m_img; //Store the main image
   IplImage* m_resimage; //Store a resized version of main image, for display in DC

};

typedef CvvImage CImage;
#endif