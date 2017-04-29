#include <iostream>
#include <fstream>
#include <list>
#include <stack>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include <bits/stdc++.h>

using namespace std;

float lmax,lmin;
int n,nnodes;
int processed=0;
int MAXP = 1000;
float timer=0;
float period = 0.01;
struct packet
{
	int pid;
	float startt;
	float path_no;
	//int state;
	int curr;
	float rem;
};
struct Node{
	int nid;
	vector<packet> q;
};
vector<packet> pq;
struct Flow{
	int fid;
	int path_len;
	vector<int> path;
	float T;
	float recg;
	vector<float> C;
	float R;
	vector<float> deadline;
	int count;
	float timed ;
};
vector< Flow > flow;
vector< Node > node;
void read_input(char *input_file){
	ifstream in(input_file);
	in>>lmax>>lmin;
	in>>n>>nnodes;
	node.resize(nnodes+1);
	flow.resize(n+1);
	for(int j=1;j<=n;j++){
		int flow_id, no_nodes, tmp;
		float t_min;
		in>>flow_id>>t_min>>no_nodes;
		flow[flow_id].fid = flow_id;
		flow[flow_id].T = t_min;
		flow[flow_id].recg = 0-t_min;
		flow[flow_id].path_len = no_nodes;
		flow[flow_id].R=0;
		for(int i=0;i<flow[flow_id].path_len;i++){
			in>>tmp;
			flow[flow_id].path.push_back(tmp);
		}
	}
	for(int i=1;i<=n;i++){
		flow[i].C.push_back(0);
		float tmp;
		for(int j=1;j<=nnodes;j++){
			in>>tmp;
			flow[i].C.push_back(tmp);
		}
	}
}
void print_output();
void simulate(void){
	srand(time(NULL));
	int mm[14]={0};
	while(processed<=MAXP){
		//Traffic generator part
		//cerr<<"flag\n";
		for(int i=1;i<=n;i++){
			//cerr<<"fa "<<timer<<endl;
			if(timer-flow[i].recg>=flow[i].T){
				//srand(time(NULL));
				int tmp = rand()%10000;
				if(tmp<=700){
					//cerr<<"ada "<<timer<<" "<<i<<endl;
					packet p;
					p.startt = timer;
					p.path_no = i;
					p.curr = 0;
					p.rem = flow[i].C[flow[i].path[0]];
					flow[i].recg = timer;
					node[flow[i].path[0]].q.push_back(p);
					//cerr<<"fafa\n";
				}
			}
		}
		//Nodes runner
		//cout<<"runner "<<timer<<endl;
		for(int i=1;i<=nnodes;i++){
			if(node[i].q.empty()) continue;
			if(node[i].q.size()>mm[i])mm[i] = node[i].q.size();
			if(node[i].q[0].rem<=period){
				if(node[i].q[0].curr==flow[node[i].q[0].path_no].path_len-1){
					processed++;
					//cout<<"running\n\n\n\n\n\n\n";
					if(processed%1000==0){
						cout<<"Processed "<<processed<<endl;
						for(int i=1;i<=nnodes;i++){
							cout<<"Max queue size at node"<<i<<"= "<<mm[i]<<endl;
						}	
						print_output();
					}
					flow[node[i].q[0].path_no].count++;
					flow[node[i].q[0].path_no].timed+= (timer-node[i].q[0].startt);
					if(timer-node[i].q[0].startt > flow[node[i].q[0].path_no].R){
						flow[node[i].q[0].path_no].R = timer-node[i].q[0].startt;
					}
				}
				else{
					packet p = node[i].q[0];
					p.curr++;
					p.rem = lmin+((rand()%1000+1)*1.0/ 1000.0)*(lmax-lmin);
					pq.push_back(p);
				}
				node[i].q.erase(node[i].q.begin());
			}
			else{
				node[i].q[0].rem-=period;
			}
		}
		//Path runner
		int tmp = pq.size();
		for(int i=0;i<tmp;i++){
			pq[i].rem-=period;
			if(pq[i].rem<=0){
				pq[i].rem = flow[pq[i].path_no].C[flow[pq[i].path_no].path[pq[i].curr]];
				node[flow[pq[i].path_no].path[pq[i].curr]].q.push_back(pq[i]);
				tmp--;
				pq.erase(pq.begin()+i);
				i--;
			}
		}
		timer+=period;
	}
	for(int i=1;i<=nnodes;i++){
		cout<<"Max queue size at node"<<i<<"= "<<mm[i]<<endl;
	}
}
void print_output(void){
	for(int i=1;i<=n;i++){
		cout<<"Worst and average for flow "<<i<<" = "<<flow[i].R<<" "\
		<<(flow[i].timed)/(flow[i].count*1.0)<<" "<<flow[i].count<<endl;
	}
	for(int i=1;i<=n;i++){
		//cout<<"Average delay for flow "<<i<<" = "<<(flow[i].timed)/(flow[i].count*1.0)<<" "<<flow[i].count<<endl;
	}
}

int main(){
	char input_file[]="input.txt";
	read_input(input_file);
	simulate();
	print_output();
}