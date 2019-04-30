#include <stdio.h>
#include <string.h>

#define FILE_NAMEANDPATH "/robotvoip/resource/unicode_to_pinyin.data"
#define PINYIN_MAX_SIZE 256

static int hex_to_bin(char src, int *unicode_buf)
{
    switch(src){
        case '0':
            unicode_buf[0] = 0;
            unicode_buf[1] = 0;
            unicode_buf[2] = 0;
            unicode_buf[3] = 0;
            break; 
        case '1':
            unicode_buf[0] = 0;
            unicode_buf[1] = 0;
            unicode_buf[2] = 0;
            unicode_buf[3] = 1;
            break; 
        case '2':
            unicode_buf[0] = 0;
            unicode_buf[1] = 0;
            unicode_buf[2] = 1;
            unicode_buf[3] = 0;
            break; 
        case '3':
            unicode_buf[0] = 0;
            unicode_buf[1] = 0;
            unicode_buf[2] = 1;
            unicode_buf[3] = 1;
            break; 
        case '4':
            unicode_buf[0] = 0;
            unicode_buf[1] = 1;
            unicode_buf[2] = 0;
            unicode_buf[3] = 0;
            break; 
        case '5':
            unicode_buf[0] = 0;
            unicode_buf[1] = 1;
            unicode_buf[2] = 0;
            unicode_buf[3] = 1;
            break; 
        case '6':
            unicode_buf[0] = 0;
            unicode_buf[1] = 1;
            unicode_buf[2] = 1;
            unicode_buf[3] = 0;
            break; 
        case '7':
            unicode_buf[0] = 0;
            unicode_buf[1] = 1;
            unicode_buf[2] = 1;
            unicode_buf[3] = 1;
            break; 
        case '8':
            unicode_buf[0] = 1;
            unicode_buf[1] = 0;
            unicode_buf[2] = 0;
            unicode_buf[3] = 0;
            break; 
        case '9':
            unicode_buf[0] = 1;
            unicode_buf[1] = 0;
            unicode_buf[2] = 0;
            unicode_buf[3] = 1;
            break; 
        case 'a':
        case 'A':
            unicode_buf[0] = 1;
            unicode_buf[1] = 0;
            unicode_buf[2] = 1;
            unicode_buf[3] = 0;
            break; 
        case 'b':
        case 'B':
            unicode_buf[0] = 1;
            unicode_buf[1] = 0;
            unicode_buf[2] = 1;
            unicode_buf[3] = 1;
            break; 
        case 'c':
        case 'C':
            unicode_buf[0] = 1;
            unicode_buf[1] = 1;
            unicode_buf[2] = 0;
            unicode_buf[3] = 0;
            break; 
        case 'd':
        case 'D':
            unicode_buf[0] = 1;
            unicode_buf[1] = 1;
            unicode_buf[2] = 0;
            unicode_buf[3] = 1;
            break; 
        case 'e':
        case 'E':
            unicode_buf[0] = 1;
            unicode_buf[1] = 1;
            unicode_buf[2] = 1;
            unicode_buf[3] = 0;
            break; 
        case 'f':
        case 'F':
            unicode_buf[0] = 1;
            unicode_buf[1] = 1;
            unicode_buf[2] = 1;
            unicode_buf[3] = 1;
            break; 
        default: return -1;
    }
    return 0;
}

static int bin_to_hex(int *unicode_ibuf)
{
    int sum = 0;
    if(unicode_ibuf == NULL){
        return -1;
    }
    sum = unicode_ibuf[0]*8 + unicode_ibuf[1]*4 + unicode_ibuf[2]*2 + unicode_ibuf[3]*1;
    if(sum == 10){
        return 'A';
    }
    else if(sum == 11){
        return 'B';
    }
    else if(sum == 12){
        return 'C';
    }
    else if(sum == 13){
        return 'D';
    }
    else if(sum == 14){
        return 'E';
    }
    else if(sum == 15){
        return 'F';
    }
    else{
        return sum+48;
    }
}

