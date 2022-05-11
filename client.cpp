#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <unistd.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include<fstream>
#include<bits/stdc++.h>
#include<cstring>
#include<thread>
#include<openssl/sha.h>
using namespace std;

struct udetails{
	char uid[512];
	char pwd[512];
	int port;
	char ip[512];
};

udetails user[50];
map< string, vector<string> > pending;
map< string, int> inc_req;
vector<string>downloading;
vector<string>complete;

int num=0;
char cur_user[512]={0};
int CHUN=-1;

void create_user(int cli_sock){
	char cu2[512],cu3[512];
	cin>>cu2>>cu3;
	memcpy(user[num].uid,cu2, sizeof(cu2));
	memcpy(user[num].pwd,cu3, sizeof(cu3));
	
	if(num!=0){
		memcpy(user[num].ip,user[0].ip, sizeof(user[0].ip));
		user[num].port=user[0].port;
		
	}
	
	//send details to tracker
	//ip
	send(cli_sock, &user[num].ip, sizeof(user[num].ip),0);
	
	//port
	send(cli_sock, &user[num].port, sizeof(user[num].port),0);
	
	//pwd
	send(cli_sock, &user[num].pwd, sizeof(user[num].pwd),0);
	
	//uid
	send(cli_sock, &user[num].uid, sizeof(user[num].uid),0);

	num++;
}


void login(int cli_sock){
	char cu2[512],cu3[512];
	cin>>cu2>>cu3;	
	
	//send details to tracker
	//uid
	send(cli_sock, &cu2, sizeof(cu2),0);
	
	//pwd
	send(cli_sock, &cu3, sizeof(cu3),0);
	
	//cout<<"Sent uid, pwd"<<cu2<<cu3;
	
	memcpy(cur_user, cu2, sizeof(cu2));
	
}


void create_group(int cli_sock){
	char gid[512];
	cin>>gid;	
	
	//send details to tracker
	//gid
	send(cli_sock, &gid, sizeof(gid),0);
	
	//uid
	send(cli_sock, &cur_user, sizeof(cur_user),0);
	cout<<"Sent details";


}

void process_input(char* arg1, char*arg2){
	
	char* ip_port = arg1;
	char* tracker_info = arg2;
	
	char ip[512];
	int port=0,port_str,coun=0,val=0;
	
	
	while(ip_port[coun]!=':'){
		ip[coun]=ip_port[coun];
		coun++;
	}
	ip[coun]='\0';
	
	
	while(ip_port[++coun]!='\0'){
		port=port*10;
		val = ip_port[coun]-'0';
		port+=val;
	}
	
	user[num].port = port;
	memcpy(user[num].ip, ip, sizeof(ip));
	
}

void check_conn(){

	
		int n_s = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in cad;
		char buffer[512];
		char bgid[512];
		int cp=user[0].port;
		int func=0;
		
		if(n_s>=0){
			cad.sin_family = AF_INET;
			cad.sin_port = htons(cp);
			cad.sin_addr.s_addr=INADDR_ANY;
			
			
			if (connect(n_s, (struct sockaddr *)&cad, sizeof(cad)) < 0)
			{
			return;
			}
			//cout<<"Connected--";
	
			recv(n_s,&func,sizeof(func),0);
			
		if(func==0){
			//cout<<"Adding to group part";
			recv(n_s, &buffer, sizeof(buffer),0);
			recv(n_s, &bgid, sizeof(bgid),0);
			//cout<<buffer<<" "<<bgid<<endl;
			
			//check if already in list, else append. ASSUME group definitely exists
			int flag=0;
			if(pending.find(string(bgid))==pending.end()){
				cout<<"Not found...Adding";
				vector<string>v;
				v.push_back(string(buffer));
				pending[(string(bgid))]=v;
				
			} else{
			
				for(auto it=pending.begin(); it!=pending.end(); it++){
					cout<< it->first<<" : ";
					for(auto jt=it->second.begin(); jt!=it->second.end() ; jt++){
						cout<< *jt<<" ";
						if(*jt==string(buffer)){
							flag=1;
							break;
						}
					}
					cout<<endl;
				}
				
				if(!flag){
					pending[string(bgid)].push_back(string(buffer));
				} else{
					cout<<endl<<"Already present in pending list"<<endl;
					
				}
			
			}
		} else {
		
			
			//cout<<"This is for downloading!";
			//get filename
			char file_name[512];
			char blocks[32],bstart[32],bend[32];
			
			recv(n_s, &file_name, sizeof(file_name),0);
			
			//get start block
			recv(n_s,&bstart, sizeof(bstart),0);
			
			//get end block
			recv(n_s,&bend, sizeof(bend),0);
			
		//	cout<<bend<<endl;
			
			int ind=0,num_b=0;
			
			int st=atoi(bstart);
			int en=atoi(bend);
			int las_block;
			
			//recv total bytes in files
			char block_ss[512];
			recv(n_s,&block_ss, sizeof(block_ss),0);
			
			//convert block_s to int
			int block_s=atoi(block_ss);
			
			//open file, read so many bytes as required
			
			FILE*fp=fopen(file_name, "r");
			if(fp==NULL){
				cout<<"File no exist here :/"<<endl;
				return;
			}		
			
			int req_bytes = (en-st)*512;
			char read_buf[req_bytes];
			
			if(st*512 + req_bytes <= block_s){
				//file has so many bytes
			
				fseek(fp, st*512, SEEK_SET);
				fread(&read_buf, 1, req_bytes, fp);
				
			
			} else{
				fseek(fp, st*512, SEEK_SET);
				las_block=block_s - (en-1-st)*512;
				req_bytes= block_s-st*512;
			//	cout<<" Last block---- "<<las_block;
			//	cout<<" To read: "<<req_bytes;
				fread(&read_buf, 1, req_bytes, fp);
			}
			
			send(n_s,&read_buf, req_bytes,0);
					
			fclose(fp);		
		}
				
			close(n_s);
		}
	
}

