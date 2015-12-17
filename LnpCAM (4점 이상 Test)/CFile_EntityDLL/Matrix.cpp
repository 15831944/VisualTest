/*****************************************************************************
 Class Name         : CMatrix class Implimetation
 Author             : YOON, Yong Han
 Contact            : yonghany@orgio.net
******************************************************************************/
#include "stdafx.h"  // mfc -- precompiled header 옵션을 사용할때만 사용
#include "Matrix.h"
#include <cassert>   // for assert
#include <strstream> // for strstream
#include <iomanip>   // for setw(), ...

// 출력 너비
const int CMatrix::MAX_WIDTH = 16;

// CMatrix 객체의 인스탄스 카운트
int CMatrix::instance = 0;

// 메모리 누출을 피하기 위해 사용됨-- 자체 소멸자가 있어서 안전하게 메모리가 해제된다.
string CMatrix::emptyString = string();        // 임시 스트링 버퍼
CMatrix CMatrix::matC = CMatrix (); // 임시 행렬

/*------------------------------------------------------------------------*/
// Exception Handling
void CMatrixException::Explain ()
{
	UINT style;
	UINT sound;

	const int TEXT_SIZE = 512;
	int response;
	char text[TEXT_SIZE];

	memset (text, 0x00, TEXT_SIZE);

	/* set values specific to error string to verity */
	switch (Error) {
	case ME_ALLOC:
		lstrcpy ((LPWSTR)text, L"Memory allocation error.");
		break;

	case ME_DET:
		lstrcpy ((LPWSTR)text, L"역행렬은 정방행렬일 때만 성립합니다.\n"
			L"정방행렬(NxN)이 아니므로 행렬식을 구할 수 없습니다.");
		break;

	case ME_OUTOFBOUND:
		lstrcpy ((LPWSTR)text, L"Row, Col값을 초과 하였습니다.");
		break;

	case ME_EMPTY:
		lstrcpy ((LPWSTR)text,L"행렬이 없습니다.");
		break;

	case ME_SIZEMISMATCH:
		lstrcpy ((LPWSTR)text,L"두 행렬간 연산을 하기위해서는 \r\n"
					L"두행렬의 각 ROW와 COL의 갯수가 일치하여야 합니다.");
		break;

	case ME_BEFOREINV:
		lstrcpy ((LPWSTR)text, L"행렬식을 구하기 전에 역행렬을 먼저 수행해야 합니다.\r\n");
		break;

	case ME_SQUARE:
		lstrcpy ((LPWSTR)text, L"정방행렬이 아닙니다. 따라서 이 연산을 수행할 수 없습니다.\r\n");
		break;

	default:
		lstrcpy ((LPWSTR)text, L"귀찮아서 정의하지 않은 오류 입니다.-.-+");
	}

	switch (errorLevel) {
	case EL_MSG:
		sound = MB_ICONINFORMATION;
		style = MB_OK;
		break;	
	case EL_WARNING:
		sound = MB_ICONEXCLAMATION;
		style = MB_OK;
		break;
	case EL_ERROR:
	case EL_FATAL:
		lstrcat((LPWSTR)text,L"\n\n프로그램을 종료하시겠습니까?");
		sound = MB_ICONSTOP;
		style = MB_YESNO;
		break;
	}

	style |= sound;
	MessageBeep (sound);
	response = MessageBox (NULL, (LPWSTR)text, L"MatrixException Handler", style);

	if (errorLevel == EL_ERROR || errorLevel == EL_FATAL && response == IDYES)
		PostQuitMessage (1);
}

/*----------------------------------------------------------------------------*/

/* CMatrix 생성자, description을 인자로 취함. */
CMatrix::CMatrix()
	: data(0), row(0), col(0), determinant(-999.0)
{
	CMatrix::instance++;
}

/* CMatrix 복사 생성자 */
CMatrix::CMatrix (const CMatrix & aRef)
	: data(0), row(0), col(0)
	, determinant(-999.0)
{
	CMatrix::instance++;
	copyFrom (aRef);
}


/* CMatrix 생성자, Row, Col등을 인자로 받아 메모리만 예약 */
CMatrix::CMatrix (uint_t Row, uint_t Col)
	: data(0), row(Row), col(Col), determinant(-999.0)
{
	CMatrix::instance++;
	New (row, col);
	init (row, col);
}

/* CMatrix 복사 생성자, double [] 로부터 Row, Col을 복사  */
CMatrix::CMatrix(double Data[], uint_t Row, uint_t Col)
	: data(0), row(Row), col(Col), determinant(-999.0)
{
	CMatrix::instance++;
	copyFrom ((double*)Data, Row, Col);
}

