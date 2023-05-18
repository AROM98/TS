#define FUSE_USE_VERSION 31

#include <syslog.h> 
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static void log_message(const char *message) {
    openlog("HelloFUSE", LOG_PID, LOG_USER);  // Open connection to syslog
    syslog(LOG_INFO, "%s", message);  // Log the message using syslog
    closelog();  // Close connection to syslog
}

static int hello_getattr(const char *path, struct stat *stbuf,
                         struct fuse_file_info *fi)
{
    printf("getattr called\n");
    log_message("[RSYSLOG] getattr called");
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    } else
        res = -ENOENT;

    return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
    printf("readdir called\n");
    log_message("[RSYSLOG] readdir called");
    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, hello_path + 1, NULL, 0, 0);

    return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    printf("open called\n");
    log_message("[RSYSLOG] open called");
    if (strcmp(path, hello_path) != 0)
        return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    printf("read called\n");
    log_message("[RSYSLOG] read called");
    //verifcar o permissoes do utilizador
    // se tiver permissoes de leitura, faz, senão retorna -EACCES
    size_t len;
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;

    len = strlen(hello_str);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, hello_str + offset, size);
    } else
        size = 0;

    return size;
}

static struct fuse_operations hello_oper = {
    .getattr    = hello_getattr,
    .readdir    = hello_readdir,
    .open       = hello_open,
    .read       = hello_read,
};

int main(int argc, char *argv[])
{   
    // Open connection to the system logger
    //openlog("HelloFUSE", LOG_PID | LOG_CONS, LOG_USER);

    // Create a new arguments array with the -f option, to run in foreground
    char *argv_mod[] = { argv[0], argv[1], "-f", NULL };

    log_message("Starting hello FUSE program");

    // Run the FUSE filesystem in foreground and log messages to syslog
    int ret = fuse_main(3, argv_mod, &hello_oper, NULL);

    // Close the connection to the system logger
    log_message("Hello FUSE program finished");
    closelog();

    return ret;
}
