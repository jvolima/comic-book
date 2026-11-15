# Comic Book Reader/Writer

[![Build](https://github.com/jvolima/comic-book/actions/workflows/build.yml/badge.svg)](https://github.com/jvolima/comic-book/actions/workflows/build.yml)

Comic Book Reader/Writer is a desktop application for reading, organizing and creating digital comic books in the `.cbz` and `.cbr` formats. It focuses on a smooth reading experience, with asynchronous loading, page prefetching and image filters that keep text sharp. Its architecture is designed so that new formats and image filters can be added easily.

## Features

### Reading
- **Comic archive support:** opens `.cbz` (ZIP) and `.cbr` (RAR) files.
- **Image formats:** JPEG, BMP and PNG.
- **Asynchronous opening:** start reading the first pages while the rest of the book is still loading.
- **Natural navigation:** next, previous, first and last page, using buttons or keyboard shortcuts.
- **Fit to screen:** pages are automatically resized to the window.
- **Two-page spread:** two pages are shown side by side. Pages with slightly different sizes are scaled so both look like the same format.
- **Zoom:** zoom in and out, plus *fit page* and *fit width* modes.
- **Prefetching:** the next and previous pages are decoded and resized in the background, so turning a page is instant.

### Image quality
- **Selectable resize filters:** choose a filter suited to the content, either sharper for text or smoother for artwork.
- **Anti-aliasing:** a low-pass filter is applied when downscaling to prevent jagged edges and blurry lettering.
- **Text enhancement:** sharpening filters reduce JPEG compression artifacts around lettering.
- **Image cache:** rendered pages are kept in an LRU cache with a memory limit, so they are not recomputed.

### Authoring
- **Extract pages** from an existing book to a folder.
- **Create a book** by assembling images or pages from other books into a new `.cbz` file.
- **Table of contents:** add, edit and browse a table of contents. It is stored as a `toc.xml` file inside the archive, so the book stays compatible with other readers.

### Library
- **Recent books:** remembers the last page read in each book.
- **Several books in progress** at the same time.
- **Tags** for classifying books by genre or style.
- **Library view** with a tag filter.

## How to build

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.16 or higher
- Qt 6 (`Widgets` module) — optional if building only the core library

### Build & Run

```bash
cmake -S . -B build
cmake --build build
./build/comic_reader
```

> **Note:** If Qt 6 is installed in a custom directory (for example via the Qt Online Installer or Homebrew on macOS), pass its path using `-DCMAKE_PREFIX_PATH`:
> ```bash
> cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x.x/macos"
> ```

To build only the `comic_core` library without the Qt interface:

```bash
cmake -S . -B build -DBUILD_UI=OFF
cmake --build build
```

## Roadmap

- [ ] **M0 - Setup & Analysis**
- [ ] **M1 - Core: Reading a Book**
- [ ] **M2 - Basic User Interface**
- [ ] **M3 - Async Loading & Performance**
- [ ] **M4 - Filters & Image Quality**
- [ ] **M5 - Display Modes**
- [ ] **M6 - Writer & Table of Contents**
- [ ] **M7 - Library**
- [ ] **M8 - Extras**

Progress is tracked in the repository's [milestones](../../milestones) and [issues](../../issues).
