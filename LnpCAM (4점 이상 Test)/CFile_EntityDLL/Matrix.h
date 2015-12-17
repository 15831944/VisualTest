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
	# ��ġ���(transpose)
	# �����(inverse)
	# ��Ľ�(determinent)
	# �밢 ���(diagonal matrix) 
	# ���� ���(unit or Identity) 
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

// ���� ����
enum ErrorLevel {
	EL_MSG,
	EL_WARNING,
	EL_ERROR,
	EL_FATAL
};

// ���� �ڵ�
enum MatrixError {
	ME_ALLOC,       // memory allocation error.
	ME_DET,         // ��������� �ƴϹǷ� ��Ľ��� ���� �� ����.
	ME_OUTOFBOUND,  // ���� ����
	ME_EMPTY,       // �� ����Դϴ�.
	ME_SIZEMISMATCH,// �� ����� ũ�Ⱑ ��ġ���� �ʽ��ϴ�.
	ME_BEFOREINV,   // ��Ľ��� ���ϱ� ���� ������� ���� �����ؾ� �մϴ�.
	ME_SQUARE       // ��������� �ƴմϴ�.
};

////////////////////////////////////////////////////////////////////
// ����ó�� Ŭ���� (Exception handler class)
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
	double  determinant;      /** ��Ľ�(Determinant) ���� ���� �� ���� */

	/** ����(dimension) ���� */
	uint_t  col, row;         /** ��,�� */
	double *data;             /** ��� ����Ÿ ������:2���� �迭�� 1���� �迭�� Ǯ����� ��. */

public:

	static const int MAX_WIDTH;

	CMatrix ();

	CMatrix (const CMatrix & aRef);

	CMatrix (uint_t row, uint_t col);

	CMatrix (double data[], uint_t row, uint_t col);

	~CMatrix ();

	/** �����(Inverse CMatrix) */
	CMatrix & inverse (double eps=1.0e-6);

	/** ��ġ��� (Transpose CMatrix) */
	CMatrix & transpose ();

	/** ���� ���(Unit)�� ���� �� �����Ѵ�.  */
	static CMatrix & unit (const uint_t order);

	/** ���� ���(Unit)�� �����. */
	void makeUnit ();
	
	/** ��������� �밢�� ���ҵ��� ��.
	    ��, n*n ��� A=(aij)�� Ʈ���̽��� ������ ����.
        trace (A)=a11+a22+...+ann */
	double trace ();

	/** ���� ����� �밢����� ����� �����Ѵ�.(�밢���: �밢������ ���� ������ ��� ���� 0. */
	CMatrix & diagonal ();

	/** ��Ľ�(determinant), ��Ľ��� ������� �������ν� ��������. */
	double getDeterminant();

	/** �຤�� */
	CMatrix getRowVector (uint_t r);

	/** ������ */
	CMatrix getColVector (uint_t c);

	/** Row�� ���� */
	uint_t Rows ()  { return row; }

	/** Col�� ���� */
	uint_t Cols ()  { return col; }

	/** �� ����� �� */
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

	/** 2���� �迭ó�� �ٷ� ��� */
	double& operator() (uint_t Row, uint_t Col);
	const double operator() (uint_t Row, uint_t Col) const;


	/* ���� */
	CMatrix clone () { return CMatrix(*this); }

	/** ��� ����Ÿ ������:2���� �迭�� 1���� �迭�� Ǯ����� ��. */
	double* getMatrix();

	/** CMatrix�� �Ķ���ͷ� �޾� this�� ���� */
	void copyFrom (const CMatrix & mat);
	void copyFrom (double *Data, uint_t Row, uint_t Col);

	/** CMatrix�� this�� ����� ���� */
	void copyTo (CMatrix & mat);

	/** ��� ����Ÿ�� ���ڿ��� ��ȯ. */
	const char* toString(int nWidth=9);

	/** ���� ���ڿ��� ���� */
	const char *getVersion ()
		{ return (const char *)S_MatrixVersion; }

protected:
	/** 1���� �迭�� �ٷ� ��� */
	double  operator [](uint_t index) const;
	double& operator [](uint_t index);

	/** ���۰� �� �����͸� ���� */
	double *begin () { return data; }
	double *end () { return data+(row*col); }

	const double *begin () const { return data; }
	const double *end () const { return data+(row*col); }

	/** row, col ��ŭ �޸� �Ҵ�. */
	void New (uint_t Row, uint_t Col);

	/** CMatrix ��ü�� �ı� */
	void destroy();

	/** ����� defVal������ �ʱ�ȭ */
	void init (uint_t row, uint_t col, double defVal=0.0);

	/** static members */
	static int instance;
	static string emptyString;
	static CMatrix matC;  /* C = A [op] B ���� ���� ����� ���� */
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