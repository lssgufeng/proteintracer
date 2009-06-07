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

#ifndef ImageViewer_h
#define ImageViewer_h

#include <FL/Fl_Gl_Window.H>

#include <common.h>
#include <images.h>

enum ImageViewerZoom 
{
    ZOOM_25 = 25,
    ZOOM_50 = 50,
    ZOOM_100 = 100,
    ZOOM_200 = 200,
    ZOOM_400 = 400,
    ZOOM_800 = 800,
    ZOOM_1600 = 1600,
    ZOOM_3200 = 3200
};

class ImageViewerEvent
{
public:
    enum Id {
		MOUSE_MOVE,
        MOUSE_CLICK,
        ZOOM_CHANGE
    } id;

    // location of move and click events in pixel coordinates
    PT::ImageIndex location;

    ImageViewerZoom zoom;
};

class ImageViewer : public Fl_Gl_Window, public PT::EventGenerator<ImageViewerEvent>
{
public:

    ImageViewer(int x, int y, int width, int height);

    void setFocus(int x, int y);

    void setZoom(ImageViewerZoom zoom);

    ImageViewerZoom getZoom() const;

    const PT::ImageIndex& getMousePosition() const;

    void setImage(PT::RGBImage::ConstPointer image);

    void setImageAndRetainFocus(PT::RGBImage::ConstPointer image);

    PT::RGBImage::ConstPointer getImage();

    int handle(int event);

    void resize(int x, int y, int w, int h);

    void draw();

private:

    static const int CURSOR_SHIFT_LENGTH = 50;

    PT::RGBImage::ConstPointer image_;

    ImageViewerZoom zoom_;

    /**
     * The size of the current image at the time when it was set, measured in
     * pixels. 
     */
    PT::ImageSize imageSize_;

    /** The focus_ point, i.e. the image pixel which is displayed at the center
     * of the widget (if possible) **/
    PT::ImageIndex focus_;

    /** The  distance of the lower left corner of the scaled image from the
     * lower left corner of the window. This variable is always set when the
     * image is drawn and not intended to be changed elsewhere. **/
    PT::ImageIndex imageOffset_;

    /** Stores the old focus when a drag operation is started. Used to compute
     * the new focus position. */
    PT::ImageIndex oldFocus_;

    /** Set to the click coordinate whenever the user pressed the left mouse
     * button */
    PT::ImageIndex clickPos_;

    /** The position of the mouse in image coordinates or (-1, -1) if the mouse
     * does not cover the image. */
    PT::ImageIndex mousePos_;

    /**
     * General event handler methods.
     */
    int mouseClicked();
    int mouseReleased();
    int mouseDragged();
    int mouseWheel();
    int mouseMoved();
    int mouseLeft();
    int keyDown();

    /**
    * Performs a cursor shift, i.e. it shifts the image by a certain amount
    * within the viewport.  The base amount by which the image is shifted in
    * horizontal and vertical direction is defined by the CURSOR_SHIFT_LENGTH
    * constant.  The actual amounts are multiples of this base amount as
    * determined by the xFactor and yFactor parameters.  Negative factor values
    * cause a change in direction.  This method is called by the event handlers
    * of the cursor keys with appropriate parameter values.
     */
    void cursorShift(int xFactor, int yFactor);

    /** increases the zoom while maintaining the focus point **/
    void increaseZoom();

    /** increases the zoom while maintaining the focus point **/
    void decreaseZoom();

    /** 
     * Pixels near the border of the image cannot be centered if the zoomed
     * image is larger than the viewport. Otherwise, parts of the viewport
     * would not be covered by the image. When the specified image coordinate
     * is to be centered in the viewport, this method adjusts it so that it
     * represents the outermost image pixel which can still be centered without
     * uncovering the viewport.
     * */
    void correctBlindMargin(PT::ImageIndex &focus);

    /** Converts screen coordinates to image coordinates. */
    void screenToImageCoordinates(int &x, int &y);

    void setMousePosition(int x, int y);

};

#endif
