#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <thread>
#include <deque>
#include <chrono>
#include <utility>
#include <unordered_map>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


#define MIN_LENGTH_MEMOIZATION 3
#define MIN_DEPTH_LEAF 4

#define RAD2DEG(x) (x * 180.f / CV_PI);

using namespace std;
using namespace cv;

using position = std::pair<int,cv::Point2f>;

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

string genKey(deque<position>& deque_, int len){

	string res = "";
	for(int i=0; i<len; i++){
		res += to_string(deque_[i].first);
	}

	return res;
}

int count_construct = 0;
int count_append = 0;

struct Circle{
	Point2f ct;
	float radius;
	vector<position> boundary;
};

float getThetaFromPoint(Point2f uv, Point2f criteria, float v_max){

	// uv -> cartesian
	Point2f p = Point2f(uv.x, v_max - uv.y);
	Point2f p_ = Point2f(criteria.x, v_max - criteria.y);

	// [-pi, pi]
	float theta_ = atan2(p.y - p_.y, p.x - p_.x);

	// [0, 2pi)
	if(theta_ < 0) {
		theta_ = CV_2PI - abs(theta_);
	}
	if(theta_ > CV_2PI) {
		theta_ = CV_2PI;
	}

	return theta_;
}

Point2f get_circle_center(float bx, float by, float cx, float cy){
	float B = bx * bx + by * by;
	float C = cx * cx + cy * cy;
	float D = bx * cy - by * cx;

	return Point2f((cy * B - by * C) / (2 * D) , (bx * C - cx * B) / (2 * D));
}

Point2f getPolarPos(Point2f pos, Circle& circle){
	Point2f res;
	res.x = norm(pos - circle.ct); // rho
	res.y = getThetaFromPoint(pos, circle.ct, circle.radius*2.f);

	return res;
}
/**
 * @brief ???????????? ????????? ????????????.
 * @param A
 * @param B
 * @param C
 * @return
 */
Circle circle_from(const position& A, const position& B, const position& C){
	Point2f I = get_circle_center(B.second.x-A.second.x, B.second.y-A.second.y,
																C.second.x-A.second.x, C.second.y-A.second.y);

	I += A.second;
	Point2f diff = I - A.second;

	Circle res;
	res.ct = I;
	res.radius = norm(diff);
	res.boundary.push_back(A);
	res.boundary.push_back(B);
	res.boundary.push_back(C);

	return res;
}

Circle circle_from(const position& A, const position& B){
	Point2f C = A.second + B.second;
	C.x /= 2.0;
	C.y /= 2.0;

	Circle res;
	res.ct = C;
	res.radius = norm(A.second-B.second) / 2.0;
	res.boundary.push_back(A);
	res.boundary.push_back(B);

	return res;
}

class Feature{
public:
	Circle circle;
	deque<position> positions;
	deque<position> polar_positions;

	pair<bool, Circle> is_valid_circle_from(const position& p1, const position& p2, const position& p3){
		pair<bool, Circle> res;
		res.first = false;

		// ??? ????????? ?????? ?????? ????????? ?????? ??????????????? ????????????.

		Circle circle_tmp = circle_from(p1, p2);
		if(is_inside(circle_tmp, p3.second)){
			res.first = true;
			res.second = circle_tmp;
			return res;
		}

		circle_tmp = circle_from(p1, p3);
		if(is_inside(circle_tmp, p2.second)){
			res.first = true;
			res.second = circle_tmp;
			return res;
		}

		circle_tmp = circle_from(p2, p3);
		if(is_inside(circle_tmp, p1.second)){
			res.first = true;
			res.second = circle_tmp;
			return res;
		}

		return res;
	}

	Feature() {};
	Feature(deque<position>& positions_, int len){
		positions.assign(positions_.begin(), positions_.begin()+len);

		// get circle cpt, radius
		// positions ????????? ?????? ??????????????? ?????? ??????
		cout << "Feature() : position size=" << positions.size() << endl;

		bool circle_generated = false;
		if(positions.size() == MIN_LENGTH_MEMOIZATION){
			// ??? ????????? ?????? ?????? ????????? ?????? ??????????????? ??????

			auto res = is_valid_circle_from(positions[0], positions[1], positions[2]);
			if(!res.first){
				circle = circle_from(positions[0], positions[1], positions[2]);
			} else{
				circle = res.second;
			}
		}

		// get polar_positions using circle cpt, radius
		for(auto& p : positions){
			polar_positions.emplace_back(p.first, getPolarPos(p.second, this->circle));
		}
		count_construct++;
	}

	bool is_inside(const Circle& c, const Point2f& p){
		if(norm(c.ct - p) <= c.radius){
			return true;
		}
		return false;
	}

