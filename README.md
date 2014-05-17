LegoMosaic
==========

About
-----

This small program converts a given PNG image file into a Lego Mosaic: it takes the source
image, converts each non-alpha pixel to a close-matching brick color, then attempts to draw
the image using only Lego bricks. The approach is to use A\* filling, where the algorithm
attempts to place all next valid bricks, but only places the most optimal one at the current
breadth. This continues until all pixels are covered.

The image can be any PNG file, though the pixels must have full alpha (e.g. fully visible
without any transparency) to be matched with a Lego piece. Any alpha'ed-out pixels are simply
ignored. Bricks and brick-colors are defined in a text file that is passed to this application.

The final output is a series of images showing the progress of how the Png image was filled,
as well as a list of bricks to buy on the Lego "[Pick a Brick](http://shop.lego.com/en-US/Pick-A-Brick-ByTheme)" store.

The code runs quickly (benefit of A\* vs. exhaustive search) and uses little run-time memory.
It converted a video game logo that had a resolution of 400 × 173 pixels in 120 seconds using ~10 MB.
The result was a list of 5,170 Lego pieces (covering 21,061 pixels), costing $965.80 in parts (bulk discount not applied).
In comparison, the "LEGO Star Wars Ultimate Collector's Millennium Falcon" has 5,922 Lego pieces costing ~$500 USD.

*The following images represent given input, Mosaic conversion, and the result of the A\* Lego fill algorithm:*

