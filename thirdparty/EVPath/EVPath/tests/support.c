static char *ssh_args[6]={NULL, NULL, NULL, NULL, NULL, NULL};
static char remote_directory[1024] = "";
static char *argv0;
static int no_fork = 0;
#ifdef _MSC_VER
#define pid_t intptr_t
#include <process.h>
#endif

static void
usage()
{
    printf("Usage:  %s <options> \n", argv0);
    printf("  Options:\n");
    printf("\t-q  quiet\n");
    printf("\t-v  verbose\n");
    printf("\t-n  No regression test.  I.E. just run the master and print \n\t\twhat command would have been run for client.\n");
    printf("\t-ssh <hostname>:<ssh_port>:<remote directory>  parameters to use for remote client via ssh.\n");
    printf("\t-ssh <hostname>:<remote directory>  parameters to use for remote client via ssh.\n");

    exit(1);
}

#define PARSE_ARGS() \
    argv0 = argv[0];\
    while (argv[1] && (argv[1][0] == '-')) {\
	if (strcmp(&argv[1][1], "control") == 0) {	\
	    control = argv[2];\
	    argv++;\
	    argc--;\
	} else if (argv[1][1] == 'c') {		\
	    regression_master = 0;\
	} else if (strcmp(&argv[1][1], "ssh") == 0) {\
	    char *destination_host;\
	    char *first_colon, *second_colon;\
	    char *ssh_port = NULL;\
	    if (!argv[2]) {\
	        printf("Missing --ssh destination\n");\
		usage();\
	    }\
	    first_colon = strchr(argv[2], ':');\
	    if (first_colon) {\
	        *first_colon = 0;\
		second_colon = strchr(first_colon+1, ':');\
	    } else {\
	        second_colon = NULL;\
	    }\
	    destination_host = strdup(argv[2]);\
	    if (first_colon) {\
	        int ssh_port_int;\
		if (second_colon) *second_colon = 0;\
		if (sscanf(first_colon+1, "%d", &ssh_port_int) != 1) {\
		    second_colon = first_colon;\
		}  else {\
		    ssh_port = first_colon + 1;\
		}\
	    }\
	    if (second_colon) {\
	        strcpy(remote_directory, second_colon+1);\
	    }\
	    if (strlen(SSH_PATH) == 0) {\
		printf("SSH_PATH in config.h is empty!  Can't run ssh\n");\
		exit(1);\
	    }\
	    ssh_args[0] = strdup(SSH_PATH);\
	    ssh_args[1] = destination_host;\
	    ssh_args[2] = NULL;\
	    if (ssh_port != NULL) {\
	        ssh_args[2] = "-p";\
	        ssh_args[3] = ssh_port;\
		ssh_args[4] = NULL;\
	    }\
	    argv++; argc--;\
	} else if (argv[1][1] == 's') {\
	    regression_master = 0;\
	} else if (argv[1][1] == 'q') {\
	    quiet++;\
	} else if (argv[1][1] == 'v') {\
	    quiet--;\
	} else if (argv[1][1] == 'n') {\
	    regression = 0;\
	    no_fork = 1;\
	    quiet = -1;\
	} else if (argv[1][1] == 't') {\
	    transport = argv[2];\
	    argv++;\
	    argc--;\
	}\
	argv++;\
	argc--;\
    }

#ifdef _MSC_VER
static int inet_aton(const char* cp, struct in_addr* addr)
{
    addr->s_addr = inet_addr(cp);
    return (addr->s_addr == INADDR_NONE) ? 0 : 1;
}
#endif

pid_t
run_subprocess(char **args)
{
    char **run_args = args;
#ifdef HAVE_WINDOWS_H
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char comm_line[8191];

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    char module[MAX_PATH];
    GetModuleFileName(NULL, &module[0], MAX_PATH);
    int i = 1;
    strcpy(comm_line, module);
    strcat(comm_line, " ");
    while (args[i] != NULL) {
      strcat(comm_line, args[i]);
      strcat(comm_line, " ");
      i++;
      
    }
    if (!CreateProcess(module,
		       comm_line,
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
		       &pi ) 
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
	printf("Args were argv[0] = %s\n", args[0]);
	printf("Args were argv[1] = %s, argv[2] = %s\n", args[1], args[2]);
        return 0;
    }
    return (intptr_t) pi.hProcess;
#else
    pid_t child;
    if (quiet <=0) {printf("Forking subprocess\n");}
    if (ssh_args[0] != NULL) {
        int i=0, j=0;
        int count = 0;
	while(args[count] != NULL) count++;
	count+= 6; /* enough */
	run_args = malloc(count * sizeof(run_args[0]));
	while(ssh_args[i]) {
	    run_args[i] = ssh_args[i];
	    i++;
	}
	if (remote_directory[0] != 0) {
	  if (strrchr(argv0, '/')) argv0 = strrchr(argv0, '/') + 1;
	  run_args[i] = malloc(strlen(remote_directory) + 
			       strlen(argv0) + 4);
	  strcpy(run_args[i], remote_directory);
	  if (remote_directory[strlen(remote_directory)-1] != '/')
	    strcat(run_args[i], "/");
	  strcat(run_args[i], argv0);
	} else {
	  run_args[i] = argv0;
	}
	i++;
	while(args[j+1]) {
	    run_args[i] = args[j+1];
	    i++; j++;
	}
	run_args[i] = NULL;
    } else {
        run_args[0] = argv0;
    }
    if (quiet <= 0) {
        int i=0;
	printf("Subproc arguments are: ");
	while(run_args[i]) {
	    printf("%s ", run_args[i++]);
	}
	printf("\n");
    }
    if (no_fork) {
	child = -1;
    } else {
        child = fork();
	if (child == 0) {
	    /* I'm the child */
	    execv(run_args[0], run_args);
	}
    }
    return child;
#endif
}

