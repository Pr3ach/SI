@echo off
windres -i res/resource.rc -O coff -o res/resource.res
gcc src/core.c src/init.c src/main.c src/option.c res/resource.res -lcomctl32 -lcomdlg32 -lgdi32 -mwindows -o build/Storm_Inj3ctor.exe
del res/resource.res

