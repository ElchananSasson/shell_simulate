Authored by Elchanan Sasson

==Description==
The program present a shell. The user insert command of his choice, and the program parses the string command to a process.
(the program does not support cd command). In the end, the program prints statistic about the commands. For each command the program also prints the sched file of the process.

functions:
There are six help methods:
Function 1(printPrompt) - prints the prompt line.
Function 2(printStatistic) - prints the statistics on the quantity and length of the commands entered.
Function 3(numOfWords) - returns how many words there are in a sentence.
Function 4(buildArr) - returns a two-dimensional array where each cell points to a word from the sentence entered by the user.
Function 5(freeCommand) - free the array memory.
Function 6(printSched) - prints information about the scheduler that runs the process.

==Program Files==
shell_simulate.c

==How to compile?==
compile: gcc shell_simulate.c -o shell
run: ./shell

==Input:==
A string that represents a command in 'shell'.

==Output:==
Prints the information about the scheduler and then the entered command and at the end also statistics are printed on all the entered commands.
