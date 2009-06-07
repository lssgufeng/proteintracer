/*==============================================================================
Copyright (c) 2009, André Homeyer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/ 

#include <gui/ImageViewer.h>

#include <math.h>

#include <FL/Fl.H>
#include <FL/gl.h>

using PT::ImageIndex;
using PT::ImageOffset;
using PT::ImageSize;
using PT::RGBImage;

ImageViewer::ImageViewer(int x, int y, int width, int height) : Fl_Gl_Window(x, y, width, height), image_(0)
{
    setImage(NULL);
}

void ImageViewer::setFocus(int focusX, int focusY)
{
    assert((image_.IsNotNull()) || (focusX < 0 && focusY < 0));

    this->focus_[0] = focusX;
    this->focus_[1] = focusY;
    this->redraw();
}

void ImageViewer::setZoom(ImageViewerZoom zoom)
{
    this->zoom_ = zoom;
    this->redraw();

    ImageViewerEvent event;
    event.id = ImageViewerEvent::ZOOM_CHANGE;
    event.zoom = zoom;
    notifyEventHandler(event); 
}

ImageViewerZoom ImageViewer::getZoom() const
{
    return this->zoom_;
}

void ImageViewer::setMousePosition(int x, int y)
{
    this->mousePos_[0] = x;
    this->mousePos_[1] = y;

    ImageViewerEvent event;
    event.id = ImageViewerEvent::MOUSE_MOVE;
    event.location[0] = x;
    event.location[1] = y;
    notifyEventHandler(event);
}

const ImageIndex& ImageViewer::getMousePosition() const
{
    return this->mousePos_;
}

ImageViewerZoom findClosestZoomFactor(int zoomGuess)
{
    static const ImageViewerZoom zooms[] = { ZOOM_25, ZOOM_50, ZOOM_100,
        ZOOM_200, ZOOM_400, ZOOM_800, ZOOM_1600, ZOOM_3200 };
    int minDist = std::numeric_limits<int>::max();
    ImageViewerZoom zoom = ZOOM_100;
    for (int i = 0; i < 8; ++i)
    {
        int dist = abs(zoomGuess - zooms[i]); 
        if (dist < minDist)
        {
            minDist = dist;
            zoom = zooms[i];
        }
    }
    return zoom;
}

void ImageViewer::setImage(RGBImage::ConstPointer image)
{
    if (image.IsNotNull())
    {
        ImageSize newSize = image->GetLargestPossibleRegion().GetSize();

        // calculate new focus
        ImageIndex newFocus;
        {
            newFocus[0] = newSize[0] / 2;
            newFocus[1] = newFocus[1] / 2;
        }

        // calculate new zoom
        ImageViewerZoom newZoom;
        {
            int newZoomGuessX = (int)((w() / (double) newSize[0]) * 100.0);
            int newZoomGuessY = (int)((h() / (double) newSize[1]) * 100.0);
            int newZoomGuess = (newZoomGuessX < newZoomGuessY) ? newZoomGuessX : newZoomGuessY;
            newZoom = findClosestZoomFactor(newZoomGuess);
        }

        image_ = image;

        imageSize_  = newSize;
        setZoom(newZoom);
        setFocus(imageSize_[0] / 2, imageSize_[1] / 2);
        setMousePosition(-1, -1);
    }
    else
    {
        image_ = NULL;

        setZoom(ZOOM_100);
        setFocus(-1, -1);
        setMousePosition(-1, -1);
    }

    redraw();
}

void ImageViewer::setImageAndRetainFocus(PT::RGBImage::ConstPointer image)
{
    if (image_.IsNotNull() && image.IsNotNull())
    {
        // image_->GetLargestPossibleRegion().GetSize() cannot be used
        // since image may point to the same instance as image_
        ImageSize oldSize = imageSize_;
        ImageSize newSize = image->GetLargestPossibleRegion().GetSize();

        // calculate new focus
        ImageIndex newFocus;
        {
            double relFocusX = focus_[0] / (double)oldSize[0];
            double relFocusY = focus_[1] / (double)oldSize[1];
			// floor(x + 0.5) is a workaround for round() 
			// since round() does not work with Visual C++
            newFocus[0] = (ImageIndex::IndexValueType) floor(relFocusX * newSize[0] + 0.5);
            newFocus[1] = (ImageIndex::IndexValueType) floor(relFocusY * newSize[1] + 0.5);
        }

        // calculate new zoom
        ImageViewerZoom newZoom = ZOOM_100;
        {
            int newZoomGuess = (int)((oldSize[0] / (double)newSize[0]) * zoom_);
            newZoom = findClosestZoomFactor(newZoomGuess);
        }

        image_ = image;

        imageSize_ = newSize; 
        setZoom(newZoom);
        setFocus(newFocus[0], newFocus[1]);
        setMousePosition(-1, -1);

        redraw();
    }
    else
    {
        setImage(image);
    }
}

RGBImage::ConstPointer ImageViewer::getImage()
{
    return image_;
}

int ImageViewer::handle(int event)
{
    switch(event)
    {
    case FL_PUSH:
        return mouseClicked();
    case FL_RELEASE:
        return mouseReleased();
    case FL_DRAG:
        return mouseDragged();
    case FL_MOUSEWHEEL:
        return mouseWheel();
    case FL_MOVE:
        return mouseMoved();
    case FL_ENTER:
        return 1;
    case FL_LEAVE:
        return mouseLeft();
    case FL_KEYDOWN:
        return keyDown();
    default:
        return Fl_Gl_Window::handle(event);
    }
}

int ImageViewer::mouseClicked()
{
    Fl::focus(this);

    if ((Fl::event_state() & FL_CTRL) != 0)
    {
        if (image_.IsNotNull())
        {
            int x = Fl::event_x();
            int y = Fl::event_y();
            this->screenToImageCoordinates(x, y);

            int eventButton = Fl::event_button();
            if (eventButton == 1)
            {
                increaseZoom();
            }
            else if (eventButton == 3)
            {
                decreaseZoom();
            }

            this->setFocus(x, y);
        }
    }
    else
    {
        if (image_.IsNotNull())
        {
            clickPos_[0] = Fl::event_x();
            clickPos_[1] = Fl::event_y();

            oldFocus_ = focus_;

            double zoomFactor = this->zoom_ / 100;
            correctBlindMargin(oldFocus_);
        }
    }

    return 1;
}

void ImageViewer::correctBlindMargin(ImageIndex &focus)
{
    double zoomFactor = 100.0 / this->zoom_;
    int blindMarginX = (int)((this->w() / 2) * zoomFactor);
    int blindMarginY = (int)((this->h() / 2) * zoomFactor);

    ImageSize size = image_->GetLargestPossibleRegion().GetSize();
    int width = size[0];
    int height = size[1];
    if (focus[0] < blindMarginX) focus[0] = blindMarginX;
    else if (focus[0] > (width - blindMarginX)) focus[0] = width - blindMarginX;
    if (focus[1] < blindMarginY) focus[1] = blindMarginY;
    else if (focus[1] > (height - blindMarginY)) focus[1] = height - blindMarginY;
}

int ImageViewer::mouseReleased()
{
    if (Fl::event_button() == 1)
    {

        int x = Fl::event_x();
        int y = Fl::event_y();

        if (x == this->clickPos_[0] && y == this->clickPos_[1])
        {
            ImageViewerEvent event;
            event.id = ImageViewerEvent::MOUSE_CLICK;
            screenToImageCoordinates(x, y);
            event.location[0] = x;
            event.location[1] = y;
            notifyEventHandler(event);
        }
    }

    return 1;
}

int ImageViewer::mouseDragged()
{
    if (image_.IsNotNull())
    {
        ImageIndex dragPos = {{ Fl::event_x(), Fl::event_y() }};

        ImageOffset dist = clickPos_ - dragPos;
        double factor = 100 / (double)zoom_;
        dist[0] = (ImageOffset::OffsetValueType)(dist[0] * factor);
        dist[1] = (ImageOffset::OffsetValueType)(dist[1] * factor);
        focus_ = oldFocus_ + dist;

        ImageSize size = image_->GetLargestPossibleRegion().GetSize();
        int imageWidth = size[0];
        int imageHeight = size[1];
        if (focus_[0] < 0) focus_[0] = 0;
        else if (focus_[0] > imageWidth) focus_[0] = imageWidth -1;
        if (focus_[1] < 0) focus_[1] = 0;
        else if (focus_[1] > imageHeight) focus_[1] = imageHeight -1;

        this->redraw();
    }

    return 1;
}

int ImageViewer::mouseWheel()
{
    if (this->image_.IsNotNull())
    {
        int dy = Fl::event_dy();
        if (dy < 0)
        {
            increaseZoom();
        }
        else
        {
            decreaseZoom();
        }
    }

    return 1;
}

int ImageViewer::mouseMoved()
{
    if (image_.IsNotNull())
    {
        int x = Fl::event_x();
        int y = Fl::event_y();

        screenToImageCoordinates(x, y);

        setMousePosition(x, y);

        return 1;
    }

    return 0;
}

int ImageViewer::mouseLeft()
{
    setMousePosition(-1, -1);

    return 1;
}

int ImageViewer::keyDown()
{
    if (image_.IsNotNull())
    {
		const char *text = Fl::event_text();
        int key = Fl::event_key();
		
		assert(text != NULL);

        // plus and minus have to be tested by character since event_key()
        // returns different values on windows and linux

        if (text[0] == '+')
        {
            increaseZoom();
        }
        else if (text[0] == '-')
        {
            decreaseZoom();
        }
        else if (key == FL_Left)
        {
            this->cursorShift(-1, 0);
        }
        else if (key == FL_Right)
        {
            this->cursorShift(1, 0);
        }
        else if (key == FL_Up)
        {
            this->cursorShift(0, -1);
        }
        else if (key == FL_Down)
        {
            this->cursorShift(0, 1);
        }
        else return 0;

        return 1;
    }
    else return 0;
}

void ImageViewer::cursorShift(int xFactor, int yFactor)
{
    double zoomFactor = 100.0 / this->zoom_;
    int dx = xFactor * (int)(CURSOR_SHIFT_LENGTH * zoomFactor);
    int dy = yFactor * (int)(CURSOR_SHIFT_LENGTH * zoomFactor);

    this->focus_[0] += dx;
    this->focus_[1] += dy;
    correctBlindMargin(focus_);

    this->redraw();
}

void ImageViewer::increaseZoom()
{
    assert(image_.IsNotNull());

    ImageViewerZoom newZoom;

    switch(this->zoom_)
    {
    case ZOOM_25:
        newZoom = ZOOM_50;
        break;
    case ZOOM_50:
        newZoom = ZOOM_100;
        break;
    case ZOOM_100:
        newZoom = ZOOM_200;
        break;
    case ZOOM_200:
        newZoom = ZOOM_400;
        break;
    case ZOOM_400:
        newZoom = ZOOM_800;
        break;
    case ZOOM_800:
        newZoom = ZOOM_1600;
        break;
    case ZOOM_1600:
        newZoom = ZOOM_3200;
        break;
    default:
        newZoom = this->zoom_;
        break;
    }

    setZoom(newZoom);
}

void ImageViewer::decreaseZoom()
{
    assert(image_.IsNotNull());

    ImageViewerZoom newZoom;

    switch(this->zoom_)
    {
    case ZOOM_50:
        newZoom = ZOOM_25;
        break;
    case ZOOM_100:
        newZoom = ZOOM_50;
        break;
    case ZOOM_200:
        newZoom = ZOOM_100;
        break;
    case ZOOM_400:
        newZoom = ZOOM_200;
        break;
    case ZOOM_800:
        newZoom = ZOOM_400;
        break;
    case ZOOM_1600:
        newZoom = ZOOM_800;
        break;
    case ZOOM_3200:
        newZoom = ZOOM_1600;
        break;
    default:
        newZoom = this->zoom_;
        break;
    }

    setZoom(newZoom);
}

void ImageViewer::screenToImageCoordinates(int &x, int &y)
{
    ImageSize size = image_->GetLargestPossibleRegion().GetSize();
    int imageWidth = size[0];
    int imageHeight = size[1];

    double zoomFactor = 100.0 / this->zoom_;
    x = (int)((x - this->imageOffset_[0]) * zoomFactor);
    y = h() - y;
    y = (int)((y - this->imageOffset_[1]) * zoomFactor);

    if (x < 0 || x >= imageWidth || y < 0 || y >= imageHeight)
    {
        x = -1;
        y = -1;
    }
}

void ImageViewer::resize(int x, int y, int w, int h)
{
    Fl_Gl_Window::resize(x, y, w, h);
}

void ImageViewer::draw()
{
    if (!valid())
    {
        // make background grey
        glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
        glShadeModel(GL_FLAT);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glMatrixMode(GL_PROJECTION);
        ortho();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    // clear and fill window black
    glClear(GL_COLOR_BUFFER_BIT);

    if (image_.IsNotNull())
    {
        double zoomFactor = this->zoom_ / 100.0;
        int widgetWidth = this->w();
        int widgetHeight = this->h();
        int subimageWidth = (int)ceil(widgetWidth / zoomFactor);
        int subimageHeight = (int)ceil(widgetHeight / zoomFactor);
        ImageSize size = image_->GetLargestPossibleRegion().GetSize();
        int imageWidth = size[0];
        int imageHeight = size[1];

        int rasterX;
        int rasterY;
        int subimageX;
        int subimageY;

        if (imageWidth < subimageWidth)
        {
            subimageX = 0;
            subimageWidth = imageWidth;
            rasterX = (int)((widgetWidth  - (imageWidth * zoomFactor)) / 2);
            this->imageOffset_[0] = rasterX;
        }
        else
        {
            subimageX = this->focus_[0] - (subimageWidth / 2);
            if (subimageX < 0)
            {
                subimageX = 0;
            }
            else if (subimageX > (imageWidth - subimageWidth))
            {
                subimageX = imageWidth - subimageWidth;
            }
            rasterX = 0;
            this->imageOffset_[0] = (int)(-subimageX * zoomFactor);
        }

        if (imageHeight < subimageHeight)
        {
            subimageY = 0;
            subimageHeight = imageHeight;
            rasterY = (int)((widgetHeight  - (imageHeight * zoomFactor)) / 2);
            this->imageOffset_[1] = rasterY;
        }
        else
        {
            subimageY = (imageHeight - this->focus_[1]) - (subimageHeight / 2);
            if (subimageY < 0)
            {
                subimageY = 0;
            }
            else if (subimageY > (imageHeight - subimageHeight))
            {
                subimageY = imageHeight - subimageHeight;
            }
            rasterY = 0;
            this->imageOffset_[1] = (int)(-subimageY * zoomFactor);
        }

        {
            // assert that the computation of the subimage region was corrent
            assert(subimageX >= 0);
            assert(subimageX + subimageWidth <= imageWidth);
            assert(subimageY >= 0);
            assert(subimageY + subimageHeight <= imageHeight);
        }

        glPixelStorei(GL_UNPACK_ROW_LENGTH, imageWidth);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, subimageX);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, subimageY);

        glRasterPos2i(rasterX, rasterY);
        glPixelZoom(zoomFactor, zoomFactor);

        const ulong* data = (const ulong*) image_->GetBufferPointer();
        glDrawPixels(subimageWidth, subimageHeight, GL_RGB, GL_UNSIGNED_BYTE, data);

    }
}
