//
// Created by minkyung on 23. 3. 9.
//

#ifndef COMBTEST_FEATURE_HPP
#define COMBTEST_FEATURE_HPP

#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <map>
#include <cmath>
#include <deque>
#include <utility>
#include <unordered_map>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#define MIN_POINTS_FOR_CIRLCE 3
#define MIN_POINTS_FOR_PERM 4

using position = std::pair<int,cv::Point2f>;


struct Circle{
	cv::Point2f ct;
	float radius;
	std::vector<std::pair<int,cv::Point2f>> boundary;
};

class CombFeatureGen {

public:

	// comb features
	Circle circle;
	std::deque<std::pair<int,cv::Point2f>> positions;
	std::deque<std::pair<int,cv::Point2f>> polar_positions;
	// comb features

	CombFeatureGen() {};
	CombFeatureGen(std::deque<std::pair<int,cv::Point2f>>& positions_, int len);
	std::pair<bool, Circle> updateCircle(std::pair<int,cv::Point2f> p);
	void append(std::pair<int,cv::Point2f>& pos);

	void GetPermutations(int k, std::vector<std::vector<std::pair<int,cv::Point2f>>>& list);

private:


	bool is_inside(const Circle& c, const cv::Point2f& p){
		if(cv::norm(c.ct - p) <= c.radius){
			return true;
		}
		return false;
	}

	std::pair<bool, Circle> is_valid_circle_from(const position& p1, const position& p2, const position& p3);
	Circle circle_from(const position& A, const position& B, const position& C);
	Circle circle_from(const position& A, const position& B);

	std::string genKey(std::deque<position>& deque_, int len);
	float getThetaFromPoint(cv::Point2f uv, cv::Point2f criteria, float v_max);
	cv::Point2f get_circle_center(float bx, float by, float cx, float cy);
	cv::Point2f getPolarPos(cv::Point2f pos, Circle& circle);

	int genSortedPerm();

};

class LocationFeature{
	std::deque<int> whichbin; // size=area
	std::pair<float,float> keypoint_location;
};


#endif //COMBTEST_FEATURE_HPP
