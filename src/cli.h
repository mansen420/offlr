#include <cstring>
#include <cstdio>
#include <cstdlib>

void cli_loop()
{
    char cmd[256], arg1[64], arg2[64], arg3[64];
    
    bool quit = false;
    while (!quit) 
    {
        printf("AiCo> ");
        fflush(stdout);
        
        if (!fgets(cmd, sizeof(cmd), stdin)) break;
        
        int nargs = sscanf(cmd, "%63s %63s %63s", arg1, arg2, arg3);
        if (nargs < 1) continue;
        
        if (strcmp(arg1, "render") == 0) {
            printf("\n");
        }
        else if (strcmp(arg1, "ls") == 0) {
            if (nargs == 1) {
                printf("ls T/Tracer | ls M/Material | ls S/Scene\n");
            }
            else if (strcmp(arg2, "T") == 0) {
                printf("T_path: Path tracing\nT_normal: Normal viz\n");
            }
        }
        else if (strcmp(arg1, "set") == 0 && strcmp(arg2, "spp") == 0) {
            printf("Samples set to %d\n", atoi(arg3));
        }
        else if (strcmp(arg1, "quit") == 0) {
            quit = true;
        }
        else {
            printf("Unknown command. Try 'ls' for help.\n");
        }
    }
}
