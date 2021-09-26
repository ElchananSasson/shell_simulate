Authored by Elchanan Sasson

==Description==
This program simulates the "shell"(Terminal) + pipe function(1 pipe/2 pipes) + signals Ctrl-z and 'fg'.

functions:
There are 5 main methods:
Function 1(numOfWords) - This method returns how many words there are in a sentence.
Function 2(buildArr) - This method returns a two-dimensional array where each cell points to a word from the sentence entered by the user.
The method knows which part to cut according to the 'numOfPipes'.
Function 3(doShell) - This method simulates the 'shell' by creating boys.
Function 4(doExec) - This method sends the command to the operating system.
Function 5(sig_handler) - This method receives signals and handles them individually.

==Program Files==
shell_simulate.c

==How to compile?==
compile: gcc shell_simulate.c -o shell_simulate
run: ./shell_simulate

==Input:==
A string that represents a command in 'shell'.

==Output:==
Print the output of the command(the string that the user input) like what the 'shell' do.
