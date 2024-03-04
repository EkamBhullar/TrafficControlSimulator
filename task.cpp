#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

using namespace std;

mutex dataMutex;

// Initializing number of threads for producer and consumer
int numProducerThreads = 3;
int numConsumerThreads = 3;

int producerCount = 0; // producer counter
int consumerCount = 0; // consumer counter
int totalRows = 0; // total number of rows in the data file

condition_variable producerCV, consumerCV; // Initializing condition variables for producer and consumer

// String variables and vectors are initialized to get values from the data file
vector<int> indexVector;
vector<int> trafficLightVector;
vector<int> carCountVector;
vector<string> timeStampVector;

// Struct for traffic data row
struct TrafficSignal
{
    int index;
    string timeStamp;
    int trafficLightID;
    int numberOfCars;
};

TrafficSignal signalArray[5] = {{0, "", 1, 0}, {0, "", 2, 0}, {0, "", 3, 0}, {0, "", 4, 0},{0, "", 5, 0}};

// Queue to store traffic light data
queue<TrafficSignal> trafficSignalQueue;


// Function to sort traffic light data
bool sortByTrafficCount(TrafficSignal first, TrafficSignal second)
{
    return  (first.numberOfCars > second.numberOfCars);
  
}

void producerFunction()
{
    while (producerCount < totalRows)
    {
        unique_lock<mutex> lock(dataMutex); // Locking until producer finishes processing

        if (producerCount < totalRows) // If count is less than the number of rows in the dataset
        {
            trafficSignalQueue.push(TrafficSignal{indexVector[producerCount], timeStampVector[producerCount], trafficLightVector[producerCount], carCountVector[producerCount]}); // Push into queue
            consumerCV.notify_all(); // Notifying consumer threads
            producerCount++;
        }
        else
        {
            producerCV.wait(lock, [] { return producerCount < totalRows; }); // If count is greater than the number of rows in the dataset, wait
        }

        lock.unlock(); // Unlock after processing
        this_thread::sleep_for(chrono::seconds(rand() % 3));
    }
}
TrafficSignal signal;
void consumerFunction()
{
    while (consumerCount < totalRows)
    {
        unique_lock<mutex> lock(dataMutex); // Lock until processing

        if (!trafficSignalQueue.empty())
        {
            signal = trafficSignalQueue.front(); // Getting the front elements of the queue

            // Add the the number of cars into the respective traffic light id
            if (signal.trafficLightID == 1)
            {
                signalArray[0].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 2)
            {
                signalArray[1].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 3)
            {
                signalArray[2].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 4)
            {
                signalArray[3].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 5)
            {
                signalArray[4].numberOfCars += signal.numberOfCars;
            }

            trafficSignalQueue.pop(); // Pop the data
            producerCV.notify_all(); // Notify producer
            consumerCount++;
        }
        else
        {
            consumerCV.wait(lock, [] { return !trafficSignalQueue.empty(); }); // If queue is empty, wait until producer produces
        }

        if (!trafficSignalQueue.empty() && consumerCount % 60 == 0)
        { // Check if an hour passed by, checking every 60th row
            sort(signalArray, signalArray + 5, sortByTrafficCount); // Sorting data
            cout << "Time: " << signal.timeStamp << endl;
            cout << "Traffic Light" << "\t" << "Number of Cars" << endl;
            for (int i = 0; i < 5; ++i)
            {
                cout << signalArray[i].trafficLightID << "\t\t" << signalArray[i].numberOfCars << endl;
            }
        }

        lock.unlock();
        this_thread::sleep_for(chrono::seconds(rand() % 3));
    }
}


int main()
{

   // readTrafficDataFromFile();
    ifstream file;

    file.open("traffic_data.txt");

    if (file.is_open())
    {
        string line;
        getline(file, line);

        while (!file.eof())
        {
            string index, timeStamp, trafficLightID, numberOfCars;
            getline(file, index, ',');
            indexVector.push_back(stoi(index));
    
            getline(file, timeStamp, ',');
            timeStampVector.push_back(timeStamp);
            getline(file, trafficLightID, ',');
            trafficLightVector.push_back(stoi(trafficLightID));
    
            getline(file, numberOfCars, '\n');
            carCountVector.push_back(stoi(numberOfCars));
     

            totalRows += 1;
        }
        file.close();
    }
    else
    {
        cout << "Could not open file, try again.";
    }

    thread producers[numProducerThreads];
    thread consumers[numConsumerThreads];

    for (int i = 0; i < numProducerThreads; i++)
        producers[i] = thread(producerFunction);
    for (int i = 0; i < numConsumerThreads; i++)
        consumers[i] = thread(consumerFunction);

    for (int i = 0; i < numProducerThreads; i++)
        producers[i].join();
    for (int i = 0; i < numConsumerThreads; i++)
        consumers[i].join();
}