void list_pending(string gid){
	
	//fetches groups from this cli owner
	if(pending.find(gid)==pending.end()){
		cout<<"Group doesnt have pending requests..";
	} else{
		vector<string> xx=pending[gid];
		for(int i=0; i<xx.size(); i++){
			cout<<xx[i]<<" ";
		}
	}
	cout<<endl;

}

void communicate(int cli_sock, int po, char u[512], string gg){
	
	//connect to port and send a msg, receive a reply, then give reply to server
	
	
		int opt=1;
			//client conn
			int n_s = socket(AF_INET, SOCK_STREAM, 0);
			struct sockaddr_in cad;
			int new_socket;
			
			cad.sin_family = AF_INET;
			cad.sin_port = htons(po);
			cad.sin_addr.s_addr=INADDR_ANY;
			if (setsockopt(n_s, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
			{
				perror("setsockopt");
				exit(EXIT_FAILURE);
			}
			
			int addrlen=sizeof(cad);
			
			// Forcefully attaching socket to the port 8080
			if (bind(n_s, (struct sockaddr *)&cad,sizeof(cad))<0)
			{
				perror("bind failed");
				exit(EXIT_FAILURE);
			}
			if (listen(n_s, 50) < 0)
			{
				perror("listen");
				exit(EXIT_FAILURE);
			}
			
			if ((new_socket = accept(n_s, (struct sockaddr *)&cad, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			
			
			int func=0;
			send(new_socket,&func, sizeof(func),0 );
			
			char gg_n[512];
			
			
			strcpy(gg_n, gg.c_str());
			
			send(new_socket , &cur_user , sizeof(cur_user), 0);
			send(new_socket, &gg_n, sizeof(gg_n),0);
			
		
			close(n_s);


}

void accept_req(int cli_sock, string gid,string uid){
	
	int flag=0,ch=0,i=0;
	string cont;
	cont="0";
	

	
	
	//check if present in pending req
	if(pending.find(gid)==pending.end()){
		cout<<"Group not present in pending list..";
	} else {
	
	vector<string> v;
	v=pending[gid];
	for(i=0;i<v.size();i++){
		if(v[i]==uid){
			flag=1;
			break;
		}
	}
	
	if(flag==1){
	//if yes, tell server accepted.
		cout<<endl<<"Wish to accept group "<<gid<<"? (1/0)";
		cin>>ch;
		
	//send(cli_sock, &ch, sizeof(ch),0);
	
		if(ch==1){		
			cont=gid;
			cont+=":";
			cont+=uid;

	//remove from pending
		
			v.erase(v.begin()+i);
			pending[gid]=v;
		}
	
	}else {
		cout<<"Not pressent in pending list..";
	}
	
	}
	
	cout<<endl<<"Choice:"<<ch<<" ";
	char contt[1024];
	strcpy(contt, cont.c_str());
	//cout<<contt;
	send(cli_sock, &contt, sizeof(contt),0);
	//cout<<endl;
}

void upload(string fpath, string gg,int clien_sock){
	const int BUF_SIZE=512;
	char fname[512];
	strcpy(fname,fpath.c_str());
	
	cout<<"File to be downloaded:"<<fname<<endl;
	
	//uid
	send(clien_sock, &cur_user, sizeof(cur_user),0);
	
			
	//send a file
	send(clien_sock,&fname,sizeof(fname),0);
	
	
	
//ASSUMPTION:Current directory
	FILE *fp;
	fp=fopen(fname,"r");
	if(fp==NULL){
		perror("File not found");
		exit(1);
	}
	char data[BUF_SIZE+1];
	int block=BUF_SIZE,cn=0,st=0;
	char msg[9000];
	bzero(data, BUF_SIZE);
	bzero(msg, 9000);
	char end[]="-1";
	int len=0;
	
	fseek(fp,0L,SEEK_END);
	int sz=ftell(fp);
	
	//convert size to a string
	string ss=to_string(sz);
	//cout<<ss<<endl;
	//send(clien_sock, &sz, sizeof(sz),0);
	
	rewind(fp);
	
	while(sz>0){
		size_t ps = (sz>=512)? 512: sz;
 		block=fread(data, sizeof(char), BUF_SIZE, fp);
		//cout<<"BLOCK: "<<block<<"_________________"<<cn++<<endl;
		data[block]='\0';
		
		
		sz-=512;
		
		
		
		unsigned char dat[block+1];
		memcpy(dat,data,ps);
		
		
		
		size_t length = sizeof(dat);

		unsigned char hash[SHA_DIGEST_LENGTH];
		SHA1(dat, length, hash);


		char mdString[SHA_DIGEST_LENGTH];
		 
		for(int i = 0; i < (SHA_DIGEST_LENGTH/2) - 1; i++)
		   sprintf(&mdString[i*2], "%02x", (unsigned int)hash[i]);
	 
	    	//printf("sha1 digest: %s\n", mdString); 
		strcat(msg,mdString);
		len+=18;
		
		bzero(data, BUF_SIZE);	
		
		
	}
	
	fclose(fp);
	
	//gid
	char ggi[512];
	strcpy(ggi,gg.c_str());
	send(clien_sock, &ggi, sizeof(ggi),0);
	
	//SHA of all blocks together
	send(clien_sock, &msg, sizeof(msg),0);
	
	
	//create an mtorrent file
	char path[512];
	strcpy(path,fname);
	strcat(path,cur_user);
	strcat(path,".mtorrent.txt");
	cout<<"File created at "<<path<<endl;
	FILE *fp1;
	fp1=fopen(path,"w");
	
	//fname
	int ina=0;
	while(fname[ina++]!='\0');
	
	
	fwrite(fname,ina-1,1,fp1);
	fwrite("\n",1,1,fp1);
	
	fwrite(msg,len,1,fp1);
	fwrite("\n",1,1,fp1);
	
	char sst[64];
	strcpy(sst,ss.c_str());
	fwrite(sst,ss.length(),1,fp1);
	fwrite("\n",1,1,fp1);
	
	string ssb=to_string(cn);
	char ssbb[64];
	strcpy(ssbb, ssb.c_str());
	fwrite(ssbb,ssb.length(),1,fp1);
	
	
	fclose(fp1);
}

void set_size(int chunks){
	CHUN=chunks;
}


int get_size(){
	return CHUN;
}


void sendData(int st, int en,int po,string ip,string uid,string ftorrent, string fn,string destp, string gd){
	//connect to port and send a msg, receive a reply, then give reply to server
		char file_o[512];
		strcpy(file_o,destp.c_str());
		
		int o=0,ind=0;
		
		
			if(st==0){	
			downloading.push_back(gd);
			downloading.push_back(fn);
			}
			
			int opt=1;
			//client conn
			int n_s = socket(AF_INET, SOCK_STREAM, 0);
			struct sockaddr_in ca;
			int new_socket;
			
			ca.sin_family = AF_INET;
			ca.sin_port = htons(po);
			ca.sin_addr.s_addr=INADDR_ANY;
			if (setsockopt(n_s, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
			{
				perror("setsockopt");
				exit(EXIT_FAILURE);
			}
			
			int addrlen=sizeof(ca);
			
			// Forcefully attaching socket to the port 8080
			if (bind(n_s, (struct sockaddr *)&ca,sizeof(ca))<0)
			{
				perror("bind failed");
				exit(EXIT_FAILURE);
			}
			if (listen(n_s, 50) < 0)
			{
				perror("listen");
				exit(EXIT_FAILURE);
			}
			
			if ((new_socket = accept(n_s, (struct sockaddr *)&ca, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			
			
			int func=1;
			send(new_socket, &func, sizeof(func),0);
			
			//tell which file i want
			char fnn[512];
			strcpy(fnn, fn.c_str());
			send(new_socket, &fnn,sizeof(fnn),0);
			
			//tell start chunk i need
			char str[32];
			strcpy(str, to_string(st).c_str());
			send(new_socket, &str, sizeof(str),0);
			
			//tell end chunk i need
			char etr[32];
			strcpy(etr, to_string(en).c_str());
			send(new_socket, &etr, sizeof(etr),0);
			
			//tell size of file coz chunks=size/512
			
			FILE*fpt;
			fpt=fopen(ftorrent.c_str(),"r");
			
			fseek(fpt, 0L, SEEK_END);
			int size_torrent=ftell(fpt);
			rewind(fpt);
			
			char read_val[size_torrent];
			bzero(read_val, sizeof(read_val));

			//total blocks
			int total_blocks=get_size();			
					
			fseek(fpt,18*total_blocks+fn.size()+1,SEEK_SET);
			fread(&read_val,1, to_string(512*total_blocks).size() +1,fpt);
			
			//sent size of all data in file
			send(new_socket, &read_val, to_string(512*total_blocks).size() +1,0);
			
			//receive data
			int d_sz=512*(en-st);
			if(en>=total_blocks){
				d_sz=atoi(read_val)-(512*st); 
			
			}
			char data[d_sz];
			bzero(data,sizeof(data));
			recv(new_socket,&data, sizeof(data),0);
			//cout<<endl<<data<<endl;
			
			//store data 
			
			FILE* of;
			of=fopen(file_o,"a+");
			
			if(of!=NULL){
				cout<<"existing";
				fseek(of,0,SEEK_END);
			}
			
			fwrite(data,sizeof(data),1,of);
			fclose(of);
			
			if(en>=total_blocks){
			
			cout<<endl<<"Integrity maintained";
			complete.push_back(gd);
			complete.push_back(fn);
			
			downloading.pop_back();
			downloading.pop_back();
			
			}
			close(new_socket);
			close(n_s);
			
		
			


}
void down(int clien_sock, string gid, string fname, string destp){
	
	int i,j,count=-1;
	char fn[512];
	char gd[512];
	char ip[512];
	char ud[512];
	int port=0,sz=0;
	vector<int>po;
	vector<string>ipp;
	vector<string>uid;
	vector<int>size;
	
	//sends fname and gid
	strcpy(fn,fname.c_str());
	//cout<<"FN"<<fn;
	
	
	strcpy(gd,gid.c_str());
	cout<<"Sending..."<<fn<<" "<<gd<<endl;
	
	send(clien_sock, &gd, sizeof(gd),0);
	send(clien_sock, &fn, sizeof(fn),0);
	
	
	//recv count
	recv(clien_sock, &count, sizeof(count),0);
	//cout<<"COUNT"<<count<<endl;
	
	//recv uploader uids. Thus calc fname+uid+.mtorrent.txt
	//recv port,ip
	for(i=0;i<count;i++){
				
		recv(clien_sock, &ud, sizeof(ud),0);
		recv(clien_sock,&ip,sizeof(ip),0);
		recv(clien_sock,&port,sizeof(port),0);
		recv(clien_sock,&sz,sizeof(sz),0);
		
		po.push_back(port);
		ipp.push_back(ip);
		uid.push_back(ud);
		size.push_back(sz);
		//cout<<"RECEIVED "<<port<<" "<<ip<<" "<<ud<<" "<<sz<<endl;
	}
	
	//for 1st uploader
	
	cout<<"DONE receiving. next..."<<endl;
	
	
	
	//recv chunks available
	int chunks=size[0]/18;
	cout<<chunks;
	int rem=chunks%po.size();
	int avg=chunks/po.size();
	
	set_size(chunks);
	//cout<<"Chunks set.. "<<endl;
	
	
	//establish connection to port
	//get what chunk it wants from which port it likes.THreads!
	thread tt[60];
	
	//PIECE SELECTION ALGO
	int fl=1,st=0,en;
	i=0;
	//cout<<"AVG:"<<avg<<":PO SIZE:"<<po.size()<<endl;
	
	//send po size
	//assume po.size()==3
	
	avg=(chunks)/po.size();
	en=avg;
	
	int max_n=po.size();
	rem=chunks%max_n;
	
	//change 3 to po.size()
	for(i=0;i<max_n;i++){
		string ftorrent=fn+uid[i]+".mtorrent.txt";
		
	//	cout<<ftorrent<<"ST"<<st<<"EN"<<en<<endl;
			
		
		if(en>chunks && st>=chunks){
	//	cout<<"en greater"<<en;
			break;
		}
		
		if(st<chunks && en>chunks){
			en=chunks;
		}
		
		
		
		if(po[i]==user[i].port){
			cout<<"SAME PORT";
			fl=0;
		}
		if(fl){
			char ipi[512], uidd[512];
			strcpy(ipi,ipp[i].c_str());
			strcpy(uidd, uid[i].c_str());
	//		cout<<"heeere";
			
			//sendData( st,en, po[i], ipi, uidd, ftorrent, fn);
			tt[j]=thread(sendData, st,en, po[i], ipi, uidd, ftorrent, fn,destp,gid);
			tt[j].join();
		}
		
		st=st+avg;
		en=en+avg;
	}
	
	
	//check sha1
	
	//write to file
	
	
}

int main(int argc, char* argv[]){

	int port=9832;
	
	if(argc<2){
		perror("Insufficient arguments");
		exit(EXIT_FAILURE);
	}
	
	
	//./client ip:port tracker_info.txt
	process_input(argv[1], argv[2]);
	
	
	//create a socket
	int cli_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(cli_sock==0){
		perror("socket failed"); 
        	exit(EXIT_FAILURE); 
	}
	
	//specify address for socket
	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port);
	serv_addr.sin_addr.s_addr=inet_addr(user[num].ip);
	int len=sizeof(serv_addr);
	
	
	//commands		
	char str[512];
	int com=1;
	int status=0;
	
	//connect
		int conn=connect(cli_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
		if(conn==-1){
			perror("Connection error");
			exit(EXIT_FAILURE);
		}
	
	while(1){	
		
		thread p(check_conn);		
		p.join();
		
		
		cin>>str;
			
				
		if(!strcmp("create_user",str)){	
		
			com=1;
		
			//send server the command
			send(cli_sock, &com , sizeof(com), 0);

			create_user(cli_sock);
			
			
			cout<<"User registered"<<endl;
		} 
		else if(!strcmp("login",str)){
			com=2;
			
			//send server the command
			send(cli_sock, &com , sizeof(com), 0);
			
			login(cli_sock);
			recv(cli_sock, &status, sizeof(status), 0);
			cout<<"User status: "<<status<<endl;
			
			
		} 
		else if(!strcmp("create_group",str)){
			com=3;
			
			//send server the command
			send(cli_sock, &com , sizeof(com), 0);
			
			create_group(cli_sock);

		}
		else if(!strcmp("join_group",str)){
			com=4;
			
			//send server the command
			send(cli_sock, &com , sizeof(com), 0);
			
			create_group(cli_sock);
			char found[512];
			
			recv(cli_sock, &found, sizeof(found),0);
			if(!strcmp(found,"-")){
				cout<<"Couldnt add to group";
			}
			else{
				cout<<endl<<found;
				
				int xx=0,fl=0,ind=0,i2=0;
				char us[512],gg[512];
				int po=0,rem;
				
				while(found[xx]!='\0'){
					if(found[xx]==':'){
						fl++;
						if(fl==1)
							us[ind]='\0';
						xx++;
						ind=0;
					}
					if(fl==1){
						rem=int(found[xx]-'0');
						po=po*10+rem;
						
					} else if(fl==2){
						gg[ind]=found[xx];
						ind++;
					
					}else{
						us[ind]=found[xx];
						ind++;
					}
					xx++;
				}
				gg[ind]='\0';
				
	//			cout<<"UID-"<<us<<"-P-"<<po<<"-gid-"<<gg<<endl;
				
				inc_req[string(gg)]=po;
				
				if(po==user[0].port){
					cout<<"Same client!";
					int flag=0;
					if(pending.find(string(gg))==pending.end()){
						cout<<"Not found...Adding";
						vector<string>v;
						v.push_back(string(cur_user));
						pending[(string(gg))]=v;
						
					} else{
					
						for(auto it=pending.begin(); it!=pending.end(); it++){
						cout<< it->first<<" : ";
						for(auto jt=it->second.begin(); jt!=it->second.end() ; jt++){
							cout<< *jt<<" ";
							if(*jt==string(cur_user)){
								flag=1;
								break;
							}
						}
						cout<<endl;
						}
					
					if(!flag){
						pending[string(gg)].push_back(string(cur_user));
					} else{
						cout<<endl<<"Already present in pending list"<<endl;
						
					}
				
				}
				} else{
					communicate(cli_sock, po,us,gg);
				}
			}
			

		}
		else if(!strcmp("requests",str)){
			string s3,rgid;
			cin>>s3>>rgid;
			
			list_pending(rgid);
			
		
		} 
		else if(!strcmp("accept_request", str)){
		//accept_request <group_id> <user_id>
			string agid, auid;
			cin>>agid>>auid;
			
			com=5;
			send(cli_sock, &com, sizeof(com), 0);
			accept_req(cli_sock, agid, auid);
		
		}
		else if(!strcmp("list_groups",str)){
			com=6;
			send(cli_sock, &com, sizeof(com),0);
			char vv[1024];
			recv(cli_sock, &vv, sizeof(vv),0);
			cout<<"GROUPS: "<<vv;
			cout<<endl;
			
			
		}
		else if(!strcmp("upload_file",str)){
		//upload_file <file_path> <group_id>
			com=7;
			send(cli_sock, &com, sizeof(com),0);
			string fpath, gg;
			cin>>fpath>>gg;
			
			upload(fpath,gg,cli_sock);
		}
		else if(!strcmp("list_files",str)){

			com=8;
			send(cli_sock, &com, sizeof(com),0);
			string ggg;
			cin>>ggg;
			char gg[512];
			strcpy(gg,ggg.c_str());
			send(cli_sock,&gg,sizeof(gg),0);
			
			//receive
			cout<<endl<<"FILES in group : ";
			char fil[9000];
			recv(cli_sock, &fil, sizeof(fil),0);
			cout<<fil<<endl;
			
		}
		else if(!strcmp("download_file",str)){
		//download_file <group_id> <file_name> <destination_path>
			com=9;
			send(cli_sock,&com,sizeof(com),0);
			char gg[512],fn[512],dp[512];
			cin>>gg>>fn>>dp;
			down(cli_sock,gg,fn,dp);	
		
		}
		else if(!strcmp("leave_group",str)){
			com=10;
			
			send(cli_sock,&com,sizeof(com),0);
			
			char gg[512];
			cin>>gg;
			cout<<gg<<" "<<cur_user;
			
			cout<<send(cli_sock, &gg, sizeof(gg),0);
			cout<<send(cli_sock, &cur_user, sizeof(cur_user),0);
			cout<<"SENT";	
		
		}
		else if(!strcmp("logout",str)){
			com=11;
			send(cli_sock,&com,sizeof(com),0);
			send(cli_sock, &cur_user, sizeof(cur_user),0);
			cout<<"LOGOUT SUCCESS"<<endl;
			strcpy(cur_user,"-");
		}
		else if(!strcmp("show_downloads",str)){
			
			int i;
			cout<<endl;
			for(i=0;i<downloading.size();i+=2){
			cout<<"[D]"<<downloading[i]<<" "<<downloading[i+1]<<endl;
			}
			
			for(i=0;i<complete.size(); i+=2){
			cout<<"[C]"<<complete[i]<<" "<<complete[i+1]<<endl;
			}
			
		}
		else if(!strcmp("stop_share",str)){
			
			com=12;
			send(cli_sock, &com, sizeof(com),0);
			
			char gg[512],fn[512];
			cin>>gg>>fn;
			
			//send gid, fname, uid
			send(cli_sock,&gg,sizeof(gg),0);
			send(cli_sock,&fn,sizeof(fn),0);
			send(cli_sock,&cur_user, sizeof(cur_user),0);
		}			
		else {
			cout<<"wrong option";	
				
		}
		
		
		
		
		
	}
	
	//close socket
	close(cli_sock);
	
	
}
