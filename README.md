# C Shell

Build with `make` (creates `a.out`), start the shell with `./a.out`.
The directory the shell is started from is its home directory (`~`).

## Commands you can run in the shell

### Built-in commands
| Command | What it does |
|---|---|
| `hop` | Go to the home directory (the directory the shell was started from) |
| `hop <path> ...` | Change directory and print the full path. Several arguments are handled left to right. Supports `.`, `..`, `~`, `-`, `~/dir`, relative and absolute paths |
| `reveal [-a] [-l] [path]` | List a directory in lexicographic order (current directory if no path). `-a` shows hidden files, `-l` gives the `ls -l` columns. Flags can be combined (`-la`, `-al`, `-l -a`, `-lala`). Blue = directory, green = executable, white = file |
| `log` | Print the last 15 stored commands, oldest first |
| `log purge` | Clear the stored commands |
| `log execute <n>` | Run the n-th command from the log (1 = most recent) |
| `proclore [pid]` | Show pid, status (`R`, `R+`, `S`, `S+`, `Z`), process group, virtual memory (KB) and executable path. No pid means the shell itself |
| `seek [-d] [-f] [-e] <name> [dir]` | Recursively search `dir` (default: current) for files/directories whose name starts with `<name>`. `-d` only directories, `-f` only files, `-e` with exactly one match prints the file or enters the directory. `-d` with `-f` prints `Invalid flags!` |
| `activities` | List jobs started by the shell as `pid : command - Running/Stopped`, sorted by command name |
| `ping <pid> <signal>` | Send a signal (number taken modulo 32) to a process |
| `fg <pid>` | Bring a background or stopped job to the foreground and continue it |
| `bg <pid>` | Continue a stopped job in the background |
| `neonate -n <secs>` | Print the most recently created process id every `<secs>` seconds until `x` is pressed |
| `iMan <command>` | Fetch the man page of `<command>` from man.he.net and print it |
| `exit` | Kill all jobs and leave the shell |

### Shell features
| Syntax | What it does |
|---|---|
| `cmd1 ; cmd2` | Run commands one after the other |
| `cmd &` | Run a system command in the background and print its pid. A message is printed when it ends |
| `cmd > file` / `cmd >> file` | Write output to a file (overwrite / append). The file is created with mode 0644 |
| `cmd < file` | Read input from a file (`No such input file found!` if missing). Can be combined with `>` or `>>` |
| `cmd1 \| cmd2 \| cmd3` | Pipe output of one command into the next, any number of pipes. `Invalid use of pipe` if a side is empty |
| `cat < a.txt \| wc \| cat > b.txt` | Pipes and redirection together |
| Any system command (`ls`, `sleep 5`, `echo hi`, `gedit`, `./a.out` ...) | Run through the exec family. A bad name prints `ERROR : '<cmd>' is not a valid command` |
| Ctrl-C | Interrupt the foreground process (no effect if none) |
| Ctrl-Z | Stop the foreground process and move it to the background as Stopped (no effect if none) |
| Ctrl-D | Kill all jobs and log out of the shell |

### Aliases and functions (from `.myshrc`)
| Command | What it does |
|---|---|
| `reveall` | Same as `reveal -l` |
| `reveala` | Same as `reveal -a` |
| `home` | Same as `hop ~` |
| `mk_hop <dir>` | Create the directory and hop into it (bonus) |
| `hop_seek <dir>` | Hop into the directory and then `seek` for its name (bonus) |

Add your own single word aliases (`name = command`) or functions using `$1`...`$9` to `.myshrc` in the home directory.

## Files and the specification they implement

| File | Purpose |
|---|---|
| `shell.h` | System includes, colours, `Process` struct, global state, includes every module header |
| `main.c` | Global state definitions, signal / terminal setup, starts the prompt loop |
| `display.c` | Spec 1 : prompt (`<user@host:path>`), input loop, Ctrl-D, "time taken" note in prompt |
| `tokenize.c` | Spec 2 : splits on `;` and `&` (remembers which one), helpers (`trim`, `next_word`, `split_args`) |
| `redirection.c` | Spec 10, 11, 12 : `<` `>` `>>`, any number of pipes, redirection with pipes, `Invalid use of pipe` |
| `command.c` | Single dispatcher used by every path (plain, pipe stage, `log execute`, functions) |
| `hop.c` | Spec 3 : `hop` (really calls `chdir`) |
| `reveal.c` | Spec 4 : `reveal -a -l` (output matches `ls -l`) |
| `log_store.c`, `log.c` | Spec 5 : `log`, `log purge`, `log execute <index>` |
| `system.c` | Spec 6 : foreground / background system commands (exec family), `SIGCHLD` handler |
| `proclore.c` | Spec 7 : `proclore [pid]` |
| `seek.c` | Spec 8 : recursive `seek` with `-d -f -e` |
| `myshrc.c`, `.myshrc` | Spec 9 : aliases and functions (`mk_hop`, `hop_seek` bonus) |
| `activities.c` | Spec 13 : job table and `activities` |
| `signals.c` | Spec 14 : `ping`, Ctrl-C / Ctrl-Z / Ctrl-D |
| `foreground.c`, `background.c` | Spec 15 : `fg`, `bg` |
| `neonate.c` | Spec 16 (bonus) : `neonate -n <secs>`, stops on `x` |
| `iman.c` | Spec 17 : `iMan <command>` using sockets |

## Assumptions

* **Log** : stored in `<home>/.shell_log` (hidden, so `reveal` shows nothing extra), shared by all sessions, max 15 entries.
  The whole input line is stored (trimmed). **Erroneous commands are stored.** A line is not stored if it equals the previous
  one or if any `;` `&` `|` separated part of it is a `log` command. `exit` is stored.
* `log execute <index>` : 1 is the most recent command; the executed command is not stored again.
* **Background pid** : printing only the pid (as in Specification 6). Finished background jobs print
  `<name> exited normally (<pid>)` or `<name> exited abnormally (<pid>)`.
* **Foreground time** : if a foreground command takes more than 2 seconds the next prompt is
  `<user@host:path cmd : Ns>` (only the command name is shown, time rounded down).
* `hop` / `reveal` / `seek` do not run in the background (`&` is ignored for them). In a pipe they run in a forked stage,
  so a `hop` inside a pipe does not change the shell's directory (same as Bash).
* Pipes : all stages run together (so large outputs cannot deadlock). A pipeline is one job (process group) for Ctrl-Z / `fg` / `bg`.
* Invalid system command : `ERROR : '<cmd>' is not a valid command`.
* `seek` : names match exactly or by prefix; results are relative paths (`./a/b`), directories blue, files green.
  With `-e` and one directory the printed path ends with `/` (as in the sample) and the shell changes into it.
  A missing target directory prints `No match found!`.
* `reveal -l` : same columns as `ls -l`; colours are blue (directory), green (executable), white (other).
* `proclore` : `+` means the process is in the foreground process group; virtual memory is `VmSize` in KB.
* `ping` : signal number taken modulo 32.
* Ctrl-C / Ctrl-Z have no effect when nothing is running in the foreground. Ctrl-D kills every job and exits.
  `exit` does the same.
* `.myshrc` is read from the home directory when the shell starts. Aliases are one word (`name = command`, with or without
  the `alias` keyword); functions use `$1 ... $9`. Comments start with `#` or `//`.
* Quotes (`"..."` or `'...'`) are removed from arguments; paths and names are assumed not to contain whitespace.
* `neonate` and `iMan` need a real terminal / network respectively. `iMan` prints the page without HTML tags and without the HTTP header.
