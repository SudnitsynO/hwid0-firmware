#include "flowcalc.h"

const double q1 = 2.903246;
const double q2 = 0.034495;
const double q3 = -0.009423;
const double q4 = -0.000021;
const double q5 = -0.3789263;
const double q6 = 0.0659318;
const double q7 = 0.0038784;
const double q8 = 0.0002222;
const double q16 = 0.0000379;
const double q9 = -1.9678409;
const double q10 = -0.0980654;
const double q11 = 0.0021279;
const double q12 = -0.0001798;
const double q13 = 0.6985173;
const double q14 = 0.0276329;
const double q15 = -0.000792;



const double Q1 = 51.5326423;
const double Q2 = 4.1289672;
const double Q3 = -0.0569409;
const double Q4 = -0.000296;
const double Q5 = -35.3815345;
const double Q6 = -1.4431731;
const double Q7 = 0.0302752;
const double Q8 = -0.0001954;
const double Q16 = 0.0000046;
const double Q9 = 5.2684273;
const double Q10 = -0.3044347;
const double Q11 = 0.0025844;
const double Q12 = 0.0000684;
const double Q13 = 0.3748739;
const double Q14 = 0.1110693;
const double Q15 = -0.0017744;

FlowCalc::FlowCalc(const float q_voltage, const float t_voltage)
{
	double x = (t_voltage - 3);
	x /= (t_voltage - 2);
	double Pt = 0.4808 * x * x * x - 0.0643 * x * x + 0.6843 * x + 3.4328;
	double T = (1000 / Pt) - 273.15;
	double t2 = T;
	t2 *= T;
	double t3 = t2 * T;
	double Q = 0;
	if (q_voltage < 3.2)
		Q = ((q16 * t3 + q15 * t2 + q14 * T + q13) * q_voltage * q_voltage * q_voltage +
		(q12 * t3 + q11 * t2 + q10 * T + q9) * q_voltage * q_voltage +
			(q8 * t3 + q7 * t2 + q6 * T + q5) * q_voltage +
			(q4 * t3 + q3 * t2 + q2 * T + q1));
	else
		Q = ((Q16 * t3 + Q15 * t2 + Q14 * T + Q13) * q_voltage * q_voltage * q_voltage +
		(Q12 * t3 + Q11 * t2 + Q10 * T + Q9) * q_voltage * q_voltage +
			(Q8 * t3 + Q7 * t2 + Q6 * T + Q5) * q_voltage +
			(Q4 * t3 + Q3 * t2 + Q2 * T + Q1));
	q = Q;
	t = T;
}