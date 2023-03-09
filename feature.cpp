//
// Created by minkyung on 23. 3. 9.
//

#include "feature.hpp"

using namespace cv;
using namespace std;


CombFeatureGen::CombFeatureGen(std::deque<position> &positions_, int len) {

	positions.assign(positions_.begin(), positions_.begin()+len);

	// positions 길이에 따라 외접원생성 코드 작성
	cout << "Feature() : position size=" << positions.size() << endl;

	bool circle_generated = false;
	if(positions.size() == MIN_POINTS_FOR_CIRLCE){
		// 점 두개로 만든 원이 나머지 점을 포함하는지 검사

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

}

std::pair<bool, Circle> CombFeatureGen::updateCircle(position p) {

	if(is_inside(this->circle, p.second)){
		pair<bool, Circle> res = make_pair(false, this->circle);
		return res;
	}

	// 기존 원이 새로운 점을 포함하지 않으면
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
		// 점 두개로 되는 지 확인
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

void CombFeatureGen::append(position &pos){

	cout << "Feature::append() : current position size=" << positions.size() << endl;
	positions.push_back(pos);

	assert(positions.size() >= MIN_POINTS_FOR_PERM);

	auto res = updateCircle(pos);

	if(res.first){
		// 외접원이 변경됨 -> polar position 업데이
		this->circle = res.second;

		polar_positions.clear();
		for(auto& p : positions){
			polar_positions.emplace_back(make_pair(p.first, getPolarPos(p.second, res.second)));
		}
	} else{
		// 외접원 유지 -> 새로운 polar position만 생성하면 된다.
		polar_positions.emplace_back(make_pair(pos.first, getPolarPos(pos.second, res.second)));
	}

	// make permutations
	//TODO
	// polar position 생성할때부터 sort하기
//	cout << genSortedPerm() << " perm generated" << endl;

}

std::pair<bool, Circle> CombFeatureGen::is_valid_circle_from(const position &p1, const position &p2, const position &p3) {

	pair<bool, Circle> res;
	res.first = false;

	// 점 두개로 만든 원이 나머지 점을 포하하는지 체크한다.

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

Circle CombFeatureGen::circle_from(const position &A, const position &B, const position &C) {
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

Circle CombFeatureGen::circle_from(const position &A, const position &B) {
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

std::string CombFeatureGen::genKey(deque<position> &deque_, int len) {
	return std::string();
}

float CombFeatureGen::getThetaFromPoint(cv::Point2f uv, cv::Point2f criteria, float v_max) {
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

cv::Point2f CombFeatureGen::get_circle_center(float bx, float by, float cx, float cy) {
	float B = bx * bx + by * by;
	float C = cx * cx + cy * cy;
	float D = bx * cy - by * cx;

	return Point2f((cy * B - by * C) / (2 * D) , (bx * C - cx * B) / (2 * D));
}

cv::Point2f CombFeatureGen::getPolarPos(cv::Point2f pos, Circle &circle) {
	Point2f res;
	res.x = norm(pos - circle.ct); // rho
	res.y = getThetaFromPoint(pos, circle.ct, circle.radius*2.f);

	return res;
}