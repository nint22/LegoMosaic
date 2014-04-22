LegoMosaic
==========

Converts an input BMP image into a Lego Mosaic: it takes the source image, converts it
to a set of close-color matching bricks (based off of the brick definition file), and then
prints out the number and type of bricks, as well as cost, to form the real-world Lego
set.

This problem is solved using A* heuristic searching, where the goal is to minimize cost
and brick count.

The input image must have black as a default no-fill color, and be small, since 1 pixel is
equal to 1 peg in Lego units. The brick definition list is just a text file with integers:
the first integer represents the number of colors, which is then followed by three integers
(RGB values, 0 - 255, inclusive) for each color. After this color list, there is the brick
shape and cost list, which starts with the number of bricks. This is then followed by rows
of brick definitions, which is space-delimited integers for width, height, and cost (in cents).

Todo
====

+ Add support back for Visual Studio C++ Express for Windows
+ Start on edges that are black-non-black, rather than doing a scan-line fill

Lego Fair-Play Usage
====================

LEGO is a trademark of the LEGO Group of companies which does not sponsor, authorize
or endorse this site. Learn more here: http://aboutus.lego.com/en-us/legal-notice/fair-play

License (MIT License)
=====================

Copyright (c) 2014 CoreS2 - Core Software Solutions. All rights reserved.
Contact author Jeremy Bridon at jbridon@cores2.com for comercial use.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

