# tmux Workflow


## References

1. [tmux man](https://man7.org/linux/man-pages/man1/tmux.1.html)


## Configuration

Copy `tmux.conf` as `~/.tmux.conf`.


## Basic Usage

**Default Leader: Ctrl + b**

- [ ] New session: tmux
- [ ] List running sessions: tmux ls
- [ ] Re-attach to tmux session: tmux attach
- [ ] New window: [Leader] + c
- [ ] Switch between windows in a session:
    - [ ] Cycle between windows: [Leader] + n
    - [ ] Specific window (window number is between 0 - 9): [Leader] + <window-num>
- [ ] Split a window into panes:
    - [ ] Vertical split: [Leader] + %
    - [ ] Horizontal split: [Leader] + “
- [ ] Switch between panes in a window: [Leader] + Arrow Key
- [ ] Detach from session: [Leader] + d
- [ ] Switch between sessions: [Leader] + s, then select session
- [ ] Scrolling: [Leader] + PgUp/PgDwn, [Leader] + Arrow Key
- [ ] Enter command mode: [Leader] + :
    - [ ] By default, tmux will execute commands from ~/.tmux.conf when starting up.
    - [ ] Rename window: rename-window <name>
    - [ ] Rename session: rename-session <name>
    - [ ] Kill current pane: kill-pane
    - [ ] Kill current window: kill-window
    - [ ] Kill current session: kill-session
    - [ ] Execute tmux commands from file: source-file /path/to/cmd-file

