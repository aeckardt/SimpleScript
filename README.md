# Simple Script

!https://travis-ci.org/aeckardt/SimpleScript.svg?branch=master!:https://travis-ci.org/aeckardt/SimpleScript

## Goal of the project

The goal of this project is to provide tools to capture screen information and effectively utilize it to automate testing or to log information.
Currently you can select an area on the screen, make screenshots, view the outcome, measure performance and make some calculations. The function are explained in further detail down below in this readme.

## How the tool is working

This version is a basic command line tool using some GUI features. You can run simple commands like

```
# Comment
x = 5 + 10 * 2
print("The result of the calculation is: " + str(x))
```

and then you see the output in a textarea.

## Datatypes

Overview:

* Int,
* Float,
* String,
* Boolean,
* Point,
* Rect,
* Datetime,
* Image

## Functions

Overview:

* capture
* msecsbetween
* now
* print
* select
* sleep
* str
* view

### select
With this command, you can select a rectangular area of the screen. For instance, type the following:

```
rect = select()
print("The selected area has the coordinates: " + str(rect))
```

and the selection tool opens and then the coordinates are printed you have selected prior.

### capture / view
The function 'capture' makes a screenshot of an area or the whole screen.

Example 1 - fullscreen image:

```
# Captures fullscreen
image = capture()
view(image)
```

Example 2 - capture rectangular area:

```
# Captures selected area
image = capture(select())
view(image)
```

### sleep / msecsbetween / now
With 'sleep' you can let the main thread wait for x milliseconds. With 'now' you get the current datetime. Example:

```
print("The current datetime is " + str(now()))
start_time = now()
sleep(1000)
print("The actual amount of milliseconds passed is " + str(msecsbetween(start_time, now())))
``` 

You can use if-else-clauses as well:

```
result = 1 + 2 * 3 + 4 * (5 + 6)
if result == 51:
    print("The expression has been evaluated correctly!")
else:
    print("Error evaluating the expression - the result is wrong!")
```

You might have noticed that the script language is pretty much Python-like. The syntax is, as far as it is implemented, the same and this has a particular reason. The idea is to replace my own parser and embed python using SWIG to make it much more powerful. However, that is not planned right now.

## Next steps

* implement 'match' function to search for an image on the screen using pattern matching
  * I previously implemented a version of this function with OpenCV, but I didn't like the performance, so I'm trying to improve it or find another solution 
  * it would probably make more sense to have multiple functions like this depending on the particular use-case

* implement 'record' function to be able to create videos of a screen area
  * this should be done using FFmpeg's video encoding
  * the purpose of this is that it's much more convenient to gather the image data from a video as compared to when you have to re-run your software every time
  * the implementation of proper video recording with FFmpeg turns out to be quite a challenge!!

* implement 'read' function to read text from the screen
  * in order to do this a font has to be gathered and stored
  * my solution so far has been to gather a font by contrasting the text with the background. Not a perfect solution, but good enough for now
  
* give users the ability to store information in an xml file
  * this should contain important variables, paths to images, positions and script functions

With all that, you should be able to record a software while running, to gather all necessary information, like buttons, images and error texts. Then you can write a script and verify, at least in-part, that the script is working properly with your recording. Then you should be able to run the test with your scripted test-case.

