#pragma once
/*****************************************************************************
 Class Name         : CMatrix class
 Author             : Yong Han, YOON
 Contact            : yonghany@orgio.net
 Creation Date      : 99-10-08
 Last Modification  : 99-10-11
                      99-12-15
                      99-12-20
                      99-12-23
                      00-10-09
                      00-10-25
                      00-11-08

 current supports:
	# arithmetic operator (+, -, *, +=, -=, *=)
	# comparison (equal)
	# 전치행렬(transpose)
	# 역행렬(inverse)
	# 행렬식(determinent)
	# 대각 행렬(diagonal matrix) 
	# 단위 행렬(unit or Identity) 
	# Exception Handler
	# toString() Debug utility

******************************************************************************/
#include <iostream>
#include <cmath>
#include <string>
using namespace std;

#if defined (_MSC_VER) && (_MSC_VER >= 1000)
#pragma warning(disable:4786)
#include "windows.h"
#endif

typedef unsigned int uint_t;

#define S_MatrixVersion "Simple CMatrix Class V1.21\nYonghan, Yoon"

// 오류 레벨
enum ErrorLevel {
	EL_MSG,
	EL_WARNING,
	EL_ERROR,
	EL_FATAL
};

// 오류 코드
enum MatrixError {
	ME_ALLOC,       // memory allocation error.
	ME_DET,         // 정방행렬이 아니므로 행렬식을 구할 수 없슴.
	ME_OUTOFBOUND,  // 범위 오류
	ME_EMPTY,       // 빈 행렬입니다.
	ME_SIZEMISMATCH,// 두 행렬의 크기가 일치하지 않습니다.
	ME_BEFOREINV,   // 행렬식을 구하기 전에 역행렬을 먼저 수행해야 합니다.
	ME_SQUARE       // 정방행렬이 아닙니다.
};

////////////////////////////////////////////////////////////////////
// 예외처리 클랫스 (Exception handler class)
class CMatrixException
{
public:
	CMatrixException (MatrixError err, ErrorLevel eLevel=EL_WARNING)
	{ Error = err; errorLevel = eLevel; }

	MatrixError WhatsWrong ()
	{ return Error; }

	void Explain ();

protected:
	ErrorLevel  errorLevel;
	MatrixError Error;
};

class CMatrix
{
private:
	double  determinant;      /** 행렬식(Determinant) 값이 저장 될 변수 */

	/** 차원(dimension) 변수 */
	uint_t  col, row;         /** 행,열 */
	double *data;             /** 행렬 데이타 포인터:2차원 배열을 1차원 배열로 풀어놓은 것. */

public:

	static const int MAX_WIDTH;

	CMatrix ();

	CMatrix (const CMatrix & aRef);

	CMatrix (uint_t row, uint_t col);

	CMatrix (double data[], uint_t row, uint_t col);

	~CMatrix ();

	/** 역행렬(Inverse CMatrix) */
	CMatrix & inverse (double eps=1.0e-6);

	/** 전치행렬 (Transpose CMatrix) */
	CMatrix & transpose ();

	/** 단위 행렬(Unit)을 생성 후 리턴한다.  */
	static CMatrix & unit (const uint_t order);

	/** 단위 행렬(Unit)로 만든다. */
	void makeUnit ();
	
	/** 정방행렬의 대각선 원소들의 합.
	    즉, n*n 행렬 A=(aij)의 트레이스는 다음과 같다.
        trace (A)=a11+a22+...+ann */
	double trace ();

	/** 현재 행렬을 대각행렬을 만들어 리턴한다.(대각행렬: 대각선상의 값만 제외한 모든 값이 0. */
	CMatrix & diagonal ();

	/** 행렬식(determinant), 행렬식은 역행렬을 구함으로써 구해진다. */
	double getDeterminant();

	/** 행벡터 */
	CMatrix getRowVector (uint_t r);

	/** 열벡터 */
	CMatrix getColVector (uint_t c);

	/** Row의 갯수 */
	uint_t Rows ()  { return row; }

