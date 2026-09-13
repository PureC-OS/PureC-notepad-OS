# PureC Notepad

Standalone graphical text editor for PureC OS (Ring-3 program, C++20, freestanding).

Self-contained repository: no includes and no link-time dependencies on the
kernel tree. The `sys/` layer talks to the OS directly through syscalls,
`gui/` is an original minimal window toolkit, all icons are drawn in code.
No third-party names, logos or assets are used.

## Layout

- Toolbar: New, Open, Save, SaveAs, Find
- Tab bar: up to 8 tabs, click to switch, `+` for a new tab
- Left panel: file browser (click a folder to enter, `.. (up)` to go up,
  click a file to open it)
- Editor: line numbers, current-line highlight, horizontal and vertical
  scroll, mouse click moves the cursor
- Status bar: `Ln / Col`, file size, hints, input prompts

## Keys

| Key | Action |
|---|---|
| Ctrl+S | Save (asks for a path for untitled tabs) |
| Ctrl+O | Open file by path |
| Ctrl+N | New tab |
| Ctrl+W | Close tab (asks again if modified) |
| Ctrl+F | Find, F3 / Ctrl+G next match |
| Ctrl+T | Next tab |
| Ctrl+X | Exit (asks again if modified) |
| Arrows, Home, End, PgUp, PgDn, Delete, Backspace, Tab, Enter | Editing |

Tab inserts 4 spaces, Enter keeps the current line indent.

## Build

Needs `x86_64-elf-g++` and `x86_64-elf-ld`:

```bash
make
```

Result: `bin/notepad` (ELF x86-64, entry `_start`).

Logic unit test with the host compiler (no OS needed):

```bash
make test
```

## Run on PureC OS

Copy `bin/notepad` to the image programs directory
(`bin/program/notepad`) and start it from the terminal:

```bash
notepad
notepad /path/to/file.txt
```

## Structure

- `sys/` — syscall layer (own headers, `int $0x80` wrappers)
- `gui/` — minimal window toolkit (frame, drag, buttons, clipped text)
- `com/` — string helpers, `memcpy/memmove/memset`
- `include/pad/` — editor: tabs and gapless buffers, file ops,
  directory browser, view geometry and drawing, input
- `src/` — implementation, `src/main.cpp` holds `_start`
- `test/` — host unit test for buffer and line logic
- `linker.ld` — userspace link script
