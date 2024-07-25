#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    FILE *fp = popen("amixer sget 'Master' | grep '%' | grep Left | awk {'print $5'}","r");
    if( NULL == fp)
    {
        perror("popen");
        return -1;
    }
    
    char buf[128] = {0};
    char volume[4] = {0};
    int i =1 ,j = 0;
    
    //每个数据项为1字节，总共读取128个数据项，总共128字节
    //读取fp的数据到buf中
    fread(buf,1,128,fp);
    
    //将buf中的数据存放到volume数组中
    while(buf[i] != '%')
    {
        volume[j] = buf[i];
        j++;
        i++;
    }
    
    //atoi 函数将字符串 "1234" 转换为整数
    printf("当前音量 %d\n", atoi(volume));
    
    int vol = atoi(volume);
    if(vol <= 95)
    {
        vol += 5;
    }
    else if( vol > 95 && vol < 100)
    {
        vol = 100;
    }
    
    memset(buf,0,32);
    
    sprintf(buf,"amixer sset 'Master' %d%%", vol);
    
    //执行 shell 命令
    system(buf);
    pclose(fp);
    
    return 0;


}