/* CMatrix 소멸자 */
CMatrix::~CMatrix()
{
	destroy();
	CMatrix::instance--;
	row = col = 0;
	determinant = -999.0;
	
}

/* 메모리를 해제한다. */
void CMatrix::destroy()
{
	if (data) {
		delete [] data;
		data = 0;
	}
#ifdef _DEBUG
	TRACE1 ("\"CMatrix instance %d\" destroyed\n", CMatrix::instance);
#endif
}

/* 할당 연산자 : = */
CMatrix & CMatrix::operator=(const CMatrix & mat)
{
	New (mat.row, mat.col);
	std::_Copy_impl(mat.begin(), mat.end(), begin());

	determinant = mat.determinant;
	return *this;
}


/* 일괄적으로 값을 초기화한다. */
void CMatrix::init (uint_t Row, uint_t Col, double defVal/*=0.0*/)
{
	if (!data) throw CMatrixException (ME_EMPTY, EL_FATAL);

	for (uint_t r=0; r<Row; r++)
		for (uint_t c=0; c<Col; c++)
			(*this)(r, c) = defVal;
}

/* 행렬 데이타 포인터를 반환 ::= 2차원 배열을 1차원 배열로 풀어놓은 것 */
double* CMatrix::getMatrix()
{
	if (!data) throw CMatrixException (ME_EMPTY, EL_FATAL);
	return data; 
}

/* 행렬식(determinant) */
double CMatrix::getDeterminant()
{
	if (row != col) throw CMatrixException (ME_DET);

	/* 행렬식은 역행렬을 구함으로서 얻어짐. */
	if (determinant == -999)
		throw CMatrixException (ME_BEFOREINV);

	return determinant;
}

/* mat 객체를 this 객체로 복사 */
void CMatrix::copyFrom (const CMatrix & mat)
{
	New (mat.row, mat.col);
	std::_Copy_impl (mat.begin(), mat.end(), begin());
	determinant = mat.determinant;
}

/* double *를 this 객체로 복사 */
void CMatrix::copyFrom (double *Data, uint_t Row, uint_t Col)
{
	New (Row, Col);
	std::_Copy_impl (Data, Data+(Row*Col), begin());
}


/* this 객체를 mat으로 복사 */
void CMatrix::copyTo (CMatrix & mat)
{
	mat.New (row, col);
	std::_Copy_impl (begin(), end(), mat.begin());
}

/* 메모리 재할당 */
void CMatrix::New (uint_t Row, uint_t Col)
{
	double *newData = new double [Row*Col];
	assert (newData);
	delete [] data;
	data = newData;
	row = Row;
	col = Col;
}

/* 두행렬 일치성 검사 */
bool CMatrix::equal (const CMatrix & mat)
{
	if (this->row != mat.row || this->col != mat.col)
		throw CMatrixException (ME_SIZEMISMATCH);

	if (!(this->row == mat.row && this->col==mat.col))
		return false;

	return std::_Equal(begin(), end(), mat.begin());
}


/* Unit */
CMatrix & CMatrix::unit (const uint_t order)
{
	matC.New (order, order);

	for (uint_t i=0; i < matC.Rows(); i++)
		for (uint_t j=0; j < matC.Cols(); j++)
			matC(i,j) = i == j ? 1 : 0;
	return matC;
}

/* Unit */
void CMatrix::makeUnit ()
{
	uint_t Row = (row < col ? row : col);

	matC.New (Row, Row);
	for (size_t i=0; i < matC.Rows(); i++)
		for (size_t j=0; j < matC.Cols(); j++)
			matC (i,j) = i == j ? 1 : 0;

	copyFrom (matC);
}

/* 전치 행렬을 구한다. */
CMatrix & CMatrix::transpose()
{
	matC.New (row, col);

	/* 행과 열을 바꾼다. */
	matC.row = col;
	matC.col = row;
	if (row < 1 || col < 1)
		throw CMatrixException (ME_OUTOFBOUND);

	for (uint_t r=0; r<row; r++)
		for (uint_t c=0; c<col; c++)
			matC(c, r) = (*this)(r, c); /* (aij)^t = aji */

	return matC;
}

