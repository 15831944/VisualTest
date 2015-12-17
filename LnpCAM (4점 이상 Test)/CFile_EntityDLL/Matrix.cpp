/*****************************************************************************
 Class Name         : CMatrix class Implimetation
 Author             : YOON, Yong Han
 Contact            : yonghany@orgio.net
******************************************************************************/
#include "stdafx.h"  // mfc -- precompiled header �ɼ��� ����Ҷ��� ���
#include "Matrix.h"
#include <cassert>   // for assert
#include <strstream> // for strstream
#include <iomanip>   // for setw(), ...

// ��� �ʺ�
const int CMatrix::MAX_WIDTH = 16;

// CMatrix ��ü�� �ν�ź�� ī��Ʈ
int CMatrix::instance = 0;

// �޸� ������ ���ϱ� ���� ����-- ��ü �Ҹ��ڰ� �־ �����ϰ� �޸𸮰� �����ȴ�.
string CMatrix::emptyString = string();        // �ӽ� ��Ʈ�� ����
CMatrix CMatrix::matC = CMatrix (); // �ӽ� ���

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
		lstrcpy ((LPWSTR)text, L"������� ��������� ���� �����մϴ�.\n"
			L"�������(NxN)�� �ƴϹǷ� ��Ľ��� ���� �� �����ϴ�.");
		break;

	case ME_OUTOFBOUND:
		lstrcpy ((LPWSTR)text, L"Row, Col���� �ʰ� �Ͽ����ϴ�.");
		break;

	case ME_EMPTY:
		lstrcpy ((LPWSTR)text,L"����� �����ϴ�.");
		break;

	case ME_SIZEMISMATCH:
		lstrcpy ((LPWSTR)text,L"�� ��İ� ������ �ϱ����ؼ��� \r\n"
					L"������� �� ROW�� COL�� ������ ��ġ�Ͽ��� �մϴ�.");
		break;

	case ME_BEFOREINV:
		lstrcpy ((LPWSTR)text, L"��Ľ��� ���ϱ� ���� ������� ���� �����ؾ� �մϴ�.\r\n");
		break;

	case ME_SQUARE:
		lstrcpy ((LPWSTR)text, L"��������� �ƴմϴ�. ���� �� ������ ������ �� �����ϴ�.\r\n");
		break;

	default:
		lstrcpy ((LPWSTR)text, L"�����Ƽ� �������� ���� ���� �Դϴ�.-.-+");
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
		lstrcat((LPWSTR)text,L"\n\n���α׷��� �����Ͻðڽ��ϱ�?");
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

/* CMatrix ������, description�� ���ڷ� ����. */
CMatrix::CMatrix()
	: data(0), row(0), col(0), determinant(-999.0)
{
	CMatrix::instance++;
}

/* CMatrix ���� ������ */
CMatrix::CMatrix (const CMatrix & aRef)
	: data(0), row(0), col(0)
	, determinant(-999.0)
{
	CMatrix::instance++;
	copyFrom (aRef);
}


/* CMatrix ������, Row, Col���� ���ڷ� �޾� �޸𸮸� ���� */
CMatrix::CMatrix (uint_t Row, uint_t Col)
	: data(0), row(Row), col(Col), determinant(-999.0)
{
	CMatrix::instance++;
	New (row, col);
	init (row, col);
}

/* CMatrix ���� ������, double [] �κ��� Row, Col�� ����  */
CMatrix::CMatrix(double Data[], uint_t Row, uint_t Col)
	: data(0), row(Row), col(Col), determinant(-999.0)
{
	CMatrix::instance++;
	copyFrom ((double*)Data, Row, Col);
}

/* CMatrix �Ҹ��� */
CMatrix::~CMatrix()
{
	destroy();
	CMatrix::instance--;
	row = col = 0;
	determinant = -999.0;
	
}

/* �޸𸮸� �����Ѵ�. */
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

