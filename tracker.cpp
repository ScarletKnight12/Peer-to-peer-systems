#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <unistd.h> 
#include <netinet/in.h> 
#include <string.h> 
#include<fstream>
#include<bits/stdc++.h>
#include <arpa/inet.h>	
#include<cstring>
#include<thread>
using namespace std;

int user_count=0;

struct c_details{
	char uid[512];
	char pwd[512];
	int port;
	char ip[512];
	int login;
	int cli;
};

c_details user[50];

map<string, string> group; //owner for a gid
map<string, vector<string> >group_members; //members for a gid

vector<int> cli_list; 
vector< vector<string> >upload_d; //uploaded < fname,uid,gid, sha>>


void create_user(int clien_sock){

	int num=user_count, fl=0;
	char uip[512], upwd[512], uuid[512];
	int uport;
	
	//receive deets
	//ip
	recv(clien_sock, &uip, sizeof(uip), 0);
	//port
	recv(clien_sock, &uport, sizeof(uport), 0);
	//pwd
	recv(clien_sock, &upwd, sizeof(upwd), 0);
	//uid
	recv(clien_sock, &uuid, sizeof(uuid), 0);
	
	
	
	for(int i=0;i<user_count; i++){
		if(!strcmp(user[i].uid, uuid)){
			cout<<"Cant register. Already present uid. Pick another choice";
			fl=1;
			break;
		}
	}
	
	if(fl==0){
		user[num].port = uport;
		memcpy(user[num].ip, uip, sizeof(uip));
		memcpy(user[num].pwd, upwd, sizeof(upwd));
		memcpy(user[num].uid, uuid, sizeof(uuid));
		
		user_count++;
	
		user[num].login=0;
	}
}

void login(int clien_sock){
	char uid[512], pwd[512];
	
	//uid
	recv(clien_sock, &uid, sizeof(uid), 0);
	//pwd
	recv(clien_sock, &pwd, sizeof(pwd), 0);
	int i=0;
	
	for(i=0;i<user_count;i++){
		if(strcmp(uid,user[i].uid)==0 && strcmp(pwd, user[i].pwd)==0){
			cout<<"Successfully logged in."<<endl;
			user[i].login=1;
			break;
		}
	}
	
	//send yes or no
	send(clien_sock, &user[i].login, sizeof(user[i].login),0);
	
}

void create_group(int clien_sock){
	
	char gid[512],uid[512]; 
	int stat;
	//gid
	recv(clien_sock, &gid, sizeof(gid), 0);
	//uid
	recv(clien_sock, &uid, sizeof(uid), 0);
	
	for(int i=0;i<user_count;i++){
	
		cout<<"UID"<<user[i].uid<<":PWD:"<<user[i].pwd<<":LOGIN:"<<user[i].login<<"CLI"<<user[i].cli<<endl;
	
		if(!strcmp(user[i].uid,uid)){
		stat=user[i].login;
		
		}
	}
	
	
	
	if(stat==0){
		cout<<"NOT LOGGED IN";
		//need to send something to client
		
	}else{
	
		if(group.count(string(gid))==0){
			group[string(gid)]=string(uid);
			vector<string>v;
			v.push_back(string(uid));
			
			group_members[string(gid)]=v;
			
		}else{
			cout<<"Already present group"<<endl;
		}
	}
}

void list_groups(){
	for(auto it=group.begin(); it!=group.end(); it++)
			cout<< it->first<<" : "<< it->second<<"----";
		
		cout<<endl;


}

void list_members(){
	for(auto it=group_members.begin(); it!=group_members.end(); it++){
		cout<< it->first<<" : ";
		for(auto jt=it->second.begin(); jt!=it->second.end() ; jt++){
			cout<< *jt<<" ";
		}
		cout<<endl;
	}
}


