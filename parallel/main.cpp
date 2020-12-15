#include <iostream>
#include <vector>
#include <chrono>
#include <pthread.h>
#include <bits/stdc++.h>

using namespace std;
using std::vector;

struct thread_struct
{
    vector<vector<float>> train;
    int num_of_same_amounts;
    vector<int> price_classes;
    int tid;
};

struct store_data_args{
    int tid;
    string datafile;
};

// globar vars
float* combined_min;
float* combined_max;
vector<vector<vector<float>>> glob_train_data;

vector<vector<float>> glob_min;
vector<vector<float>> glob_max;

const int thread_num = 4;
pthread_mutex_t my_mutex;
thread_struct td[thread_num];
float well_estimated_classes;
vector<vector<float>> weight_data;

void printak(vector<vector<float>> in){
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 7 ; ++j) {
            cout<<in[i][j]<<"\t";
        }
        cout<<"\n";
    }
    cout<<"\n";
}

void big_printak(vector<vector<vector<float>>> a){
    for(int i; i<a.size(); i++){
        printak(a[i]);
        cout<<endl;
    }
}

void *thread_store_data(void* args){
    store_data_args *data = (struct store_data_args*) args;
    fstream filestream;
    int tid = data->tid;
    string train_filename = data->datafile;
    char ind = '0' + tid;
    train_filename = train_filename +"train_" + ind + ".csv";

//    printf("this is thread %d, starting and working on ", tid);
//    cout<<train_filename<<"\n";

    filestream.open(train_filename, ios::in);
//    cout<<"file opened\n";
    string line, word, temp;
    int line_count = 0;
    int index;

    vector<vector<float>> this_part;
    vector<float> this_min;
    vector<float> this_max;

    while (filestream>>line){
        stringstream s(line);
        vector<float> vtmp;

        if(line_count>0) {
            index = 0;
            while (getline(s, word, ',')) {
                vtmp.push_back(stof(word));
                if(line_count == 1){
                    this_max.push_back(stof(word));
                    this_min.push_back(stof(word));
                }
                else{
                    if(stof(word) > this_max[index]){
                        this_max[index] = stof(word);
                    }

                    if(stof(word) < this_min[index]){
                        this_min[index] = stof(word);
                    }

                }
                index++;
            }
            this_part.push_back(vtmp);
        }
        line_count++;
    }

//    printak(this_part);

    pthread_mutex_lock(&my_mutex);
    glob_train_data.push_back(this_part);
    glob_max.push_back(this_max);
    glob_min.push_back(this_min);
    pthread_mutex_unlock(&my_mutex);
    pthread_exit((void*)0);
}

void store_data(fstream *file_stream,vector<vector<float>> *data){
    string line, word, temp;
    int line_count = 0;

    while (*file_stream>>line){
        //getline(file_stream, line);
        stringstream s(line);
        vector<float> vtmp;

        if(line_count>0) {
            while (getline(s, word, ',')) {
                vtmp.push_back(stof(word));
            }
            data->push_back(vtmp);
        }
        line_count++;
    }

}

int main(int argc, char *argv[]) {
    char * datafile;
    datafile = argv[1];
    string train_filename  = datafile;
    string weight_filename = datafile;
    weight_filename = weight_filename + "weights.csv";

    store_data_args data_array[thread_num];
    pthread_t threads[thread_num];
    int return_code;
    auto begin1 = std::chrono::high_resolution_clock::now();
    for(long i = 0; i< thread_num; i++)
    {
        data_array[i].tid = i;
        data_array[i].datafile = datafile;
//        printf("Creating thread %ld\n", i);
        return_code = pthread_create(&threads[i], NULL, thread_store_data,
                                     (void*)&data_array[i]);

        if (return_code)
        {
            printf("ERROR; return code from pthread_create() is %d\n",
                   return_code);
            exit(-1);
        }
    }
    std::fstream file_stream_prime;
    file_stream_prime.open(weight_filename);

    //auto begin2 = std::chrono::high_resolution_clock::now();
    store_data(&file_stream_prime, &weight_data);
    //auto end2 = std::chrono::high_resolution_clock::now();
    //cout<<"time spent to store weights.csv:"<< (end2 - begin2).count()<<"\n";

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    
    
    return 0;
}
