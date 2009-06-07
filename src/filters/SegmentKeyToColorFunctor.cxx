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

#include <filters/SegmentKeyToColorFunctor.h>

namespace PT
{

SegmentKeyToColorFunctor SegmentKeyToColorFunctor::INSTANCE;

SegmentKeyToColorFunctor::SegmentKeyToColorFunctor()
{
    // initialize black
    black_[0] = 64; 
    black_[1] = 64; 
    black_[2] = 64; 

    // generate distinct colors
    std::vector<RGBPixel> colors;
    {
        for (int r = 0; r < 5; ++r)
            for (int g = 0; g < 5; ++g)
                for (int b = 0; b < 5; ++b)
                {
                    // avoid gray values and assure a minimum brightness
                    if ( ! (r == g && r == b) && (r + g + b >= 2))
                    {
                        RGBPixel color;
                        color[0] = r * 63;
                        color[1] = g * 63;
                        color[2] = b * 63;
                        colors.push_back(color); 
                    }
                }
        random_shuffle(colors.begin(), colors.end());
    }

    // fill color array
    {
        assert(colors.size() == NUM_COLORS);

        std::vector<RGBPixel>::const_iterator it = colors.begin();
        std::vector<RGBPixel>::const_iterator end = colors.end();
        for (int i = 0; it != end; ++it, ++i)
        {
            colors_[i][0][0] = (*it)[0];
            colors_[i][0][1] = (*it)[1];
            colors_[i][0][2] = (*it)[2];

            // color should not be black
            assert (! (colors_[i][0][0] == 0 && colors_[i][0][1] == 0 && colors_[i][0][2] == 0) );
            
            colors_[i][1][0] = (RGBPixel::ComponentType)((*it)[0] * 0.75);
            colors_[i][1][1] = (RGBPixel::ComponentType)((*it)[1] * 0.75);
            colors_[i][1][2] = (RGBPixel::ComponentType)((*it)[2] * 0.75);

            colors_[i][2][0] = (RGBPixel::ComponentType)((*it)[0] * 0.5);
            colors_[i][2][1] = (RGBPixel::ComponentType)((*it)[1] * 0.5);
            colors_[i][2][2] = (RGBPixel::ComponentType)((*it)[2] * 0.5);
        }
    }
}

}

