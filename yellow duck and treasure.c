#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<string.h>
#include <windows.h>
#define N 101 //N即为地图最大大小
char seq[100001],map[N][N],map2[N][N];//seq存储从开始到结束所有的操作,最多支持100002步（不包括撤回和退出）,map存储整个地图,最多支持N*N地图;map2是初始地图，不受改变
int vig[100001],state=0;//vig[i]是第i步后的体力消耗,state代表当前所处界面（0是主界面（选关界面）；1是选择模式界面；2是游戏界面；3是结算界面）
int map_select=0;//map_select指选的是哪张地图
bool mode=0;//mode是控制模式：0是实时模式，1是编程模式
int step=0,treasure_acquired=0,treasure_target=0;//step指已经走了几步（从1开始，即第1步），treasure_acquired指获得宝藏数，treasure_target指目标宝藏数
char comment[50];//comment是注释（如操作方法等）
int x=10,y=10,posx,posy;//x和y表示地图为x行y列的地图（均包含0行0列），posx和posy表示鸭的坐标
int detection[1001];//detection[i]记录小黄鸭第i次移动的碰撞检测（与seqal的下标+1一一对应）：0代表不碰撞到一次性事物，1代表碰撞到宝藏
char instruction0[1000]="控制方法：按 W 向上移动，按 S 向下移动，按<Enter>选择。";
char instruction2[1000]="控制方法：按 W 向上移动，按 S 向下移动，按 A 向左移动，按 D 向右移动，按 Z 撤回，按 Q 结束冒险";
char seqal[3001]="\0";//seqal存储所有行动，从0开始
void color(int x){
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), x);
}
void process(){
    for(int i=0;i<x;i++)
        for(int j=0;j<y;j++){
            if(map2[i][j]=='Z')
                map[i][j]=(step%2==0)?'Z':' ';
            if(map2[i][j]=='z')
                map[i][j]=(step%2==1)?'Z':' ';
        }
    map[posx][posy]='Y';
}
void outputmap(char map[N][N]){
    process();
    for(int i=0;i<x;i++){
        for(int j=0;j<y;j++){
            switch(map[i][j]){
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
void detect(){//负责碰撞检测
    if(map[posx][posy]==' '||map[posx][posy]=='D')
        detection[step]=0;
    if(map[posx][posy]=='T'){
        detection[step]=1;treasure_acquired++;}
}
void changecur(){
    if(map2[posx][posy]=='T'||map2[posx][posy]=='Y'||map2[posx][posy]==' ')//如果原位置为宝箱或空格，或者为小黄初始位置
        map[posx][posy]=' ';
    if(map2[posx][posy]=='Z')//如果原位置为前间隔陷阱
        map[posx][posy]=(step%2==0)?'Z':' ';
    if(map2[posx][posy]=='z')//如果原位置为后间隔陷阱
        map[posx][posy]=(step%2==1)?'Z':' ';
    if(map2[posx][posy]=='D')//如果原位置为陷阱
        map[posx][posy]='D';
}
void changecur_withdraw(){
    if(map2[posx][posy]=='T'&& detection[step]==1){
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
int move_detect(int a){//a为移动类型：0为左，1为下，2为右，3为上
    switch(a){
        case 0:return (map[posx][posy-1]!='W' && posy-1>=0);
        case 1:return (map[posx+1][posy]!='W' && posx<=x-2);
        case 2:return (map[posx][posy+1]!='W' && posy<=y-2);
        case 3:return (map[posx-1][posy]!='W' && posx-1>=0);
        default: return 0;
    }
}
void move(int type){//type代表移动类型：0左1下2右3上
    step++;
    vig[step]=vig[step-1]+1;
    if(move_detect(type)){
        changecur();
        if(map[posx][posy]=='D')
            vig[step]++;
        if(map2[posx][posy]=='z'||map2[posx][posy]=='Z'){
            if(map[posx][posy]==' ')
                vig[step]++;
        }
        switch(type){
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
void withdraw(){//令要撤销的步为A步
    system("cls");
    changecur_withdraw();
    switch(seqal[step-1]){
        case 'w': posx++;break;
        case 's': posx--;break;
        case 'a': posy++;break;
        case 'd': posy--;break;
        default: break;
    }
    map[posx][posy]='Y';
    step--;
    outputmap(map);
    printf("体力消耗：%d\n",vig[step]);
    printf("%s",instruction2);
}//撤回功能
void state0(){
    char c;
    int t=1;//t代表在那个选项：1,2,3为关数，0为退出
    do{
        system("cls");
        printf("小黄的奇妙冒险！\n\n");
        printf("%c开始第一关\n",(t==1)?'>':' ');
        map_select=1;
        printf("%c开始第二关\n",(t==2)?'>':' ');
        printf("%c开始第三关\n",(t==3)?'>':' ');
        printf("%c退出\n\n",(t==0)?'>':' ');
        printf("%s",instruction0);
        c=getch();
        if(c=='w')
            t--;
        if(c=='s')
            t++;
        t=(t+4)%4;
        if(c=='\r'&& t!=0)
            state=1;
        if(c=='\r' && t==0)
            exit(0);
    }while(c!='\r');
    map_select=t;
}
void state1(){//控制选择模式界面的函数
    system("cls");
    printf("请选择控制模式：\n");
    printf(">0：实时模式\n");
    printf(" 1：编程模式\n\n");
    printf("%s",instruction0);
    char c;
    do{
        c=getch();
        if(c=='\r')
            state=2;
        if(c=='w'){
            mode=0;
            system("cls");
            printf("请选择控制模式：\n");
            printf(">0：实时模式\n");
            printf(" 1：编程模式\n\n");
            printf("%s",instruction0);
        }
        if(c=='s'){
            mode=1;
            system("cls");
            printf("请选择控制模式：\n");
            printf(" 0：实时模式\n");
            printf(">1：编程模式\n\n");
            printf("%s",instruction0);
        }
            
    }while(c!='\r');
}
void state2_mode0(){//控制实时模式下游戏界面的函数
    char c;
    system("cls");
    outputmap(map);
    printf("体力消耗：%d\n",vig[0]);
    printf("%s",instruction2);
    do{
        c=getch();
        if(c=='d'||c=='a'||c=='w'||c=='s'||c=='z'){    
            if(c!='z'){
                seqal[strlen(seqal)]=c;
                seqal[strlen(seqal)]='\0';
            }
            switch(c){
                case 'a':move(0);break;
                case 'w':move(3);break;
                case 's':move(1);break;
                case 'd':move(2);break;
                case 'z':if(step>0)
                            {withdraw();}
                        break;
                default:break;
            }
            if(c=='z'){
                seqal[step]='\0';
                vig[step+1]=0;
            }   
            system("cls");
            outputmap(map);
//            printf("step:%d\n",step);
//            printf("seqal:%s\n",seqal);
//            printf("seqal[step]=%c\n",seqal[step-1]);
//            printf("鸭坐标为%d,%d\n",posx,posy);
            printf("体力消耗：%d\n",vig[step]);
            printf("%s",instruction2);
        }
    }while(c!='q'&&treasure_acquired<treasure_target);
}
void state2_mode1(){//控制编程模式下的游戏界面
    //编程模式输入
    char c;
    while(treasure_acquired<treasure_target){
        system("cls");
        outputmap(map);
        printf("体力消耗：%d\n",vig[step]);
        printf("行动路径：");
        do{
            c=getch();
            if(c=='d'||c=='a'||c=='w'||c=='s'||c=='z'||c=='\b'){
                if(c=='d'||c=='a'||c=='w'||c=='s'){//正常移动
                    seq[strlen(seq)]=c;
                    seq[strlen(seq)]='\0';
                }
                if(c=='z'||c=='\b'){
                    if(strlen(seq)>0)
                        seq[strlen(seq)-1]='\0';
                }
                system("cls");
                outputmap(map);
                printf("体力消耗：%d\n",vig[step]);
                printf("行动路径：");
                printf("%s",seq);
            }
            if(c=='q'){
                state=3;
                break;//如果按q，则直接结束输入
            }
        }while(c!='\r');
        if(state!=3){//如果是按q结束输入，那么直接跳过此部分进入结算界面
            for(int i=0;i<strlen(seq);i++){
                switch(seq[i]){
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
}
void state3(){
    system("cls");
    if(treasure_acquired==treasure_target)
        printf("恭喜你，小黄找到了所有宝藏！\n\n");
    else    
        printf("小黄结束了它的冒险！\n\n");
    printf("行动路径：\n");
    printf("%s\n",seqal);
    printf("消耗的体力为：%d\n",vig[step]);
    printf("找到的宝藏数为：%d\n\n",treasure_acquired);
    printf("按任意键结束游戏");
    getch();
    exit(0);   
}
int main(){
    vig[0]=0;
    char c;
    SetConsoleOutputCP(65001);//将编码方式转变为UTF-8，以支持中文输出
    memset(map,' ',sizeof(map));
    state0();
    state1();
    system("cls");
    switch(map_select){
        case 1:freopen("map1.txt","r",stdin);break;
        case 2:freopen("map2.txt","r",stdin);break;
        case 3:freopen("map3.txt","r",stdin);break;
        default: break;
    }
    scanf("%d%d\n",&x,&y);
    for(int i=0;i<x;i++){
        for(int j=0;j<y;j++){
            char c;
            c=getchar();
            map[i][j]=c;
            map2[i][j]=c; 
            if(c=='Y')
                map2[i][j]=' ';
        }
        getchar();
    }
    fclose(stdin);
    for(int i=0;i<x;i++)
        for(int j=0;j<y;j++){
            if(map[i][j]=='T')
                treasure_target++;
            if(map[i][j]=='Y'){
                posx=i;
                posy=j;
            }
        }
    if(mode==0)
        state2_mode0();
    else
        state2_mode1();
    state3();
    return 0;
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