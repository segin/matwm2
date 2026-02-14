# Comprehensive Audit Report for matwm2

This report details security flaws, implementation issues, and suggestions for improvements and features for the `matwm2` window manager.

## 1. Security Flaws

### 1.1 Unsafe Signal Handling (`main.c`)
The `sighandler` function in `main.c` performs several operations that are not async-signal-safe. Specifically, it calls:
- `XOpenDisplay`
- `XInternAtom`
- `XSendEvent`
- `XFlush`
- `XSync`
- `XCloseDisplay`
- `fprintf` (via `stderr`)

**Risk:** Calling these functions from a signal handler can lead to deadlocks, memory corruption, or undefined behavior if the signal interrupts an ongoing Xlib call or other critical section. Xlib functions are generally not reentrant.

**Recommendation:** The signal handler should only set a `volatile sig_atomic_t` flag. The main event loop (`main.c`) should check this flag periodically (e.g., using `XCheckMaskEvent` with a timeout or `select`/`poll` on the connection file descriptor) and perform the shutdown/reinit logic safely in the main thread of execution.

### 1.2 Deprecated and Unsafe `vfork` Usage (`misc.c`)
The `spawn` function in `misc.c` uses `vfork()`.
**Risk:** `vfork` suspends the parent process until the child calls `exec` or `_exit`. If the child modifies any memory (including the stack) or returns from the function before `exec`/`_exit`, it can corrupt the parent's state. While `vfork` was a historical optimization, modern `fork` implementations with Copy-On-Write (COW) make it largely redundant and safer. POSIX.1-2008 removed `vfork`.

**Recommendation:** Replace `vfork()` with `fork()`. The performance difference is negligible on modern systems, and `fork()` is much safer.

### 1.3 Command Injection Risk (`misc.c` / `config.c`)
The `spawn` function executes commands using `sh -c`. The command string originates from the configuration file (`config.c`).
**Risk:** If the configuration file is writable by an untrusted user or if the configuration parsing logic (`cfg_read`) flaws allow injection, arbitrary commands can be executed. While configuration files are typically trusted, this is a potential attack vector if permissions are weak.

**Recommendation:** Ensure configuration files have strict permissions (read/write only by owner). Consider using `execvp` directly where possible to avoid shell interpretation, though this may limit flexibility for users who expect shell features (pipes, redirection).

### 1.4 String Manipulation Robustness (`misc.c`)
The `eat` and `unescape` functions modify strings in-place.
**Risk:** `eat` relies on finding a delimiter or null terminator. If a malformed string (not null-terminated) is passed, it could read beyond bounds. `read_file` ensures null-termination, but any future usage of these functions on other inputs requires careful validation.

**Recommendation:** Add explicit length checks or use standard string tokenization functions (`strtok_r`, `strsep`) where appropriate, ensuring bounds safety.

## 2. Implementation Issues

### 2.1 Fixed-Size `_NET_WORKAREA` (`ewmh.c`)
The `ewmh_update_strut` function calculates the work area for desktops. It initializes a `workarea` array of 16 `long`s and uses it to set the `_NET_WORKAREA` property.
**Issue:** This hardcodes support for effectively 4 desktops (4 values * 4 desktops = 16 longs). If `dc` (desktop count) is greater than 4, the work area for subsequent desktops will not be set correctly (or at all), leading to inconsistent behavior for EWMH-compliant pagers and taskbars.

**Recommendation:** Dynamically allocate the `workarea` array based on `dc * 4 * sizeof(long)`.

### 2.2 Global State Management
The codebase relies heavily on global variables (e.g., `dpy`, `root`, `clients`, `stacking`, `dc`, `current`).
**Issue:** This makes the code harder to test, maintain, and reason about. It also makes it difficult to implement features like multiple screens (Xinerama/RandR) cleanly if state is not encapsulated per-screen.

**Recommendation:** Encapsulate related state into structures (e.g., `WMState`, `ScreenState`) and pass these contexts to functions.

