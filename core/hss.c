#include "hss.h"

HSS_SOCK s_sock; // global server sock

int main(void){
    hss_socket_open();
    hss_process_chroot();
    hss_process_ignore_sigpipe();
    // hss_process_worker();
    return 0;
}
