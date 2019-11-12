# Tasc
Test Automation Script

The idea is to provide tools to automate test processes for GUI tests. For instance, to verify that after pushing a button a certain text appears on the screen.

The core parts what it can do is:
- Select Area on screen
- Record screen in area
- Extract images from recording
- Use pattern matching to quickly find images on screen
- Use text recognition
- Allow to customize as much as possible in a script language

-> save images, videos in variables and load/save them
-> make calculations/decisions based on the information found in images/texts

```
rect=select() # Select a rectangle on the screen
screen_rec=record(rect, 15) # the second parameter is the frame-rate
view(screen_rec)
```

This simple example allows to select an area on the screen, to record it with a frame rate of 15fps until a hotkey is pressed (which is Ctrl+. at the moment) and then can be viewed with a tool for that. This part in itself can be used as screencast-software.

You can then extract images and sequences from the recording. For instance, call one image 'logo' and then you can initialize the save image and locate it as follows

```
register("software.xml") # loads saved variables (images/locations) in software.xml
point=find(logo)
print(point)
```

This should print out the coordinates (x, y) of the logo on the screen. You could use this information to click on it or to locate other buttons / images that have a fixed position relative to the logo. You can also always search the whole screen for an image, but be aware: Running pattern matching algorithms on the whole screen is a costly operation. So if your use case is performance critical, you should consider minimizing the usage of pattern-matching.

Another thing to implement is text recognition, that can be used to do calculations in the script and make decisions based on it. For that I will enhance the script language such that you can implement basic loops.

I consider to go another route and just use python in my program and to create a plugin for that. That would probably be better, even now I have already created the whole toolchain of lexer, parser and syntax tree walker.

To effectively automate GUI tests, there will be more to follow.

I am well aware that there are probably 100 tools like that out there already! I don't usually like the idea of re-inventing the wheel. However, I set this challenge to myself to learn about different things, e.g. writing a parser, making a video out of screenshots, threading, using pattern matching algorithms to detect images and using text recognition.

Other than that, I am trying to solve one particular problem first and for that particular use-case, it will probably be the best tool there is. For other use-cases, I will look later if I can generalize the tool.

You are welcome to ask questions. Any feedback is appreciated!
