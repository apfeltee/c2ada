/* this file is in the public domain */

extern int printf(const char* fmt, ...);

static int dothething(int argc, char** argv)
{
    int i;
    const char* arg;
    if(argc > 1)
    {
        printf("commandline arguments:\n");
        for(i=1; i<argc; i++)
        {
            arg = argv[i];
            printf("  argv+%3d = \"%s\"\n", i, arg);
        }
    }
    printf("hello world!\n");
    return 0;
}

int main(int argc, char* argv[])
{
    return dothething(argc, argv);
}