	pair<bool, Circle> updateCircle(position p){
		if(is_inside(this->circle, p.second)){
			pair<bool, Circle> res = make_pair(false, this->circle);
			return res;
		}

		// ?????? ?????? ????????? ?????? ???????????? ?????????
		if(this->circle.boundary.size() == 3){

			pair<bool,Circle> res01 = is_valid_circle_from(p, this->circle.boundary[0], this->circle.boundary[1]);
			Circle c01;
			if(res01.first){
				c01 = res01.second;
			} else{
				c01 = circle_from(p, this->circle.boundary[0], this->circle.boundary[1]);
			}
			if(is_inside(c01, this->circle.boundary[2].second)){
				pair<bool,Circle> res = make_pair(true, c01);
				return res;
			}

			pair<bool,Circle> res02 = is_valid_circle_from(p, this->circle.boundary[0], this->circle.boundary[2]);
			Circle c02;
			if(res02.first){
				c02 = res02.second;
			} else{
				c02 = circle_from(p, this->circle.boundary[0], this->circle.boundary[2]);
			}
			if(is_inside(c02, this->circle.boundary[1].second)){
				pair<bool,Circle> res = make_pair(true, c02);
				return res;
			}

			pair<bool,Circle> res12= is_valid_circle_from(p, this->circle.boundary[1], this->circle.boundary[2]);
			Circle c12;
			if(res12.first){
				c12= res12.second;
			} else{
				c12= circle_from(p, this->circle.boundary[1], this->circle.boundary[2]);
			}
			if(is_inside(c12, this->circle.boundary[0].second)){
				pair<bool,Circle> res = make_pair(true, c12);
				return res;
			}

		} else if(this->circle.boundary.size() == 2){
			// ??? ????????? ?????? ??? ??????
			auto res = is_valid_circle_from(p, this->circle.boundary[0], this->circle.boundary[1]);
			if(res.first){
				return res;
			} else{
				Circle circle_tmp = circle_from(p, this->circle.boundary[0], this->circle.boundary[1]);
				pair<bool, Circle> res_ = make_pair(true, circle_tmp);
				return res_;
			}
		}

		pair<bool, Circle> res = make_pair(true, circle_from(p, this->circle.boundary[0], this->circle.boundary[1]));
		return res;
	}

	void append(position& pos){
		cout << "Feature::append() : current position size=" << positions.size() << endl;
		positions.push_back(pos);

		auto res = updateCircle(pos);
		if(res.first){
			// ???????????? ????????? -> polar position ?????????
			this->circle = res.second;
			polar_positions.clear();
			for(auto& p : positions){
				polar_positions.emplace_back(make_pair(p.first, getPolarPos(p.second, res.second)));
			}
		} else{
			// ????????? ?????? -> ????????? polar position??? ???????????? ??????.
			polar_positions.emplace_back(make_pair(pos.first, getPolarPos(pos.second, res.second)));
		}

		count_append++;
	}

};

//TODO
// bucket size ?????????
unordered_map<string, Feature> feature_map;

int count_leaf = 0;
double ttl_new = 0.f;
double ttl_append = 0.f;

/**
 *
 * @param items
 * @param data
 * @param dst
 * @param start
 * @param end
 * @param index
 * @param k ?????? ??????
 */
void genComb(vector<position> &items, deque<position> & data, vector<deque<position>>& dst,
						 int start, int end, int index, int k){

	if(index == k){
		// memoization??? ???????????? ??????????????? ?????? ?????????.

		auto time_new = std::chrono::high_resolution_clock::now();
		string key = genKey(data, k);

		// memoization
		cout << "[" << count_leaf << "] new element inserted: (" << key << ") ";
		auto insert_res = feature_map.insert(make_pair(key, Feature(data, k)));

//		if(insert_res.second)
		count_leaf++;

		dst.push_back(data);

		std::chrono::duration<double, micro> du_new = chrono::high_resolution_clock::now() - time_new;
		ttl_new += du_new.count();
		return;
	}

	// ????????? ???????????? ?????? ?????? ???????????? i??????
	for(int i=start; i<=end && end-i+1 >= k-index; i++){
		data[index] = items[i];

		int next_idx = index + 1;
		if(next_idx ==k && next_idx >= MIN_DEPTH_LEAF){

			auto time_append = std::chrono::high_resolution_clock::now();
			// feature_map?????? find
			string key = genKey(data, index);

			cout << "find key (" << key << ")" << endl;
			Feature feature_prev = feature_map[key];

			// ????????? ????????? feature??? ?????? ????????????.
			feature_prev.append(data[index]);
			string key_new = genKey(feature_prev.positions, index+1);

			cout << "[" << count_leaf << "] insert appended element (" << key_new << ")" << endl;

			count_leaf++;

			auto insert_res = feature_map.insert(make_pair(key_new, feature_prev));
			std::chrono::duration<double, micro> du_append = chrono::high_resolution_clock::now() - time_append;
			ttl_append += du_append.count();

		} else{
			genComb(items, data, dst, i+1, end, index+1, k);
		}
	}

}

struct cmp{
	bool operator() (position a, position b){
		return a.second.y > b.second.y;
	}
};

bool cmp(const position a, const position b){
	return a.second.y < b.second.y;
}

