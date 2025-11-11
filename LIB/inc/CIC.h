#pragma once
#include <cstring>

namespace CIC
{
	template <class T,int order, int decimation_factor>
	class Decimator
	{
		public:
		Decimator()
		{
			N_int = 0;
			diff_index = 0;
			out_sample = 0;
			out_sample_ready = false;
			for (size_t i = 0; i < order; i++)
			{
				integrators[i] = 0;
				diff[i][0] = 0;
				diff[i][1] = 0;
			}
		}

		T gain()
		{
			T r = 1;
			for (size_t i = 0; i < order; i++)
				r *= decimation_factor * 2;
			return r;
		}

		void InSample(const T &in_sample)
		{
			//расчет интеграторов
			integrators[0] += in_sample;
			for (size_t i = 1; i < order; i++)
			{
				integrators[i] += integrators[i - 1];
			}
			N_int++;
			if(N_int >= decimation_factor)		//децимация
			{
				//нужно просчитать дифференциаторы
				N_int = 0;
				T d_in = integrators[order - 1];
				for (size_t i = 0; i < order; i++)
				{
					T d_out = d_in - diff[i][diff_index];
					diff[i][diff_index] = d_in;
					d_in = d_out;
				}
				diff_index++;
				if (diff_index >= 2)
					diff_index = 0;
				out_sample = d_in;
				out_sample_ready = true;
			}
		}

		bool IsOutSampleReady()
		{
			return out_sample_ready;
		}

		T GetOutSample()
		{
			out_sample_ready = false;
			return out_sample;
		}

	private:
		T integrators[order];	//переменные интеграторов
		T diff[order][2];			//переменные дифферернциаторов
		size_t N_int;
		size_t diff_index;
		T out_sample;
		volatile bool out_sample_ready;
	};
}
