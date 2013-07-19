console-qrcode
==============

Simple C program that draws a QR code on the framebuffer.

compile
=======

sudo apt-get install libqrencode-dev

usage
=====

./console-qrcode [OPTION]... [INPUT]
echo [INPUT] | sudo ./console-qrcode [OPTION]...

-p --module_pixels 
	size of module in pixels: 1-9
-w --border_width_modules
	width of the border in modules: 0-9
-h --border_height_modules
	height of the border in modules: 0-9
-x --x_offset
	horizontal offset from top right in pixels
-y --y_offset
	vertical offset from top right in pixels
-v --verbose