	/** Col의 갯수 */
	uint_t Cols ()  { return col; }

	/** 두 행렬을 비교 */
	bool equal (const CMatrix & mat);

	/** Arithmetic operator with CMatrix object */
	friend const CMatrix operator+(const CMatrix & matA, const CMatrix & matB);
	friend const CMatrix operator-(const CMatrix & matA, const CMatrix & matB);
	friend const CMatrix operator*(const CMatrix & matA, const CMatrix & matB);

	/** Arithmetic operator with Scala value */
	friend const CMatrix operator+(const double scala, const CMatrix & matA);
	friend const CMatrix operator-(const double scala, const CMatrix & matA);
	friend const CMatrix operator*(const double scala, const CMatrix & matA);

	friend const CMatrix operator+(const CMatrix & matA, const double scala)
		{ return scala+matA; }
	friend const CMatrix operator-(const CMatrix & matA, const double scala)
		{ return -scala+matA; }
	friend const CMatrix operator*(const CMatrix & matA, const double scala)
		{ return scala*matA; }

	/** assign operator */
	CMatrix & operator=(const CMatrix & mat);

	CMatrix & operator+=(const CMatrix & mat)
		{	return (*this) = (*this) + mat; }
	CMatrix & operator-=(const CMatrix & mat)
		{ return (*this) = (*this) - mat; }
	CMatrix & operator*=(const CMatrix & mat)
		{ return (*this) = (*this) * mat; }

	CMatrix & operator+=(const double scala)
		{	return (*this) = (*this) + scala; }
	CMatrix & operator-=(const double scala)
		{ return (*this) = (*this) - scala; }
	CMatrix & operator*=(const double scala)
		{ return (*this) = (*this) * scala; }

	/** 2차원 배열처럼 다룰 경우 */
	double& operator() (uint_t Row, uint_t Col);
	const double operator() (uint_t Row, uint_t Col) const;


	/* 복제 */
	CMatrix clone () { return CMatrix(*this); }

	/** 행렬 데이타 포인터:2차원 배열을 1차원 배열로 풀어놓은 것. */
	double* getMatrix();

	/** CMatrix를 파라미터로 받아 this에 복사 */
	void copyFrom (const CMatrix & mat);
	void copyFrom (double *Data, uint_t Row, uint_t Col);

	/** CMatrix에 this의 행렬을 복사 */
	void copyTo (CMatrix & mat);

	/** 행렬 데이타를 문자열로 변환. */
	const char* toString(int nWidth=9);

	/** 버전 문자열을 리턴 */
	const char *getVersion ()
		{ return (const char *)S_MatrixVersion; }

protected:
	/** 1차원 배열로 다룰 경우 */
	double  operator [](uint_t index) const;
	double& operator [](uint_t index);

	/** 시작과 끝 포인터를 리턴 */
	double *begin () { return data; }
	double *end () { return data+(row*col); }

	const double *begin () const { return data; }
	const double *end () const { return data+(row*col); }

	/** row, col 만큼 메모리 할당. */
	void New (uint_t Row, uint_t Col);

	/** CMatrix 객체를 파괴 */
	void destroy();

	/** 행렬을 defVal값으로 초기화 */
	void init (uint_t row, uint_t col, double defVal=0.0);

	/** static members */
	static int instance;
	static string emptyString;
	static CMatrix matC;  /* C = A [op] B 등의 연산 결과를 저장 */
};


inline
double CMatrix::operator [](uint_t index) const
{
	if (index < 0 || index >= row*col)
		throw CMatrixException (ME_OUTOFBOUND);

	return data[index];
}

inline
double& CMatrix::operator [](uint_t index)
{
	if (index < 0 || index >= row*col)
		throw CMatrixException (ME_OUTOFBOUND);

	return data[index];
}

inline
double& CMatrix::operator() (uint_t Row, uint_t Col)
{
	if (Row < 0 || Col < 0 || Row >= row || Col >= col)
		throw CMatrixException (ME_OUTOFBOUND);

	return data[Row * col + Col];
}

