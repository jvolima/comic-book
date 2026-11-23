# Sample Comic Archives

This directory contains small, synthetically generated comic archives designed for automated and manual testing. All images are original, numbered, solid-colored placeholders created to avoid copyright issues while keeping the repository lightweight (each archive is under ~600 KB).

These samples cover both baseline functionality and common real-world edge cases found in digital comic collections (`.cbz` and `.cbr`).

## Test Files Overview

| File | Archive Format | Entries | Contents & Structure |
| :--- | :--- | :---: | :--- |
| **`basic.cbz`** | ZIP | 10 | 5 JPEG pages (`01.jpg` – `05.jpg`, all 1000×1540) plus 5 macOS metadata files (`__MACOSX/._01.jpg` – `__MACOSX/._05.jpg`). |
| **`basic.cbr`** | RAR 5 | 5 | The same 5 JPEG pages as `basic.cbz`, **without** the `__MACOSX/` files. |
| **`natural_sort.cbz`** | ZIP | 24 | 12 JPEG pages (`1.jpg` – `12.jpg`) named **without** leading zeros, plus 12 `__MACOSX/._*.jpg` metadata files. |
| **`mixed.cbz`** | ZIP | 9 | 2 JPEGs (`page_01.jpg`, `page_02.jpg`), 2 BMPs (`page_03.bmp`, `page_04.bmp`), `Thumbs.db`, `info.xml`, and a `subfolder/` directory entry containing `page_05.jpg` and `page_06.bmp`. |
| **`fake_extension.cbr`** | ZIP *(renamed)* | 10 | Byte-for-byte the same archive as `basic.cbz`, saved with a `.cbr` extension. |
| **`different_sizes.cbz`** | ZIP | 4 | 2 JPEG pages with mismatched dimensions (`02.jpg` is 800×768, `05.jpg` is 1024×768), plus 2 `__MACOSX/._*.jpg` metadata files. |
| **`corrupt_entry.cbz`** | ZIP *(stored, no compression)* | 2 | `01.jpg` (intact, 44680 bytes) and `02.jpg`, which has **one byte flipped** in the middle of its data. The archive opens and lists normally; only the CRC check reveals that `02.jpg` is corrupt. |
| **`truncated.cbz`** | ZIP *(broken)* | – | The first half of `different_sizes.cbz`. It starts with the ZIP signature, so `matches()` accepts it, but the central directory at the end is missing, so it cannot be opened. |

### About the `__MACOSX/` entries

The ZIP samples were created on macOS, which adds a hidden `__MACOSX/._<name>` file (AppleDouble metadata, a few hundred bytes) next to each real file. Real-world comic archives often contain them too, so they are kept on purpose:

- `Archive::listEntries()` must return them, since it lists the archive as it is (e.g. 10 entries for `basic.cbz`).
- `Book` is responsible for skipping them, along with other system files such as `Thumbs.db`, so `basic.cbz` and `basic.cbr` both end up with the same 5 pages.

## Verification

All sample files except `corrupt_entry.cbz` and `truncated.cbz` (which are broken on purpose, to test error handling) are valid archives and have been verified to open cleanly in standard desktop comic readers (such as **CDisplayEx** and **YACReader**).
