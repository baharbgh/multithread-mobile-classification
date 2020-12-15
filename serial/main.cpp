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

class classification {
public:
    float compute_accuracy(int num_of_total_records, int num_of_well_estimated);

    vector<int> classificate(vector<vector<float>> train, vector<vector<float>> weights);

    int compute_num_of_well_estimated_prices(vector<int> price_classes, vector<float> train_price);

private:
    int maximum(float arr[], int n);

    // int set_all_zero(int arr);
};

vector<int> classification::classificate(vector<vector<float>> train, vector<vector<float>> weights)
{
    vector<int> price_classes;
    float each_price = 0;
    int num_of_classes = weights.size();

    float temp[num_of_classes];
    int max = 0;
    for (int i = 0; i < train.size(); i++)
    {
        for (int j = 0; j < num_of_classes; j++)
        {
            for (int k = 0; k < train.at(i).size() - 1; k++)
            {
                each_price += weights.at(j).at(k) * train.at(i).at(k);
            }
            temp[j] = each_price + weights.at(j).at(weights.at(j).size() - 1);
            each_price = 0;
        }
//        for (int j = 0; j < num_of_classes ; ++j) {
//            cout<<temp[j]<<"\t";
//        }
//        cout<<endl;
        price_classes.push_back(maximum(temp,num_of_classes));
        int size = sizeof(temp)/sizeof(temp[0]);
        for (int i = 0; i < size; i++)
        {
            temp[i] = 0;
        }
    }
    return price_classes;
}

int classification::maximum(float arr[], int n)
{
    int max = 0;

    for (int i = 0; i < n; i++)
    {
        max = (arr[max] >= arr[i])? max : i;
    }
    return max;
}

int classification::compute_num_of_well_estimated_prices(vector<int> price_classes, vector<float> train_price)
{
    int num_of_same_amounts = 0;
    for (int i = 0; i < price_classes.size(); i++)
    {
        num_of_same_amounts += (price_classes.at(i) == train_price.at(i))? 1 : 0;
    }
    return num_of_same_amounts;
}

float classification::compute_accuracy(int num_of_total_records, int num_of_well_estimated)
{
    return num_of_well_estimated / num_of_total_records;
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

void normalize (vector<vector<float>>* df){
    int n_columns = df->at(0).size() - 1;

    for (int i = 0; i < df->size(); ++i) {

        for (int j = 0; j < n_columns; ++j) {
            float min   = col_min.at(j);
            float max   = col_max.at(j);
            float value = df->at(i).at(j);

            df->at(i).at(j) = (value - min) / (max - min) ;
        }

    }
}

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
    
    auto begin3 = std::chrono::high_resolution_clock::now();
    normalize(&train_data);
    auto end3 = std::chrono::high_resolution_clock::now();
    cout<<"time spent to normalize train data:"<< (end3 - begin3).count()<<"\n";

    classification classify;
    auto begin4 = std::chrono::high_resolution_clock::now();
    vector<int> predictions = classify.classificate(train_data, weight_data);
//    auto end4 = std::chrono::high_resolution_clock::now();
//    cout<<"time spent to classify data:"<< (end4 - begin4).count()<<"\n";

    //auto begin5 = std::chrono::high_resolution_clock::now();
    vector<float> train_price;
    int last_col_index = train_data[0].size()-1;
    for(int i=0; i<train_data.size(); i++){
        train_price.push_back(train_data[i][last_col_index]);
    }
    //auto end5 = std::chrono::high_resolution_clock::now();
    //cout<<"time spent to copy the last column:"<< (end5 - begin5).count()<<"\n";

    //auto begin6 = std::chrono::high_resolution_clock::now();
    float good_pred = classify.compute_num_of_well_estimated_prices(predictions, train_price);
    auto end4 = std::chrono::high_resolution_clock::now();
    cout<<"time spent to classificate and compute number of accure predections:"<< (end4 - begin4).count()<<"\n";

    cout<<"number of good_pred:"<< good_pred<<"\n";

    float accuracy = good_pred/train_data.size();
    float temp = int(accuracy*100);
    //cout<<"temp:"<<temp<<endl;
    accuracy = temp/100;
    //auto begin8 = std::chrono::high_resolution_clock::now();
    cout<<"ACCURACY:"<<accuracy<<"\n";
    //auto end8 = std::chrono::high_resolution_clock::now();
    //cout<<"time spent for cout:"<< (end8 - begin8).count()<<"\n";
    
    return 0;
}