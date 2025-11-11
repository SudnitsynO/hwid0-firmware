#pragma once
#include <stdint.h>
#include "gpio_interface.h"



	enum class LmxFoldPinMode
	{
		TriState,
		RDividerOutput,
		NDividerOutput,
		SerialDataOutput,
		LDOutput,
		OpenDrainLockDetect,
		High,
		Low
	};

	enum class LmxPolarity
	{
		Normal,
		Inverse
	};

	enum class LmxCPCurrent
	{
		Icp0_25ma,
		Icp1ma
	};


template <class SclkPin, class MosiPin, class LatchPin>
class LMX2306
{
public:
	

private:

	static constexpr uint32_t FOLD_MASK = 0b11100;
	static constexpr uint32_t FOLD_MASK_3 = 0b00100;
	static constexpr uint32_t FOLD_MASK_4 = 0b01000;
	static constexpr uint32_t FOLD_MASK_5 = 0b10000;
	static constexpr uint32_t PD_POLARITY_MASK = 0b100000;
	static constexpr uint32_t GAIN_MASK = 0b1000000000000000000;
	static constexpr uint32_t R_MASK = 0b11111111111111;
	static constexpr uint32_t N_MASK = 0b111111111111100111;


	uint32_t R_reg = 0;
	uint32_t N_reg = 0;
	uint32_t F_reg = 0;

	uint32_t N = 0;
	uint32_t R = 0;

public:
	float _ref_frequency = 0;

	void init(float ref_frequency)
	{
		_ref_frequency = ref_frequency;
		R_reg = 0;
		N_reg = 0;
		F_reg = 0b000001000000000000;	//CP polarity is normal;
		LatchPin::Set();
		MosiPin::Reset();
		SclkPin::Reset();
	}

	void SendData(uint32_t data)
	{
		LatchPin::Reset();
		for (int i = 20; i >= 0; i--)
		{
			if(data & (1 << i))
			{
				MosiPin::Set();
			}
			else
				MosiPin::Reset();
			SclkPin::Set();
			SclkPin::Reset();
		}
		LatchPin::Set();
	}

	void UpdateLMX()
	{
		SendData(0b00 | R_reg << 2);
		SendData(0b01 | N_reg << 2);
		SendData(0b10 | F_reg << 2);
	}

	void config(LmxFoldPinMode mode, LmxPolarity pd_polarity, LmxCPCurrent cp_current)
	{
		F_reg &= ~FOLD_MASK;
		switch (mode)
		{
		case LmxFoldPinMode::TriState:
			F_reg |= 0;
			break;
		case LmxFoldPinMode::RDividerOutput:
			F_reg |= FOLD_MASK_5;
			break;
		case LmxFoldPinMode::NDividerOutput:
			F_reg |= FOLD_MASK_4;
			break;
		case LmxFoldPinMode::SerialDataOutput:
			F_reg |= FOLD_MASK_4 | FOLD_MASK_5;
			break;
		case LmxFoldPinMode::LDOutput:
			F_reg |= FOLD_MASK_3;
			break;
		case LmxFoldPinMode::OpenDrainLockDetect:
			F_reg |= FOLD_MASK_3 | FOLD_MASK_5;
			break;
		case LmxFoldPinMode::High:
			F_reg |= FOLD_MASK_3 | FOLD_MASK_4;
			break;
		case LmxFoldPinMode::Low:
			F_reg |= FOLD_MASK;
			break;
		}

		switch (pd_polarity)
		{
		case LmxPolarity::Normal:
			F_reg |= PD_POLARITY_MASK;
			break;
		case LmxPolarity::Inverse:
			F_reg &= ~PD_POLARITY_MASK;
			break;
		}

		switch (cp_current)
		{
		case LmxCPCurrent::Icp0_25ma:
			N_reg &= ~GAIN_MASK;
			break;
		case LmxCPCurrent::Icp1ma:
			N_reg |= GAIN_MASK;
			break;
		}

		UpdateLMX();
	}

	float get_vco_freq()
	{
		return 0;
	}

	float get_true_vco_frequency(float ref_frequency)
	{

	}

	void get_ref_frequency(float true_vco_frequency)
	{

	}

	void set_r(uint32_t r_value)
	{
		r_value &= 0b11111111111111;	//14 bit
		R = r_value;
		R_reg &= ~R_MASK;
		R_reg |= r_value;
		UpdateLMX();
	}

	void set_n(uint32_t n_value)
	{
		n_value &= 0b1111111111111111;	//16 bit
		N = n_value;
		N_reg &= ~N_MASK;
		N_reg |= (n_value & 0b111) | ((n_value & 0b1111111111111000) << 2);
		UpdateLMX();
	}

};
