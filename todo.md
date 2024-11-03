- [x] Draw debug shapes ready for collision detection.
- [x] Fix transparency issues
- [x] Import SDL2
- [x] Open Window
- [x] Draw bird
- [x] Make bird correct aspect ratio
- [x] Make portrait render view

# Goals 

- [x] Create a game world with a floor.
- [x] Add an object that represents the main character. Apply a constant force to the character so it falls to the floor.
- [x] Add obstacles on the left of the game area. The obstacles should slide across the screen toward the right. The obstacles will appear in pairs, with a vertical gap between them.
- [x] You could create a really long level and hope that the player never gets to the end of it, but it would make more sense if you used code to create obstacles when you need them.
- [x] The game obstacles (pipes) will move off of the screen and never be used again. You might want to delete them once they are no longer useful. Or, better yet, re-use them by moving them to the left side of the screen once they move off of the right side!
- [x] Detect when the character collides with the floor or obstacles, and reset the game when a collision occurs.
- [x] Accumulate one point for each obstacle that the player passes. Display the score.

# Stretch goals:

- [ ] Interpolate state in game loop.
- [ ] Don't do fullscreen by default. Add F11 key to go fullscreen.
- [x] Track the high-score between play sessions and display the high score at the start.
- [ ] Add some sounds that will play each time the player gains a point, and when the player loses.
- [x] Add some background art! Try layering the background and scrolling at a different rate to the foreground obstacles. This is called Parallax scrolling.
- [x] Add clouds 
- [ ] Build for Android