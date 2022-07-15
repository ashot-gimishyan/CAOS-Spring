/*
Problem inf-IV-04-2: fuse/unzipfs
Реализуйте файловую систему, доступную только для чтения, которая обеспечивает доступ к содержимому ZIP-архива.

Внутри файловой системы могут быть только регулярные файлы с правами 0444 и подкаталоги с правами 0555.

Программа для реализации файловой системы должна поддерживать стандартный набор опций FUSE и опцию для указания имени файла с образом файловой системы --src ИМЯ_ФАЙЛА.zip.

Используйте библиотеку FUSE версии 3.0 и выше, и библиотеку libzip. На сервер нужно отпарвить только исходный файл, который будет скомпилирован и слинкован с нужными опциями.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stddef.h>
#include <zip.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

#ifndef FUSEZIP_H
#define FUSEZIP_H

enum file_t
{
    ZIP_INVALID = -1,
    ZIP_FOLDER,
    ZIP_FILE,
};

#endif

static zip_t* ziparchive;
static char* zipname;

void open_filesystem(char *file_name) {
    zipname = file_name;
    ziparchive = zip_open(zipname, 0, NULL); // open zip file
    if (!ziparchive)
    {
        printf("Error opening file\n");
        return;
    }
   }

static char* append_slash(const char* path)
{
    char* search = malloc(strlen(path) + 2);
    strcpy(search, path);
    search[strlen(path)] ='/';
    search[strlen(path) + 1] = 0;

    return search;
}

static enum file_t fzip_file_type(const char* path)
{
    if (strcmp(path, "/") == 0)
        return ZIP_FOLDER;

    char* slash = append_slash(path + 1);
    int r1 = zip_name_locate(ziparchive, slash, 0);
    int r2 = zip_name_locate(ziparchive, path + 1, 0);

    free(slash);

    if (r1 != -1)
        return ZIP_FOLDER;
    else if (r2 != -1)
        return ZIP_FILE;
    else
        return ZIP_INVALID;
}

static int fzip_getattr(const char *path, struct stat *stbuf, struct fuse_file_info * inf)
{
    // printf("getting attr: %s\n", path);

    if (strcmp(path, "/") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
        stbuf->st_size = 1;
        return 0;
    }

    zip_stat_t sb;
    zip_stat_init(&sb);

    char* slash = append_slash(path + 1);

    switch (fzip_file_type(path))
    {
    case ZIP_FILE:
        zip_stat(ziparchive, path + 1, 0, &sb);
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = sb.size;
        stbuf->st_mtime = sb.mtime;
        break;
    case ZIP_FOLDER:
        zip_stat(ziparchive, slash, 0, &sb);
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
        stbuf->st_size = 0;
        stbuf->st_mtime = sb.mtime;
        break;
    default:
        free(slash);
        return -ENOENT;
    }

    free(slash);
    return 0;
}

static int fzip_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info* fi, enum fuse_readdir_flags fl)

{
    // printf("readdir: %s, offset = %lu\n", path, offset);

    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    for (int i = 0; i < zip_get_num_entries(ziparchive, 0); i++)
    {
        struct stat st;
        memset(&st, 0, sizeof(st));
        zip_stat_t sb;
        zip_stat_index(ziparchive, i, 0, &sb);

        char* zippath = malloc(strlen(sb.name) + 2);
        *zippath = '/';
        strcpy(zippath + 1, sb.name);

        char* dpath = strdup(zippath);
        char* bpath = strdup(zippath);

        if (strcmp(path, dirname(dpath)) == 0)
        {
            if (zippath[strlen(zippath) - 1] == '/') zippath[strlen(zippath) - 1] = 0;
            fzip_getattr(zippath, &st, 0);
            char* name = basename(bpath);
            if (filler(buf, name, &st, 0,0))
                break;
        }

        free(zippath);
        free(dpath);
        free(bpath);
    }

    return 0;
}

static int fzip_open(const char *path, struct fuse_file_info *fi)
{
    // printf("open: %s\n", path);

    (void) fi;

    if(zip_name_locate(ziparchive, path + 1, 0) < 0)
        return -ENOENT; // some error that says the file does not exist

    return 0;
}

static int fzip_read(const char *path, char *buf, size_t size,
                     off_t offset, struct fuse_file_info* fi)
{
    // printf("read: %s offset: %lu\n", path, offset);
    int res;
    (void) fi;

    zip_stat_t sb;
    zip_stat_init(&sb);

    zip_stat(ziparchive, path + 1, 0, &sb);
    zip_file_t* file = zip_fopen(ziparchive, path + 1, 0);

    char temp[sb.size + size + offset];
    memset(temp, 0, sb.size + size + offset);

    res = zip_fread(file, temp, sb.size);

    if (res == -1)
        return -ENOENT;

    memcpy(buf, temp + offset, size);
    zip_fclose(file);

    return size;
}

static int fzip_mkdir(const char *path, mode_t mode)
{
    // printf("mkdir: %s\n", path);

    (void) mode;

    zip_dir_add(ziparchive, path + 1, 0);

    zip_close(ziparchive); // we have to close and reopen to write the changes
    ziparchive = zip_open(zipname, 0, NULL);

    return 0;
}

static int fzip_rename(const char *from, const char *to, unsigned int suc)
{
    // printf("rename: %s to %s \n", from, to);

    zip_int64_t index = zip_name_locate(ziparchive, from + 1, 0);
    if (zip_file_rename(ziparchive, index, to + 1, 0)  == -1)
        return -ENOENT;

    zip_close(ziparchive); // we have to close and reopen to write the changes
    ziparchive = zip_open(zipname, 0, NULL);
    return 0;
}

static int fzip_truncate(const char *path, off_t size, struct fuse_file_info * inf)
{
    // printf("truncate: %s size: %ld\n", path, size);

    char tbuf[size];
    memset(tbuf, 0, size);
    if (zip_name_locate(ziparchive, path + 1, 0) > 0)
    {
        zip_file_t *f = zip_fopen(ziparchive, path+1, 0);
        zip_fread(f, tbuf, size);
        zip_fclose(f);
    }

    zip_source_t *s;

    if ((s = zip_source_buffer(ziparchive, tbuf, size, 0)) == NULL ||
            zip_file_add(ziparchive, path+1, s, ZIP_FL_OVERWRITE) < 0)
    {
        zip_source_free(s);
        // printf("Error adding file %s\n", path);
        return 0;
    }

    zip_close(ziparchive); // we have to close and reopen to write the changes
    ziparchive = zip_open(zipname, 0, NULL);

    return 0;
}

static int fzip_write(const char* path, const char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    // printf("write: %s buf: %s, size: %ld offset: %ld\n", path, buf, size, offset);

    (void) fi;

    zip_source_t *s;
    zip_stat_t sb;
    zip_stat_init(&sb);
    zip_stat(ziparchive, path + 1, 0, &sb);

    char tbuf[sb.size + size + offset];
    memset(tbuf, 0, sb.size + size + offset);
    zip_file_t *f = zip_fopen(ziparchive, path + 1, 0);
    if (f != NULL)
    {
        zip_fread(f, tbuf, sb.size);
        zip_fclose(f);
    }
    memcpy(tbuf + offset, buf, size);

    if ((s = zip_source_buffer(ziparchive, tbuf, sb.size + size + offset, 0)) == NULL ||
            zip_file_add(ziparchive, path + 1, s, ZIP_FL_OVERWRITE) < 0)
    {
        zip_source_free(s);
        // printf("Error adding file %s\n", path);
        return 0;
    }

    zip_close(ziparchive); // we have to close and reopen to write the changes
    ziparchive = zip_open(zipname, 0, NULL);

    return size;
}

static int fzip_mknod(const char* path, mode_t mode, dev_t rdev)
{
    // printf("mknod: %s mode: %u\n", path, mode);

    (void) rdev;

    if ((mode & S_IFREG) == S_IFREG)
    {
        zip_source_t *s;

        if ((s = zip_source_buffer(ziparchive, NULL, 0, 0)) == NULL ||
                zip_file_add(ziparchive, path + 1, s, ZIP_FL_OVERWRITE) < 0)
        {
            zip_source_free(s);
            // printf("Error adding file %s\n", path);
            return 0;
        }
    }

    return 0;
}

static int fzip_unlink(const char* path)
{
    // printf("unlink: %s\n", path);

    int ret = zip_delete(ziparchive, zip_name_locate(ziparchive, path + 1, 0));
    zip_close(ziparchive); // we have to close and reopen to write the changes
    ziparchive = zip_open(zipname, 0, NULL);
    return ret;
}

static int fzip_rmdir(const char *path)
{
    // printf("rmdir: %s\n", path);
    char* folder = append_slash(path);
    fzip_unlink(folder);
    free(folder);

    return 0;
}

static int fzip_access(const char* path, int mask)
{
    // printf("access: %s\n", path);

    (void) mask;

    if (fzip_file_type(path) >= 0)
        return 0;

    return -ENOENT;
}

static int fzip_utimens(const char* path, const struct timespec ts[2], struct fuse_file_info * inf)
{
    // printf("utimens: %s mtime: %ld\n", path, ts[1].tv_sec);

    int i;
    int ret;

    if ((i = zip_name_locate(ziparchive, path + 1, 0)) < 0)
    {
        char* slash = append_slash(path);
        i = zip_name_locate(ziparchive, slash + 1, 0);
        free(slash);
    }
    ret = zip_file_set_mtime(ziparchive, i, ts[1].tv_sec, 0);
    zip_close(ziparchive); // we have to close and reopen to write the changes
    ziparchive = zip_open(zipname, 0, NULL);

    return ret;
}

static void fzip_destroy(void* private_data)
{
    (void) private_data;

    zip_close(ziparchive);
}

static struct fuse_operations fzip_oper =
{
    .access         = fzip_access,
    .getattr        = fzip_getattr,
    .readdir        = fzip_readdir,
    .open           = fzip_open,
    .read           = fzip_read,
    .mkdir          = fzip_mkdir,
    .mknod          = fzip_mknod,
    .rename         = fzip_rename,
    .truncate       = fzip_truncate,
    .write          = fzip_write,
    .unlink         = fzip_unlink,
    .rmdir          = fzip_rmdir,
    .destroy        = fzip_destroy,
    .utimens        = fzip_utimens,
};

typedef struct { char *src; } my_options_t;
my_options_t my_options;
struct fuse_opt opt_specs[] = { { "--src %s", offsetof(my_options_t, src), 0 }, { NULL, 0, 0}};


int main(int argc, char *argv[])
{
 	struct fuse_args arguments = FUSE_ARGS_INIT(argc, argv);
	fuse_opt_parse(&arguments, &my_options, opt_specs, NULL);
	open_filesystem(my_options.src);

	return fuse_main(arguments.argc, arguments.argv, &fzip_oper, NULL);
}
