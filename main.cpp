#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <random>
#include <algorithm>
#include <deque>
#include <chrono>
#include <utility>
#include <unordered_map>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


#define MIN_LENGTH_MEMOIZATION 3
#define MIN_DEPTH_LEAF 4

using namespace std;
using namespace cv;

using position = std::pair<int,cv::Point>;

ostream& operator<<(ostream& os, const pair<int, Point2f> pair){
	os << "[" << pair.first << "]:" << pair.second;
}

template <typename S>
ostream& operator<<(ostream& os, const vector<S>& vector){
	// Printing all the elements
	// using <<
	for(auto element : vector){
		os << element << " ";
	}
	return os;
}

template <typename S>
ostream& operator<<(ostream& os, const deque<S>& vector){
	// Printing all the elements
	// using <<
	for(auto element : vector){
		os << element << " ";
	}
	return os;
}

unordered_map<string, deque<position>> dict;

string genKey(deque<position>& deque_, int len){

	string res = "";
	for(int i=0; i<len; i++){
		res += to_string(deque_[i].first);
	}

	return res;
}

/**
 *
 * @param items
 * @param data
 * @param dst
 * @param start
 * @param end
 * @param index
 * @param k 타겟 개수
 */
void genComb(vector<position> &items, deque<position> & data, vector<deque<position>>& dst,
						 int start, int end, int index, int k){

	if(index == k){

		cout << "Leaf: ";
		for(int i=0; i<k; i++){
			cout << data[i] << " ";
		}
		string key = genKey(data, k);
		cout << "(" << key << ")" << endl;

		pair<string, deque<position>> tmp(key, data);
		dict.insert(tmp);

		dst.push_back(data);
		return;
	}

	// 타겟의 인덱스를 넘지 않는 범위까지 i증가
	for(int i=start; i<=end && end-i+1 >= k-index; i++){
		data[index] = items[i];

		int next_idx = index+1;
		if(next_idx ==k && next_idx > MIN_DEPTH){
			cout << "Find ";
			for(int i=0; i<index; i++){
				cout << data[i] << " "; // current path
			}

			string key = genKey(data, index);
			cout << " from map for";

			for(int i=0; i<next_idx; i++){
				cout << data[i] << " "; // next path
			}
			cout << "-> key:" << key << " + " << data[index] << endl;

		} else{
			genComb(items, data, dst, i+1, end, index+1, k);
		}
	}
}

int main(){
	int n = 6;
	int c = 3;

	vector<position> items;

	for(int i=0; i<n; i++){
		items.push_back(make_pair(i,Point(i,i)));
	}

	vector<deque<position>> target;

	for(int i=c; i<=n; i++){
		deque<pair<int,Point>> data(i);
		genComb(items, data, target,
		        0, n-1, 0, i);
	}

	cout << endl;
	cout << "Print fin dst" << endl;
	for(auto& comb : target){
		cout << comb << "(" << comb.size() << ")" << endl;
	}
	
}