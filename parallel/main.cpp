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
int size = 0;
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

    while (filestream>>line) {
        stringstream s(line);
        vector<float> vtmp;

        if (line_count > 0) {
            index = 0;
            while (getline(s, word, ',')) {
                vtmp.push_back(stof(word));
                if (line_count == 1) {
                    this_max.push_back(stof(word));
                    this_min.push_back(stof(word));
                } else {
                    if (stof(word) > this_max[index]) {
                        this_max[index] = stof(word);
                    }

                    if (stof(word) < this_min[index]) {
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
    size += this_part.size();
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

void* tNormalize (void* tid){
    long threadID = (long) tid;
    // tikke ye vazifamoon ro joda mikonim.
//    vector<vector<float>> segment = glob_train_data.at(threadID);
//    int n_columns = segment.at(0).size() - 1; // farz bar ineke segment sotoon e target ham dare
    int n_columns = glob_train_data.at(0).at(0).size()-1;
    int n_rows = glob_train_data.at(threadID).size();
    // normalize esh mikonim
    for (int i = 0; i < n_rows; ++i) {
        for (int j = 0; j < n_columns; ++j) {
            float min   = combined_min[j];
            float max   = combined_max[j];
            float value = glob_train_data.at(threadID).at(i).at(j);
            glob_train_data.at(threadID).at(i).at(j) = (value - min) / (max - min);
        }
    }

    // jaygozin esh mikonim
//    pthread_mutex_lock(&my_mutex); // lock()
//    glob_train_data.at(threadID) = segment;
//    pthread_mutex_unlock(&my_mutex); // unlock()
//    cout << "thread e " << threadID << " be andaze ye " << n_rows << " normalize kard." << endl;
    pthread_exit((void*) threadID ); // chiziam ke return lazem nadarim
}

int maximum(float arr[], int size) {
    int max = 0;
    for (int i = 0; i < size; i++)
    {

        max = (arr[max] < arr[i])? i : max;
    }
    return max;
}

void* classificate_and_compute_num_of_well_estimated_prices(void* arg)
{
    struct thread_struct trans_data = *(struct thread_struct*) arg;
    float each_price = 0;
    auto num_of_classes = weight_data.size();
    float temp[num_of_classes];

    int size = trans_data.train.size();
    int col = trans_data.train.at(0).size();

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < num_of_classes; j++)
        {
            for (int k = 0; k < col - 1; k++)
            {
                each_price += weight_data.at(j).at(k) * trans_data.train.at(i).at(k);
            }
            temp[j] = each_price + weight_data.at(j).at(weight_data.at(j).size() - 1);
            each_price = 0;
        }
        trans_data.price_classes.push_back(maximum(temp, num_of_classes));
        for (int i = 0; i < num_of_classes; i++)
        {
            temp[i] = 0;
        }
    }
    // MAYBE LOCKING NEEDED ************************************
    int count = thread_num - 1;
    while (count != -1) {
        if (trans_data.tid == count) {
            td[count].price_classes = trans_data.price_classes;
            break;
        }
        count--;
    }

    for (size_t i = 0; i < trans_data.price_classes.size(); i++)
    {
        trans_data.num_of_same_amounts +=
                (trans_data.price_classes.at(i) == trans_data.train.at(i).at(col-1))? 1 : 0;
    }
    pthread_mutex_lock (&my_mutex);
    well_estimated_classes += trans_data.num_of_same_amounts;
    pthread_mutex_unlock (&my_mutex);
    pthread_exit(NULL);

    pthread_exit((void*)0);
}


float compute_accuracy(int num_of_total_records)
{
    return well_estimated_classes / num_of_total_records;
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

    //    cout<<"PARALLEL:\nall data reading and storing time: "<<(end1-begin1).count()<<endl;
//    big_printak(glob_train_data);
//    cout<<"\n";
//    printak(weight_data);

    auto begin2 = std::chrono::high_resolution_clock::now();
    const int n_columns = glob_train_data.at(0).at(0).size();
    combined_min = (float*)malloc(n_columns * sizeof(float));
    combined_max = (float*)malloc(n_columns * sizeof(float));
    cout<<"that\n";
    for (int i = 0; i < n_columns; ++i) {
        combined_min[i] = 100000;
        combined_max[i] = 0;

        for (int j = 0; j < thread_num; ++j) {
            if (glob_max.at(j).at(i) > combined_max[i]){
                combined_max[i] = glob_max.at(j).at(i);
            }
            if (glob_min.at(j).at(i) < combined_min[i]){
                combined_min[i] = glob_min.at(j).at(i);
            }
        }
    }
    

//    auto oonVasat = std::chrono::high_resolution_clock::now();

    // multi-threading
    return_code = 0;
    void* status;

    for(long tid = 0; tid < thread_num; tid++) {
        return_code = pthread_create(&threads[tid], NULL, tNormalize, (void*)tid);
        if (return_code) {
            printf("ERROR; return code from pthread_create() is %d\n", return_code);
            exit(-1);
        }
    }

    for(long tid = 0; tid < thread_num; tid++)
    {
        return_code = pthread_join(threads[tid], &status);
        if (return_code)
        {
            printf("ERROR; return code from pthread_join() is %d\n", return_code);
            exit(-1);
        }
        //printf("Main: completed join with thread %ld having a status of %ld\n", tid, (long)status);
    }


    auto end2 = std::chrono::high_resolution_clock::now();
//    cout << "normalizing: " << (end2 - begin2).count()<< endl;

    auto begin3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_num; i++)
    {
        td[i].tid = i;
        td[i].num_of_same_amounts = 0;
        td[i].train = glob_train_data.at(i);
    }

    pthread_mutex_init(&my_mutex, NULL);

    status = NULL;
    for (long i = 0; i < thread_num; i++) {
        pthread_create(&threads[i], NULL, classificate_and_compute_num_of_well_estimated_prices, &(td[i]));
    }

    for (long i = 0; i < thread_num; i++)
        pthread_join(threads[i], &status);

    auto end3 = std::chrono::high_resolution_clock::now();
    //cout<<"classification & comute well_estimated_num time: "<<(end3-begin3).count()<<endl;


    float accuracy = compute_accuracy(size);

    float temp = int(accuracy*100);
    accuracy = temp/100;
    cout << "\naccuracy is: " << accuracy << "%\n";

    pthread_mutex_destroy(&my_mutex);

    return 0;
}
