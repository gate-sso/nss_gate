//
// Created by ajey on 5/7/16.

#include "nss_http.h"
#include <nss.h>
#include <pwd.h>
#include "nss_http-passwd.c"

int main(int argc, char *argv[])
{
    int retval=0;
    const char *command="nobody";
    char host_name[255];
    char token[255];
    get_config_host(host_name, token);

    printf("host %s token %s\n", host_name, token);

    const char *name="dev1";
    struct passwd *structpasswd;
    structpasswd = malloc(sizeof(struct passwd));
    char * buffer = malloc(512);
    int error_no;

    enum nss_status status;
    status = _nss_http_getpwnam_r_locked(name, structpasswd, buffer, 512, &error_no);

    if ( status == NSS_STATUS_UNAVAIL || status == NSS_STATUS_TRYAGAIN)
    {
        printf("User not found \n");
    }
    if(argc == 3) {
        command = argv[1];
    }

    if(argc > 3 || argc < 3) {
        fprintf(stderr, "Usage: check_nss [user|group] [user or group name]\n");
        exit(1);
    }

    if (!strncmp(command, "group", strlen(command)))
    {

    }
    return retval;
}

//




