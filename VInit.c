#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>

#define FS_BLOCK_SIZE 1024     // 文件系统块的大小
#define SUPER_BLOCK 1          // 超级块的块号
#define BITMAP_BLOCK 14        // 位图块
#define ROOT_DIR_BLOCK 15      // 根目录块的块号
#define MAX_DATA_IN_BLOCK 1016 // 每个块中用于存储数据的最大字节数（减去size_t和long inode_number各占4个字节）
#define MAX_DIR_IN_BLOCK 8     // 每个块中存储的最大目录项数
#define MAX_FILENAME 24        // 文件名的最大长度（不包括扩展名）
#define MAX_EXTENSION 8        // 扩展名的最大长度

// 超级块中记录的，大小为 32 bytes（4个long）
struct super_block
{
    long fs_size;        // 文件系统大小（以块为单位）
    long inode_start;    // inode的起始块位置
    long bitmap_start;   // 位图的起始块位置
    long root_dir_start; // 根目录起始块位置
};

// inode结构体，大小为 32 bytes
struct inode
{
    int flag;            // 文件类型 0: unused; 1: file; 2: directory
    size_t fsize;        // 文件大小
    long direct_block;   // 直接块号
    long indirect_block; // 间接块号
};

// 文件内容存放用到的数据结构，大小为 1024 bytes
struct data_block
{
    size_t size;                  // 文件的数据部分使用了这个块里面的多少Bytes
    long inode_number;            // 该数据块对应的inode编号
    char data[MAX_DATA_IN_BLOCK]; // 实际使用空间
};

// 记录文件信息的数据结构，统一存放在目录文件里面，也就是说目录文件里面存的全部都是这个结构
struct file_directory
{
    char fname[MAX_FILENAME]; // 文件名
    char fext[MAX_EXTENSION]; // 扩展名
    size_t fsize;             // 文件大小
    long inode_number;        // 目录项对应的inode编号
    int flag;                 // 文件类型 0: unused; 1: file; 2: directory
};

int main()
{
    FILE *fp = NULL;
    fp = fopen("./vdisk", "r+"); // 打开文件
    if (fp == NULL)
    {
        printf("打开文件失败，文件不存在\n");
        return 0;
    }

    // 1. 初始化super_block, 大小：1块
    struct super_block *super_blk = malloc(sizeof(struct super_block));
    super_blk->fs_size = 102400;
    super_blk->inode_start = 2;     // inode区的起始块位置
    super_blk->bitmap_start = 14;   // 位图区的起始块位置
    super_blk->root_dir_start = 15; // 根目录区的起始块位置
    fwrite(super_blk, sizeof(struct super_block), 1, fp);
    printf("Initialized super_block successfully!\n");

    // 2. 初始化inode区
    fseek(fp, FS_BLOCK_SIZE * (super_blk->inode_start), SEEK_SET);
    struct inode *root_inode = malloc(sizeof(struct inode));
    root_inode->flag = 2;                      // 标记为目录
    root_inode->fsize = 0;                     // 根目录的大小为0
    root_inode->direct_block = ROOT_DIR_BLOCK; // 根目录的直接块号为ROOT_DIR_BLOCK
    root_inode->indirect_block = -1;           // 根目录没有间接块
    fwrite(root_inode, sizeof(struct inode), 1, fp);
    printf("Initialized inode successfully!\n");

    // 3. 初始化位图区
    fseek(fp, FS_BLOCK_SIZE * (super_blk->bitmap_start), SEEK_SET);
    char *bitmap = calloc(super_blk->fs_size, sizeof(char));
    fwrite(bitmap, super_blk->fs_size, 1, fp);
    printf("Initialized bitmap successfully!\n");

    // 4. 初始化根目录
    fseek(fp, FS_BLOCK_SIZE * (super_blk->root_dir_start), SEEK_SET);
    struct file_directory *root_dir = malloc(sizeof(struct file_directory));
    strcpy(root_dir->fname, "/"); // 根目录名
    root_dir->fext[0] = '\0';     // 根目录扩展名为空
    root_dir->fsize = 0;          // 根目录大小为0
    root_dir->inode_number = 0;   // 根目录的inode编号为0
    root_dir->flag = 2;           // 标记为目录
    fwrite(root_dir, sizeof(struct file_directory), 1, fp);
    printf("Initialized root directory successfully!\n");

    // 释放内存并关闭文件
    free(super_blk);
    free(root_inode);
    free(bitmap);
    free(root_dir);
    fclose(fp);

    return 0;
}