void join_group(int clien_sock){

	char found[512]="-";
	char uid_cli[512];
				
	char gid[512],uid[512]; 
	int stat,flag=0;
	
	//gid
	recv(clien_sock, &gid, sizeof(gid), 0);
	//uid
	recv(clien_sock, &uid, sizeof(uid), 0);
	
	for(int i=0;i<user_count;i++){
		if(!strcmp(user[i].uid,uid)){
		stat=user[i].login;
		break;
		}
	}
	
	cout<<"USER"<<uid<<endl;
	
	if(stat==0){
		cout<<"NOT LOGGED IN"<<endl;
		//need to send something to client
		
	}else{
	
		if(group.count(string(gid))==0){
			cout<<"Group doesnt exist"<<endl;
			
		}else{
			cout<<"hmm?";
		
			if(count(group_members[string(gid)].begin(), group_members[string(gid)].end(), string(uid) ) !=0) {
				cout<<"Already member of grooup";
				//send a msg to cli		
				
			} 
			else{
			
				//group exists.NEed to join;
				//tells the client someone wants to join
				
				string ow = group[string(gid)];
				char owner[512];
				strcpy(owner,ow.c_str());
				
				cout<<"Owner:"<<owner<<" "<<ow;;
				
				
				for(int tt=0; tt<user_count; tt++){
					if(!strcmp(owner, user[tt].uid)){
					
						cout<<"Found owner!"<<user[tt].uid<<" "<<user[tt].cli;
						memcpy(uid_cli, owner,sizeof(owner));
						string ow_po=to_string(user[tt].port);
						char ow_p[512];
						strcpy(ow_p, ow_po.c_str());
				
						strcat(uid_cli,":");
						strcat(uid_cli,ow_p);
						strcat(uid_cli,":");
						strcat(uid_cli,gid);
						
						break;
					}
				}
				
				//send to client owner:owner-uid:port:gid
				cout<<"ucli"<<uid_cli;
				
				
				flag=1;
				//send uid
				send(clien_sock, &uid_cli, sizeof(uid_cli),0);
				
				//NEED to update list. When??
				
			}		
		}
	}
	
	if(flag==0){
		//send a faux msg
		send(clien_sock, &found, sizeof(found), 0);
	}
}

void accept_req(int clien_sock){
	
	char res[1024];
	recv(clien_sock, &res, sizeof(res), 0);
	
	cout<<"CHOICE:"<<res<<endl;
	if(!strcmp(res,"0")){
		cout<<"Group Owner not accepting.";
	
	} else{
	
		//get gid, uid
		int xx=0,fl=0,ind=0;
		char us[512], gg[512],ch;
		while(res[xx]!='\0'){
		
			if(res[xx]==':'){
				fl++;
				
				gg[ind]='\0';
				xx++;
				ind=0;
			}
			if(fl==1){
				us[ind]=res[xx];
				ind++;
			
			}else{
				gg[ind]=res[xx];
				ind++;
			}
			xx++;
		}
		us[ind]='\0';
		
		cout<<"GG:US:"<<gg<<"--"<<us<<endl;
		
		//add to group_members
		group_members[gg].push_back(us);
	
	}
}

void upload(int clien_sock){

	//uid
	char uid[512];
	recv(clien_sock,uid,sizeof(uid),0);

	//need to upload. Get file name
	char fname[512];
	recv(clien_sock,fname, sizeof(fname),0);
	
	//gid
	char gid[512];
	recv(clien_sock,gid,sizeof(gid),0);
	
	//SHA1
	char msg[9000];
	recv(clien_sock, msg,sizeof(msg),0);
	cout<<uid<<"_"<<fname<<"_"<<gid<<"_"<<msg;
	
	vector<string> vec(4);
	vec[0]=string(fname);
	vec[1]=string(uid);
	vec[2]=string(gid);
	vec[3]=string(msg);
	
	
	upload_d.push_back(vec);
	

}
void list_files(int clien_sock){
	char gid[512];
	recv(clien_sock,&gid,sizeof(gid),0);
	string files="";

	for(int i=0;i<upload_d.size();i++){
		if(upload_d[i][2]==gid){
		cout<<"come heere";
			files+=upload_d[i][0];
			files+=", ";
		}
	}
	
	char enf[9000];
	strcpy(enf,files.c_str());
	send(clien_sock, &enf, sizeof(enf),0);
	

}

