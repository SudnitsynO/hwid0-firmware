#pragma once

template <class T,int ORDER, const int* DL,const int* NL, const T (* DEN)[3], const T (* NUM)[3]>
class IRR
{
private:
	T WHistory[ORDER][2];
public:
    T filt(T X) {
        for (auto i = 0; i < ORDER; i++) {
            T W;
            W = X;
            for (auto j = 1; j < DL[i]; j++) {
                W -= DEN[i][j] * WHistory[i][j - 1];
            };
            W /= DEN[i][0];

            X = W * NUM[i][0];
            for (int j = 1; j < NL[i]; j++) {
                X += NUM[i][j] * WHistory[i][j - 1];
            };
            WHistory[i][1] = WHistory[i][0];
            WHistory[i][0] = W;
        };
        return X;
    };
};