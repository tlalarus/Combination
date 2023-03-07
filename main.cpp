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

struct Circle{
	Point ct;
	float radius;
};

class Feature{
public:
	Circle circle;
	deque<position> positions;
	deque<position> polar_positions;

	Feature() {};
	Feature(deque<position>& positions_, int len){
		positions.assign(positions_.begin(), positions_.begin()+len);

		// get circle cpt, radius

		// get polar_positions using circle cpt, radius
		polar_positions = positions;
	}
	void append(position& pos){
		positions.push_back(pos);
		polar_positions.push_back(pos);
	}
};

//TODO
// bucket size 최적화
unordered_map<string, Feature> feature_map;

int count_leaf = 0;
int count_insert_success = 0;
int count_insert_fail = 0;

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
		// memoization에 존재하지 않았을경우 새로 만든다.

		cout << "Leaf: ";
		for(int i=0; i<k; i++){
			cout << data[i] << " ";
		}
		string key = genKey(data, k);
		cout << "(" << key << ")" << endl;

		// memoization
		auto insert_res = feature_map.insert(make_pair(key, Feature(data, k)));
		count_leaf++;
		if(insert_res.second){
			count_insert_success++; // insert 성공한 것만 카운트
		} else{
			count_insert_fail++;
		}

		pair<string, deque<position>> tmp(key, data);
		dict.insert(tmp);

		dst.push_back(data);
		return;
	}

	// 타겟의 인덱스를 넘지 않는 범위까지 i증가
	for(int i=start; i<=end && end-i+1 >= k-index; i++){
		data[index] = items[i];

		int next_idx = index + 1;
		if(next_idx ==k && next_idx > MIN_DEPTH_LEAF){

			// memoization 에서 find
			cout << "Find ";
//			for(int i=0; i<index; i++){
//				cout << data[i] << " "; // current path
//			}

			string key = genKey(data, index);
			Feature feature_prev = feature_map[key];
			for(auto ele : feature_prev.positions){
				cout << ele << " ";
			}

			cout << " from map for";

			for(int i=0; i<next_idx; i++){
				cout << data[i] << " "; // next path
			}
			cout << "-> key:" << key << " + " << data[index] << endl;

			// 새롭게 생성된 feature를 맵에 삽입한다.
			feature_prev.append(data[index]);
			string key_new = genKey(feature_prev.positions, index+1);
			auto insert_res = feature_map.insert(make_pair(key_new, feature_prev));
			count_leaf++;

			if(insert_res.second){
				count_insert_success++; // insert 성공한 것만 카운트
			} else{
				count_insert_fail++;
			}

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
	cout << "Print feature map position" << endl;
	for(auto& ele : feature_map){
		cout << "key: " << ele.first;

		cout << " value: ";
		for(auto& sub_ele : ele.second.positions){
			cout << sub_ele << " ";
		}
		cout << endl;
	}

	cout << "map size: " << feature_map.size() << ", insert count:" << count_leaf <<
	", insert success: " << count_insert_success << ", insert fail: " << count_insert_fail << endl;

}