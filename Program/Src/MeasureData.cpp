#include "MeasureData.h"
#include <cmath>
#include "BinarySerialize.h"
#include <algorithm>


MeasureData::MeasureData(MutexBase * lock_object):
	 _lock_object(lock_object)
{
	samples.reserve(200);
}

//std::vector<uint8_t> MeasureData::serialise()
//{
//	BinarySerializer s;
//	return s.binary_data;
//}

void MeasureData::calc_stat()
{
	if(_lock_object)
	_lock_object->lock();
	median_q = 0;
	median_t = 0;
	sqr_q = 0;
	min_q = max_q = samples[0].q;
	min_t = max_t = samples[0].t;
	sqr_t = 0;

	for (const auto& sample : samples)
	{
		median_q += sample.q;
		median_t += sample.t;
		max_q = std::max(max_q, sample.q);
		min_q = std::min(min_q, sample.q);
		max_t = std::max(max_t, sample.t);
		min_t = std::min(min_t, sample.t);
	}
	median_q /= samples.size();
	median_t /= samples.size();

	for (const auto& sample : samples)
	{
		const float delta = sample.q - median_q;
		sqr_q += delta * delta;
		const float delta_t = sample.t - median_t;
		sqr_t += delta_t * delta_t;
	}

	sqr_q = std::sqrt(sqr_q / samples.size());
	sqr_t = std::sqrt(sqr_t / samples.size());
	if (_lock_object)
	_lock_object->release();
}

void MeasureData::reset()
{
	if (_lock_object)
	_lock_object->lock();
	samples.clear();
	if (_lock_object)
	_lock_object->release();
}

void MeasureData::insert_point(MeasurePoint&& point)
{
	if (_lock_object)
	_lock_object->lock();
	samples.push_back(point);
	if (_lock_object)
	_lock_object->release();
}

MeasurePoint MeasureData::last_n_median()
{
	MeasurePoint m = { 0,0 };
	if (_lock_object)
	_lock_object->lock();
	auto it = samples.end();
	int n = 0;
	while((it != samples.begin()) && (n < 10))
	{
		--it;
		m.q += it->q;
		m.t += it->t;
		n++;
	}
	if (_lock_object)
	_lock_object->release();
	if (n > 0)
	{
		m.q /= n;
		m.t /= n;
	}
	return m;
}

size_t MeasureData::count_of_points()
{
	if (_lock_object)
	_lock_object->lock();
	const auto count = samples.size();
	if (_lock_object)
	_lock_object->release();
	return count;
}

MeasureData::MeasureData(MeasureData&& other) noexcept
	: create_time(std::move(other.create_time)),
	samples(std::move(other.samples)),
	shtyb_amount(other.shtyb_amount),
	shpur_lenght(other.shpur_lenght),
	zaboy_id(other.zaboy_id),
	id(other.id),
    boring_pos(other.boring_pos)
{
}

MeasureData& MeasureData::operator=(MeasureData&& other) noexcept
{
	if (this == &other)
		return *this;
	create_time = std::move(other.create_time);
	samples = std::move(other.samples);
	shtyb_amount = other.shtyb_amount;
	shpur_lenght = other.shpur_lenght;
	zaboy_id = other.zaboy_id;
	id = other.id;
	boring_pos = other.boring_pos;
	return *this;
}
