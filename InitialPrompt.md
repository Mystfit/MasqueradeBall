I would like to create a platformer game in C++. 

This game will play in a terminal window and will use the FTXUI library from https://github.com/ArthurSonzogni/FTXUI as the rendering layer. The game will be compiled using C++20 and support modules. I would also like to use the box2D library to handle collision, rigid body simulation and pseudo-softbody simulation. The game will support pipe commands to accept text from an input stream that will be used to generate levels to play on in the spirit of games like the classic PS1 game Vib-Ribbon which used CD audio to generate levels. Input will incorporate either controller support using libgamepad https://github.com/univrsal/libgamepad or regular keyboard support, contextually depending on if a button is pressed. Music will handled by libmikmod and audio by STK - https://github.com/thestk/stk 

We will use CMAKE to create the project and incorporate dependencies.

The game will consist of two screens initially, start menu, and game.

The following describes the initial layout of the game.


START MENU
===========

Style
-----
Draw a window that will fill the terminal with a border around the outside. To start, we require a start menu that will display the title of title of the game "Masquerade Ball". Underneath the game, the intro menu will include the following selectable options:

- Start
- Options

These options will be highlightable using either the up and down arrow keys, or gamepad up and down on the left thumbstick or dpad. To select an option, enter on the keyboard is used, or the A button (from an XBox Controller but relative to whatever gamepad active).

If start is selected, the screen will transition to the main game window. If options is selected, an Option menu will appear containing placeholder Option 1, Option 2, Option 3, entries and a back button to close the options window.


GAME
====

Goal
----
Control a ball along a randomly generated path as long as possible to achieve the most points. Points are allocated based on distance travelled along a randomly generated level and is measured in characters. The faster you travel will set a speed multiplier that will increase your character score for each character earned. The win condition is to get the highest score by the end of the level.


Gameplay
--------

The game is a side scrolling physics platformer. The ball is constructed using Box2d distance constraints between circles to make a rim and a central circle to act as a core support in order to create a flexible and bouncy surface. When the ball moved forwards or backwards, a rotational force will be applied that will spin the ball to make it move in a direction. Pressing the jump button briefly will result in a small jump using a physics impulse. If the jump button is held, the ball will slow its horizontal movement and "compress" downwards and when released will jump higher due to the compression with a larger physics impulse. 

Visuals
-------
The game window will be drawn using the canvas object from FTXUI. The ball outline will be drawn using a cubic spline around the outside of the outer circles using the braille rendering style to draw lines between pairs of points on the spline.The level curve will be drawn using the characters that are getting piped in from STDIN.

The game will use parallax to draw additional graphics in the background. Any characters or items in foreground layers will occlude background layers, unless the background element can be expressed as a highlight instead of replacing the character or drawn element. The layers will include:

- Foreground layer: visual only elements in front of the level and player ball. These will be thin line elements that will represent "speed lines" and will grow in number and length depending on the speed the ball travels through the level. 
- Game layer: interactable elements like the player ball and level.
- Background layer 1: visual only parallax BG elements that move slower than the main layer. This will be drawn as mid-grey blocks and will also be a random curve like the foreground layer, but will vary in height less (since they are further away).
- Background layer 2: visual only parallax BG elements that move much slower than the main layer. To start, these will be drawn as mid-blue characters representing "stars" a flickering effect could be added by changing between different asterix styled characters to create the illusion of a flickering atmospheric star effect.

Input
-----
The player will control the ball using either the arrow keys, or the left thumbstick to move, and the spacebar or A controller button to jump. The escape keyboard button or the gamepad start button will open a menu to quit the game,

Level
-----
The level will be created from input that is piped into the program using STDIN. If nothing is piped in then lorem ipsum will be used. The ground will be formed from a curve that will be formed from each line of text that is fed into STDIN. Each new line of text will create a gap that will need to be jumped over, otherwise if the ball falls through a gap, then the game will stop, and the message GAME OVER will be printed centrally over the game level. Pressing the enter key will restart the game from the last line reached in the buffer and will regenerate the level curve. The curve will be formed from a cubic spline and will be randomly generated from horizontally spaced points that will be placed randomly vertically higher from the previous point based on a difficulty number that will increase the longer the game is played. The end of the level will be reached when STDIN ends when an EOF character is received or the pipe is closed and will create a "goal" represented by a two vertical lines drawn at the end of the level drawn using block characters. Crossing the goal ends the level and displays the final point total.