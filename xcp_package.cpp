// xcp_package.cpp
 
#include "stdafx.h"
 
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
 
#define XCP_FILE "C:\\halo\\reach.xcp"
 
 
#define CHUNK 16384
 
__int64 lastStream=0;
 
/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
 
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
 
    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
 
        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
 
        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
    lastStream+=strm.total_in;
    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
 
/* report a zlib or i/o error */
void zerr(int ret)
{
    printf("zpipe: ");
    switch (ret) {
    case Z_ERRNO:
            printf("error reading stdin\n");
        break;
    case Z_STREAM_ERROR:
        printf("invalid compression level\n");
        break;
    case Z_DATA_ERROR:
        printf("invalid or incomplete deflate data\n");
        break;
    case Z_MEM_ERROR:
        printf("out of memory\n");
        break;
    case Z_VERSION_ERROR:
        printf("zlib version mismatch!\n");
    }
}
#define HEADER_SIZE (44*1024)//44kb
#define OPT_BUFF_SIZE (16*1024)//256kb
#define DATA_SIZE 170459136//162mb
 
//Split all the files 44 + X * 162Mb
 
int first_pass(int argc)
{
    FILE * pFile,*pFileO;
    printf("Extracting XCP to \"out\" before splitting to GOD...\r\n");
    pFile = fopen (XCP_FILE,"rb");
    pFileO = fopen ("C:\\halo\\out","wb");
 
    __int64 srcSize=0;
    _fseeki64(pFile,0,SEEK_END);
    srcSize=_ftelli64(pFile);
    while(srcSize-lastStream)
    {
        _fseeki64(pFile,lastStream,SEEK_SET);
        inf(pFile, pFileO);
        printf(".");
    }
    printf("\r\nExtract Ok\r\n");
    fclose(pFile);
    fclose(pFileO);
    system("pause");
    return 0;
}
 
int second_pass(FILE *pFile,char * OUT_DIR)
{
    printf("Splitting extracted XCP to \"\\files\\\" in GOD format...\r\n");
    __int64 bigfilesize=0;
    __int64 copied=0;
    unsigned char header_buff[HEADER_SIZE+1];//44ko
    memset(header_buff,0,HEADER_SIZE+1);
    char fname[256];
    memset(fname,0,256);
    FILE * pFheader;
    sprintf(fname,"%s\\REACH",OUT_DIR);
    pFheader=fopen(fname,"wb");
 
    //create the small file
    _fseeki64(pFile,0,SEEK_END);
    bigfilesize=_ftelli64(pFile);
    rewind(pFile);
 
    fread(header_buff,HEADER_SIZE,1,pFile);
    fwrite(header_buff,HEADER_SIZE,1,pFheader);
 
    copied = HEADER_SIZE;
 
    fclose(pFheader);
 
    //create the other file
    int data=0;
    unsigned char bigbuf[OPT_BUFF_SIZE];
    while(copied<bigfilesize)
    {
        printf("... ");
        sprintf(fname,"%s\\REACH.data\\Data%04d",OUT_DIR,data);
        FILE *pData=fopen(fname,"wb");
        for(int i=0;i<(DATA_SIZE/OPT_BUFF_SIZE);i++)
        {
            int read =(copied+OPT_BUFF_SIZE)<bigfilesize ? OPT_BUFF_SIZE :  bigfilesize - copied;
 
            int r = fread(bigbuf,read,1,pFile);
            int w = fwrite(bigbuf,read,1,pData);
           
            copied += read;
        }
        fclose(pData);
        data++;
    }
    printf("\r\nFile splitting Ok\r\n");
    system("pause");
    return 0;
}
 
int main()
{
    FILE * pFile;
    system("mkdir C:\\halo\\files\\REACH.data");
    printf("\r\nNotes: Decompresses contents of C:\\halo\\reach.xcp into GOD format.\r\n(Make sure C:\\halo\\files\\ exists!)\r\nOriginal zlib/offsets by a friend, fixed by Jester \r\n");
 
    pFile = fopen ("C:\\halo\\reach.xcp","rb");
    if(pFile==NULL)
    {
        printf("\r\nFile not found! Please make sure C:\\halo\\reach.xcp exists!\r\n");
        system("pause");
        return 0;
    }
    first_pass(1);
    fclose(pFile);
    pFile = fopen ("C:\\halo\\out","rb");
    second_pass(pFile,"C:\\halo\\files");
    fclose(pFile);
    return 0;
}
