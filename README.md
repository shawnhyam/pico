# pico
Raspberry Pico code.

## ssd1306

The beginnings of a library to drive a 128x32 SSD1306 OLED display.

## ili9341

Simple code to drive a 320x240 ILI9341 LCD display; a text graphics mode, called `mode0` that is used in a basic text editor; and the beginnings of a sprite graphics mode called `mode1` that supports slightly-better-than-NES graphics and horizontal scrolling.

### Mode 1

The general idea of Mode 1 is:
* 320x240 pixel resolution, 16-bit color (RGB565)
* single background layer composed of a grid of Tiles
* foreground layer composed of Sprites

Most of the Background code is quickly cobbled together and only half working, but the basics are:
* can define up to 256 Tiles
* each Tile is 8x8 pixels with up to 8 colors
* can define 8 Tile palettes of 8 colors (color 0 is the screen background color)
* there is a "Map" of X * 30 tile and palette ids
* can specify a horizontal scroll offset; no vertical scrolling
* can specify a global background color (so really they are 7-color palettes)

Sprites aren't started yet, but:
* can define up to 256 Sprites
* each Sprite is 8x8 pixels with up to 8 colors
* can define 8 Sprite palettes of 8 colors (color 0 is transparent)
* each active Sprite:
  * sprite-index: 8 bits
  * x-location: 9 bits
  * y-location: 8 bits
  * orientation: 3 bits
  * palette-index: 3 bits
  * (further ahead) layer: 1 bit

## kilo

A quick port of the Kilo text editor to the Pico. This version is built for a 320x240 ILI9341-based LCD instead of the Unix terminal. The Kilo editor is featured here:
* https://github.com/antirez/kilo
* https://viewsourcecode.org/snaptoken/kilo/

