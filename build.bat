@echo off
REM Build without make (Windows + MinGW). Run from the project root.
gcc -Wall -Wextra -std=c99 -g -Isrc -o scanner.exe src\main.c src\hashtable.c src\editdistance.c src\ocr.c src\cleaner.c src\tokenizer.c src\profile.c src\analyser.c src\sort.c src\history.c src\util.c
if %errorlevel%==0 (echo Build OK. Run scanner.exe) else (echo Build FAILED)