int main(){

	auto start = std::chrono::high_resolution_clock::now();
	int n = 8;
	int c = 3;

	vector<position> items;

	items.push_back(make_pair(0, Point(8,1)));
	items.push_back(make_pair(1, Point(3,2)));
	items.push_back(make_pair(2, Point(5,4)));
	items.push_back(make_pair(3, Point(2,5)));
	items.push_back(make_pair(4, Point(6,7)));
	items.push_back(make_pair(5, Point(8,5)));
	items.push_back(make_pair(6, Point(6,3)));
	items.push_back(make_pair(7, Point(4,6)));

	Mat img_ori = Mat(512, 512, CV_8UC3, Scalar(255,255,255));
	// resize
	for(auto& ele : items){
		ele.second *= 40.f;
		circle(img_ori, ele.second, 1, Scalar(255,0,0), 1, 8, 0);

		Point txt_pos = ele.second;
		txt_pos.y += 5;
		putText(img_ori, to_string(ele.first), txt_pos, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
	}

	vector<deque<position>> target;

	for(int i=c; i<=n; i++){
		deque<pair<int,Point2f>> data(i);
		genComb(items, data, target,
		        0, n-1, 0, i);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto diff1 = end - start;
	std::chrono::duration<double, std::micro> dp_time = diff1;
	cout << "time: " << dp_time.count() << endl;
	cout << endl;


	cout << "Print feature map position" << endl;
	start = std::chrono::high_resolution_clock::now();
	int map_count = 0;
	for(auto& ele : feature_map){
		cout << "[" << map_count << "] key: " << ele.first;
		int key_size = ele.first.size();

		cout << " value: ";
		cout << "C" << ele.second.circle.ct << "(" << ele.second.circle.radius << ")" << endl;

		Mat img = img_ori.clone();
		circle(img, ele.second.circle.ct, ele.second.circle.radius, Scalar(0,0,255), 1, 8, 0);

		Point2f ct_gt;
		float rad_gt;
		vector<Point> pts;

		cout << "Pos: ";
		for(auto& sub_ele : ele.second.positions){
			cout << sub_ele << " ";
			circle(img, sub_ele.second, 1, Scalar(0,0,255), 1,8, 0);
			pts.emplace_back(Point(sub_ele.second.x, sub_ele.second.y));
		}
		cout << endl;
		int pos_size = ele.second.positions.size();

		minEnclosingCircle(pts, ct_gt, rad_gt);
		circle(img, ct_gt, rad_gt, Scalar(0,255,0), 1, 8, 0);

		cout << "PolarPos: ";
		for(auto& sub_ele : ele.second.polar_positions){
			cout << "(" << sub_ele.first << "): [" << sub_ele.second.x << ", " << RAD2DEG(sub_ele.second.y);
			cout << ") ";
		}
		cout << endl << endl;
		int polar_pos_size = ele.second.polar_positions.size();

		imshow(ele.first, img);
//		waitKey(0);

		destroyAllWindows();

		map_count++;
	}
	end = std::chrono::high_resolution_clock::now();
	auto diff2 = end - start;
	std::chrono::duration<double, std::micro> cv_time = diff2;
	cout << "time2: " << cv_time.count() << endl;

	cout << "map size: " << feature_map.size() << ", insert count:" << count_leaf;
	cout << " constructor count:" << count_construct << ", append count:" << count_append << endl;
	cout << " new avg:" << (ttl_new / count_construct) << " append avg:" << (ttl_append / count_append) << endl;

	// priority queue test
	// position.second.y??? ?????? ?????????
	map<int, vector<vector<position>>> permutations; // [size, permutations]

	//TODO
	// polar_positions ?????????????????? ???????????? ??? ????????????

	deque<position> example = feature_map["01346"].polar_positions;
	sort(example.begin(), example.end(), cmp);

	cout << "print example input" << endl;
	for(auto& pos : example){
		cout << pos << " ";
	}
	cout << endl << endl;

	// perm ?????? ??????
	int len = example.size();
	for(int i=0; i<len; i++){
		position front_pos = example[i];
		cout << "front: " << front_pos << endl;
		deque<position> remains;
		for(int j=0; j<len; j++){
			if(i!=j){
				remains.push_back(example[j]);
			}
		}

		// remain roll
		int n_remain = remains.size();
		int cnt_remain = 0;
		while(cnt_remain < n_remain){
			deque<position> d_tmp = remains;
			cout << "perm[" << cnt_remain << "]: ";

			vector<position> perm_tmp;
			perm_tmp.push_back(front_pos);

			while(!d_tmp.empty()){
				cout << d_tmp.front() << " ";
				perm_tmp.push_back(d_tmp.front());
				d_tmp.pop_front();
			}
			cout << endl;
			permutations[len].push_back(perm_tmp);

			position p = remains.front();
			remains.pop_front();
			remains.push_back(p);

			cnt_remain++;
		}
		cout << endl;
	}

	cout << endl << endl << "Print perms" << endl;
	for(auto pair_ : permutations){
		cout << "[" << pair_.first << "]" << endl;
		for(auto e : pair_.second){
			cout << e << endl;
		}
		cout << endl;

	}
}