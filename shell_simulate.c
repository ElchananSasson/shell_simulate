/// ex2b: Elchanan Sasson - 208272625
/*
This program simulates the "shell"(Terminal) + Prints details of each process.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#define LEN 512

void printPrompt();
void printStatistic(int numOfCommand, int lenOfCommand, float avgOfCommands);
int numOfWords(char const str[]);
char** buildArr(char str[]);
void freeCommand(char** command, int length);
void printSched();

int main() {

    int numOfCommands = 0;
    int lenOfCommands = 0;
    float avgOfCommands = 0;
    int flag = 0;
    char **command = NULL;
    char str[LEN];

    printPrompt();
    while (fgets(str, LEN, stdin) == NULL);

    while (strcmp(str, "done\n") != 0) {
        int length = numOfWords(str);
        numOfCommands++;
        lenOfCommands += (int) strlen(str) - 1;
        ///if the user press only 'enter' or 'space' so we dont do nothing and dont calc those input to the statistic.
        if (length > 0) {
            command = buildArr(str);
            if ((strcmp(command[0], "done") == 0) && (length == 1)) {
                flag = 1;
                freeCommand(command, length);
                break;
            } else if (strcmp(command[0], "cd") == 0) {
                printf("command not supported (Yet)\n");
                freeCommand(command, length);
            } else {
                pid_t s1;
                s1 = fork();
                if (s1 < 0) {
                    perror("fork failed.\n");
                    exit(1);
                } else if (s1 == 0) {
                    pid_t s2;
                    s2 = fork();
                    ///grandson - print scheduler.
                    if (s2 < 0) {
                        perror("fork failed.\n");
                        exit(1);
                    } else if (s2 == 0) {
                        printSched();
                    } else {
                        ///son - print command.
                        wait(NULL);
                        if (execvp(command[0], command) == -1) {
                            perror("command not found.\n");
                            freeCommand(command, length);
                            exit(0);
                        }
                    }
                } else {
                    ///father.
                    wait(NULL);
                    freeCommand(command, length);
                }
            }
        }
        printPrompt();
        while (fgets(str, LEN, stdin) == NULL);
    }
    if (flag == 1) {
        /// to know if we 'exit' from the while(0) or from the command.
        printStatistic(numOfCommands, lenOfCommands, avgOfCommands);
    } else {
        numOfCommands++;
        lenOfCommands += (int) strlen(str) - 1;
        printStatistic(numOfCommands, lenOfCommands, avgOfCommands);
    }
    return 0;
}

void printPrompt() {
/// This method prints the prompt line.
    struct passwd *pw;
    char *cwd;
    char buff[LEN + 1];
    cwd = getcwd(buff, LEN + 1);
    if (cwd == NULL) {
        exit(1);
    }
    if ((pw = getpwuid(getuid())) == NULL) {
        printf("null@%s>", cwd);
    } else {
        printf("%s@%s>", pw->pw_name, cwd);
    }
}

void printStatistic(int numOfCommands, int lenOfCommands, float avgOfCommands) {
/// This method prints the statistics on the quantity and length of the commands entered.
    avgOfCommands = (float) lenOfCommands / (float) numOfCommands;
    printf("Num of commands: %d\nTotal length of all commands: %d\nAverage length of all commands: %f\nSee you Next time !\n",
           numOfCommands, lenOfCommands, avgOfCommands);
}

int numOfWords(char const str[]) {
/// This method returns how many words there are in a sentence.
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if ((str[i] != ' ' && str[i + 1] == ' ') || (str[i] != ' ' && str[i + 1] == '\n')) {
            count++;
        }
    }
    return count;
}

char** buildArr(char str[]) {
/// This method returns a two-dimensional array where each cell points to a word from the sentence entered by the user.
/// This method is an improved version of the method 'printParser' from Exercise 1.
    char **command;
    int length = numOfWords(str);
    command = (char **) malloc((length + 1) * sizeof(char *));
    if (command == NULL) {
        fprintf(stderr, "Error! the allocate not made.");
        exit(1);
    }
    command[length] = NULL;
    int i = 0;
    int numOfLetters = 0;
    char word[LEN];
    for (int j = 0; str[j] != '\0'; j++) {
        if (str[j] != ' ' && str[j] != '\n') {
            word[numOfLetters] = str[j];
            numOfLetters++;
        } else if ((str[j] == ' ' || str[j] == '\n') && numOfLetters > 0) {
            word[numOfLetters] = '\0';
            numOfLetters++;
            command[i] = (char *) malloc((numOfLetters + 1) * sizeof(char));
            if (command[i] == NULL) {
                fprintf(stderr, "Error! the allocate not made.");
                freeCommand(command, i);
                exit(1);
            }
            strcpy(command[i], word);
            numOfLetters = 0;
            i++;
        }
    }
    return command;
}

void freeCommand(char** command, int length) {
/// This method free the array memory.
    for (int i = 0; i < length; i++) {
        free(command[i]);
    }
    free(command);
}

void printSched() {
///This method prints information about the scheduler that runs the process.
    char proc[LEN];
    sprintf(proc, "/proc/%d/sched", getppid());
    char *sched[3];
    sched[0] = "cat";
    strcpy(sched[1], proc);
    sched[2] = NULL;
    if (execvp(sched[0], sched) == -1) {
        printf("command not found.\n");
        freeCommand(sched, 2); /// 2 cuz we dont need to free the last cell with null.
        exit(0);
    }
}
