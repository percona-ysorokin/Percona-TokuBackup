/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ident "Copyright (c) 2012-2013 Tokutek Inc.  All rights reserved."
#ident "$Id$"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "backup.h"
#include "backup_test_helpers.h"

// Test for #6317 dest dir is not a directory.
const int expect_result = EINVAL; // 
int   error_count=0;
bool  ok=true;

static void expect_error(int error_number, const char *error_string, void *error_extra) {
    if (error_count!=0)              { ok=false; fprintf(stderr, "%s:%d error function called twice\n", __FILE__, __LINE__); }
    error_count++;
    if (error_number!=expect_result) { ok=false; fprintf(stderr, "%s:%d error_number=%d expected %d\n", __FILE__, __LINE__, error_number, expect_result); }
    if (error_string==NULL)          { ok=false; fprintf(stderr, "%s:%d expect error_string nonnull\n", __FILE__, __LINE__); }
    printf("error_string (expected)=%s\n", error_string);
    if (error_extra!=NULL)           { ok=false; fprintf(stderr, "%s:%d expect error_extra NULL\n", __FILE__, __LINE__); }
}
    
int test_main(int argc __attribute__((__unused__)), const char *argv[] __attribute__((__unused__))) {
    cleanup_dirs(); // remove destination dir
    setup_source();
    setup_dirs();
    {
        char *dst = get_dst();
        int fd = open(dst, O_CREAT | O_WRONLY | O_EXCL, 0777);
        assert(fd>=0);
        int r = close(fd);
        assert(r==0);
        free(dst);
    }
    // Dest dir is a file, not a directory
    pthread_t thread;
    start_backup_thread_with_funs(&thread, get_src(), get_dst(), simple_poll_fun, NULL, expect_error, NULL, expect_result);
    finish_backup_thread(thread);
    if (ok && error_count!=1) {
        ok=false;
        fprintf(stderr, "%s:%d expect error_count==1 but it is %d\n", __FILE__, __LINE__, error_count);
    }
    cleanup_dirs();
    if (ok) {
        pass();
    } else {
        fail();
    }
    printf(": test6317()\n");
    return 0;
}
