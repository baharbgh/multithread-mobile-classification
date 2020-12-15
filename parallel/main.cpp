#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <stdio.h>
#include <pthread.h>

using namespace std;
using std::vector;

vector<float> col_min;
vector<float> col_max;

void printak(vector<vector<float>> in){
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 7 ; ++j) {
            cout<<in[i][j]<<"\t";
        }
        cout<<"\n";
    }
}


void store_data(fstream *file_stream,vector<vector<float>> *data){
    string line, word, temp;
    int line_count = 0;
    int index;

    while (*file_stream>>line){
        //getline(file_stream, line);
        stringstream s(line);
        vector<float> vtmp;

        if(line_count>0) {
            index = 0;
            while (getline(s, word, ',')) {
                vtmp.push_back(stof(word));
                if(line_count == 1){
                    col_max.push_back(stof(word));
                    col_min.push_back(stof(word));
                }else{
                    if(stof(word) > col_max[index]){
                        col_max[index] = stof(word);
                    }

                    if(stof(word) < col_min[index]){
                        col_min[index] = stof(word);
                    }
                }
                index++;
            }
            data->push_back(vtmp);
        }
        line_count++;
    }

}

void original_store_data(fstream *file_stream,vector<vector<float>> *data){
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

using namespace std;

int main(int argc, char *argv[]) {
    char * datafile;
    datafile = argv[1];
    string train_filename  = datafile;
    string weight_filename = datafile;
    auto begin0 = std::chrono::high_resolution_clock::now();
    train_filename = train_filename + "train.csv";
    weight_filename = weight_filename + "weights.csv";

    std::fstream file_stream;
    std::fstream file_stream_prime;
    file_stream.open(train_filename);
    file_stream_prime.open(weight_filename);
    vector<vector<float>> train_data;
    vector<vector<float>> weight_data;

    //auto begin1 = std::chrono::high_resolution_clock::now();
    store_data(&file_stream, &train_data);
    auto end1 = std::chrono::high_resolution_clock::now();
    //cout<<"time spent to store train.csv:"<< (end1 - begin1).count()<<"\n";

    //auto begin2 = std::chrono::high_resolution_clock::now();
    original_store_data(&file_stream_prime, &weight_data);
    //auto end2 = std::chrono::high_resolution_clock::now();
    //cout<<"time spent to store weights.csv:"<< (end2 - begin2).count()<<"\n";
    auto end0 = std::chrono::high_resolution_clock::now();
    cout<<"SERIAL:\nall data reading and storing time: "<<(end0-begin0).count()<<endl;

    
    return 0;
}
