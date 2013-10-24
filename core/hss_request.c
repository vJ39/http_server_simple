void parse_request(struct hss_req *req_ptr, struct hss_res *res_ptr) {
    // initialize response header struct
    *res_ptr = (struct hss_res) {"", "", ""};

    // 501 Not Implemented
    if( strcmp(req_ptr->method, "GET") != 0 ) {
        *res_ptr = (struct hss_res) {
            "501",
            "Not Implemented",
            "text/html"
        };
        req_ptr->fd = -1;
        return;
    }

    struct stat st, *st_ptr; st_ptr = &st;
    if( lstat(req_ptr->uri, st_ptr) == -1 ) {
        int errsv = errno;
        if(errsv == ENOENT) {
            *res_ptr = (struct hss_res) {
                "404",
                "Not Found",
                "text/html"
            };
        }
        else {
            *res_ptr = (struct hss_res) {
                "403",
                "Forbidden",
                "text/html"
            };
        }
        req_ptr->fd = -1;
        return;
    }
    // is a directory
    if( S_ISDIR(st_ptr->st_mode) ) {
        strncat(req_ptr->uri, "/index.html", strlen(req_ptr->uri) + 11);
        if( (req_ptr->fd = open(req_ptr->uri, O_RDONLY)) == -1) {
            *res_ptr = (struct hss_res) {
                "403",
                "Forbidden",
                "text/html"
            };
            req_ptr->fd = -1;
            return;
        }
        // 200 OK
        *res_ptr = (struct hss_res) {
            "200",
            "OK",
            "text/html"
        };
        return;
    }
    else {
        // open a regular file
        if( (req_ptr->fd = open(req_ptr->uri, O_RDONLY)) == -1) {
            *res_ptr = (struct hss_res) {
                "403",
                "Forbidden",
                "text/html"
            };
            req_ptr->fd = -1;
            return;
        }
        // 200 OK
        strncpy(res_ptr->status_code, "200", 3);
        strncpy(res_ptr->status_message, "OK", 2);
        setmimetype(req_ptr, res_ptr);
        return;
    }
}