void download(int clien_sock){
	int i,j,count=0;
	
	//check if file is already uploaded in group
	char fn[512],gid[512];
	
	recv(clien_sock, &gid, sizeof(gid),0);
	recv(clien_sock, &fn, sizeof(fn),0);
	cout<<"FN"<<fn<<"=GID="<<gid<<endl;
	
	vector<string>uploader;
	vector<string>ipp;
	vector<int>portt;
	vector<int>msg_s;
	
	for(i=0; i<upload_d.size();i++){
		//cout<<upload_d[i][0]<<" "<<upload_d[i][1]<<" "<<upload_d[i][2]<<endl;
		
		if(upload_d[i][0]==fn && upload_d[i][2]==gid){
			cout<<"INSID:";
			cout<<upload_d[i][0]<<" "<<upload_d[i][1]<<" "<<upload_d[i][2]<<endl;
			count++;
			uploader.push_back(upload_d[i][1]);
			
		for(j=0;j<user_count;j++){
			
			if(user[j].uid==upload_d[i][1]){
				cout<<"INSIDE User deets:" <<user[j].uid <<"__"<<user[j].port <<"__" <<user[j].ip <<endl;
				ipp.push_back(user[j].ip);
				portt.push_back(user[j].port);
				//send size of SHA1
				int ss=upload_d[i][3].size();
				msg_s.push_back(ss);
				
				break;
				}
			}	
		}
	}
	
	//send count of uploaders
	send(clien_sock,&count,sizeof(count),0);
	
	//send gid of uploader, ip,port,msg_s 
	for(i=0;i<count;i++)	{
		cout<<ipp[i]<<" "<<portt[i]<<" "<<uploader[i].c_str()<<" "<<msg_s[i]<<endl;
		
		send(clien_sock, uploader[i].c_str(), 512,0);
		send(clien_sock, ipp[i].c_str(), 512,0);
		send(clien_sock, &portt[i], sizeof(int),0);
		send(clien_sock, &msg_s[i], sizeof(int),0);
	}

	cout<<"DONE sending"<<endl;
}


void remove_g(int clien_sock){

	char gid[512],uid[512];
	cout<<"HHHEREE";
	recv(clien_sock, gid, sizeof(gid),0);
	string gg=string(gid);
	cout<<"To remove from group:"<<gg;
	
	recv(clien_sock, uid, sizeof(uid),0);
	string uu=string(uid);
	cout<<"USER:"<<uid;
	
	
	//if uid is a group owner, delete group 
	if(group[gg]==uu){
			cout<<endl<<"Owner of"<<gg<<" , deleting group as well";
			group.erase(gg);
			group_members.erase(gg);
	}
	
	//else, remove only gg from group
	if(group_members.count(gg)){
		vector<string> v=group_members[gg];
		
		for(int x=0; x<v.size();x++){
			if(v[x]==uu){
				cout<<"DELETING as member";
				v.erase(v.begin()+x);
			}
		}
		group_members[gg]=v;
	
	}
	
	
	
}

