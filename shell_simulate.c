/*
This program simulates the "shell"(Terminal) + pipe function(1 pipe/2 pipes) + signals Ctrl-z and 'fg'.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>

#define LEN 512

void printPrompt();
void printStatistic(int numOfCommand, int sumOfPipes);
int pipesInStr(char const str[]);
int numOfWords(char const str[], int index);
void doShell(char ** command0, char ** command1, char ** command2, int numOfPipes);
char ** buildArr(char str[], int numOfPipes);
void doExec(char ** command);
void freeCommand(char ** command);
void sig_handler(int sigNum);
pid_t savePid; /// To know which process need to 'fg'.

int main() {
    int numOfCommands = 0;
    int sumOfPipes = 0;
    char ** command0 = NULL; ///if we dont have pipe.
    char ** command1 = NULL; /// if we have 1 pipe.
    char ** command2 = NULL; /// if we have 2 pipe.
    char str[LEN];

    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
        perror("signal ERROR.");
        exit(1);
    }
    if (signal(SIGCHLD, sig_handler) == SIG_ERR) {
        perror("signal ERROR.");
        exit(1);
    }
    printPrompt();
    while (fgets(str, LEN, stdin) == NULL);
    while (strcmp(str, "done\n") != 0) {
        int numOfPipes = pipesInStr(str);
        if (numOfPipes > 2) {
            printf("This program does not support more than 2 pipes.\n");
        } else {
            sumOfPipes += numOfPipes;
            numOfCommands++;
            if (numOfPipes == 0) {
                int length = numOfWords(str, 0);
                ///if the user press only 'enter' or 'space' so we dont do nothing and dont calc those input to the statistic.
                if (length > 0) {
                    command0 = buildArr(str, 0);
                    if ((strcmp(command0[0], "done") == 0) && (length == 1)) {
                        numOfCommands--;
                        freeCommand(command0);
                        break;
                    } else if ((strcmp(command0[0], "fg") == 0) && (length == 1)) {
                        if (kill(savePid, SIGCONT) == -1) {
                            freeCommand(command0);
                            exit(1);
                        }
                        waitpid(savePid, NULL, WUNTRACED);
                        freeCommand(command0);
                    } else {
                        doShell(command0, command1, command2, numOfPipes);
                    }
                } else {
                    numOfCommands--;
                }
            } else if (numOfPipes == 1) {
                command0 = buildArr(str, 0);
                command1 = buildArr(str, 1);
                doShell(command0, command1, command2, numOfPipes);
            } else if (numOfPipes == 2) {
                command0 = buildArr(str, 0);
                command1 = buildArr(str, 1);
                command2 = buildArr(str, 2);
                doShell(command0, command1, command2, numOfPipes);
            }
        }
        printPrompt();
        while (fgets(str, LEN, stdin) == NULL);
    }
    printStatistic(numOfCommands, sumOfPipes);
    return 0;
}

void printPrompt() { /// This method prints the prompt line.
    struct passwd * pw;
    char * cwd;
    char buff[LEN + 1];
    cwd = getcwd(buff, LEN + 1);
    if (cwd == NULL) {
        exit(1);
    }
    if ((pw = getpwuid(getuid())) == NULL) {
        printf("null@%s>", cwd);
    } else {
        printf("%s@%s>", pw -> pw_name, cwd);
    }
}

void printStatistic(int numOfCommands, int sumOfPipes) {
    /// This method prints the amount of commands and pipes inserted.
    printf("Number of commands: %d\nNumber of pipes: %d\nSee you Next time !\n", numOfCommands, sumOfPipes);
}

int pipesInStr(char const str[]) {
    /// This method counts how many pipes there are in the string.
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '|') {
            count++;
        }
    }
    return count;
}

int numOfWords(char const str[], int index) {
    /// This method returns how many words there are in a sentence.
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (((str[i] != ' ' && str[i + 1] == ' ') || (str[i] != ' ' && str[i + 1] == '\n') ||
             (str[i] != ' ' && str[i + 1] == '"') || (str[i] != ' ' && str[i + 1] == '|')) && str[i] != '|' &&
            str[i] != '"') {
            count++;
        }
        if (str[i] == '"') {
            /// In the case of " " we will count everything inside as one word.
            count++;
            i++;
            while (str[i] != '"' && str[i] != '\0') {
                i++;
            }
        }
        if (str[i] == '|' && index == 0) {
            return count;
        }
        if (str[i] == '|' && (index == 1 || index == 2)) {
            count = 0;
            index--;
        }
    }
    return count;
}

void doShell(char ** command0, char ** command1, char ** command2, int numOfPipes) {
    /// This method simulates the 'shell' by creating boys.
    int pipe_fd1[2], pipe_fd2[2];
    pid_t pid1, pid2, pid3;
    if (pipe(pipe_fd1) == -1) {
        perror("pipe failed.\n");
        exit(1);
    }
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork failed.\n");
        exit(1);
    } else if (pid1 == 0) {
        if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
            perror("signal ERROR.");
            exit(1);
        }
        if (signal(SIGTSTP, sig_handler) == SIG_ERR) {
            perror("signal ERROR.");
            exit(1);
        }
        if (numOfPipes > 0) {
            close(pipe_fd1[0]);
            int val = dup2(pipe_fd1[1], STDOUT_FILENO);
            if (val == -1) {
                printf("dup failed.\n");
                exit(1);
            }
        }
        doExec(command0);
    } else { /// back to father.
        if (numOfPipes == 0) {
            close(pipe_fd1[0]);
            close(pipe_fd1[1]);
            freeCommand(command0);
        } else {
            if (pipe(pipe_fd2) == -1) {
                perror("pipe failed.\n");
                exit(1);
            }
            pid2 = fork();
            if (pid2 < 0) {
                perror("fork failed.\n");
                exit(1);
            } else if (pid2 == 0) {
                if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
                    perror("signal ERROR.");
                    exit(1);
                }
                if (signal(SIGTSTP, sig_handler) == SIG_ERR) {
                    perror("signal ERROR.");
                    exit(1);
                }
                close(pipe_fd1[1]);
                int val = dup2(pipe_fd1[0], STDIN_FILENO);
                if (val == -1) {
                    printf("dup failed.\n");
                    exit(1);
                }
                if (numOfPipes > 1) {
                    close(pipe_fd2[0]);
                    int val2 = dup2(pipe_fd2[1], STDOUT_FILENO);
                    if (val2 == -1) {
                        printf("dup failed.\n");
                        exit(1);
                    }
                }
                doExec(command1);
            } else { /// father.
                if (numOfPipes == 1) {
                    close(pipe_fd1[0]);
                    close(pipe_fd1[1]);
                    close(pipe_fd2[0]);
                    close(pipe_fd2[1]);
                    freeCommand(command0);
                    freeCommand(command1);
                } else {
                    pid3 = fork();
                    if (pid3 < 0) {
                        perror("fork failed.\n");
                        exit(1);
                    } else if (pid3 == 0) {
                        if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
                            perror("signal ERROR.");
                            exit(1);
                        }
                        if (signal(SIGTSTP, sig_handler) == SIG_ERR) {
                            perror("signal ERROR.");
                            exit(1);
                        }
                        close(pipe_fd1[0]);
                        close(pipe_fd1[1]);
                        close(pipe_fd2[1]);
                        int val = dup2(pipe_fd2[0], STDIN_FILENO);
                        if (val == -1) {
                            printf("dup failed.\n");
                            exit(1);
                        }
                        doExec(command2);
                    } else { /// father.
                        close(pipe_fd1[0]);
                        close(pipe_fd1[1]);
                        close(pipe_fd2[0]);
                        close(pipe_fd2[1]);
                        freeCommand(command0);
                        freeCommand(command1);
                        freeCommand(command2);
                    }
                }
            }
        }
    }
    waitpid(pid1, NULL, WUNTRACED);
    if (numOfPipes > 0) {
        waitpid(pid2, NULL, WUNTRACED);
        if (numOfPipes > 1) {
            waitpid(pid3, NULL, WUNTRACED);
        }
    }
}

char ** buildArr(char str[], int numOfPipes) {
    /// This method returns a two-dimensional array where each cell points to a word from the sentence entered by the user.
    /// The method knows which part to cut according to the 'numOfPipes'.
    char ** command;
    int length = numOfWords(str, numOfPipes);
    command = (char ** ) malloc((length + 1) * sizeof(char * ));
    if (command == NULL) {
        fprintf(stderr, "Error! the allocate not made.");
        exit(1);
    }
    command[length] = NULL;
    int i = 0, j = 0;
    int numOfLetters = 0;
    char word[LEN];
    while (numOfPipes > 0) { /// Lets 'j' move to the right place.
        if (str[j] == '|') {
            numOfPipes--;
        }
        j++;
    }
    for (; str[j] != '\0'; j++) {
        if (str[j] != ' ' && str[j] != '\n' && str[j] != '|') {
            if (str[j] == '"') {
                j++;
                while (str[j] != '"' && str[j] != '\0') {
                    word[numOfLetters] = str[j];
                    numOfLetters++;
                    j++;
                }
            } else {
                word[numOfLetters] = str[j];
                numOfLetters++;
            }
        }
        if ((str[j] == ' ' || str[j] == '\n' || str[j] == '|' || str[j] == '"') && numOfLetters > 0) {
            word[numOfLetters] = '\0';
            command[i] = (char * ) malloc((numOfLetters + 1) * sizeof(char));
            if (command[i] == NULL) {
                fprintf(stderr, "Error! the allocate not made.");
                freeCommand(command);
                exit(1);
            }
            strcpy(command[i], word);
            numOfLetters = 0;
            if (str[j] == '|' && numOfPipes == 0) {
                break;
            }
            i++;
        } else if (str[j] == '|' && numOfPipes == 0) {
            break;
        }
    }
    return command;
}

void doExec(char ** command) { /// This method sends the command to the operating system.
    if (strcmp(command[0], "cd") == 0) {
        fprintf(stderr, "command not supported (Yet)\n");
        freeCommand(command);
        exit(0);
    } else if (execvp(command[0], command) == -1) {
        perror("command not found.\n");
        freeCommand(command);
        exit(0);
    }
}

void freeCommand(char ** command) { /// This method free the array memory.
    for (int i = 0; command[i] != NULL; i++) {
        free(command[i]);
    }
    free(command);
}

void sig_handler(int sigNum) {
    /// This method receives signals and handles them individually.
    if (sigNum == SIGCHLD) {
        waitpid(-1, NULL, WNOHANG);
    }
    if (sigNum == SIGTSTP) {
        savePid = getpid();
    }
}
