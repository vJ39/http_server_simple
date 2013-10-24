#include "hss_process.h"

int hss_process_chroot() {
    if( chdir(HSS_DOCROOT) == -1 )
        return -1;
    if( chroot(HSS_DOCROOT) == -1 )
        return -1;
    setuid( (uid_t)HSS_PROCESS_UID );
    setgid( (gid_t)HSS_PROCESS_GID );
    return 0;
}

int hss_process_ignore_sigpipe() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    if(sigaction(SIGPIPE, &act, 0) == -1)
        return -1;
    return 0;
}

void hss_process_worker() {
    int i;
    for(i = 0; i < HSS_MAX_CHILDREN; i++) {
        if(fork() == 0) {
            while(1){
            }
        }
            hss_socket_(&s_sock);
    }
}