static int utf8_to_unicode(char *utf8_buf, char *unicode_buf)
{
    int ret = -5;
    int i = 0;
    int utf8_ibuf[6][4];
    char condition[4];
    int unicode_ibuf[4][4];
    int utf = 0;
    int *pc = NULL;
    int *pf = NULL;

    if(utf8_buf == NULL || utf8_buf[0] == '\0'){
        return -1;
    }

    memset(utf8_ibuf,0x0,sizeof(utf8_ibuf));
    for(i = 0; i<6; i++){
        ret =hex_to_bin(utf8_buf[i],utf8_ibuf[i]);
        if(ret < 0){
            fprintf(stderr,"[%s][%d]--[%d]error!\n",__FILE__, __LINE__,i);
            return -1;
        }
    }
    
    memset(condition,0x0,sizeof(condition));
    sprintf(condition,"%d%d%d%d",utf8_ibuf[0][0],utf8_ibuf[0][1],utf8_ibuf[0][2],utf8_ibuf[0][3]);
    pc = (int *)unicode_ibuf;
    pf = (int *)utf8_ibuf;
    if(strcmp(condition,"1110") == 0){
        utf = 4;
        memset(unicode_ibuf,0x0,sizeof(unicode_ibuf));
        for(i=0; i<16; i++){
            if(utf == 8 || utf == 9 || utf == 16 || utf == 17){
                utf = utf + 2;
            }
            pc[i] = pf[utf]; 
            utf++;
        }
        for(i=0; i<4; i++){
            condition[i] = bin_to_hex(unicode_ibuf[i]);
        }
    }

    sprintf(unicode_buf,"%s",condition);
    
    return 0;
}

static int ow_pow(int n, int x)
{
    if(x == 0){
        return 1;
    }
    else{
        return n*ow_pow(n,x-1);
    }
}

static int get_coord(char *unicode_buf)
{
    int i = 0;
    char result[4];
    char buf[4];
    int rlt1 = 0;
    int rlt0 = 0;
    int hex[4][4];
    int *ph = NULL;
    int sum = 0;

    if(unicode_buf == NULL || unicode_buf[0] == '\0'){
        return -1;
    }
    if(strcmp(unicode_buf,"3007") == 0){
        return 1;
    }
    sprintf(buf,"%s",unicode_buf);
    memset(result,0x0,sizeof(result));
    if(buf[1] < 65){
        buf[1] = buf[1]+7;
    }
    if(buf[1] < 'E'){
        buf[0] = buf[0] - 1;
        buf[1] = buf[1] + 16;
    }
    rlt1 = buf[1] - 'E';
    if(rlt1 == 15){
        rlt1 = 70;
    }
    else if(rlt1 == 14){
        rlt1 = 69;
    }
    else if(rlt1 == 13){
        rlt1 = 68;
    }
    else if(rlt1 == 12){
        rlt1 = 67;
    }
    else if(rlt1 == 11){
        rlt1 = 66;
    }
    else if(rlt1 == 10){
        rlt1 = 65;
    }
    else{
        rlt1 = rlt1 + 48;
    }
    if(buf[0] < '4'){
        return -1;
    }
    rlt0 = buf[0] - '4' + 48;

    sprintf(result,"%c%c%c%c",rlt0,rlt1,unicode_buf[2],unicode_buf[3]);
    memset(hex,0x0,sizeof(hex));
    for(i=0; i<4; i++){
        if(hex_to_bin(result[i],hex[i]) < 0){
            fprintf(stderr,"[%s][%d]---[%d] error!\n",__FILE__, __LINE__,i);
            return -1;
        } 
    }
    ph = (int *)hex;
    for(i=0; i<16; i++){
        sum = sum+ph[i]*ow_pow(2,15-i);
    }
    return sum+2;
}

static int fget_appoint(FILE *fp, char *result, int line)
{
    int count = 0;

//    printf("line:%d\n",line);
    if(fp == NULL || result == NULL ){
        return -1;
    }
    if(line > 1){
        while(1){
            if(fgetc(fp) == '\n'){
                count++;
            }
            if(count == line - 1){
                break;
            }
            if(feof(fp)){
                return -1;
            }
        }
    }
    fgets(result,64,fp);
    return 0;
}