inline
const double  CMatrix::operator() (uint_t Row, uint_t Col) const
{
	if (Row < 0 || Col < 0 || Row >= row || Col >= col)
		throw CMatrixException (ME_OUTOFBOUND);

	return data[Row * col + Col];
}


/*------------------------------------------------------------------------*/
/* @{friend  */
/* C = A + B */
inline
const CMatrix operator+(const CMatrix & matA, const CMatrix & matB)
{
	uint_t Row=matA.row, Col=matA.col;
	
	if (matA.row != matB.row || matA.col != matB.col || Row<1 || Col<1)
		throw CMatrixException (ME_SIZEMISMATCH);

	CMatrix::matC.New (Row, Col);

	for (uint_t r=0; r<Row; r++) {
		for (uint_t c=0; c<Col; c++) {
			CMatrix::matC(r,c) = matA(r,c) + matB(r,c);
		}
	}

	return CMatrix::matC;
}

/* C = A - B */
inline
const CMatrix operator-(const CMatrix & matA, const CMatrix & matB)
{
	CMatrix matC;
	uint_t Row=matA.row, Col=matA.col;

	if (matA.row != matB.row
		|| matA.col != matB.col 
		|| Row<1 || Col<1)
		throw CMatrixException (ME_SIZEMISMATCH);

	CMatrix::matC.New (Row, Col);

	for (uint_t r=0; r<Row; r++) {
		for (uint_t c=0; c<Col; c++) {
			CMatrix::matC(r,c) = matA(r,c) - matB(r,c);
		}
	}

	return CMatrix::matC;
}

/* C = A x B */
inline
const CMatrix operator*(const CMatrix & matA, const CMatrix & matB)
{
	uint_t i, j, k;
	double w;

	if (matA.row<1 || matA.col < 1 
		|| matB.row < 1 || matB.col < 1
		|| matA.col != matB.row)
		throw CMatrixException (ME_OUTOFBOUND);

	CMatrix::matC.New (matA.row, matB.col);

	for (i=0; i<matA.row; i++) {
		for (j=0; j<matB.col; j++) {
			w = 0.0;
			for (k=0; k<matA.col; k++) {
				w += matA(i,k) * matB(k,j);
			}
			CMatrix::matC(i,j) = w;
		}
	}
	return CMatrix::matC;
}

/* C = scala + A */
inline
const CMatrix operator+(const double scala, const CMatrix & matA)
{
	uint_t Row=matA.row, Col=matA.col;
	
	if ( Row < 1 || Col < 1 )
		throw CMatrixException (ME_EMPTY);

	CMatrix::matC.New (Row, Col);

	for (uint_t r=0; r<Row; r++) {
		for (uint_t c=0; c<Col; c++) {
			CMatrix::matC(r, c) = scala + matA(r, c);
		}
	}

	return CMatrix::matC;
}

/* C = scala - B */
inline
const CMatrix operator-(const double scala, const CMatrix & matA)
{
	uint_t Row=matA.row, Col=matA.col;

	if ( Row < 1 || Col < 1 )
		throw CMatrixException (ME_EMPTY);

	CMatrix::matC.New (Row, Col);

	for (uint_t r=0; r<Row; r++) {
		for (uint_t c=0; c<Col; c++) {
			CMatrix::matC(r, c) = scala - matA(r, c);
		}
	}

	return CMatrix::matC;
}

/* C = scala x B */
inline
const CMatrix operator*(const double scala, const CMatrix & matA)
{
	uint_t Row=matA.row, Col=matA.col;
	
	if ( Row < 1 || Col < 1 )
		throw CMatrixException (ME_EMPTY);

	CMatrix::matC.New (Row, Col);

	for (uint_t r=0; r<Row; r++) {
		for (uint_t c=0; c<Col; c++) {
			CMatrix::matC(r, c) = scala * matA(r, c);
		}
	}

	return CMatrix::matC;
}