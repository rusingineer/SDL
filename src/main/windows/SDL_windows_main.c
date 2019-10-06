/*
    SDL_windows_main.c, placed in the public domain by Sam Lantinga  4/13/98

    The WinMain function -- calls your program's main() function
*/
#include "SDL_config.h"

#ifdef __WIN32__

/* Include this so we define UNICODE properly */
#include "../../core/windows/SDL_windows.h"
#include <shellapi.h> /* CommandLineToArgvW() */

/* Include the SDL main definition header */
#include "SDL.h"
#include "SDL_main.h"

#ifdef main
#  undef main
#endif /* main */

static void
UnEscapeQuotes(char *arg)
{
    char *last = NULL;

    while (*arg) {
        if (*arg == '"' && (last != NULL && *last == '\\')) {
            char *c_curr = arg;
            char *c_last = last;

            while (*c_curr) {
                *c_last = *c_curr;
                c_last = c_curr;
                c_curr++;
            }
            *c_last = '\0';
        }
        last = arg;
        arg++;
    }
}

/* Parse a command line buffer into arguments */
static int
ParseCommandLine(char *cmdline, char **argv)
{
    char *bufp;
    char *lastp = NULL;
    int argc, last_argc;

    argc = last_argc = 0;
    for (bufp = cmdline; *bufp;) {
        /* Skip leading whitespace */
        while (*bufp == ' ' || *bufp == '\t') {
            ++bufp;
        }
        /* Skip over argument */
        if (*bufp == '"') {
            ++bufp;
            if (*bufp) {
                if (argv) {
                    argv[argc] = bufp;
                }
                ++argc;
            }
            /* Skip over word */
            lastp = bufp;
            while (*bufp && (*bufp != '"' || *lastp == '\\')) {
                lastp = bufp;
                ++bufp;
            }
        } else {
            if (*bufp) {
                if (argv) {
                    argv[argc] = bufp;
                }
                ++argc;
            }
            /* Skip over word */
            while (*bufp && (*bufp != ' ' && *bufp != '\t')) {
                ++bufp;
            }
        }
        if (*bufp) {
            if (argv) {
                *bufp = '\0';
            }
            ++bufp;
        }

        /* Strip out \ from \" sequences */
        if (argv && last_argc != argc) {
            UnEscapeQuotes(argv[last_argc]);
        }
        last_argc = argc;
    }
    if (argv) {
        argv[argc] = NULL;
    }
    return (argc);
}
#define WIN_WStringToUTF8(S) SDL_iconv_string("UTF-8", "UTF-16LE", (char *)(S), (SDL_wcslen(S)+1)*sizeof(WCHAR))

/* Pop up an out of memory message, returns to Windows */
static BOOL
OutOfMemory(void)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", "Out of memory - aborting", NULL);
    return FALSE;
}

#if defined(_MSC_VER)
/* The VC++ compiler needs main/wmain defined */
# define console_ansi_main main
# if UNICODE
#  define console_wmain wmain
# endif
#endif

/* Gets the arguments with GetCommandLine, converts them to argc and argv
   and calls SDL_main */
static int
main_getcmdline(void)
{
    LPWSTR *argvw;
    char **argv;
    int i, argc, result;

    argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argvw == NULL) {
        return OutOfMemory();
    }

    /* Parse it into argv and argc */
    argv = (char **)SDL_calloc(argc + 1, sizeof(*argv));
    if (!argv) {
        return OutOfMemory();
    }
    for (i = 0; i < argc; ++i) {
        argv[i] = WIN_WStringToUTF8(argvw[i]);
        if (!argv[i]) {
            return OutOfMemory();
        }
    }
    argv[i] = NULL;
    LocalFree(argvw);

    SDL_SetMainReady();

    /* Run the application main() code */
    result = SDL_main(argc, argv);

    /* Free argv, to avoid memory leak */
    for (i = 0; i < argc; ++i) {
        SDL_free(argv[i]);
    }
    SDL_free(argv);

    return result;
}

/* This is where execution begins [console apps, ansi] */
int
console_ansi_main(int argc, char *argv[])
{
    return main_getcmdline();
}


#if UNICODE
/* This is where execution begins [console apps, unicode] */
int
console_wmain(int argc, wchar_t *wargv[], wchar_t *wenvp)
{
    return main_getcmdline();
}
#endif

/* This is where execution begins [windowed apps] */
int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    return main_getcmdline();
}

#endif /* __WIN32__ */

/* vi: set ts=4 sw=4 expandtab: */
