// Library Header Files
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>

// Imported Files
#include "../include/process_runner.h"
#include "../include/builtins.h"

pid_t g_running_fg_pid = 0;
volatile sig_atomic_t g_line_interrupted = 0;

static void exit_signal_of_child(int sig) {
    (void) sig;
    int err = errno;
    pid_t r_pid;
    int exit_state;
    char buf[128];

    while ((r_pid = waitpid(-1, &exit_state, WNOHANG)) > 0) {
        int len = 0;

        if (WIFEXITED(exit_state)) {
            len = snprintf(buf, sizeof(buf),
                "\n[Process with PID %d exited normally]\n", (int)r_pid);
        } else if (WIFSIGNALED(exit_state)) {
            len = snprintf(buf, sizeof(buf),
                "\n[Process with PID %d terminated by signal %d]\n",
                (int)r_pid, WTERMSIG(exit_state));
        } else {
            continue;
        }

        if (len > 0) {
            if (len >= (int)sizeof(buf)) {
                len = sizeof(buf) - 1;
            }
            write(STDERR_FILENO, buf, len);
        }
    }
    errno = err;
}

static void handle_sigint(int sig) {
    (void) sig;
    if (g_running_fg_pid > 0) {
        kill(-g_running_fg_pid, SIGINT);
    } else {
        g_line_interrupted = 1;
        write(STDOUT_FILENO, "\n", 1);
    }
}

static void sigtstp_received(int sig) {
    (void) sig;
    if (g_running_fg_pid > 0) {
        kill(-g_running_fg_pid, SIGTSTP);

        char buf[64];
        int len = snprintf(buf, sizeof(buf),
            "\n[Process with PID %d stopped]\n", (int)g_running_fg_pid);
        if (len > 0) {
            if (len >= (int)sizeof(buf)) {
                len = sizeof(buf) - 1;
            }
            write(STDOUT_FILENO, buf, len);
        }

        g_running_fg_pid = 0;
    } else {
        g_line_interrupted = 1;
        write(STDOUT_FILENO, "\n", 1);
    }
}

void init_signal() {
    struct sigaction sa_child, sa_int, sa_tstp;

    // 1. Reaping handler for SIGCHLD
    memset(&sa_child, 0, sizeof(sa_child));
    sa_child.sa_handler = exit_signal_of_child;
    sigemptyset(&sa_child.sa_mask);
    sa_child.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    sigaction(SIGCHLD, &sa_child, NULL);

    // 2. SIGINT (CTRL + C)
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // 3. SIGTSTP (CTRL-Z)
    memset(&sa_tstp, 0, sizeof(sa_tstp));
    sa_tstp.sa_handler = sigtstp_received;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}

void dispatch_external_cmd(struct Command *cmd) {
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork error");
        return;
    }

    if (child_pid == 0) {
        setpgid(0, 0);

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        if (execvp(cmd->args[0], cmd->args) == -1) {
            fprintf(stderr, "%s: Command is not found\n", cmd->args[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        setpgid(child_pid, child_pid);

        if (cmd->ends_with_ampersand) {
            printf("[%d]\n", (int)child_pid);
        } else {
            int status = 0;

            g_running_fg_pid = child_pid;
            while (waitpid(child_pid, &status, WUNTRACED) == -1 && errno == EINTR) {
                //Keep waiting intil child has not exited;
            }

            g_running_fg_pid = 0;
        }
    }
}

void execute_pipeline(struct Command *cmd, int num_cmd, bool is_it_background) {
    int prev_fd_read = -1;
    pid_t pipe_pgid = 0;
    pid_t last_pid = 0;

    for (int i = 0; i < num_cmd; i++) {
        int pipe_fd[2];

        if (i < num_cmd - 1 && pipe(pipe_fd) < 0) {
            perror("pipe failed");
            return;
        }

        pid_t pid = fork();

        if (pid == 0) {
            if (i == 0) {
                pipe_pgid = getpid();
            }
            setpgid(0, pipe_pgid);

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (cmd[i].input_file != NULL) {
                int fd_in = open(cmd[i].input_file, O_RDONLY);
                if (fd_in < 0) {
                    perror("Input error");
                    exit(1);
                }
                if (dup2(fd_in, 0) < 0) {
                    perror("Input redirection failed");
                    close(fd_in);
                    exit(1);
                }
                close(fd_in);
            } else if (prev_fd_read != -1) {
                if (dup2(prev_fd_read, 0) < 0) {
                    perror("Input redirection failed");
                    exit(1);
                }
            }

            if (cmd[i].output_file != NULL) {
                int fd_out = open(cmd[i].output_file,
                    O_WRONLY | O_CREAT | (cmd[i].append_mode ? O_APPEND : O_TRUNC),
                    0644);
                if (fd_out < 0) {
                    perror("Output error");
                    exit(1);
                }

                if (dup2(fd_out, 1) < 0) {
                    perror("Output redirection failed");
                    close(fd_out);
                    exit(1);
                }
                close(fd_out);
            } else if (i + 1 < num_cmd) {
                if (dup2(pipe_fd[1], 1) < 0) {
                    perror("Output redirection failed");
                    exit(1);
                }
            }

            if (prev_fd_read != -1)
                close(prev_fd_read);
            if (i + 1 < num_cmd) {
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }

            if (execute_builtin(&cmd[i]))
                exit(0);
            execvp(cmd[i].args[0], cmd[i].args);
            perror("Command not found");
            exit(1);
        } else {
            if (i == 0)
                pipe_pgid = pid;

            setpgid(pid, pipe_pgid);
            last_pid = pid;

            if (prev_fd_read != -1)
                close(prev_fd_read);

            if (i + 1 < num_cmd) {
                close(pipe_fd[1]);
                prev_fd_read = pipe_fd[0];
            }

            if (!is_it_background && i + 1 == num_cmd) {
                g_running_fg_pid = pipe_pgid;
                int status;
                while (waitpid(last_pid, &status, WUNTRACED) == -1 && errno == EINTR) {
                    //Keep waiting intil child has not exited;
                }
                g_running_fg_pid = 0;
            } else if (is_it_background && i + 1 == num_cmd) {
                printf("[%d]\n", (int)pid);
            }
        }
    }
}