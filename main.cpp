#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <map>
#include <cmath>
#include <deque>
#include <chrono>
#include <utility>
#include <unordered_map>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "pattern.hpp"


#define MIN_POINTS_FOR_CIRLCE 3
#define MIN_POINTS_FOR_PERM 4

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

struct cmp{
	bool operator() (position a, position b){
		return a.second.y > b.second.y;
	}
};


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

	Pattern pattern;

	// n : 최대 개수(검출된 블롭 개수)
	// c : 최소 개수
	for(int i=c; i<=n; i++){
		deque<pair<int,Point2f>> data(i);
		pattern.genFeaturemap(items, data, 0, n-1, 0, i);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto diff1 = end - start;
	std::chrono::duration<double, std::micro> dp_time = diff1;
	cout << "time: " << dp_time.count() << endl;
	cout << endl;

	cout << "Print feature map position" << endl;
	start = std::chrono::high_resolution_clock::now();
	int map_count = 0;
	for(auto& ele : pattern.memo){
		cout << "[" << map_count << "] key: " << ele.first;
		int key_size = ele.first.size();

		cout << " value: ";
		cout << "C" << ele.second.circle.ct << "(" << ele.second.circle.radius << ")" << endl;

//		Mat img = img_ori.clone();
//		circle(img, ele.second.circle.ct, ele.second.circle.radius, Scalar(0,0,255), 1, 8, 0);

		Point2f ct_gt;
		float rad_gt;
		vector<Point> pts;

		cout << "Pos: ";
		for(auto& sub_ele : ele.second.positions){
			cout << sub_ele << " ";
//			circle(img, sub_ele.second, 1, Scalar(0,0,255), 1,8, 0);
			pts.emplace_back(Point(sub_ele.second.x, sub_ele.second.y));
		}
		cout << endl;
		int pos_size = ele.second.positions.size();

		minEnclosingCircle(pts, ct_gt, rad_gt);
//		circle(img, ct_gt, rad_gt, Scalar(0,255,0), 1, 8, 0);

		cout << "PolarPos: ";
		for(auto& sub_ele : ele.second.polar_positions){
			cout << "(" << sub_ele.first << "): [" << sub_ele.second.x << ", " << RAD2DEG(sub_ele.second.y);
			cout << ") ";
		}
		cout << endl << endl;
		int polar_pos_size = ele.second.polar_positions.size();

//		imshow(ele.first, img);
//		waitKey(0);

//		destroyAllWindows();

		map_count++;
	}
	end = std::chrono::high_resolution_clock::now();
	auto diff2 = end - start;
	std::chrono::duration<double, std::micro> cv_time = diff2;
	cout << "time2: " << cv_time.count() << endl;

	cout << "map size: " << pattern.memo.size();

	pattern.genSortedPerm(n);
	cout << endl << endl << "Print perms" << endl;
	int cnt = 0;
	for(auto pair_ : pattern.permutations){
		cout << "[" << pair_.first << "]" << endl;
		for(auto e : pair_.second){
			cout << e << endl;
			cnt++;
		}
		cout << endl;
	}

	cout << "ttl cnt:" << cnt << endl;
}