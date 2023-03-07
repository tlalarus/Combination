#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <random>
#include <algorithm>
#include <deque>
#include <chrono>
#include <utility>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


#define MIN_DEPTH 4

using namespace std;
using namespace cv;


ostream& operator<<(ostream& os, const pair<int, Point2f> pair){
	os << "[" << pair.first << "]=" << pair.second;
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

struct Pos{
	int x;
	int y;
	Pos() {};
	Pos(int x_, int y_){
		x = x_;
		y = y_;
	}
};

int n_cnt = 0;

class CombNode{
	vector<int> path;
	map<int, CombNode> children;
	bool leaf;
	int depth;
	
	vector<Pos> feature;
	
public:
		CombNode() {};
	void InsertNode(vector<int>& indice_in_, vector<int>& path_, int concat_idx, int depth_);
	void SearchNode(vector<int>& path_in_, vector<int>& path_out_, vector<Pos>& feature_);
	
};

void Func(vector<Pos>& feat_){
	
	cout << "Gen Feature: ";
	mt19937 mt_rand(n_cnt++);
	uniform_int_distribution<int> range(0, 100);
	
	for(int i=0; i<5; i++){
		int rand_val = range(mt_rand);
		cout << "("<< rand_val << ", " << rand_val+10 << ")  ";
		feat_.emplace_back(rand_val, rand_val+10);
	}
	cout << endl;
}
void CombNode::InsertNode(vector<int>& indice_in_, vector<int>& path_, int concat_idx, int depth_) {
	
	this->depth = depth_;
	// 현재까지의 경로를 현재 노드에 저장한다.
	copy(path_.begin(), path_.end(), std::back_inserter(path));
	path.push_back(indice_in_[concat_idx]);
	
	string space = "";
	for(int i=0; i<depth_; i++){
		space += " ";
	}
	
	
	cout << space << "Current path: ";
	cout << path << "(" << this->depth << ")" << endl;
	
	// 현재 인덱스번호 다음 인덱스부터 자식노드에 추가한다.
	// 자식노드의 값 자체가 children의 key가 된다.
	for(int i=concat_idx+1; i<indice_in_.size(); i++){
		
		// 현재 자식노드에 존재하는지 검색
		if(children.find(indice_in_[i]) == children.end()){
			cout << space << "insert children: " << indice_in_[i] << endl;
			children.insert({indice_in_[i], CombNode()});
			children[indice_in_[i]].InsertNode(indice_in_, path, i, depth_+1);
		} else{
			cout << space << "already exist" << endl;
		}
	}
	
	// 최소 노드 수 만족하면 leaf 노드 생
	if(path.size() >= MIN_DEPTH){
		// Polar histogram 생성
		leaf = true;
		
		// 현재 path 출력
		cout << space << "@@ Leaf node path: ";
		cout << path << "(" << this->depth << ")" << endl;
		Func(feature);
	}

}

void CombNode::SearchNode(vector<int> &path_in_, vector<int> &path_out_, vector<Pos>& feature_) {
	
	if(path_in_.empty()){
		// found
		copy(path.begin(), path.end(), path_out_.begin());
//		copy(path.begin(), path.end(), back_inserter(path_out_));
		copy(feature.begin(), feature.end(), feature_.begin());
//		copy(feature.begin(), feature.end(), back_inserter(feature_));
		
		return;
	}
	
	if(children.find(path_in_.front()) != children.end()) {
		// search next sub path in children
		vector<int> sub_path_;
		int path_len = path_in_.size();
		copy(path_in_.begin()+1, path_in_.end(), back_inserter(sub_path_));
		
		return (children[path_in_.front()].SearchNode(sub_path_, path_out_, feature_));
	}
	
}


// 생성시 : combination(순서고려X)의 feature를 저장한다. ccw ordered permutation list를 생성한다.
// 검색시 : ccw ordered permutation list를 돌면서 feature를 rearrange 한다.
class Combination{
public:
	CombNode root;
	int base;
	Combination() {};
	Combination(vector<int>& indice_);
	bool Search(vector<int>& path_in_, vector<Pos>& feature_);
};

Combination::Combination(vector<int>& indice_) {
	// sorted indice 입력받음
	base = indice_.front();
	vector<int> path_init;
	root.InsertNode(indice_, path_init, 0, 0);
}

bool Combination::Search(vector<int> &path_in_, vector<Pos>& feature_) {
	if(path_in_[0] !=  base){
		return false;
	}
	
	vector<int> indice_ret;
	vector<int> target;
	copy(path_in_.begin()+1, path_in_.end(), back_inserter(target));
	
	root.SearchNode(target, indice_ret, feature_);
	
	if(indice_ret.empty()){
		return false;
	}
	
	cout << "Return path:" << indice_ret << endl;
	return true;
}


//#define PERM_TEST
#ifdef PERM_TEST
bool ccw(cv::Point2f& p1, cv::Point2f& p2, cv::Point2f& p3){
	int cross_product = (p2.x - p1.x)*(p3.y-p1.y) - (p3.x-p1.x)*(p2.y-p1.y);
	if(cross_product > 0){
		// ccw
		return true;
	} else if(cross_product < 0){
		// clockwise
		return false;
	} else{
		return true;
	}
}

bool is_ccw(vector<Point2f>& positions_, int y_max){
	
	// get cpt
	Point2f cpt;
	float rad;
	minEnclosingCircle(positions_, cpt, rad);
	
	// reflection
//	cpt.y = (float)y_max - cpt.y;
	
	for(int i=0; i<positions_.size(); i++){
		
		Point2f p2;
		Point2f p3;
		if(i == positions_.size()-1){
			p2 = positions_[i];
			p3 = positions_[0];
		} else{
			p2 = positions_[i];
			p3 = positions_[i+1];
		}
		
//		p2.y = (float)y_max - p2.y;
//		p3.y = (float)y_max - p3.y;
		if(!ccw(cpt, p2, p3)){
			return false;
		}
	}
	return true;
	
	
	
}
void perms(vector<pair<int,Point2f>>& positions_, vector<int>& path_, int depth, map<int,vector<vector<int>>>& path_res_){
	
	// ccw 판단하기 위한 최소 조건
	if(path_.size() >= 3){
		
		// 좌표 인덱싱
		vector<Point2f> pos_selected;
		float y_max = -1;
		for(auto& i : path_){
			pos_selected.push_back(positions_[i].second);
			if(y_max < positions_[i].second.y){
				y_max = positions_[i].second.y;
			}
		}
		
		// 유망한지 판단
		if(is_ccw(pos_selected, y_max)){
			cout << "Promising : " << endl << pos_selected << endl;
			if(depth >= MIN_DEPTH){
				// 결과 저장
				path_res_[depth].push_back(path_);
//				path_res_.push_back(path_);
			}
		} else{
			cout << "Not Promising: " << endl << pos_selected << endl;
			
		}
		
	}
	
	for(int i=0; i<positions_.size(); i++){
		// 현재 path에 없으면
		if(find(path_.begin(), path_.end(), i) == path_.end()){
			// new instance 생성
			vector<int> path_clone;
//			copy(path_.begin(), path_.end(), path_clone.begin());
			copy(path_.begin(), path_.end(), back_inserter(path_clone));
			path_clone.push_back(i);
			perms(positions_, path_clone, depth+1, path_res_);
		}
	}
	
}

void perms(vector<int>& indice, vector<int>& path_, int depth, vector<vector<int>>& path_res_){
	
	if(depth >= MIN_DEPTH){
		// 결과 저장
		path_res_.push_back(path_);
	}
	
	for(int i=0; i<indice.size(); i++){
		// 현재 path에 없으면
		if(find(path_.begin(), path_.end(), i) == path_.end()){
			// new instance 생성
			vector<int> path_clone;
			copy(path_.begin(), path_.end(), back_inserter(path_clone));
			path_clone.push_back(i);
			perms(indice, path_clone, depth+1, path_res_);
		}
	}
}

int main(){
	// in : (u, v) 좌표
	// out : ccw ordered permutation list
	
	int n = 5;
	vector<pair<int, Point2f>> positions;
	positions.push_back(make_pair(0, Point2f(10.f,13.f)));
	positions.push_back(make_pair(1, Point2f(12,9)));
	positions.push_back(make_pair(2, Point2f(14,15)));
	positions.push_back(make_pair(3, Point2f(16,5)));
	positions.push_back(make_pair(4, Point2f(19,7)));
	
	vector<int> indice;
	vector<vector<int>> indice_res;
	map<int, vector<vector<int>>> indice_map;
	
	for(int i=0; i<n; i++){
		indice.push_back(i);
	}
	
	auto start_t = chrono::high_resolution_clock::now();
	// 첫번째꺼는 담아야 함.
	for(int i=0; i<n; i++){
		vector<int> path;
		path.push_back(i);
		perms(positions, path, 1, indice_map);
//		perms(indice, path, 1, indice_res);
	}
	
	
	auto end_t = chrono::high_resolution_clock::now();
	auto duration = end_t - start_t;
	cout << "ccw ordered list gen End!! (" << chrono::duration<double, std::micro>{duration}.count() << " us) ++" << endl << endl << endl;
	
	cout << "Print Result" << endl;
	for(map<int,vector<vector<int>>>::iterator it = indice_map.begin(); it!= indice_map.end(); it++){
		for(auto& ele : it->second){
			cout << "[" << it->first << "] " << ele << endl;
		}
	}
//	for(auto& indice_it : indice_res){
//		cout << indice_it << endl;
//	}
//	cout << indice_res.size() << endl;
	
}
#endif


#define SIMPLE_COMB
#ifdef SIMPLE_COMB
void genComb(vector<int> &items, deque<int> & data, vector<deque<int>>& dst,
						 int start, int end, int index, int k){
	if(index == k){
		cout << "Leaf: ";
		for(int i=0; i<k; i++){
			cout << data[i] << " ";
		}
		cout << endl;
		cout << "comb data size:" << data.size() << endl;
		dst.push_back(data);
		return;
	}
	
	for(int i=start; i<=end && end-i+1 >= k-index; i++){
		data[index] = items[i];
		genComb(items, data, dst, i+1, end, index+1, k);
	}
}

int main(){
	int n = 5;
	int c = 4;
	
	vector<int> items;
	for(int i=0; i<n; i++){
		items.push_back(i);
	}

	vector<deque<int>> target;

	for(int i=c; i<=n; i++){
		deque<int> data(i);
		genComb(items, data, target,
		        0, n-1, 0, i);
	}

	cout << endl;
	cout << "Print fin dst" << endl;
	for(auto& comb : target){
		cout << comb << "(" << comb.size() << ")" << endl;
	}
	
}
#endif

//#define BASIC_TEST
#ifdef BASIC_TEST
int main() {
	// in : 조합 요소 개수
	// out : combination
	vector<Combination> comb_tree;
	int n = 5;
	
	cout << "++ Comb tree Insertion Begin!! (" << n << ") ++" << endl;
	auto start_t = chrono::high_resolution_clock::now();
	
	for(int i=0; i<n; i++){
		vector<int> indice;
		indice.push_back(i);
		for(int j=0; j<n; j++){
			if(i != j){
				indice.push_back(j);
			}
		}
		comb_tree.emplace_back(indice);
	}
	
	auto end_t = chrono::high_resolution_clock::now();
	auto duration = end_t - start_t;
	
	cout << "++ Comb tree Insertion End!! (" << chrono::duration<double, std::micro>{duration}.count() << " us) ++" << endl << endl << endl;
	
	
	cout << "++ Comb tree Search Begin!! ++" << endl;
	vector<vector<int>> targets;
	targets.push_back({0, 2, 3});
	targets.push_back({1, 3, 0});
	targets.push_back({1, 1, 1});
	targets.push_back({1, 0, 3});
	targets.push_back({2, 1, 3, 0});
	targets.push_back({0, 1, 3, 2});
	targets.push_back({0, 4, 7, 6, 1, 2, 3, 5});
	
	vector<vector<Pos>> features;
	vector<double> micro_times;
	
	for(auto target : targets){
		
		start_t = chrono::high_resolution_clock::now();
		int base_ = target.front();
		vector<int> tmp;
		copy(target.begin(), target.end(), back_inserter(tmp));
		sort(tmp.begin()+1, tmp.end());
		
		cout << "Find " << target << " -> " << tmp << endl;
		
		vector<Pos> tmp_ret;
		if(comb_tree[base_].Search(tmp, tmp_ret)){
			micro_times.push_back(chrono::duration<double, std::micro>{chrono::high_resolution_clock::now()-start_t}.count());
			features.push_back(move(tmp_ret));
		} else{
			micro_times.push_back(0.f);
			features.push_back({});
		}
		
	}
	
	int n_out = targets.size();
	for(int i=0; i<n_out; i++){
		cout << "In: " << targets[i] << endl;
		
		cout << "Out: ";
		for(int j=0; j<features[i].size(); j++){
			cout << features[i][j].x << "," << features[i][j].y << "  ";
		}
		cout << micro_times[i] << " us" << endl;
	}
	
	return 0;
}
#endif

//#define SIMPLE_PERM_TEST
#ifdef SIMPLE_PERM_TEST
int main(){
	int n = 4;
	vector<int> vec(n);
	
	for(int i=0; i<n; i++){
		vec[i] = i+10;
//		vec.push_back(i+10);
	}
	
	cout << "@ Print input vector" << endl;
	for(int i=0; i<n; i++){
		cout << vec[i] << " ";
	}
	cout << endl;
	
	// permutation
	int cnt = 0;
	
	cout << "@ Print permutation" << endl;
	for(int i=0; i<n; i++){
		for(int j=0; j<n-1; j++){
			if(i == j){
				continue;
			}
			for(int k=j+1; k<n; k++){
				if(i == k){
					continue;
				}
				cout << i << " " << j << " " << k << endl;
				cnt++;
			}
		}
		
	}
	
	cout << " total count: " << cnt << endl;
}
#endif
