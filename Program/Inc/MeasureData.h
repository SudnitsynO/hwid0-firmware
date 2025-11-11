#pragma once
#include <vector>
#include "DateTime.h"
#include "MutexBase.h"

struct MeasurePoint
{
	float t;
	float q;
};

class MeasureData
{
public:
	MeasureData(MutexBase * lock_object);
//	std::vector<uint8_t> serialise();

	void calc_stat();
	void reset();
	void insert_point(MeasurePoint && point);
	MeasurePoint last_n_median();
	size_t count_of_points();

	DateTime create_time;
	DateTime shtyb_time;
	DateTime start_time;

	vector<MeasurePoint> samples;

	MeasureData(const MeasureData& other) = delete;

	MeasureData(MeasureData&& other) noexcept;

	MeasureData& operator=(const MeasureData& other) = delete;

	MeasureData& operator=(MeasureData&& other) noexcept;

	float median_q{0};
	float median_t{0};
	float sqr_q{0};
	float max_t{0};
	float min_t{0};
	float max_q{0};
	float min_q{0};

	MutexBase * _lock_object;
	float sqr_t{0};

	float shtyb_amount{};
	float shpur_lenght{};
	int zaboy_id;
	int id;
	int boring_pos;
};
