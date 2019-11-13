# Tasc
### Test Automation Script

The idea is to provide tools to automate test processes for GUI tests. For instance, to verify that after pushing a button a certain text appears on the screen. This project is created with QtCreator.

The core parts what it should be doing:
- Record screen in a selected area
- Extract images from recording
- Find images on screen using smart algorithms (e.g. pattern matching)
- Identify images based on stored image lists
- Use text recognition
- Allow to customize as much as possible in a script language
  - save images, videos in variables and load/save them
  - make calculations/decisions based on the information found in images/texts

I know that there are probably 100 tools like that out there! On the one hand, I set this challenge to myself to learn something, e.g. writing a parser, making a video out of screenshots, threading, using pattern matching algorithms to detect images and using text recognition.

On the other hand, I want to do a tool that focuses on performance critical tasks. I have tested and drawn inspiration from other software, e.g. Sikuli, which uses pattern-matching algorithms from OpenCV and I realized those operations are very costly. It is entirely possible that your task of finding a specific image on the screen can be done in 1/1000 of the time that the algorithm of matchTemplate in OpenCV uses. It is a very robust and fancy algorithm, which is fun to use, but unfortunately it is very slow and unsuited for a lot of tasks.

Furthermore, the screen recording is going to be a critical part of this, because you will be able to gather all information you need from a recording without having to start the software, you want to test, over and over again. You will also be able to verify that the script works correctly.

The script language itself is inspired by python and allows for simple calculations, loops, storing variables and it provides useful functions. The follwing example shows how to select an area on the screen, to record the screen (until a hotkey is pressed, by now it is hardcoded as Ctrl+.) and to view the recording:

```
rect=select() # Select a rectangle on the screen
window_rec=record(rect, 15) # the second parameter is the frame-rate
view(window_rec)
```

This part in itself could be used as screencast-software.

You can then extract images and sequences from the recording. For instance, call one image 'logo' and then you can initialize the saved image and locate it as follows

```
register("software.xml") # loads stored variables (images,locations,rects,etc.) in software.xml
point=find(logo)
if point is not None:
    print("Logo seen at " + str(point))
else:
    print("Logo not visible on screen")
```

This should either print out the coordinates (x, y) of the logo on the screen or that the logo is not visible. You could then use this information to click on it or to locate other buttons / images which have a fixed position relative to the logo. It is possible to always search the whole screen for an image, but be aware: Running pattern matching algorithms on the whole screen is a costly operation. If your use-case is performance critical, you should consider minimizing the usage of pattern-matching and instead teach your script to work smart.

Another thing to implement is text recognition, that can be used to do calculations in the script and make decisions based on it.

Considering the scripting language I actually contemplate to go another route entirely and to embed python instead of my own parser in the program. That would probably be smarter, since you'd have way more options with that, e.g. testing with SQL queries. Even now I have already implemented the whole toolchain of lexer, parser and syntax tree walker.

To effectively automate GUI tests, there will be more to follow.
Other than that, I am trying to solve one particular problem first and for that particular use-case, it will probably be the best tool there is. For other use-cases, I will look later if I can generalize the tool.

Please note that I have just started with this project and that the above text represents the general idea and that most things are not implemented yet.

You are welcome to ask questions. Any feedback is appreciated!
