/*
 * @Author: LQT
 * @Date: 2018-07-05 11:50:50
 * @LastEditors: LQT
 * @LastEditTime: 2018-07-05 20:11:07
 * @Description: 网络爬虫
 */
#include <cstdio>
#include<iostream>
#include<fstream>
#include<string>
#include<regex>         //正则表达式
#include<vector>
#include<regex>
#include<queue>
#include<algorithm>     //标准算法
#include<sys/socket.h>
#include<map>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <arpa/inet.h>

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)
using namespace std;
char **pptr;
char str[32];
char host[500];
int num=1;
char othPath[800];      //路径
string allHtml;          //HTML全部网页
vector<string> photoUrl;    //图片地址
vector<string> comUrl;      //链接地址
map<string,int>mp;          //防止相同网址重复遍历
int sock;

//分析URL地址
bool analyUrl(char *url){
    char *pos=strstr(url,"http://");
    if(pos==NULL){
        return false;
    }
    else{
        pos=pos+7;
    }
    scanf(pos,"%[`/]%s",host,othPath);      //http://后一直到/之前的主机名称
    cout<<"host:"<<host<<"repath:"<<othPath<<endl;
    return true;
}

//正则表达式提取图片
void regexGetimage(string &allHtml){
    smatch mat;
    regex pattern("src=\"(.*?\.jpg)\"");
    string::const_iterator  start=allHtml.begin();
    string::const_iterator  end=allHtml.end();
    while(regex_search(start,end,mat,pattern)){
        string msg(mat[1].first,mat[1].second);
        photoUrl.push_back(msg);
        start=mat[0].second;
    }
}

//提取网页中的http://的url
void regexGetcom(string &allHtml){
    smatch mat;
    regex pattern("href=\"(http://['\s'\"]+)\"");
    string::const_iterator start=allHtml.begin();
    string::const_iterator end=allHtml.end();
    while(regex_search(start,end,mat,pattern)){
        string msg(mat[1].first,mat[1].second);
        comUrl.push_back(msg);
        start=mat[0].second;
    }
}

//socket进行网络编程
void preConnect(){
    sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sock<0){
        ERR_EXIT("建立socket失败！错误码：");
    }
    struct hostent *p =gethostbyname("www.baidu.com");
    if(p==NULL){
        ERR_EXIT("hostent");
    }
    struct sockaddr_in myaddr;
    myaddr.sin_family=AF_INET;
    //memcpy(&myaddr.sin_addr.s_addr,p->h_addr,4);
    myaddr.sin_addr=*(struct in_addr*)*p->h_addr_list;
    myaddr.sin_port=htons(80);
    int n=bind(sock,(struct sockaddr*)&myaddr,sizeof(myaddr));
    if(n<0){
        ERR_EXIT("bind");
    }
    n=connect(sock,(sockaddr*)&myaddr,sizeof(myaddr));
    if(n<0){
        ERR_EXIT("connect");
    }
    //向服务器发送GET请求，下载图片
    string reqInfo="GET"+(string)othPath+"HTTP/1.1\r\nHost"+(string)host+"\r\nConnection:Close\r\n\r\n";
    if(write(sock,reqInfo.c_str(),reqInfo.size())){
        close(sock);
        ERR_EXIT("write");
    }
    
}

//将图片命名保存在目录下
void outImage(string imageUrl){
    int n;
    char temp[800];
    strcpy(temp,imageUrl.c_str());
    analyUrl(temp);
    preConnect();
    string photoName;
    photoName.resize(imageUrl.size());
    int k=0;
    for(int i=0;i<imageUrl.length();i++){
        char ch=imageUrl[i];
        if(ch !='\\'&&ch!='/'&&ch!=':'&&ch!='*'&&ch!='?'&&ch!='"'&&ch!='<'&&ch!='>'&&ch!='|'){
            photoName[k++]=ch;
        }

    }
    photoName="./img/"+photoName.substr(0,k)+".jpg";
    fstream file;
    file.open(photoName,ios::out|ios::binary);
    char buf[1024];
    memset(buf,0,sizeof(buf));
    n=read(sock,buf,sizeof(buf)-1);
    if(n<0){
        ERR_EXIT("read");
    }
    char *cpos=strstr(buf,"\r\n\r\n");
    file.write(cpos+strlen("\r\n\r\n"),n-(cpos-buf)-strlen("\r\n\r\n"));
    while(((read(sock,buf,sizeof(buf)-1)))>0){
        file.write(buf,n);
    }
    file.close();

}

//解析整个HTML代码
void putImageToSet(){
    int n;
    char buf[1024];
    while((n=read(sock,buf,sizeof(buf)-1))>0){
        buf[n]='\0';
        allHtml+=string(buf);
    }
    regexGetimage(allHtml);
    regexGetcom(allHtml);
}

//宽度优先搜索，遍历网页
void bfs(string beginUrl){
    queue<string> q;
    q.push(beginUrl);
    while(!q.empty()){
        string cur=q.front();
        mp[cur]++;
        q.pop();
        char tmp[800];
        strcpy(tmp,cur.c_str());
        analyUrl(tmp);
        preConnect();
        putImageToSet();
        vector<string>::iterator ita=photoUrl.begin();
        for(ita;ita!=photoUrl.end();++ita){
            outImage(*ita);
        }
        photoUrl.clear();
        vector<string>::iterator it=comUrl.begin();
        for(it;it!=comUrl.end();++it){
            if(mp[*it]==0){
                q.push(*it);
            }          
        }
        comUrl.clear();
    }
}
int main(){
    string surl;
    cout<<"请输入网址"<<endl;
    cin>>surl;
    if((mkdir("./img",0766)<0)){
        ERR_EXIT("mkdir");
    }
    //string beginUrl="war.163.com/";
    bfs(surl);
    return 0;
}

