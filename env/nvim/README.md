# neovim Guide


## Standard Motions Guide (largely applicable to vim/vi too)


### Notes

- Default Leader: Space
- System Clipboard Register:
    - macOS: “*
    - Linux: “+
- Current File Path Register: “%

### Modes

- Normal mode: Default mode. Navigation allowed.
    - Use Esc to enter normal mode.
    - Enter other modes via this mode.
- Insert mode: Text insertion allowed.
    - I: Insert mode + Go to start of line.
    - i: Insert mode + Go back one character.
    - a: Insert mode + Go forward one character.
    - A: Insert mode + Go to end of line
    - o: Insert mode + Add new line
- Replace mode: Text replacement allowed.
    - Use Shift + R from Normal mode to enter this mode.
- Command mode: Use : to enter this mode.
- V-Line mode: Use Shift + v.
    - To delete (cut): d
    - To copy (yank): y

### Navigation (do from Normal Mode)

- Move cursor per-character: h (left), j (down), k (up), l (right). Arrows keys also work.
- Move cursor per-word: w (forward), b (backward)
    - Shift/Ctrl + Arrow Key may also work.
- Move cursor to start of line: 0
    - Home will also work.
- Move cursor to start of first non-whitespace character in a line: ^
- Move cursor to end of line: $
    - End will also work.
- Move cursor to start of document: gg
- Move cursor to end of document: Shift + g
- Find first occurrence of given character on a line: f + character:
    - Jump to next occurrence: ;
    - Jump to previous occurrence: ,
    - Use F + character to find backwards from the beginning.
- Go to matching brackets: %

### Text Manipulation (do from Normal Mode)

- Repeat last operation: .
- Undo: u
- Redo: Ctrl + R
- Operators & Text Objects:
    - Operation + [Motion] + Range
    - Delete (cut):
        - Entire line: d + d
        - Word from current cursor position: d + w
        - From current cursor position till end of the line: d + $
        - Entire word: d + i  + w
        - Entire paragraph: d + i  + p
    - Paste:
        - After current cursor position: p
        - Before current cursor position: P
        - From specified register: <register-name> + p/P
            - E.g. Paste from system clipboard (macOS used as example) before current cursor position: “ + * + p
    - Copy (yank):
        - Entire line: y + y
        - Word from current cursor position: y + w
        - From current cursor position till end of the line: y + $
        - Entire word: y + i  + w
        - Entire paragraph: y + i  + p
        - To specified register: <register-name> + <copy operation>
            - Copy operation can be one of the operations shown above. Can also be used with V-Line mode.
            - E.g. copy entire line to system clipboard (* used as example): “ + * + y + y
    - Toggle case:
        - From current cursor position: g + ~ + w
        - Entire word: g + ~ + i  + w
    - Change word:
        - From current cursor position: c + w
        - Entire word: c + i + w
- Find string: / + string + Enter
    - Find Next: n
    - Find Previous: N
- Macros: Record actions for replaying.
    - Start recording to specified register: q + <register-name>
        - Register names are any letter between a - z.
    - Perform the operations.
    - Stop recording: q
    - To replay macro: @ + <register-name>

### Commands In Command Mode (end with Enter)
- Save file: :w
- Search and replace:
    - Replace all occurrences in file automatically: %s/<regex-to-replace>/<replacement>/g
    - Replace all occurrences in file with manual confirmation for each: %s/<regex-to-replace>/<replacement>/gc
- Exiting (append “!” to force):
    - Exit file with save: :wq
    - Exit file w/o save: :q!
    - Exit all files with saving: :wqa
    - Exit all files w/o saving: :qa
- Edit a file: :e, :e!
    - Useful to reload a file for editing that has changed outside of the editor.
    - The “!” will discard unsaved changes in the buffer, if any.
- Go to specific line: :<line-num>
- Use built-in file explorer: :Ex
    - Create a new directory: d + directory name
    - Create a new file: % + filename
    - Files opened from here will open in a new buffer.
    - Delete a file/directory: D
    - Rename a file/directory: R
- Show register contents: :reg
- Source currently open .lua config. file: :so

**Window Management (do from Normal Mode)**
- Switch windows: [Ctrl + w] + w
- Quit window: [Ctrl + w] + q
- Split vertically: [Ctrl + w] + v
- Split horizontally: [Ctrl + w] + s


## Plugins Guide


- Open Lazy (package manager, must install first): :Lazy
    - Press q to quit.
- gitsigns.nvim
    - Blame
        - For current line: :Gitsigns blame_line
        - For entire file: :Gitsigns blame
    - Changes
        - Per-word diff: :Gitsigns toggle_word_diff
        - Reset hunk: :Gitsigns reset_hunk
        - Preview hunk changes: :Gitsigns preview_hunk
- nvim-tree
    - Show hidden files: H
- Telescope
    - Close telescope: Esc
    - Find files in workspace: [Leader] + ff
    - Grep files in workspace: [Leader] + fg
    - Access open buffers: [Leader] + fb
    - Access nvim help documentation: [Leader] + fh
    - Scroll in preview window: Ctrl + u (up), Ctrl + d (down)
