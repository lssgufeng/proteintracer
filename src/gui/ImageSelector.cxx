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

#include <gui/ImageSelector.h>

#include <string.h>

#include <sstream>
#include <string>

#include <FL/Fl_Box.H>

#include <gui/common.h>

using std::string;
using std::vector;

using PT::ImageKey;
using PT::ImageLocation;
using PT::ImageSeries;
using PT::IntRange;
using PT::ImageSeriesSet;

class LocationData
{
public:
    ImageLocation location;
    std::string name;

    LocationData(const ImageLocation& loc) : location(loc)
    { 
        std::stringstream nameStream;
        nameStream << "W " << location.well << " / P " << location.position << " / S " << location.slide;
        name = nameStream.str();
    }

    bool operator!=(const LocationData& m)
    {
        return location != m.location;
    }
};

ImageSelector::ImageSelector(int x, int y, int width, int height) : Fl_Group(x, y, width, height), imageSeriesSet_(NULL)
{
    // create user interface
    {
        static const int COUNTER_WIDTH = 50;
        static const int INPUT_HEIGHT = 25;
        static const int LABEL_HEIGHT = 14;
        static const int SMALL_MARGIN = 5;

        locationChoice_ = new Fl_Choice(
            x,
            y + LABEL_HEIGHT,
            width - SMALL_MARGIN - COUNTER_WIDTH,
            INPUT_HEIGHT);
        locationChoice_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
        locationChoice_->callback(fltk_member_cb<ImageSelector, &ImageSelector::handleSelection>, this);
        locationChoice_->label("Image Location");

        timeCounter_ = new Fl_Counter(
            locationChoice_->x() + locationChoice_->w() + SMALL_MARGIN,
            locationChoice_->y(),
            COUNTER_WIDTH,
            INPUT_HEIGHT);
        timeCounter_->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        timeCounter_->callback(fltk_member_cb<ImageSelector, &ImageSelector::handleSelection>, this);
        timeCounter_->label("Time");
        timeCounter_->step(1);
        timeCounter_->type(FL_SIMPLE_COUNTER);
        timeCounter_->when(FL_WHEN_RELEASE);

        this->end();
    }

    setImageSeriesSet(0);
}

void ImageSelector::setImageSeriesSet(const ImageSeriesSet *imageSeriesSet)
{
    locationDatas_.clear();

    imageSeriesSet_ = imageSeriesSet;

    if (imageSeriesSet != 0)
    {
        // fill locationDatas_
        {
            ImageSeriesSet::ImageSeriesConstIterator it = imageSeriesSet->getImageSeriesStart();
            ImageSeriesSet::ImageSeriesConstIterator end = imageSeriesSet->getImageSeriesEnd();
            for (;it != end; ++it)
            {
                const ImageSeries& imageSeries = *it;
                LocationData locationData(imageSeries.location);
                locationDatas_.push_back(locationData); 
            }
        }
        
        // create menu items
        {
			Fl_Menu_Item* menuItems = new Fl_Menu_Item[locationDatas_.size() + 1];

            for (int i = 0; i < locationDatas_.size(); ++i)
            {
                LocationData* locationData = &locationDatas_[i];
                Fl_Menu_Item* menuItem = &menuItems[i];

                menuItem->text = locationData->name.c_str();
                menuItem->shortcut_ = 0;
                menuItem->callback_ = 0;
                menuItem->user_data_ = locationData;
                menuItem->flags = FL_MENU_VALUE;
                menuItem->labeltype_ = FL_NORMAL_LABEL;
                menuItem->labelfont_ = FL_HELVETICA;
                menuItem->labelsize_ = 14;
                menuItem->labelcolor_ = FL_BLACK;
            }
            
            // add closing item
            menuItems[locationDatas_.size()].text = 0;

            locationChoice_->copy(menuItems);

			delete[] menuItems;

            // just copying the menu items does not cause a redraw
            locationChoice_->redraw();
        }
    }

    locationChanged();
}

void ImageSelector::setSelection(const ImageKey& key)
{
    LocationData locationData(key.location);
    const Fl_Menu_Item* menuItem = locationChoice_->find_item(locationData.name.c_str());

    assert(menuItem != 0);
    locationChoice_->value(menuItem);

    assert(key.time >= timeCounter_->minimum() && key.time <= timeCounter_->maximum());
    timeCounter_->value(key.time);
}

ImageKey ImageSelector::getSelection() const
{
    const Fl_Menu_Item* menuItem = locationChoice_->mvalue();
    const LocationData* locationData = (LocationData*) menuItem->user_data_;
    return ImageKey(locationData->location, (short) timeCounter_->value());
}

void ImageSelector::selectFirstImage()
{
    if (this->imageSeriesSet_ == 0)
        return;

    ImageKey selection = getSelection();
    const PT::ImageSeries& imageSeries = 
        imageSeriesSet_->getImageSeries(selection.location);
    selection.time = imageSeries.timeRange.min;
    setSelection(selection);
}

void ImageSelector::selectNextImage()
{
    if (this->imageSeriesSet_ == 0)
        return;

    ImageKey selection = getSelection();
    const PT::ImageSeries& imageSeries =
        imageSeriesSet_->getImageSeries(selection.location);
    if (selection.time < imageSeries.timeRange.max)
        selection.time++;
    setSelection(selection);
}

void ImageSelector::selectPreviousImage()
{
    if (this->imageSeriesSet_ == 0)
        return;

    ImageKey selection = getSelection();
    const PT::ImageSeries& imageSeries =
        imageSeriesSet_->getImageSeries(selection.location);
    if (selection.time > imageSeries.timeRange.min)
        selection.time--;
    setSelection(selection);
}

void ImageSelector::selectLastImage()
{
    if (this->imageSeriesSet_ == 0)
        return;

    ImageKey selection = getSelection();
    const PT::ImageSeries& imageSeries = 
        imageSeriesSet_->getImageSeries(selection.location);
    selection.time = imageSeries.timeRange.max;
    setSelection(selection);
}

void ImageSelector::handleSelection()
{
    locationChanged();

    if (imageSeriesSet_ != NULL)
    {
        ImageSelectorEvent event;
        event.id = ImageSelectorEvent::IMAGE_SELECTION;
        event.key = getSelection();
        notifyEventHandler(event);
    }
}

void ImageSelector::locationChanged()
{
    if (imageSeriesSet_ != NULL)
    {
        const Fl_Menu_Item* menuItem = locationChoice_->mvalue();
        const LocationData* locationData = (LocationData*) menuItem->user_data_;

        const ImageSeries& imageSeries = imageSeriesSet_->getImageSeries(locationData->location);
        const ImageSeries::TimeRange& timeRange = imageSeries.timeRange;

        short time = (short) timeCounter_->value();
        if (time < timeRange.min || time > timeRange.max)
            time = timeRange.min;
        timeCounter_->minimum(timeRange.min);
        timeCounter_->maximum(timeRange.max);
        timeCounter_->value(time);
        if (timeRange.min != timeRange.max)
            timeCounter_->activate();
        else
            timeCounter_->deactivate();
    }
    else
    {
        timeCounter_->deactivate();
        timeCounter_->minimum(0);
        timeCounter_->maximum(0);
        timeCounter_->value(0);
    }
}