![Input; Samus from Super Metroid](https://raw.githubusercontent.com/nint22/LegoMosaic/master/Samus.png)
![Mosaic Conversion; Colors now match Lego colors](https://raw.githubusercontent.com/nint22/LegoMosaic/master/LegoMosaicProgress_Output.png)
![Lego Conversion; Samus now built out of Lego plates / bricks](https://raw.githubusercontent.com/nint22/LegoMosaic/master/LegoMosaicProgress_00361.png)

Video link of converting a Mario / Bowser image: [YouTube Link](https://www.youtube.com/watch?v=ECu_di70cPY).

Here's a great example of sample output; this comes from converting the SNES front-facing Samus sprint from Super Metroid:

	Color "Black" has 114 parts:
		51 needed for part #0 ( 1 x 1, 10 cents per unit )
		7 needed for part #1 ( 1 x 2, 10 cents per unit )
		13 needed for part #2 ( 2 x 1, 10 cents per unit )
		18 needed for part #3 ( 1 x 3, 10 cents per unit )
		16 needed for part #4 ( 3 x 1, 10 cents per unit )
		1 needed for part #9 ( 1 x 8, 25 cents per unit )
		2 needed for part #10 ( 8 x 1, 25 cents per unit )
		2 needed for part #17 ( 2 x 2, 15 cents per unit )
		2 needed for part #20 ( 2 x 4, 20 cents per unit )
		2 needed for part #22 ( 2 x 6, 25 cents per unit )
	Color "Blue" is unused
	Color "Green" has 116 parts:
		47 needed for part #0 ( 1 x 1, 10 cents per unit )
		25 needed for part #1 ( 1 x 2, 10 cents per unit )
		6 needed for part #2 ( 2 x 1, 10 cents per unit )
		27 needed for part #3 ( 1 x 3, 10 cents per unit )
		11 needed for part #4 ( 3 x 1, 10 cents per unit )
	Color "Gray" has 9 parts:
		8 needed for part #0 ( 1 x 1, 10 cents per unit )
		1 needed for part #3 ( 1 x 3, 10 cents per unit )
	Color "Orange-Brown" has 69 parts:
		37 needed for part #0 ( 1 x 1, 10 cents per unit )
		15 needed for part #1 ( 1 x 2, 10 cents per unit )
		2 needed for part #2 ( 2 x 1, 10 cents per unit )
		11 needed for part #3 ( 1 x 3, 10 cents per unit )
		4 needed for part #4 ( 3 x 1, 10 cents per unit )
	Color "Purple" is unused
	Color "Red" has 29 parts:
		11 needed for part #0 ( 1 x 1, 10 cents per unit )
		2 needed for part #1 ( 1 x 2, 10 cents per unit )
		2 needed for part #2 ( 2 x 1, 10 cents per unit )
		7 needed for part #3 ( 1 x 3, 10 cents per unit )
		3 needed for part #4 ( 3 x 1, 10 cents per unit )
		2 needed for part #10 ( 8 x 1, 25 cents per unit )
		2 needed for part #21 ( 4 x 2, 20 cents per unit )
	Color "White" has 1 parts:
		1 needed for part #0 ( 1 x 1, 10 cents per unit )
	Color "Yellow" has 23 parts:
		16 needed for part #0 ( 1 x 1, 10 cents per unit )
		2 needed for part #1 ( 1 x 2, 10 cents per unit )
		3 needed for part #2 ( 2 x 1, 10 cents per unit )
		2 needed for part #3 ( 1 x 3, 10 cents per unit )
	> Total bricks: 361
	> Total cost: $37.65

Usage
-----

The program is executed with the following command-line arguments:

    ./LegoBitmap <Brick Definitions Text File> <PNG Image to Convert>

For example, you can test the application by running through a very simple image, like the
cursive "Hello" png file with the default brick definitions file:

    ./LegoBitmap BrickDefinitions.txt HelloMac.png

The default "BrickDefinitions.txt" file defines 12 colors, picked from the "Pick-a-Brick"
Lego store [online here](http://shop.lego.com/en-US/Pick-A-Brick-ByTheme). It also defines 18
bricks, ranging from single 1x1 pegs to the 8x1 tall / wide brick, and the classic 2x4 brick.

To define your own BrickDefinitions.txt file, follow this format: a file must start with a positive
integer representing the number of colors, C. On following C-number of lines, the color needs
an ASCII non-spaced name, followed by the RGB values in normalized byte value (e.g. 0 - 255, inclusive).
This list is then followed by the number of bricks you want to define, B. On the following B-number
of lines, a brick is defined as three space-delimited positive integers: width, height, and cost (in pennies).

Software Design
---------------

This software has a very simple high-level architecture since it's a straight-forward toy project.
There are three main classes, with two supporting files:

+ "LegoBitmap.h/cpp" loads a given PNG file (the term "Bitmap" is interchangeable with "Image", but does
  not represents the "*.BMP" file format). Once loaded, you can convert it to Lego-matched
  colors by calling the "ConvertMosaic(...)" function.
+ "LegoSet.h/cpp" defines common structures and a list of bricks mapped to a "LegoBitmap" board. It handles
  the validation logic when adding new bricks, as well as gives speed-ups for looking for next placement
  positions or cost data. This class is loosely coupled with LegoBitmap, and only refers to that
  data-structure when adding new bricks.
+ "LegoMosaic.h/cpp" is the single high-level manager that executes the A\* search algorithm over the
  given image (loaded as a "LegoBitmap" instance) producing possible solutions (instances of "LegoSet").

The supporting code includes a "Vec2.h" class, which is a simple integer tuple (useful for position
and size data), and a "main.cpp" source file, where the application parses input and instantiates the
main class "legoMosaic".

The A\* search implementation is as follows: given a brick-colored image, find pixels that have
yet to be covered by a brick, and are directly adjacent to other pixels that have been covered by bricks
or are invalid pixels (alpha'ed out). Iterate over this list, placing all possible combinations of
Lego bricks with the appropriate color. Compute the rank, which is Cost (in pennies) divided by coverage
count (number of pegs). Sort these solutions based on rank, saving the best value. Repeat the algorithm,
saving these optimal breadth-search'ed bricks. The flaw with A\* is that it picks locally optimal results,
not globally optimal results. This is acceptable, since locally optimal still produces good coverage (e.g.
it places the best brick for most of the time).

Todo
----

+ Finish the brute-force search feature; good to compare the A* results against
+ Randomize the brick definition ID choice in the loop (so there isn't a preference of tall vs. wide
+ Instead of defining both a width and length piece, let the program rotate the plates
+ Support complex shapes that aren't just a width x length, like the "L" brick ("Corner Plate")
+ Allow brick / color relationship definitions: some bricks shapes only come in certain colors!
+ Add support back Windows & Linux (others are welcomed to submit patches for this)

License
=======

Lego Fair-Play Usage
--------------------

LEGO is a trademark of the LEGO Group of companies which does not sponsor, authorize
or endorse this site. Learn more here: http://aboutus.lego.com/en-us/legal-notice/fair-play

License (MIT License)
---------------------

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