static int unicode_to_pinyin(char *unicode_buf, char *pinyin, FILE *fp)
{
    //FILE *fp = NULL;
    //char filename[128];
    char result[64];
    int line = 0;
    char pinyin_buf[32];
    int i = 0;

    //memset(filename,0x0,sizeof(filename));
    //sprintf(filename,"%s",FILE_NAMEANDPATH);

    //fp = fopen(filename,"r");

    line = get_coord(unicode_buf);
    memset(result,0x0,sizeof(result));
    fget_appoint(fp,result,line);

    while(1){
        if(result[i] == '('){
            break;
        }
        i++;
        if(i == 64){
            return -1;
        }
    }
    
    i = i + 1;
    line = 0;
    memset(pinyin_buf,0x0,sizeof(pinyin_buf));
    while(1){
        pinyin_buf[line] = result[i];
        if(pinyin_buf[line] < 65){
            break;
        }
        line++;
        i++;

    }
    pinyin_buf[line] = '\0';
    sprintf(pinyin,"%s",pinyin_buf);
    fseek(fp,0L,SEEK_SET);
    //fclose(fp);
    return 0;
}

int word_to_pinyin( unsigned char *word,char *pinyin)
{
    FILE *fp = NULL;
    char filename[128];
    char utf8_word[30][7];
    char pinyin_buf[PINYIN_MAX_SIZE];
    char pinyin_temp[8];
    char ubuf[4];
    int len = 0;
    int i = 0;
    int pu = 0;
    //int ret = 0;

    memset(filename,0x0,sizeof(filename));
    sprintf(filename,"%s",FILE_NAMEANDPATH);

    if(word == NULL){
        return -1;
    }
    if((len = strlen((char *)word)) < 3){
        return -1;
    }
    
    memset(utf8_word,0x0,sizeof(utf8_word));
    for(i=0; i<len+1; i++){
        if(word[i] > 0xe0){
            sprintf(utf8_word[pu],"%x%x%x",word[i],word[i+1],word[i+2]);
            pu++;
            i = i + 2;
        }
        if(pu == 30){
            break;
        }
    }

    fp = fopen(filename,"r");
    memset(pinyin_buf,0x0,sizeof(pinyin_buf));
    for(i=0; i<pu; i++){
        memset(ubuf,0x0,sizeof(ubuf));
        memset(pinyin_temp,0x0,sizeof(pinyin_temp));
        if(utf8_to_unicode(utf8_word[i],ubuf) != -1){
            unicode_to_pinyin(ubuf,pinyin_temp,fp);
            strcat(pinyin_buf,pinyin_temp);
        }
        if(i < pu-1){
            strcat(pinyin_buf," ");
        }
    }

//    fprintf(stderr,"拼音:%s\n",pinyin_buf);
    sprintf(pinyin,"%s",pinyin_buf);
    fclose(fp);
    return 0;   
}

#if 0
int main()
{
   unsigned char src[128];
    char buf[6];
    char ubuf[4];
    char pinyin[64];

    memset(pinyin,0x0,sizeof(pinyin));
    memset(ubuf,0x0,sizeof(ubuf));
   memset(src,0x0,sizeof(src));
   while(1){
        memset(src,0x0,sizeof(src));
        printf("原文:");
        scanf("%s",src);
        fflush(stdin);
        word_to_pinyin(src,pinyin);
   }
   /*
   printf("inpu::\n");
   scanf("%s",src);
   fflush(stdin);
    sprintf(buf,"%x%x%x",src[0],src[1],src[2]);
    fprintf(stderr,"utf8:[%s]\n",buf);
    utf8_to_unicode(buf,ubuf);
    fprintf(stderr,"utf8:[%s]\n",ubuf);
    unicode_to_pinyin(ubuf,pinyin);
    printf("pinyin:%s\n",pinyin);
    */
   return 0;
}
#endif
