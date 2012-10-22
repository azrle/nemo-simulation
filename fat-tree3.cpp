#include <iostream>
#include <ctime>
#include <cstdlib>
using namespace std;

#define MAX_K 60
#define MAX_RECV 100000
#define MAX_DEGREE 9

class Edge {
private:
    int from, to;
    int weight;
public:
    Edge(int f = -1, int t = -1, int w = 1) { init(f, t, w); }
    void init(int f, int t, int w = 1) {
        from = f; to = t;
        weight = w;
    }
    bool isConnect(int f, int t, bool directed = false) {
        if (from == f && to == t) return true;
        if (!directed) {
            if (to == f && from == t) return true;
        }
        return false;
    }
    int getWeight() {
        return weight;
    }
};


int main(int argc, char *argv[]) {
    int edge_num = 0;
    // Edge edges[MAX_K*MAX_K*MAX_K<<1];
    // Edge w_edges[MAX_K*MAX_K*3];
    short w_matrix[MAX_K*MAX_K/2][MAX_K*MAX_K/2]; // w_matrix[i][j] means ToR_i connects to ToR_j via wireless link
    bool w_matrix_pod[MAX_K*MAX_K/2][MAX_K]; // w_matrix_pod[i][j] means ToR_i connects to Pod_j via wireless link

    int pod_num; // number of pods, i.e. k
    int recv_num = 10; // number of targets (servers)
    int sim_num = 100;
    int BIC = 3;

    cout<<"Number of pods: ";
    cin>>pod_num;
    cout<<"C: ";
    cin>>BIC;
    cout<<"Number of target servers: ";
    cin>>recv_num;
    cout<<"Number of simulations: ";
    cin>>sim_num;
    
    if (pod_num & 1) {
        cout<<"Pod number should be even."<<endl;
        return -1;
    }

    int pod_i, pod_sq = pod_num*pod_num, pod_half = (pod_num>>1), i, j;
    
    // Links between ToRs and Aggr. switches
    for (pod_i=0; pod_i<pod_num; ++pod_i) {
        for (i=0; i<(pod_half); ++i) {
            for (j=0; j<pod_half;++j) {
            //    edges[edge_num++].init(pod_i*pod_half+j, (pod_sq>>1)+pod_i*pod_half+i);
            }
        }
    }

    // Links between Aggr. switches and cores
    j = 0;
    int ports = pod_half, t;
    for (i=pod_sq; i<pod_sq+pod_half*pod_half; ++i) {
        for (t=0; t<pod_num; ++t) {
            if (ports == 0) {
                ++j;
                ports = pod_half;
            }
            // edges[edge_num++].init(i, j+(pod_half*t)+(pod_sq>>1));
        }
        --ports;
    }
    
    // Fat-tree done.

    for (i=0; i<(pod_sq>>1); ++i)  for (j=0; j<(pod_sq>>1); ++j) w_matrix[i][j] = 0;
    for (i=0; i<(pod_sq>>1); ++i) for (j=0; j<pod_num; ++j) w_matrix_pod[i][j] = false;
    // Add wireless links
    int w_edge_num = 0;
    int dept;
    for (i=0; i<(pod_sq>>1); ++i) {
        if (i-pod_num>=0) {
//           w_edges[w_edge_num++].init(i-pod_num, i);
            w_matrix[i-pod_num][i] = w_matrix[i][i-pod_num] = 1;
            w_matrix_pod[i][(i-pod_num)/pod_half] = w_matrix_pod[(i-pod_num)][i/pod_half] = true;

            dept = 1;
            while (((i-pod_num+dept)%pod_num != 0 || (i-pod_num+dept == 0)) && dept<BIC) {
//                w_edges[w_edge_num++].init(i-pod_num+1, i);
                w_matrix[i-pod_num+dept][i] = w_matrix[i][i-pod_num+dept] = 1;
                w_matrix_pod[i][(i-pod_num+dept)/pod_half] = w_matrix_pod[(i-pod_num+dept)][i/pod_half] = true;
                ++dept;
            }
            
            dept = 1;
            while ((i+dept)%pod_num != 0 && dept<BIC) {
//                w_edges[w_edge_num++].init(i-pod_num, i+1);
                w_matrix[i-pod_num][i+dept] = w_matrix[i+dept][i-pod_num] = 1;
                w_matrix_pod[(i+dept)][(i-pod_num)/pod_half] = w_matrix_pod[(i-pod_num)][(i+dept)/pod_half] = true; 
                ++dept;
            }
        } 
        if ((i+1)%pod_num != 0) {
            w_matrix[i][i+1] = w_matrix[i+1][i] = 1;
            w_matrix_pod[i][(i+1)/pod_half] = w_matrix_pod[(i+1)][i/pod_half] = true;
        }
    }

    // Wireless links done.
    

    /*
     * Simulation begins here
     *
     * 1 Randomly pick up a source (ToR) and many targets (ToRs)
     * 2 Compute original length
     * 3 Find links for replacement
     *  3.1 Find directed wireless link from ToR to ToR (ToR->ToR) 
     *  3.2 No directed wireless link but in same pod (ToR--->A--->ToR) 
     *  3.3 Source connects to Aggre. in which target in via wireless link (ToR->ToR--->A--->ToR)
     *          or ToR--->A--->ToR->ToR
     *  3.4 User wired link (4 hops)
     *
     */
    srand( (unsigned)time(NULL));
    double TOT_percent = 0;

    int src, ee, cur_pod, target_pod, aggr, aggr2, core;
    bool targets[MAX_RECV];
    bool edge_flag[MAX_K*MAX_K*MAX_K];
    int path[MAX_K*MAX_K>>1][3];
    int card[4] = {0};
for (int sim = 0; sim<sim_num; ++sim) {
    // Step 1
    src = rand()%(pod_sq>>1);
    for (i=0; i<MAX_RECV; ++i) targets[i] = false;
    for (i=0; i<(MAX_K*MAX_K*MAX_K>>2); ++i) edge_flag[i] = false;

    cur_pod = src/pod_half;

    cout<<"Source (ToR): "<<src<<endl;
    if (sim_num<4 && recv_num<10) cout<<"Targets (ToRs): "<<endl;
    for (i=0; i<recv_num; ++i) {
        do {
            t = rand()%(pod_sq>>1);
        } while (t==src);
        if (sim_num<4 && recv_num<10) cout<<"#"<<i+1<<"\t"<<t<<endl;
        targets[t] = true;
    }
    
    // Step 2

    int origin_len = 0;
    for (i=0; i<pod_num*pod_half; ++i) {
        if (! targets[i]) continue;
        target_pod = i/pod_half;
        if (target_pod == cur_pod) { 
            // in same pod
            aggr = rand()%pod_half;
            ee = cur_pod * (pod_sq>>2) + aggr*pod_half + src-cur_pod*pod_half;
            if (!edge_flag[ee]) { origin_len++; edge_flag[ee] = true; }
            ee = target_pod * (pod_sq>>2) + aggr*pod_half + i-target_pod*pod_half;
            if (!edge_flag[ee]) { origin_len++; edge_flag[ee] = true; }
            // origin_len += 2;
        } else { 
            // in different pod
            // origin_len += 4;
            aggr = rand()%pod_half;
            ee = cur_pod * (pod_sq>>2) + aggr*pod_half + src-cur_pod*pod_half;
            if (!edge_flag[ee]) { origin_len++; edge_flag[ee] = true; }
            
            core = rand()%pod_half;
            ee = ((pod_sq*pod_num)>>2) + cur_pod*(pod_sq>>2) + aggr*pod_half + core;
            if (!edge_flag[ee]) { origin_len++; edge_flag[ee] = true; }
            
            aggr2 = rand()%pod_half;
            ee = ((pod_sq*pod_num)>>2) + target_pod*(pod_sq>>2) + aggr2*pod_half + core;
            if (!edge_flag[ee]) { origin_len++; edge_flag[ee] = true; }

            ee = target_pod * (pod_sq>>2) + aggr2*pod_half + i-target_pod*pod_half;
            if (!edge_flag[ee]) { origin_len++; edge_flag[ee] = true; }
        }
    }

    // Step 3 
    int new_len = 0;
    
    bool hop[4][MAX_K*MAX_K>>1];
    for (i=0; i<4; ++i) for (j=0; j<(MAX_K*MAX_K>>1); ++j) hop[i][j]=false;
    
    int st=0, ed=0, cur, po, dep;
    int queue[MAX_K*MAX_K>>1];
    int prev[MAX_K*MAX_K>>1];
    queue[ed++] = src;
    hop[0][src] = true;
    
    for (i=0; i<(MAX_K*MAX_K>>1); ++i) prev[i] = -1;

    while (st < ed) {
        cur = queue[st];
        for (j=0; j<3; ++j) if (hop[j][cur]) break;
        if (j >= 3) continue;

       for (i=0; i<(pod_sq>>1); ++i) {
           if (w_matrix[cur][i] && !hop[0][i] && !hop[1][i] && !hop[2][i] && !hop[3][i]) {
                if (!hop[j+1][i]) ++card[j+1];
               hop[j+1][i] = true;

               dep = j;
               po = st;
               path[i][dep--] = i*(pod_sq>>1)+queue[po];
               while (po != -1 && prev[po] != -1 && dep>=0) {
                   path[i][dep--] = queue[po]*(pod_sq>>1)+queue[prev[po]];
                   po = prev[po];
               }
               // cout<<"To "<<i<<" in "<<j+1<<"hops"<<endl;
               if (j < 2) {
                   prev[ed] = st;
                   queue[ed++] = i;
               }
           }
       }
       st++;
    }

    for (i=0; i<(MAX_K*MAX_K*MAX_K>>2); ++i) edge_flag[i] = false;
    for (i=0; i<pod_num*pod_half; ++i) {
        if (! targets[i]) continue;
        target_pod = i/pod_half;
        if (w_matrix[src][i]) {
            // exist wireless link between source and target
cout<<"=====> 1 hop"<<endl;
            if (w_matrix[src][i] == 1) {
                new_len += 1;
                w_matrix[i][src] = 2;
                w_matrix[src][i] = 2;
            }
            targets[i] = false;
        } else if (
                i/pod_half == src/pod_half // no directed wireless link but in same pod
                ||
                hop[2][i] // two hops via wireless
                ) {
            // optimal
cout<<"=====> 2 hops"<<endl;
            if (target_pod == cur_pod) { 
                // in same pod
                aggr = rand()%pod_half;
                ee = cur_pod * (pod_sq>>2) + aggr*pod_half + src-cur_pod*pod_half;
                if (!edge_flag[ee]) { new_len++; edge_flag[ee] = true; }
                ee = target_pod * (pod_sq>>2) + aggr*pod_half + i-target_pod*pod_half;
                if (!edge_flag[ee]) { new_len++; edge_flag[ee] = true; }
            }
            if (hop[2][i]) {
                dep = 1;
                while (dep>=0) {
                    int from = path[i][dep]/(pod_sq>>1), to = path[i][dep]%(pod_sq>>1);
                    if (w_matrix[from][to] == 1) {
                        new_len++;
                        w_matrix[from][to] = 2;
                        w_matrix[to][from] = 2;
                    }
                    dep--;
                }
            }
            // new_len += 2;
            targets[i] = false;
        } else if (
                w_matrix_pod[src][i/pod_half] || w_matrix_pod[i][src/pod_half] // ToR--->A--->ToR->ToR or ToR->ToR--->A--->ToR
                ||
                hop[3][i] // three hops via wireless
                ) {
cout<<"=====> 3 hops"<<endl;
                        
                if (hop[3][i]) {
                    dep = 2;
                    while (dep>=0) {
                        int from = path[i][dep]/(pod_sq>>1), to = path[i][dep]%(pod_sq>>1);
                        if (w_matrix[from][to] == 1) {
                            new_len++;
                            w_matrix[from][to] = 2;
                            w_matrix[to][from] = 2;
                        }
                        dep--;
                    }
                } else new_len+=3;
            // new_len += 3;
            targets[i] = false;
        } else { 
cout<<"=====> 4 hops"<<endl;
            // in different pod
            // origin_len += 4;
            aggr = rand()%pod_half;
            ee = cur_pod * (pod_sq>>2) + aggr*pod_half + src-cur_pod*pod_half;
            if (!edge_flag[ee]) { new_len++; edge_flag[ee] = true; }
            
            core = rand()%pod_half;
            ee = ((pod_sq*pod_num)>>2) + cur_pod*(pod_sq>>2) + aggr*pod_half + core;
            if (!edge_flag[ee]) { new_len++; edge_flag[ee] = true; }
            
            aggr2 = rand()%pod_half;
            ee = ((pod_sq*pod_num)>>2) + target_pod*(pod_sq>>2) + aggr2*pod_half + core;
            if (!edge_flag[ee]) { new_len++; edge_flag[ee] = true; }

            ee = target_pod * (pod_sq>>2) + aggr2*pod_half + i-target_pod*pod_half;
            if (!edge_flag[ee]) { new_len++; edge_flag[ee] = true; }
        }
    }

    cout<<"#"<<sim+1<<" Result:"<<endl;

    cout<<"    Original length: "<<origin_len<<endl; 
    cout<<"    New length: "<<new_len<<" ("<<(double)(origin_len-new_len)/origin_len*100<<"\% down)"<<endl;
    cout<<"----------------------------------------------"<<endl;
    
    TOT_percent += (double)(origin_len-new_len)/origin_len*100;
}
    cout<<"1-hop: "<<card[1]<<endl; 
    cout<<"2-hop: "<<card[2]<<endl; 
    cout<<"3-hop: "<<card[3]<<endl; 
    cout<<endl<<"================ AVG: "<< (double)TOT_percent/sim_num <<"\% down ======================="<<endl<<TOT_percent/sim_num<<endl;

    /*
     * All done.
     *
     */
    return 0;
}
