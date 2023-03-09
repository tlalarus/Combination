//
// Created by minkyung on 23. 3. 9.
//

#include "pattern.hpp"

using namespace std;
using namespace cv;

bool cmp(const std::pair<int,cv::Point2f>& a, const std::pair<int,cv::Point2f>& b){
	return a.second.y < b.second.y;
}

void Pattern::genFeaturemap(vector<pair<int, Point2f>> &items, deque<pair<int, Point2f>> &data,
                            int start, int end, int index, int k) {

	if(index == k){
		string key = genKey(data, k);
		auto insert_res = this->memo.insert(make_pair(key, CombFeatureGen(data, k)));

		return;
	}

	// 타겟의 인덱스를 넘지 않는 범위까지 i가 증가한다.
	for(int i=start; i<=end && end-i+1 >= k-index; i++){
		data[index] = items[i];

		int next_idx = index + 1;
		if(next_idx==k && next_idx>=MIN_POINTS_FOR_PERM){
			string key = genKey(data, index);
			CombFeatureGen feature_prev = this->memo[key];

			// 새롭게 생성된 feature를 맵에 삽입한다.
			feature_prev.append(data[index]);
			string key_new = genKey(feature_prev.positions, index+1);

			auto insert_res = this->memo.insert(make_pair(key_new, feature_prev));

		} else{
			genFeaturemap(items, data, i+1, end, index+1, k);
		}
	}

}

string Pattern::genKey(deque<position> &deque_, int len) {

	string res = "";
	for(int i=0; i<len; i++){
		res += to_string(deque_[i].first);
	}

	return res;
}

void Pattern::genSortedPerm(int len_max) {

	for(auto& map: this->memo){
		sort(map.second.polar_positions.begin(), map.second.polar_positions.end(), cmp);

		int perm_cnt = 0;

		// perm 생성하기 위한(front, remain)설정
		int len = map.second.polar_positions.size();

		// len이 최대개수일때 히스토그램을 생성한다.

		for(int i=0; i<len; i++){
			position front_p = map.second.polar_positions[i];
			deque<position> remains;

			for(int j=0; j<len; j++){
				if(i != j){
					remains.push_back(map.second.polar_positions[j]);
				}
			}

			//TODO
			// 위에서 설정한 front keypoint와 나머지 remain keypoint를 가지고 히스토그램을 생성한다?

			// remain position 돌리기(순서를 유지한채로 경우의수 만들기)
			int n_remain = remains.size();
			int cnt_remain = 0;

			while(cnt_remain < n_remain){
				deque<position> d_tmp = remains;

				vector<position> perm_tmp;
				perm_tmp.push_back(front_p);

				while(!d_tmp.empty()){
					perm_tmp.push_back(d_tmp.front());
					d_tmp.pop_front();
				}

				this->permutations[len].push_back(perm_tmp);
				perm_cnt++;

				position p = remains.front();
				remains.pop_front();
				remains.push_back(p);

				cnt_remain++;
			}
		}

	}
}

