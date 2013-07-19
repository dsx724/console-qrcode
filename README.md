[blog](http://www.xuetech.com/2013/07/console-qrcode.html)
------

console-qrcode
==============
Simple C program that draws a QR code on the linux console via modifying the framebuffer.
The QR code will only show up on the host console and not on X or over SSH.  See [example.png](https://raw.github.com/dsx724/console-qrcode/master/example.png).
The QR code is generated via [libqrencode](https://github.com/fukuchi/libqrencode). [Apache 2.0 License](https://raw.github.com/dsx724/console-qrcode/master/LICENSE).
compile
=======
```bash
sudo apt-get install libqrencode-dev
gcc console-qrcode.c -o console-qrcode
```
usage
=====
```bash
sudo apt-get install libqrencode3
sudo ./console-qrcode [OPTION]... [INPUT]
echo [INPUT] | sudo ./console-qrcode [OPTION]...
```
option
------
<pre>
-p --module_pixels		size of module in pixels: 1-9
-w --border_width_modules	width of the border in modules: 0-9
-h --border_height_modules	height of the border in modules: 0-9
-x --x_offset			horizontal offset from top right in pixels
-y --y_offset			vertical offset from top right in pixels
-v --verbose
</pre>