/* 역행렬 계산 (알고리즘: Gauss-Jordan) */
CMatrix &  CMatrix::inverse (double eps/*=1.0e-6*/)
{
	matC.copyFrom (*this);

	int *work;
	unsigned int i, j, k, r, iw, s, t, u, v;
	double w, wmax, pivot, api, w1;
	
	/* 역행렬은 정방행렬일 때만 성립한다. */
	if (row<2 || row!=col || eps <= 0.0)
		throw CMatrixException (ME_DET);

	work = new int [row];
	w1 = 1.0;
	for ( i=0; i<row; i++ ) work[i] = i;

	for ( k=0; k<row; k++ )  {
		wmax = -999999999.0;
		for ( i=k; i<row; i++ ) {
			w = fabs ( matC[i*col+k] );
			if( w > wmax ) {
				wmax = w;
				r = i;
			}
		}
		pivot = matC [r*col+k];
		api = fabs (pivot);
		/* 피봇요소가 e보다 작을 정도로 0에 가까울 때 */
		if ( api <= eps ) {
			matC.determinant = determinant = w1;
			delete [] work;
			return matC;
		}
		w1 *= pivot;
		u = k * col;
		v = r * col;
		/* pivoting */
		if (r != k) {
			w1 = -w;
			iw = work[k];
			work[k] = work[r];
			work[r] = iw;
			for ( j=0; j<row; j++ ) {
				s = u + j;
				t = v + j;
				w = matC[s];
				matC[s] = matC[t];
				matC[t] = w;
			}
		}
		for ( i=0; i<row; i++ )
			matC[u+i] /= pivot;
		for ( i=0; i<row; i++ ) {
			if ( i != k) {
				v= i*col;
				s = v+k;
				w = matC[s];
				if ( w != 0.0 ) {
					for ( j=0; j<row; j++ )
						if (j != k) matC[v+j] -= w * matC[u+j];
					matC[s] = -w / pivot;
				}
			}
		}
		matC[u+k] = 1.0 / pivot;
	}

	for ( i=0; i<row; i++ ) {
		while (1) {
			k = work[i];
			if (k == i) break;
			iw = work[k];
			work[k] = work[i];
			work[i] = iw;
			for ( j=0; j<row; j++ ) {
				u = j*col;
				s = u + i;
				t = u + k;
				w = matC[s];
				matC[s] = matC[t];
				matC[t] = w;
			}
		}
	}

	/* 행렬식은 역행렬을 구할 때 얻어짐!!! */
	matC.determinant = determinant = w1;
	delete [] work;

	return matC;
}

/* 문자열로 변환한다. */
const char* CMatrix::toString(int nWidth/*=13*/)
{
	if (!data) {
		return "empty CMatrix\n";
	}

	int  bufSize = (col+1)*nWidth*row;
	char *membuff = new char [bufSize+1];

	memset (membuff, 0x00, bufSize+1);
	strstream sout(membuff, bufSize);

	for (uint_t i=0; i<row; i++) {
		for (uint_t j=0; j<col; j++) {

			// MAX_WIDTH >= 이면 scientific으로 표기(값이 너무 작아 잘리기 때문에...)
			if (nWidth>=MAX_WIDTH)
				sout	<< setw(MAX_WIDTH) 
						<< setiosflags(ios::scientific)
						<< (*this)(i,j);
			else
				sout	<< setw(nWidth)
						<< setiosflags(ios::fixed)
						<< (*this)(i,j);

			if ((j+1)%col==0) sout << endl;
			else              sout << " ";
		}
	}

	emptyString = string(sout.str());
	delete [] membuff;

	return emptyString.c_str();
}

/* 현재 행렬을 대각행렬을 만들어 리턴한다.(대각행렬: 대각선상의 값만 제외한 모든 값이 0. */
CMatrix & CMatrix::diagonal ()
{
	if (Rows() < 1 || Cols() < 1)
		throw CMatrixException (ME_OUTOFBOUND);

	matC.New (Rows(), Cols());

	for (uint_t r=0; r<row; r++) {
		for (uint_t c=0; c<col; c++) {
			if (r == c)
				matC(c, r) = (*this)(r, c);
			else
				matC(c, r) = 0;
		}
	}

	return matC;
}

/* 정방행렬의 대각선 원소들의 합.
   즉, n*n 행렬 A=(aij)의 트레이스는 다음과 같다.
   trace (A)=a11+a22+...+ann */
double CMatrix::trace ()
{
	if (Rows() < 1 || Cols() < 1)
		throw CMatrixException (ME_OUTOFBOUND);
	if (Rows() != Cols())
		throw CMatrixException (ME_SQUARE);

	double tr=0.0;
	for (uint_t r=0; r<row; r++) {
		for (uint_t c=0; c<col; c++) {
			if (r == c)
				tr += (*this)(r, c);
		}
	}

	return tr;
}

/*--------<EOF>----------------------------------------------------------*/