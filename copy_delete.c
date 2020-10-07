#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <unistd.h>
#include <utime.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int copy_files(const char* from, const char* to) {
  int status = 0;
  const int n = 256;
  struct dirent *entry = NULL;
  DIR *dir = NULL;
  DIR *sub_dir = NULL;
  FILE *fp = NULL;
  FILE *to_fp = NULL;
  unsigned int sz = 0;
  unsigned int sz_read = 0;
  unsigned int sz_write = 0;
  char* fdata = NULL;
  char* abs_path = NULL;
  char* to_abs_path = NULL;
  dir = opendir(from);
  if (dir) {
    abs_path = (char*)malloc(n*sizeof(char));
    if (abs_path == NULL) {
      status = errno;
      printf("Memory allocation error, error code %d\n", status);
      return status;
    }
    to_abs_path = (char*)malloc(n*sizeof(char));
    if (to_abs_path == NULL) {
      status = errno;
      printf("Memory allocation error, error code %d\n", status);
      return status;
    }

    while (entry = readdir(dir)) {
      sub_dir = NULL;
      fp = NULL;
      memset(abs_path, 0, n);
      memset(to_abs_path, 0, n);
      if ((entry->d_name[0] == '.' && strlen(entry->d_name) == 1) || ((strlen(entry->d_name) == 2) && (entry->d_name[0] == '.') && (entry->d_name[1] == '.')))
        continue;
      
      snprintf(abs_path, n, "%s/%s", from, entry->d_name);
      if (sub_dir = opendir(abs_path)) {
        closedir(sub_dir);
        printf("%s is directory!\n", abs_path);
        continue;
      }
      if (fp = fopen(abs_path, "r")) {
        fseek(fp, 0L, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        fdata = malloc(sizeof(char)*sz);
        if (!fdata) {
          status = errno;
          printf("Allocation for %d bytes failed wit herror %d!\n", sz, status);
          return status;
        }
        sz_read = fread(fdata, 1, sz, fp);
        if (sz != sz_read) {
          printf("File %s size is %d, but has been read %d \n", abs_path, sz, sz_read);
        }
        snprintf(to_abs_path, n, "%s/%s", to, entry->d_name);
        to_fp = fopen(to_abs_path, "w");
        if (!to_fp) {
          status = errno;
          printf("Can not create file %s, error %d\n", to_abs_path, status);
          return status;
        }
        sz_write = fwrite(fdata, 1, sz_read, to_fp);
        if (sz != sz_write) {
          printf("buffer size is %d, but has been written %d \n", sz_read, sz_write);
        }
        fclose(to_fp);
        fclose(fp);
        if (remove(abs_path)) {
          status = errno;
          printf("Can't remove file %s. Error code %d\n", abs_path, status);
        }
      }
    }
    free(abs_path);
    free(to_abs_path);
  }
  return status;
}

int main(int argc, char* argv[]) {
  int ret = 0;
  if (argc < 3) {
    printf("usage: tool <from_folder> <to_folder>\n");
    return 1;
  }
  ret = copy_files(argv[1], argv[2]);
  printf("copying finished with code %d\n", ret);
  return ret;
}