### 2.3 Error Handling (Fail-Fast)
Functions like `_malloc`, `_realloc`, and `xerrorhandler` (implied) tend to exit the program immediately on error (`error()`, `exit(EXIT_FAILURE)`).
**Issue:** While acceptable for a simple WM, this provides poor resilience. A single allocation failure or X error crashes the entire window manager, potentially losing user data in other applications.

**Recommendation:** Implement more graceful error handling and recovery where possible. Log errors to `stderr` or a log file without crashing if the error is recoverable.

### 2.4 Hardcoded Initial `workarea` Calculation (`ewmh.c`)
The loop in `ewmh_update_strut` initializes `workarea[0..3]` to 0 and then attempts to find a bounding box using logic that assumes `0` is a valid starting point for min/max comparisons.
**Issue:**
```c
workarea[0] = ((workarea[0] < x0) ? workarea[0] : x0); // effectively min(0, x0)
```
If `x0` (screen x + strut) is positive (which is typical), `workarea[0]` remains 0, which might be incorrect if the screen starts at x=100. It effectively anchors the top-left to (0,0) regardless of the actual screen layout.

**Recommendation:** Initialize `workarea` with the values of the first screen/strut, then iterate through the rest.

## 3. Suggestions for Improvements

### 3.1 Code Modernization
- **Standard Types:** Use `stdint.h` types (`uint32_t`, etc.) for X11 32-bit values to ensure portability.
- **`fork` vs `vfork`:** As mentioned, switching to `fork` is a quick win for safety.
- **Safe Signal Handling:** Implement the "self-pipe trick" or a simple flag check in the event loop.

### 3.2 Dynamic Configuration
- **Dynamic Desktops:** Fix the EWMH workarea issue to support any number of desktops.
- **Reloading:** The `sighandler` supports `SIGUSR1` for reloading (`XA_REINIT`). Ensure this path properly frees all resources before re-reading the config to avoid memory leaks (the current `cfg_reinitialize` handles some but potentially not all resources).

### 3.3 Event Loop Refactoring
- **Dispatcher:** Refactor `handle_event` in `events.c` to use a table of function pointers based on event type. This would clean up the large `switch` statement and make it easier to add handlers.

## 4. Feature Suggestions (Retro-Themed)

Keeping with the "retro" aesthetic (late 90s / early 2000s):

### 4.1 "Themes" Support
- **Idea:** Allow loading color schemes and font settings from separate theme files.
- **Retro Angle:** Include presets like "Hot Dog Stand" (Windows 3.1), "Solaris CDE" (teal/grey), and "Classic Motif".
- **Implementation:** Extend `config.c` to support an `include` directive or a `theme` command that loads a set of color variables.

### 4.2 Simple Menu System
- **Idea:** A root menu (right-click on desktop) to launch applications.
- **Retro Angle:** A simple text-based list, similar to `9wm` or early `blackbox`.
- **Implementation:** Create a simple override-redirect window that draws a list of items. Parse a `menu` section in the config file.

### 4.3 Visual Effects (Low-Fidelity)
- **Idea:** Add "scanlines" or a "dithered" background pattern.
- **Retro Angle:** Mimic CRT monitors or old X11 root stipples.
- **Implementation:** Use `XCreateBitmapFromData` to create a stipple pattern for the root window background or window borders.

### 4.4 Titlebar Gradients
- **Idea:** Simple horizontal gradients for titlebars.
- **Retro Angle:** Mimic Windows 98 or early KDE/GNOME window decorations.
- **Implementation:** Draw lines of changing colors in `client_draw_title` instead of a solid rectangle.

### 4.5 Iconified Windows "Taskbar"
- **Idea:** A simple bar at the bottom of the screen showing iconified windows.
- **Retro Angle:** Windows 95 style taskbar.
- **Implementation:** The codebase currently has a window list (`wlist`). Enhancing this to be a persistent bar at the bottom with buttons for each window would fit the era.
