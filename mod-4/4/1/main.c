/*
Problem inf-IV-04-1: fuse/mergefs
Реализуйте файловую систему, доступную только для чтения, которая строит объединение нескольких каталогов в одно дерево, по аналогии с UnionFS, по следующим правилам:

если встречаются два файла с одинаковым именем, то правильным считается тот, у которого более поздняя дата модификации;
если встречаются два каталога с одинаковым именем, то нужно создавать объединение их содержимого.
Внутри файловой системы могут быть только регулярные файлы с правами не выше 0444 и подкаталоги с правами не выше 0555.

Программа для реализации файловой системы должна поддерживать стандартный набор опций FUSE и опцию для указания списка каталогов для объединения --src КАТАЛОГ_1:КАТАЛОГ_2:...:КАТАЛОГ_N.

Используйте библиотеку FUSE версии 3.0 и выше. На сервер нужно отправить только исходный файл, который будет скомпилирован и слинкован с нужными опциями.
*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

#define FILES_NUM 4096
#define PATH 4096

typedef struct {
    int IsDir;
    int ErrorVal;
    struct stat MyStat;
    char Path[PATH];
} MyFileT;

typedef struct {
    char dirs[FILES_NUM][PATH];
    int dirs_num;
} FS_t;

static FS_t my_FS;

MyFileT Find(const char *path) {
    char Path[PATH];
    struct stat MyStat;
    MyFileT file;
    file.ErrorVal = ENOENT;
    file.IsDir = 0;
    int exists = 0;
    uint32_t i = 0;

    while (i < my_FS.dirs_num) {
        sprintf(Path, "%s%s", my_FS.dirs[i], path);
        if (0 == stat(Path, &MyStat)) {
            if (MyStat.st_mtime > file.MyStat.st_mtime || 0 == exists) {
                file.ErrorVal = 0;
                file.IsDir = (0 != (MyStat.st_mode & S_IFDIR));
                sprintf(file.Path, "%s", Path);
                file.MyStat = MyStat;
                exists = 1;
            }
        } else if (0 == exists) {
            file.ErrorVal = errno;
        }
        i++;
    }
    return file;
}

int Stat(const char *path, struct stat *st, struct fuse_file_info * info) {
    if (0 == strcmp("/", path)) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2;
        return 0;
    }

    MyFileT file = Find(path);

    if (file.ErrorVal) {
        return -file.ErrorVal;
    }

    if (file.IsDir) {
        *st = file.MyStat;
        st->st_mode = S_IFDIR | 0555;
    } else {
        *st = file.MyStat;
        st->st_mode = S_IFREG | 0444;
    }
    return 0;
}

int Check(char fl_dict[FILES_NUM][PATH], char *path) {
    int i = 0;
    while ( i < FILES_NUM) {
        if (!fl_dict[i][0]) {
            strcpy(fl_dict[i], path);
            return 1;
        } else if (strncmp(fl_dict[i], path, strlen(path) + 1) == 0) {
            return 0;
        }
        i++;
    }
    return 0;
}

int Readdir(const char *path, void *out, fuse_fill_dir_t filler, off_t off,
                   struct fuse_file_info *fi, enum fuse_readdir_flags fl) {
    char another_path[PATH];
    char fl_dict[FILES_NUM][PATH];
    strcpy(another_path, path);
    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);
    if (1 == strlen(another_path))
        another_path[0] = 0;
    uint32_t i = 0;
    while ( i < my_FS.dirs_num) {
        char cwd[PATH], base_path[PATH];
        getcwd(cwd, PATH);
        sprintf(base_path, "%s%s", my_FS.dirs[i], another_path);
        struct dirent *entity;
        DIR *dir;
        if (NULL != (dir = opendir(base_path))) {
            while (NULL != (entity = readdir(dir))) {
                if ('.' != entity->d_name[0] && Check  (fl_dict, entity->d_name)) {
                    filler(out, entity->d_name, NULL, 0, 0);
                }
            }
            closedir(dir);
        }
        i++;
    }
    return 0;
}

int Open(const char *path, struct fuse_file_info *fi) {
    MyFileT file = Find(path);

    if (( O_ACCMODE & fi->flags) != O_RDONLY) {
        return -EACCES;
    }

    int fd = open(file.Path, O_RDONLY);
    if (-1 == fd )
        return -errno;

    fi->fh = fd;
    return 0;
}

int Read(const char *path, char *out, size_t size, off_t off, struct fuse_file_info *info) {
    int res = read((int) info->fh, out, size);
    if (-1 != res) return res;
    return errno;
}

struct fuse_operations operations = {
        .readdir = Readdir,
        .getattr = Stat,
        .open    = Open,
        .read    = Read,
};

typedef struct {char *src;} cust_opts_t;
struct fuse_opt opt_specs[] = { {"--src %s", offsetof(cust_opts_t, src), 0}, {NULL, 0, 0}};
cust_opts_t cust_opts;

int main(int argc, char **argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, &cust_opts, opt_specs, NULL);

    char *ret = strtok(cust_opts.src, ":");
    if (NULL == ret) {
      puts("The string cannot be divided into parts");
      exit(1);
    }

    char cwd[PATH];
    getcwd(cwd, PATH);
    int count = 0;

    do {
        sprintf(my_FS.dirs[count], "%s/%s", cwd, ret);
        count += 1;
        ret = strtok(0, ":");
    } while (ret);
    my_FS.dirs_num = count;

    return fuse_main(args.argc, args.argv, &operations, NULL);;
}
