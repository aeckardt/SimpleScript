# Tasc
Test Automation Script

The idea is to provide tools to automate test processes for GUI tests. For instance, push a button with a certain text and then something should appear on the screen.

The core parts what it can do is:
- Select Area on screen
- Record screen in area
- Extract images from recording
- Use pattern matching to quickly find images on screen
- Allow to customize as much as possible in a script language, somthing like

```
rect=select() # Select a rectangle on the screen
screen_rec=record(rect, 15) # the second parameter is the frame-rate
view(screen_rec)
```

This simple example allows to select an area on the screen, to record it with a frame rate of 15fps until a hotkey is pressed (which is Ctrl+. at the moment) and then can be viewed with a tool for that. This part could be like as a screencast-software.

To effectively automate GUI tests, there will be much more to follow.

P.S.: I know that there are probably already 100 tools like that out there. I don't usually like the idea of re-inventing the wheel. However, I set this challenge to myself to learn about different things, e.g. writing a parser, making a video out of screenshots, threading, using pattern matching algorithms to detect images and using text recognition. Another reason is to be able to use for future applications, so it'll better be good :)

You are welcome to ask questions. Any feedback is appreciated!
