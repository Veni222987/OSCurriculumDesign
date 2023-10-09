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
#define ROOT_DIR_BLOCK 1       // 根目录块的块号
#define MAX_DATA_IN_BLOCK 1016 // 每个块中用于存储数据的最大字节数（减去size_t和long nNextBlock各占4个字节）
#define MAX_DIR_IN_BLOCK 8     // 每个块中存储的最大目录项数
#define MAX_FILENAME 24        // 文件名的最大长度（不包括扩展名）
#define MAX_EXTENSION 8        // 扩展名的最大长度
// 超级块中记录的，大小为 24 bytes（3个long），占用1块磁盘块
struct super_block
{
    long fs_size;   // （文件系统大小,以块为单位）
    long first_blk; // f（根目录的起始块位置，以块为单位）
    long bitmap;    //(位图大小以块为单位）
};
// 记录文件信息的数据结构,统一存放在目录文件里面，也就是说目录文件里面存的全部都是这个结构
struct file_directory
{
    char fname[MAX_FILENAME]; // 文件名
    char fext[MAX_EXTENSION]; // 扩展名
    size_t fsize;             // 文件大小
    long nStartBlock;         // 目录开始块位置
    int flag;                 // 文件类型 0: unused; 1: file; 2: directory
};
// 文件内容存放用到的数据结构，大小为 1024 bytes，占用1块磁盘块
struct data_block
{
    size_t size;                  // 文件的数据部分使用了这个块里面的多少Bytes
    long nNextBlock;              // （该文件太大了，一块装不下，所以要有下一块的地址）   long的大小为4Byte
    char data[MAX_DATA_IN_BLOCK]; // 实际使用空间
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
    struct super_block *super_blk = malloc(sizeof(struct super_block)); // 动态内存分配，申请super_blk
    super_blk->fs_size = 102400;
    super_blk->first_blk = 15; // 根目录的data_block在15编号的块中（从0开始编号）
    super_blk->bitmap = BITMAP_BLOCK;
    fwrite(super_blk, sizeof(struct super_block), 1, fp);
    printf("initial super_block success!\n");

    // 2. 初始化bitmap,将已使用的部分置1，未使用部分置0
    if (fseek(fp, FS_BLOCK_SIZE * 1, SEEK_SET) != 0) // 首先要将指针移动到文件的第二块的起始位置
        fprintf(stderr, "bitmap fseek failed!\n");
    int userBit[4]; // used 16=4*sizeof(int)
    memset(userBit, -1, sizeof(userBit));
    fwrite(userBit, sizeof(userBit), 1, fp);

    // 最后要将bitmap剩余的块全部置0，用int数组设置，一个int为4byte
    int rest_of_bitmap = 102400 - 16;
    int d[rest_of_bitmap];
    memset(d, 0, sizeof(d));
    fwrite(d, sizeof(d), 1, fp);
    printf("initial bitmap_block success!\n");
    // 3. 初始化data_block
    fseek(fp, FS_BLOCK_SIZE * (BITMAP_BLOCK + 1), SEEK_SET);
    struct data_block *root = malloc(sizeof(struct data_block));
    root->size = 0;
    root->nNextBlock = -1;
    root->data[0] = '\0';
    fwrite(root, sizeof(struct data_block), 1, fp); // 写入磁盘，初始化完成
    printf("initial data_block success!\n");
    fclose(fp);
    printf("super_bitmap_data_blocks init success!\n");
}
