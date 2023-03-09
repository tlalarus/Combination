//
// Created by minkyung on 23. 3. 9.
//

#ifndef COMBTEST_PATTERN_HPP
#define COMBTEST_PATTERN_HPP

#include <iostream>
#include <map>
#include <vector>

#include "feature.hpp"


class Pattern{
public:
	// feature map
	std::map<std::string, CombFeatureGen> memo; // [key, comb features]
	std::map<int, std::vector<std::vector<position>>> permutations; // [size, permutations]
	std::map<int, LocationFeature> r_features; // [front id, remains]

	Pattern() {};
	std::string genKey(std::deque<position>& deque_, int len);
	void genFeaturemap(std::vector<std::pair<int,cv::Point2f>>& items, std::deque<std::pair<int,cv::Point2f>>& data,
	             int start, int end, int index, int k);

	void genSortedPerm(int len_max);
	void genRelativeFeatures(std::pair<int,cv::Point2f>& front, std::deque<std::pair<int,cv::Point2f>>& remains);

private:

};


#endif //COMBTEST_PATTERN_HPP