void handle_request(int clien_sock, int command){
	
	
	
	if(command==1){
		
		//create user and store details
		cout<<"Com 1: ";
		create_user(clien_sock);
		int num=user_count-1;
		cout<<"Details received--"<< user[num].ip<< user[num].port<< user[num].pwd<< user[num].uid<<endl;	
		user[num].cli = clien_sock;	

	
	} else if(command==2){
		//check
		cout<<"Com 2: ";
		login(clien_sock);
		cout<<"Done login"<<endl;
		
	} 
	else if(command==3){
		cout<<"Com 3: ";
		create_group(clien_sock);
		cout<<"Done create group. elements: ";
		
		list_groups();	
		
			
	}
	else if(command==4){
		cout<<"Com 4: ";
		join_group(clien_sock);
		cout<<" Joined the group. Elements: ";
		
		list_members();
		
			
	}
	else if(command==5){
		cout<<"Com 5:";
		accept_req(clien_sock);
	}
	else if(command==6){
	//list groups in network
		cout<<"GROUPS:";
		string v;
		for(auto it=group.begin(); it!=group.end(); it++){
		
			cout<< it->first <<",";
			v+=it->first;
			v+=",";
		}
		char vv[1024];
		strcpy(vv, v.c_str());
		send(clien_sock, vv, sizeof(vv), 1);
		cout<<endl;
	}
	else if(command==7){
	
		upload(clien_sock);
		int i,j;
		cout<<endl<<"UPLOADED: ";
		for(i=0;i<upload_d.size();i++){
			for(j=0;j<upload_d[i].size();j++){
				cout<<upload_d[i][j]<<" ";
			}
			cout<<endl;
		}
	}
	else if(command==8){
		list_files(clien_sock);
	}
	else if(command==9){
		download(clien_sock);
	}
	else if(command==10){
		remove_g(clien_sock);
	}
	else if(command==11){
		char uid[512];
		recv(clien_sock,&uid,sizeof(uid),0);
		
		for(int k=0;k<user_count;k++){
			if(!strcmp(user[k].uid,uid)){
				user[k].login=0;
				cout<<"Have set to 0.";
			}
		}
		
		
	}
	else if(command==12){
	cout<<"inside 12";
		char gid[512],fn[512],uid[512];
		int i,j;
		recv(clien_sock,&gid,sizeof(gid),0);
		recv(clien_sock,&fn,sizeof(fn),0);
		recv(clien_sock,&uid,sizeof(uid),0);
		
		//uploaded < fname,uid,gid, sha>>
		
		for(i=0; i<upload_d.size(); i++){
			vector<string>v=upload_d[i];
			if(v[0]==string(fn) && v[1]==string(uid) && v[2]==string(gid)){
				cout<<"Stop sharing.";
				v.clear();
				upload_d.erase(upload_d.begin()+i);
			}
			
			
		}
		
	}
	else {
		cout<<"Not found";
	}
		
	
}

void new_cli(int clien_sock){

	int command;		
	//handle request
	
		while(1){
	
			//get choice
			int n=recv(clien_sock,&command,sizeof(command),0);
			
			if(n<=0){
				cout<<"Done with this client";
				break;
			}
			cout<<"Client wants command "<<command<<endl;
			
			handle_request(clien_sock,command);
			
		}
				
	
}

int main(){
	int num=0;
	int port=9832;
	int opt=1;
	
	//create a socket
	int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(serv_sock==0){
		perror("socket failed"); 
        	exit(EXIT_FAILURE); 
	}
	
	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	//specify address for socket
	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port);
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	int len=sizeof(serv_addr);
	
	//bind
	int b=bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if(b<0){
		perror("Error in bind");
		exit(EXIT_FAILURE);
	}
	
	//listen
	if(listen(serv_sock, 50)<0){
		perror("Listen error");
		exit(EXIT_FAILURE);
	}
	
	thread thr[50];
	int i=0,ch,command,count_conn=0;
	
	
	while(1){
	
	
	
		//accept conn
		int clien_sock = accept(serv_sock, (struct sockaddr*)&serv_addr, (socklen_t*)&len);
		cout<<"SOCK "<<clien_sock<<endl;
		if(clien_sock<0){
			perror("Accept error");
			exit(EXIT_FAILURE);
		}
		
		cli_list.push_back(clien_sock);
		cout<<"count:"<<count_conn;
	
		thr[count_conn]=thread(new_cli,clien_sock );
		
		thr[count_conn].detach();
		count_conn++;
		
	
		
	}
	
	
	close(serv_sock);
	
	return 0;
}