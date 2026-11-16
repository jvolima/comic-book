# Sample Comic Archives

This directory contains small, synthetically generated comic archives designed for automated and manual testing. All images are original, numbered, solid-colored placeholders created to avoid copyright issues while keeping the repository lightweight (each archive is under ~500 KB).

These samples cover both baseline functionality and common real-world edge cases found in digital comic collections (`.cbz` and `.cbr`).

## Test Files Overview

| File | Archive Format | Contents & Structure |
| :--- | :--- | :--- |
| **`basic.cbz`** | ZIP | 5 JPEG images (`01.jpg` – `05.jpg`) with uniform dimensions. |
| **`basic.cbr`** | RAR | The exact same 5 JPEG images (`01.jpg` – `05.jpg`) as `basic.cbz`. |
| **`natural_sort.cbz`** | ZIP | 12 JPEG images (`1.jpg` – `12.jpg`) named **without** leading zeros. |
| **`mixed.cbz`** | ZIP | JPEGs, BMPs, `Thumbs.db`, `info.xml`, and a `subfolder/` containing additional images. |
| **`fake_extension.cbr`** | ZIP *(renamed)* | Standard ZIP archive containing JPEG pages, saved with a `.cbr` extension. |
| **`different_sizes.cbz`** | ZIP | 2 JPEG pages with slightly mismatched pixel dimensions. |

## Verification

All sample files are valid archives and have been verified to open cleanly in standard desktop comic readers (such as **CDisplayEx** and **YACReader**).