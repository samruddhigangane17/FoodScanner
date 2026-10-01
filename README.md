# Food Label Scanner

Reads the ingredient list from a food-packet photo (OCR), explains every ingredient in
simple language, and warns people with allergies or diets (nuts, dairy, gluten, soy,
vegetarian, diabetic). DSA mini project written in C.

## DSA used
| Task | Data structure / algorithm | File |
|---|---|---|
| Ingredient database lookup | Hash table with chaining (djb2) | `hashtable.c` |
| Fixing OCR spelling mistakes | Edit distance (dynamic programming) | `editdistance.c` |
| Splitting text into ingredients | String tokenizing with bracket handling | `tokenizer.c` |
| User restrictions + warnings | Bitmask and bitwise AND | `profile.c`, `analyser.c` |
| Warning order | Merge sort (stable) | `sort.c` |
| Scan history | Singly linked list + file | `history.c` |

## Requirements
- GCC (MinGW-w64 on Windows)
- Tesseract OCR on PATH (only needed for image scanning; typing ingredients works without it)
  - Windows: UB-Mannheim installer, then add its folder to PATH
  - Linux: `sudo apt install tesseract-ocr`

## Build and run (always from the project root folder)
```
make            # builds ./scanner
./scanner       # Windows: scanner.exe
make test       # runs automated tests
```
No `make`? On Windows run `build.bat`.

## Project layout
```
src/    all C source files (see ownership below)
data/   ingredients.csv (database); profile.txt and history.txt are created at run time
images/ sample label photos for testing
tests/  test_core.c
docs/   project report and execution plan
```

## Adding ingredients
Edit `data/ingredients.csv`, one line each: `name,tags,explanation`
`tags` = sum of: nuts 1, dairy 2, gluten 4, soy 8, non-veg 16, sugar 32
Example: soy sauce contains soy and gluten -> 8 + 4 = 12.

## Team and file ownership
| Member | Files |
|---|---|
| A - data and lookup | `hashtable.c`, `editdistance.c`, `data/ingredients.csv` |
| B - input pipeline | `ocr.c`, `cleaner.c`, `tokenizer.c`, `images/`, `Makefile`, `build.bat` |
| C - analysis and integration | `common.h`, `profile.c`, `analyser.c`, `sort.c`, `history.c`, `util.c`, `main.c` |

`common.h` is the shared contract. Change it only after discussing with the team.

## Git workflow
```
git checkout member-a          # your own branch (member-b / member-c)
git add .
git commit -m "what you did"
git pull origin main
git push origin member-a
```
Merge into `main` only after `make test` passes.

## Limitations
- OCR quality depends on the photo (flat, well lit, only the ingredients section).
- The database has about 190 entries; unknown ingredients are reported, not guessed.
- "May contain traces" statements are treated as ingredients to stay on the safe side.
- Informational only - always read the real label; consult a doctor for severe allergies.
