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

//Implementation of trajectory approach for computation of worst case end to end delay in switched ethernet network.

/* Inputs
	1. Lmax and Lmin (Upper and Lower bounds on network delay between any two nodes in the network)
	2. Set of flows 
		Number of flows, Number of nodes
		Flow specifications:
			(Flow No.) (Minimum Interarrival Time) (No. of nodes visited by flow) ({List of nodes in the path of flow})
		A table of size (No_flows * No_nodes)
			entry(i,j) = Max processing time of packet of flow i on node j
*/

/* Expected Output:
	Worst case end-to-end delay (response time) for each flow in the network.
*/

// Global variables
float lmax, lmin;
int n,nnodes;
struct Flow{
	int fid;
	int path_len;
	vector<int> path;
	float T;
	vector<float> C;
	vector<float> worst;
	float R;
};

vector<Flow> flow;

// Functions definitions

void read_model(char *input_file){
	ifstream in(input_file);
	in>>lmax>>lmin;
	in>>n>>nnodes;
	flow.resize(n+1);
	for(int j=1;j<=n;j++){
		int flow_id, no_nodes, tmp;
		float t_min;
		in>>flow_id>>t_min>>no_nodes;
		flow[flow_id].fid = flow_id;
		flow[flow_id].T = t_min;
		flow[flow_id].path_len = no_nodes;
		flow[flow_id].worst.resize(no_nodes);
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

bool verify_model(){
	return true;
}

float computeW(int i, float t, int node_ind);

int last(int fl){
	return flow[fl].path[flow[fl].path_len-1];
}
int first1(int fl1, int fl2, int node_ind){
	for(int i=0;i<=node_ind;i++){
		int tmp1 = flow[fl1].path[i];
		for(int j=0;j<flow[fl2].path_len;j++){
			if(tmp1==flow[fl2].path[j])
				return tmp1;
		}
	}
	return -1;
}
int first2(int fl1, int fl2, int node_ind){
	for(int i=0;i<flow[fl1].path_len;i++){
		int tmp1 = flow[fl1].path[i];
		for(int j=0;j<=node_ind;j++){
			if(tmp1==flow[fl2].path[j])
				return tmp1;
		}
	}
	return -1;
}

int slow(int j, int i, int node_ind){
	float minval=-1;
	int found=0;
	int ans=0;
	for(int it=0;it<flow[j].path_len;it++){
		int tmp1 = flow[j].path[it];
		for(int it1 = 0;it1<=node_ind;it1++){
			if(tmp1==flow[i].path[it1]){
				found = 1;
				if(minval<flow[j].C[tmp1]){
					minval = flow[j].C[tmp1];
					ans = tmp1;
				}
			}
		}
	}
	if(found==1)return ans;
	return -1;
}

int slow(int i, int node_ind){
	float minval=-1;
	for(int j=0;j<=node_ind;j++){
		if(flow[i].C[flow[i].path[j]]>minval){
			minval = flow[i].C[flow[i].path[j]];
		}
	}
	return minval;
}
int getind(int i, int node){
	for(int j = 0;j<flow[i].path_len;j++){
		if(node==flow[i].path[j])
			return j;
	}
	return -1;
}

float Smax(int i, int node_ind){
	if(node_ind==0){
		return 0;
	}
	return computeW(i,0,node_ind-1)+lmax+flow[i].C[flow[i].path[node_ind-1]];
}

float Smin(int i, int node_ind){
	float ans=0;
	for(int j=0;j<node_ind;j++){
		ans+= flow[i].C[flow[i].path[j]];
	}
	ans= ans+(node_ind)*lmin;
	return ans;
}

float M(int i, int node_ind){
	float ans=0;
	for(int iter=0;iter<node_ind;iter++){
		int h = flow[i].path[iter];
		float mintmp=1e9;
		for(int j=1;j<=n;j++){
			if(first1(i,j,node_ind)==first2(j,i,node_ind)){
				if(flow[j].C[h]<mintmp)mintmp = flow[j].C[h];
			}
		}
		ans+=mintmp;
		ans+=lmin;
	}
	return ans;
}

float A(int i, int j, int node_ind){
	return Smax(i, getind(i,first2(j,i,node_ind)))-Smin(j,getind(j,first2(j,i,node_ind)))\
	-M(i,getind(i,first1(i,j,node_ind)))+Smax(j,getind(j,first1(i,j,node_ind)));
}

float computeW(int i, float t, int node_ind){

	float sumterm[5]={0};
	// Computing sumterm[0]
	for(int j=1;j<=n;j++){
		if(j==i)continue;
		int tmp = slow(j,i,node_ind);
		if(tmp!=-1){
			sumterm[0]+= ((1+ floor((t+A(i,j,node_ind))/flow[j].T))*flow[j].C[tmp]);
		}
	}
	//Computing sumterm[1]
	int tmp = slow(i,node_ind);
	sumterm[1] = (1+floor(t/flow[i].T))*flow[i].C[tmp];
	//Computing sumterm[2]
	for(int iter=0;iter<=node_ind;iter++){
		int h = flow[i].path[iter];
		if(h==tmp)continue;
		float maxtmp=0;
		for(int j=1;j<=n;j++){
			if(first1(i,j,node_ind)==first2(j,i,node_ind)){
				if(flow[j].C[h]>maxtmp)maxtmp = flow[j].C[h];
			}
		}
		sumterm[2]+=maxtmp;
	}
	//Computing sumterm[3]
	sumterm[3] = (-1)*flow[i].C[flow[i].path[node_ind]];
	//Computing sumterm[4]
	sumterm[4] = (node_ind)*lmax;
	float summ=0;
	for(int tmpi=0;tmpi<5;tmpi++)
		summ+=sumterm[tmpi];
	return summ;
}

void computeR(int fl){
	float t=0;
	flow[fl].R = computeW(fl,t,flow[fl].path_len-1)+flow[fl].C[last(fl)]-t;
}

void compute_delays(void){
	float t = 0;
	for(int i=1;i<=n;i++)
		computeR(i);
}

void print_output(void){
	for(int i=1;i<=n;i++){
		cout<<"Worst case estimated delay of flow no. "<<i<<" = "<<flow[i].R<<endl;
	}
	ofstream out("out.txt");
	for(int i=1;i<=n;i++){
		for(int j=0;j<flow[i].path_len;j++){
			out<<computeW(i,0,j)+flow[i].C[flow[i].path[j]]<<" ";
		}
		out<<endl;
	}
}
int main(){
	char input_file[] = "input.txt";
	read_model(input_file);
	if(verify_model()==false){
		printf("Invalid model. Please check the model and make in consistent\n");
		exit(0);
	}
	compute_delays();
	print_output();
}