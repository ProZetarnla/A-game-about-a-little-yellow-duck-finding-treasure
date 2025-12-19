#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<string.h>
#include <windows.h>
#include <sys/stat.h>
#include <time.h>
#include<unistd.h>
#define N 101 //N即为地图最大大小
char seq[100001],map[N][N],map2[N][N];                     //seq存储从开始到结束所有的操作,最多支持100002步（不包括撤回和退出）,map存储整个地图,最多支持N*N地图;map2是初始地图，不受改变
float vig[100001];                                         //vig[i]表示第i步后的体力消耗，支持100002步
int state=0;                                               //state代表当前所处界面（0是主界面（选关界面）；1是选择模式界面；2是游戏界面；3是结算界面）
int map_select=0;                                          //map_select指选的是哪张地图
bool mode=0;                                               //mode是控制模式：0是实时模式，1是编程模式
int step=0,treasure_acquired=0,treasure_target=0;          //step指已经走了几步（从1开始，即第1步），treasure_acquired指获得宝藏数，treasure_target指目标宝藏数
int x=10,y=10,posx,posy;                                   //x和y表示地图为x行y列的地图（均包含0行0列），posx和posy表示鸭的坐标
int detection[1001];                                       //detection[i]记录小黄鸭第i次移动的碰撞检测（与seqal的下标+1一一对应）：0代表不碰撞到一次性事物，1代表碰撞到宝藏
char instruction0[1000]="控制方法：按 W 向上移动，按 S 向下移动，按<Enter>选择。";
char instruction2[1000]="控制方法：按 W 向上移动，按 S 向下移动，按 A 向左移动，按 D 向右移动，按 Z 撤回，按 Y 恢复，按 Q 结束冒险";
char seqal[3001]="\0";                                     //seqal存储所有行动，从0开始
int mapsum=3;                                              //mapsum表示当前游戏内置的地图数量
bool selected=0;                                           //selected表示是否已经选择了地图
char saved_map='0';                                        //saved_map表示存档的地图
char saveq[30]="（上次）";                                  //saveq表示存档提示符号


//----------------------------以下是函数声明----------------------------
void flushstdin();
void color(int x);
int typereturn(char c);
void loadmap(char map[N][N]);
void loadsavemap(FILE* fp);
void process();
void detect();
void changecur();
void changecur_withdraw();
int move_detect(int a);
void move(int type);
void withdraw();
void recovery();
void saveselection();
void mapselection();
void modeselection();
void gamemode_mode0();
void gamemode_mode1();
void endscreen();
void rankingselection();
void outputmap(char map[N][N]);
void outputranking(int map_select);
void streamoutranking(int map_select,int val,char name[]);
int count_chinese_characters(const char *str);
void savemap(char map[N][N], int x, int y, int posx, int posy, int treasure_target, int treasure_acquired, float a, int mode, int map_select);


//----------------------------以下是主函数----------------------------
int main()
{
    
    vig[0]=0;
    char c;
    SetConsoleOutputCP(65001);//将编码方式转变为UTF-8，以支持中文输出
    int q = !access("save.map",R_OK);
    memset(map,' ',sizeof(map));
    FILE* fp=fopen("save.map","r");
    if(q){
        fgetc(fp);
        fgetc(fp);
        saved_map = fgetc(fp);
    }
    mapselection();
    while(map_select == mapsum+1)
        rankingselection();
    if(!selected){
        loadmap(map);
        modeselection();
    }
    if(mode==0)
        gamemode_mode0();
    else
        gamemode_mode1();
    endscreen();
    return 0;
}


