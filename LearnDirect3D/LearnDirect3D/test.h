#pragma once

#include <Windows.h>//为了使得XMVerifyCPUSupport函数返回正确值
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

//重载"<<"运算符，这样就可以通过count函数输出XMVECTOR对象
ostream& XM_CALLCONV operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
	return os;
}

int test()
{
	cout.setf(ios_base::boolalpha);

	//检查是否支持SSE2指令集
	if (!XMVerifyCPUSupport())
	{
		cout << "directx math not supported" << endl;
		return 0;
	}

	/*XMVECTOR p = XMVectorZero();
	XMVECTOR q = XMVectorSplatOne();
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorReplicate(-2.0f);
	XMVECTOR w = XMVectorSplatZ(u);

	cout << "p = " << p << endl;
	cout << "q = " << q << endl;
	cout << "u = " << u << endl;
	cout << "v = " << v << endl;
	cout << "w = " << w << endl;*/

	XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
	XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

	XMVECTOR a = u + v;

	XMVECTOR b = u - v;

	XMVECTOR c = 10.0f * u;

	XMVECTOR L = XMVector3Length(u);

	XMVECTOR d = XMVector3Normalize(u);

	XMVECTOR s = XMVector3Dot(u, v);

	XMVECTOR e = XMVector3Cross(u, v);

	XMVECTOR projW;
	XMVECTOR perpW;
	XMVector3ComponentsFromNormal(&projW, &perpW, w, n);

	bool equal = XMVector3Equal(projW + perpW, w) != 0;
	bool noteual = XMVector3NotEqual(projW + perpW, w) != 0;

	XMVECTOR angleVec = XMVector2AngleBetweenVectors(projW, perpW);
	float angleRadians = XMVectorGetX(angleVec);

	float angleDegress = XMConvertToDegrees(angleRadians);

	cout << "u                 = " << u << endl;
	cout << "v                 = " << v << endl;
	cout << "w                 = " << w << endl;
	cout << "n                 = " << n << endl;
	cout << "a = u + v         = " << a << endl;
	cout << "b = u - v         = " << b << endl;
	cout << "c = 10 * u        = " << c << endl;
	cout << "d = u / ||u||     = " << d << endl;
	cout << "e = u x v         = " << e << endl;
	cout << "L = ||u||         = " << L << endl;
	cout << "s = u.v           = " << s << endl;
	cout << "projW             = " << projW << endl;
	cout << "perpW             = " << perpW << endl;
	cout << "projW + perpW == w = " << equal << endl;
	cout << "projW + perpW != w = " << noteual << endl;
	cout << "angle             = " << angleDegress << endl;

	return 0;
}
