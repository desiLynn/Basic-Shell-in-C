# Basic-Shell-in-C
Basic Linux Shell in C that saves history across sessions and has a simulated change directory function

# Functionality:
  
  #movetodir directory - Simulated move directory through an intenal varibale
  
  #whereami - shows contents of varaible containing the simulated current location
  
  #history [-c] - prints saved history (cross sessions) in decending order with a number -c clears the history
  
  #byebye - Exits the shell
  
  #replay number - replays a selected number from history
  
  #start program [parameters] - starts a program and doesnt return until closed
  
  #background program [parameters] - starts in background and returns a PID
  
  #dalek PID - kills selected PID
