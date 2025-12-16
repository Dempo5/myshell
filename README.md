# myshell

A small Unix-style shell written in C.

This project was built to understand how a basic shell actually works under the hood — reading input, parsing commands, handling built-ins, and running external programs using fork/exec.

It’s intentionally simple and readable. The goal wasn’t to recreate bash, but to fully understand each part of the control flow and be able to explain it line by line.

---

## Features

- Custom prompt showing command count and current working directory
- Built-in commands:
  - `cd`
  - `help`
  - `history`
  - `exit`
- Executes external commands using `fork`, `execvp`, and `waitpid`
- Fixed-size command history (most recent commands)
- Input parsing with basic argument tokenization

---

## How it works (high level)

1. The shell prints a prompt and waits for input
2. Input is read using `fgets` and cleaned
3. The original command is saved into history
4. The command is parsed into `argv`
5. Built-in commands are handled first
6. Non built-ins are executed in a child process

This mirrors the structure of a real Unix shell, just without advanced features like pipes or redirection.

---

## Why this project

I built this as a low-level systems exercise to get comfortable with:
- Process creation (`fork`)
- Program execution (`execvp`)
- Parent/child synchronization (`waitpid`)
- Managing state across iterations of a program

This is meant to be understandable, not clever.

---

## Build & Run

```bash
gcc myshell.c -o myshell
./myshell
```

Tested using Ubuntu (WSL).

---

## Notes

This is not meant to be production-ready.  
It’s a learning project that prioritizes clarity over feature count.