//----------------------------以下是函数具体代码----------------------------
void flushstdin()//清空输入缓冲区
{
    int c;
    while ( (c=getchar()) && c != EOF);
}
void color(int x)//设置控制台文字颜色
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), x);
}
int typereturn(char c)//根据移动类型返回对应int值
{//将输入的字符转换为移动类型
    switch(c)
    {
        case 'w':return 3;
        case 's':return 1;
        case 'a':return 0;
        case 'd':return 2;
        default:return -1;
    }
}
void loadmap(char map[N][N])//加载地图
{//负责加载地图的函数
    system("cls");
    switch(map_select)
    {
        case 1:freopen("maps\\平凡之路.map","r",stdin);break;
        case 2:freopen("maps\\康庄大道.map","r",stdin);break;
        case 3:freopen("maps\\魔王之旅.map","r",stdin);break;
        default: break;
    }
    scanf("%d %d\n",&x,&y);
    scanf("%d %d\n",&posx,&posy);
    for(int i=0;i<x;i++)
    {
        for(int j=0;j<y;j++)
        {
            char c,m;
            m=getchar();
            switch(m){
                case '0': c=' ';break;
                case '1': c='W';break;
                case '2': c='D';break;
                case '3': c='T';break;
                default: c=' ';break;
            }
            map[i][j]=c;
            map2[i][j]=c;
            if(c == 'Y')
                map2[i][j] = ' ';
            if(map[i][j] == 'T')
                treasure_target++;
            getchar();
        }
    }
    fclose(stdin);
}
void loadsavemap(FILE* fp)//加载存档地图
{//负责加载存档地图的函数
    fscanf(fp,"%d %d\n",&mode,&map_select);
    fscanf(fp,"%d %d\n",&x,&y);
    fscanf(fp,"%d %d\n",&posx,&posy);
    for(int i=0;i<x;i++)
    {
        for(int j=0;j<y;j++)
        {
            char c,m;
            m=fgetc(fp);
            switch(m){
                case '0': c=' ';break;
                case '1': c='W';break;
                case '2': c='D';break;
                case '3': c='T';break;
                default: c=' ';break;
            }
            map[i][j]=c;
            map2[i][j]=c;
            if(c == 'Y')
                map2[i][j] = ' ';
            if(map[i][j] == 'T')
                treasure_target++;
            fgetc(fp);
        }
    }
    fscanf(fp,"%d %d %f\n",&treasure_target,&treasure_acquired, &vig[0]);
    fclose(fp);
}
void process()//在输出地图前处理地图
{
    map[posx][posy]='Y';
}
void outputmap(char map[N][N])//输出地图
{
    process();
    for(int i=0;i<x;i++)
    {
        for(int j=0;j<y;j++)
        {
            switch(map[i][j])
            {
                case 'Y':color(6);printf("Y ");break;
                case 'D':color(4);printf("D ");break;
                case 'Z':color(13);printf("Z ");break;
                case 'T':color(3);printf("T ");break;
                default:color(7);printf("%c ",map[i][j]);break;
            };    
        }
        printf("\n");
    }
    color(7);
}//outputmap函数可以输出整个地图
void detect()//判断碰撞并记录
{//负责碰撞检测
    if(map[posx][posy]==' '||map[posx][posy]=='D')
        detection[step]=0;
    if(map[posx][posy]=='T'){
        detection[step]=1;treasure_acquired++;}
}
void changecur()//根据当前位置改变地图
{
    if(map2[posx][posy]=='T'||map2[posx][posy]==' ')//如果原位置为宝箱或空格，或者为小黄初始位置
        map[posx][posy]=' ';
    if(map2[posx][posy]=='Z')//如果原位置为前间隔陷阱
        map[posx][posy]=(step%2==0)?'Z':' ';
    if(map2[posx][posy]=='z')//如果原位置为后间隔陷阱
        map[posx][posy]=(step%2==1)?'Z':' ';
    if(map2[posx][posy]=='D')//如果原位置为陷阱
        map[posx][posy]='D';
}
void changecur_withdraw()//根据撤回位置改变地图
{
    if(map2[posx][posy]=='T'&& detection[step]==1)
    {
        map[posx][posy]='T';
        treasure_acquired--;
    }
    if(map2[posx][posy]=='T'&&detection[step]!=1)
        map[posx][posy]=' ';
    if(map2[posx][posy]=='D')
        map[posx][posy]='D';
    if(map2[posx][posy]==' '||map2[posx][posy]=='Y')
        map[posx][posy]=' ';
}
int move_detect(int a)//判断是否可以移动
{//a为移动类型：0为左，1为下，2为右，3为上
    switch(a)
    {
        case 0:return (map[posx][posy-1]!='W' && posy-1>=0);
        case 1:return (map[posx+1][posy]!='W' && posx<=x-2);
        case 2:return (map[posx][posy+1]!='W' && posy<=y-2);
        case 3:return (map[posx-1][posy]!='W' && posx-1>=0);
        default: return 0;
    }
}
void move(int type)//移动操作
{//type代表移动类型：0左1下2右3上
    step++;
    vig[step]=vig[step-1]+1;
    if(move_detect(type))
    {
        changecur();
        if(map[posx][posy]=='D')
            vig[step]++;
        switch(type)
        {
            case 0: posy--;break;
            case 1: posx++;break;
            case 2: posy++;break;
            case 3: posx--;break;
            default: break;
        }
        detect();
        map[posx][posy]='Y';
    }
}
void withdraw(){//撤回操作
    system("cls");
    changecur_withdraw();
    int tmp=typereturn(seqal[step-1]);
    switch(tmp)
    {
        case 0: 
        if(move_detect(2))
            posy++;
        break;
        case 1:
        if(move_detect(3)) 
            posx--;
        break;
        case 2: 
        if(move_detect(0))
            posy--;
        break;
        case 3: 
        if(move_detect(1))
            posx++;
        break;
        default: break;
    }
    map[posx][posy]='Y';
    step--;
    outputmap(map);
    printf("体力消耗：%d\n",vig[step]);
    printf("%s",instruction2);
}//撤回功能
void recovery(){//恢复操作
    system("cls");
    if(seqal[step]=='\0')
        return ;
    move(typereturn(seqal[step]));
}
void mapselection(){//控制选关界面的函数
    char c;
    int t=1;//t代表在那个选项：1,2,3为关数，0为退出
    do
    {
        system("cls");
        printf("小黄的奇妙冒险！\n\n");
        printf("%c开始<平凡之路>%s\n",(t==1)?'>':' ',(saved_map=='1')?saveq:"");
        map_select=1;
        printf("%c开始<康庄大道>%s\n",(t==2)?'>':' ',(saved_map=='2')?saveq:"");
        printf("%c开始<魔王之旅>%s\n",(t==3)?'>':' ',(saved_map=='3')?saveq:"");
        printf("%c排行榜\n",(t == mapsum+1)?'>':' ');
        printf("%c退出\n\n",(t==0)?'>':' ');
        printf("%s",instruction0);
        c=getch();
        if(c=='w')
            t--;
        if(c=='s')
            t++;
        t=(t+mapsum+2)%(mapsum+2);
        if(c=='\r' && t==0)
            exit(0);
    }while(c!='\r');
    map_select=t;
    if(saved_map==(char)(map_select+'0'))
        saveselection();
}
void saveselection(){//控制是否加载存档的函数
    FILE* fp=fopen("save.map","r");
    const char* str="save.map";
    struct stat fileStat;
    if(stat(str,&fileStat)<0)
        return ;
    struct tm* timeinfo;
    timeinfo=localtime(&fileStat.st_mtime);
    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    system("cls");
    printf("检测到存档，是否加载存档？\n");
    printf("上次游玩时间：%s\n",timebuf);
    printf(">是\n");
    printf(" 否\n\n");
    printf("%s",instruction0);
    int t=1;
    char c;
    do
    {
        c=getch();
        if(c=='w')
            t=1;
        if(c=='s')
            t=2;
        system("cls");
        printf("检测到存档，是否加载存档？\n");
        printf("%c是\n",(t==1)?'>':' ');
        printf("%c否\n\n",(t==2)?'>':' ');
        printf("%s",instruction0);
    }while(c!='\r');
    if(t==1)
    {
        selected=1;
        FILE* fp=fopen("save.map","r");
        loadsavemap(fp);
    }
}
void modeselection()//控制选择模式界面的函数
{//控制选择模式界面的函数
    system("cls");
    printf("请选择控制模式：\n");
    printf(">0：实时模式\n");
    printf(" 1：编程模式\n\n");
    printf("%s",instruction0);
    char c;
    do
    {
        c=getch();
        if(c=='w')
            mode=0;
        else if(c == 's')
            mode=1;
        system("cls");
        printf("请选择控制模式：\n");
        printf("%c0：实时模式\n",(mode==0)?'>':' ');
        printf("%c1：编程模式\n\n",(mode==1)?'>':' ');
        printf("%s",instruction0); 
    }while(c!='\r');
}
void gamemode_mode0()//控制实时模式下游戏界面的函数
{
    char c;
    system("cls");
    outputmap(map);
    printf("体力消耗：%f\n",vig[0]);
    printf("%s",instruction2);
    do
    {
        c=getch();
        if(c=='d'||c=='a'||c=='w'||c=='s'||c=='z'||c=='y')
        {    
            if(c!='z'&&c!='y')
            {
                seqal[step]=c;
                seqal[step+1]='\0';
            }
            switch(c)
            {
                case 'a':move(0);break;
                case 'w':move(3);break;
                case 's':move(1);break;
                case 'd':move(2);break;
                case 'z':if(step>0)
                            {withdraw();}
                        break;
                case 'y':recovery();
                        break;
                default:break;
            }
            system("cls");
            outputmap(map);
//            printf("step:%d\n",step);
//            printf("seqal:%s\n",seqal);
//            printf("seqal[step]=%c\n",seqal[step-1]);
//            printf("鸭坐标为%d,%d\n",posx,posy);
            printf("体力消耗：%f\n",vig[step]);
            printf("%s\n",instruction2);
        }
    }while(c!='q'&&treasure_acquired<treasure_target);
    float temp=vig[step];
    if(c=='q')
        savemap(map,x,y,posx,posy,treasure_target,treasure_acquired, temp,mode,map_select);
}
void gamemode_mode1()//控制编程模式下的游戏界面
{
    //编程模式输入
    char c;
    while(treasure_acquired<treasure_target)
    {
        system("cls");
        outputmap(map);
        printf("体力消耗：%f\n",vig[step]);
        printf("行动路径：");
        do
        {
            c=getch();
            if(c=='d'||c=='a'||c=='w'||c=='s'||c=='z'||c=='\b')
            {
                if(c=='d'||c=='a'||c=='w'||c=='s')
                {//正常移动
                    seq[strlen(seq)]=c;
                    seq[strlen(seq)]='\0';
                }
                if(c=='z'||c=='\b')
                {
                    if(strlen(seq)>0)
                        seq[strlen(seq)-1]='\0';
                }
                system("cls");
                outputmap(map);
                printf("体力消耗：%f\n",vig[step]);
                printf("行动路径：");
                printf("%s",seq);
            }
            if(c=='q')
            {
                state=3;
                break;//如果按q，则直接结束输入
            }
        }while(c!='\r');
        if(state!=3)
        {//如果是按q结束输入，那么直接跳过此部分进入结算界面
            for(int i=0;i<strlen(seq);i++)
            {
                switch(seq[i])
                {
                    case 'a':move(0);break;
                    case 'w':move(3);break;
                    case 's':move(1);break;
                    case 'd':move(2);break;
                    default:break;
                }
            }
            memset(seq,0,sizeof(seq));
            system("cls");
            outputmap(map);
            printf("体力消耗：%d\n",vig[step]);
            strcat(seqal,seq);
            for(int i=0;i<strlen(seq);i++)
                seq[i]='\0';
            printf("行动路径：");
        }
    }
    if(state == 3)
    {
        float temp=vig[step];
        savemap(map,x,y,posx,posy,treasure_target,treasure_acquired, temp, mode,map_select);
    }
}
void endscreen()//结算界面
{
    system("cls");
    if(treasure_acquired==treasure_target)
        printf("恭喜你，小黄找到了所有宝藏！\n\n");
    else    
        printf("小黄结束了它的冒险！\n\n");
    printf("行动路径：\n");
    seqal[step]='\0';
    printf("%s\n",seqal);
    printf("消耗的体力为：%f\n",vig[step]);
    printf("找到的宝藏数为：%d\n\n",treasure_acquired);
    if(treasure_acquired < treasure_target)
    {
        printf("由于小黄未找到所有宝藏，无法进入排行榜！\n");
        printf("\n\n按任意键结束游戏");
        getch();
        exit(0);
    }
    FILE *fp=fopen("save.map","w");
    DeleteFile("save.map");
    fclose(fp);
    printf("请输入你的名字（不超过12个字符）：");
    char name[13];
    freopen("CON", "r", stdin);
    fgets(name,13,stdin);
    name[strcspn(name,"\n")] = '\0'; // 去除换行符
    streamoutranking(map_select,vig[step],name);
    printf("\n\n按任意键结束游戏");
    getch();
    remove("save.map");
    exit(0);   
}
void rankingselection()//控制选择排行榜界面的函数
{
    int t=1;
    char c;
    do
    {
        system("cls");
        printf("%c<平凡之路>\n",(t==1)?'>':' ');
        printf("%c<康庄大道>\n",(t==2)?'>':' ');
        printf("%c<魔王之旅>\n",(t==3)?'>':' ');
        printf("%c回到选关界面\n\n",(t==0)?'>':' ');
        printf("%s",instruction0);
        c=getch();
        if(c=='w')
            t--;
        if(c=='s')
            t++;
        t=(t+mapsum+1)%(mapsum+1);
    }while(c!='\r');
    if(t==0)
    {
        mapselection();
        return ;
    }
    outputranking(t);
    return ;
}
void outputranking(int map_select)//输出排行榜
{
    switch(map_select)
    {
        case 1:freopen("ranking\\平凡之路rank.map","r",stdin);break;
        case 2:freopen("ranking\\康庄大道rank.map","r",stdin);break;
        case 3:freopen("ranking\\魔王之旅rank.map","r",stdin);break;
        default: break;
    }
    system("cls");
    switch(map_select)
    {
        case 1:printf("<平凡之路>排行榜\n");break;
        case 2:printf("<康庄大道>排行榜\n");break;
        case 3:printf("<魔王之旅>排行榜\n");break;
        default: break;
    }
    printf("排名  挑战者          消耗体力\n");
    int n;
    scanf("%d\n",&n);
    for(int i=1;i<=n;i++)
    {
        int val;
        char name[13];
        scanf("%s %d\n",name,&val);
        int count=count_chinese_characters(name);
        printf("%d     %-16s",i,name);
        for(int j=0;j<count;j++)
            printf(" ");
        printf("%d\n",val);
    }
    printf("\n\n按<Enter>返回选关界面,按C键清空当前排行榜");
    Sleep(200);
    flushstdin();
    fclose(stdin);
    freopen("CON", "r", stdin);
    char c;
    while((c=getch())!='\r'&&c!='c');
    if(c=='c')
    {
        FILE *fp;
        switch(map_select)
        {
            case 1:fp=fopen("ranking\\平凡之路rank.map","w");break;
            case 2:fp=fopen("ranking\\康庄大道rank.map","w");break;
            case 3:fp=fopen("ranking\\魔王之旅rank.map","w");break;
            default: break;
        }
        fprintf(fp,"0\n");
        fclose(fp);
        freopen("CON", "r", stdin);
        outputranking(map_select);
    }
    if(c=='\r')
        mapselection();
    return ;
}
void streamoutranking(int map_select,int val,char name[])//将分数写入排行榜
{
    FILE *fp;
    switch(map_select)
    {
        case 1:fp=fopen("ranking\\平凡之路rank.map","r+");break;
        case 2:fp=fopen("ranking\\康庄大道rank.map","r+");break;
        case 3:fp=fopen("ranking\\魔王之旅rank.map","r+");break;
        default: break;
    }
    int n;
    fscanf(fp,"%d\n",&n);
    struct record{
        char name[13];
        int val;
    }records[1000];
    bool inserted=false;
    for(int i=0;i<n;i++)
    {
        fscanf(fp,"%s %d\n",records[i].name,&records[i].val);
    }
    for(int i=0;i<n;i++)
    {
        if(!inserted && val<records[i].val)
        {
            for(int j=n;j>i;j--)
                records[j]=records[j-1];
            strcpy(records[i].name,name);
            records[i].val=val;
            n++;
            inserted=true;
            break;
        }
    }
    if(!inserted)
    {
        strcpy(records[n].name,name);
        records[n].val=val;
        n++;
    }
    fseek(fp,0,SEEK_SET);//复位文件指针
    fprintf(fp,"%d\n",n);
    for(int i=0;i<n;i++)
    {
        fprintf(fp,"%s %d\n",records[i].name,records[i].val);
    }
    fclose(fp);
}
int count_chinese_characters(const char *str) {//计算字符串中中文字符的数量
    int count = 0;
    while (*str) {
        if((*str & 0xF0) == 0xE0) { // 判断是否为中文字符的第一个字节
            count++;
            str += 3; // 跳过中文字符的三个字节
        }
        else
            str++;
    }
    return count;
}
void savemap(char map[N][N], int x, int y, int posx, int posy, int treasure_target, int treasure_acquired, float a, int mode, int map_select)//保存地图
{
    FILE *fp = fopen("save.map", "w");
    if (fp == NULL) {
        printf("无法打开文件进行保存！\n");
        return;
    }
    fprintf(fp, "%d %d\n", mode, map_select);
    fprintf(fp, "%d %d\n", x, y);
    fprintf(fp, "%d %d\n", posx, posy);
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            char c = map[i][j];
            char m;
            switch(c){
                case ' ': m='0';break;
                case 'W': m='1';break;
                case 'D': m='2';break;
                case 'T': m='3';break;
                default: m='0';break;
            }
            if(j==y-1)
                fprintf(fp, "%c", m);
            else
                fprintf(fp, "%c ", m);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "%d %d %f\n", treasure_target, treasure_acquired, a);
}


/*移动逻辑(move):
1.判断是否可移动(move_detect())
2.依据detection将当前坐标地点变换:changecur()（宝箱只能踩1一次，陷阱可以反复踩）
3.增加体力消耗（若当前格为陷阱，则体力消耗额外+1）
4.将posx和posy变换为目标坐标
5.根据posx和posy坐标上的地点类型变换detection[step]:detect()
6.将目标坐标替换为Y
*/
/*撤回逻辑(withdraw):第i步后进行撤回操作
1.根据detection[i]决定复原什么一次性东西
2.根据vig[i]决定体力恢复至什么值
3.根据seqal[i]决定小黄鸭的目标位置
*/