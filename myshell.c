/*
myshell - a small Unix-style shell in C

What it does:
- Shows a prompt with command count + current directory
- Takes user input, splits it into argv[]
- Runs built-ins in the shell process (cd, help, history, exit)
- Runs everything else with fork + execvp + waitpid
- Saves the last 10 commands using a simple circular history buffer

Made for my C progression / showcase project.
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>     // getcwd, chdir, fork, execvp
#include <sys/wait.h>   // waitpid

#define MAX_LINE 1024
#define MAX_ARGS 64
#define CWD_SIZE 256
#define HISTORY_SIZE 10


// last 10 commands (circular buffer)
static char history[HISTORY_SIZE][MAX_LINE];
static int history_count = 0;   // total commands typed
static int history_next = 0;    // index to write next command


static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}


static void print_prompt(int cmd_count) {
    char cwd[CWD_SIZE];

    // try to show current directory in the prompt
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
    strcpy(cwd, "?");
}
    printf("[#%d %s] myshell> ", cmd_count, cwd);
fflush(stdout);

}


static int parse_args(char *line, char *argv[], int max_args) {
    int argc = 0;
    char *save = NULL;
    char *tok = strtok_r(line, " \t", &save);

    // split on spaces/tabs, build argv for execvp
    while (tok != NULL && argc < max_args - 1) {
        argv[argc++] = tok;
        tok = strtok_r(NULL, " \t", &save);
    }
    argv[argc] = NULL;
    return argc;
}


static void history_add(const char *line) {
    if (line == NULL) return;
    if (line[0] == '\0') return;

    // save command into the next slot (overwrites oldest when full)
    snprintf(history[history_next], MAX_LINE, "%s", line);

    history_next++;
    if (history_next >= HISTORY_SIZE) history_next = 0;

    history_count++;
}


static void history_print(void) {
    if (history_count == 0) {
        puts("No history.");
        return;
    }

    int entries = history_count;
    if (entries > HISTORY_SIZE) entries = HISTORY_SIZE;

    int start_index;
    int start_number;

    // if we haven't filled the buffer yet, start at 0
    if (history_count < HISTORY_SIZE) {
        start_index = 0;
        start_number = 1;
    } else {
        // once full, history_next points to the oldest entry
        start_index = history_next;
        start_number = history_count - entries + 1;
    }

    // print in the correct order even though the buffer wraps
    for (int i = 0; i < entries; i++) {
        int idx = start_index + i;
        if (idx >= HISTORY_SIZE) idx = idx % HISTORY_SIZE;

        printf("%d  %s\n", start_number + i, history[idx]);
    }
}


// returns:
//  -1 : exit shell
//   1 : handled builtin (do not exec)
//   0 : not a builtin
static int handle_builtin(int argc, char *argv[]) {
    if (argc == 0) return 1; 

    if (strcmp(argv[0], "exit") == 0) {
        return -1; 
    }

    // cd must be builtin because it has to change THIS process's cwd
    if (strcmp(argv[0], "cd") == 0) {
        const char *target = (argc >= 2) ? argv[1] : getenv("HOME");
        if (target == NULL) target = ".";

        if (chdir(target) != 0) {
            perror("cd");
        }
        return 1; 
    }

    if (strcmp(argv[0], "help") == 0) {
        puts("Builtins:");
        puts("  help");
        puts("  cd [dir]");
        puts("  history");
        puts("  exit");
        return 1; 
    }

    if (strcmp(argv[0], "history") == 0) {
        history_print();
        return 1;
    }

    return 0; 
}


// external commands: fork → execvp in child → wait in parent
static void run_external(char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
    }
}


int main(void) {
    char line[MAX_LINE];
    char line2[MAX_LINE];
    char *argv[MAX_ARGS];
    int cmd_count = 0;

    while (1) {
        cmd_count++;
        print_prompt(cmd_count);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            puts("\nexit");
            break;
        }

        // flow:
        // 1) read line
        // 2) save original for history (parse_args will mess with the string)
        // 3) parse into argv
        // 4) builtins first, otherwise run external command
        trim_newline(line);
        snprintf(line2, sizeof(line2), "%s", line);

        history_add(line2);

        int argc = parse_args(line, argv, MAX_ARGS);

        
        int b = handle_builtin(argc, argv);
if (b == -1) break;      
if (b == 1) continue;    


if (argc > 0) {
    run_external(argv);
}

    }
    return 0;
}