/* �Ҵ� ������ : = */
CMatrix & CMatrix::operator=(const CMatrix & mat)
{
	New (mat.row, mat.col);
	std::_Copy_impl(mat.begin(), mat.end(), begin());

	determinant = mat.determinant;
	return *this;
}


/* �ϰ������� ���� �ʱ�ȭ�Ѵ�. */
void CMatrix::init (uint_t Row, uint_t Col, double defVal/*=0.0*/)
{
	if (!data) throw CMatrixException (ME_EMPTY, EL_FATAL);

	for (uint_t r=0; r<Row; r++)
		for (uint_t c=0; c<Col; c++)
			(*this)(r, c) = defVal;
}

/* ��� ����Ÿ �����͸� ��ȯ ::= 2���� �迭�� 1���� �迭�� Ǯ����� �� */
double* CMatrix::getMatrix()
{
	if (!data) throw CMatrixException (ME_EMPTY, EL_FATAL);
	return data; 
}

/* ��Ľ�(determinant) */
double CMatrix::getDeterminant()
{
	if (row != col) throw CMatrixException (ME_DET);

	/* ��Ľ��� ������� �������μ� �����. */
	if (determinant == -999)
		throw CMatrixException (ME_BEFOREINV);

	return determinant;
}

/* mat ��ü�� this ��ü�� ���� */
void CMatrix::copyFrom (const CMatrix & mat)
{
	New (mat.row, mat.col);
	std::_Copy_impl (mat.begin(), mat.end(), begin());
	determinant = mat.determinant;
}

/* double *�� this ��ü�� ���� */
void CMatrix::copyFrom (double *Data, uint_t Row, uint_t Col)
{
	New (Row, Col);
	std::_Copy_impl (Data, Data+(Row*Col), begin());
}


/* this ��ü�� mat���� ���� */
void CMatrix::copyTo (CMatrix & mat)
{
	mat.New (row, col);
	std::_Copy_impl (begin(), end(), mat.begin());
}

/* �޸� ���Ҵ� */
void CMatrix::New (uint_t Row, uint_t Col)
{
	double *newData = new double [Row*Col];
	assert (newData);
	delete [] data;
	data = newData;
	row = Row;
	col = Col;
}

/* ����� ��ġ�� �˻� */
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

/* ��ġ ����� ���Ѵ�. */
CMatrix & CMatrix::transpose()
{
	matC.New (row, col);

	/* ��� ���� �ٲ۴�. */
	matC.row = col;
	matC.col = row;
	if (row < 1 || col < 1)
		throw CMatrixException (ME_OUTOFBOUND);

	for (uint_t r=0; r<row; r++)
		for (uint_t c=0; c<col; c++)
			matC(c, r) = (*this)(r, c); /* (aij)^t = aji */

	return matC;
}

/* ����� ��� (�˰���: Gauss-Jordan) */
CMatrix &  CMatrix::inverse (double eps/*=1.0e-6*/)
{
	matC.copyFrom (*this);

	int *work;
	unsigned int i, j, k, r, iw, s, t, u, v;
	double w, wmax, pivot, api, w1;
	
	/* ������� ��������� ���� �����Ѵ�. */
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
		/* �Ǻ���Ұ� e���� ���� ������ 0�� ����� �� */
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

	/* ��Ľ��� ������� ���� �� �����!!! */
	matC.determinant = determinant = w1;
	delete [] work;

	return matC;
}

/* ���ڿ��� ��ȯ�Ѵ�. */
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

			// MAX_WIDTH >= �̸� scientific���� ǥ��(���� �ʹ� �۾� �߸��� ������...)
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

/* ���� ����� �밢����� ����� �����Ѵ�.(�밢���: �밢������ ���� ������ ��� ���� 0. */
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

/* ��������� �밢�� ���ҵ��� ��.
   ��, n*n ��� A=(aij)�� Ʈ���̽��� ������ ����.
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