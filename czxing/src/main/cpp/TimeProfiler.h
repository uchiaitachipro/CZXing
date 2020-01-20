//
// Created by uchiachen on 2020-01-14.
//

#ifndef CZXING_TIMEPROFILER_H
#define CZXING_TIMEPROFILER_H

#include <utility>
#include <memory>
#include <map>
#include <stack>
#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

class TimeProfiler{

public:

    TimeProfiler(){
        _data = make_shared<std::vector<std::map<std::string,long>>>(std::vector<std::map<std::string,long>>(8));
    }

    void Prepare(){
        _currentRecord = std::map<std::string,long>();
        _prefixStack = std::stack<std::string>();
        _startTimePointStack = std::stack<high_resolution_clock::time_point>();
        isPrepared = true;
    }

    void StartRecord(const std::string& prefix){
        _prefixStack.push(std::string(prefix));
        _startTimePointStack.push(high_resolution_clock::now());
    }

    void CompleteRecord(){
        if (_startTimePointStack.empty() || _prefixStack.empty()){
            return;
        }
        auto startTime = _startTimePointStack.top();
        auto prefix = _prefixStack.top();
        high_resolution_clock::time_point endTime = high_resolution_clock::now();
        milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - startTime);
        _currentRecord[prefix] = timeInterval.count();
        _startTimePointStack.pop();
        _prefixStack.pop();
    }

    void Commit(){
        if (!isPrepared){
            return;
        }
        _data->push_back(_currentRecord);
        auto d = _data.get();
        isPrepared = false;
    }

    std::shared_ptr<std::vector<std::map<std::string,long>>> GetRecords(){
        return _data;
    }

private:
    bool isPrepared = false;
    std::shared_ptr<std::vector<std::map<std::string,long>>> _data;
    std::map<std::string,long> _currentRecord;
    std::stack<std::string> _prefixStack;
    std::stack<high_resolution_clock::time_point> _startTimePointStack;

};

#endif //CZXING_TIMEPROFILER_H
