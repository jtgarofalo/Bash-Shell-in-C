# Bash-Shell-in-C
This is a shell that I coded in C that can perform many basic functions a bash shell would be able to perform.

Capabilities Include:
- Ability to reroute output to a file instead of to terminal
- Ability to accept commands directly from user: "./wish" 
- Ability to accept commands read in from a file: "./wish filename.txt"
- Has 3 hard coded commands, "cd /directory" to change directories, a path command to set paths to search through when looking for executables and finally an "exit" command to end the shell process and return to terminal.
- Disregarding hard coded commands, it has capabilities to run all basic bash commands such as ls, cat, etc. through use of a fork() and exec() commands.
- Implemented error checking for many cases that will output generic error message to shell user, telling them they passed a wrongly formatted command without exiting the program or causing fatal errors.

Capabilities I intend to add if time allows:
- Ability to parse and execute if then statements passed into program as well as nested if then statements
- Ability to execute parallel commands: "cmd1 & cmd2 & cmd3" passed in on one line with "&" to separate them would ideally be able to all be executed at once.


Improvements I would make:
- Change the path array to be dynamically allocated each run through, instead of a static double array 20 long.
- Modulize my built in commands as well as my parsing
- Try to minimize my for loops to make code slightly more efficient
