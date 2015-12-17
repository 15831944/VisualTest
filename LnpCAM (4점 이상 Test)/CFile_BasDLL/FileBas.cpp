#include "StdAfx.h"
#include "FileBas.h"
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

#define new DEBUG_NEW
#define m_iXLower 3
#define m_iXUpper 3
#define m_iYLower 3
#define m_iYUpper 3

#define PHI 3.141592653589793238462643383279 

CFileBas::CFileBas(void)
{
	Init_Bas();
	InitializeFileFolder();
}

CFileBas::~CFileBas(void)
{
	DeleteDrawBufferMem();
	DeleteToolInfoMem();
}


bool CFileBas::Load(LPCTSTR FileName)
{
	CStdioFile	OpenFile;
	BOOL		continue_flag;
	int			LengthTotal;

	m_iPCode = 1;
	basLength = 0;
	countPoint = 0;
	InitEntity();

	if(OpenFile.Open(FileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone) == FALSE)
	{
		AfxMessageBox(L"파일을 불러오는데 실패하였습니다.");
		return FALSE;
	}
	continue_flag = TRUE;
	startEnd = 0;
	LengthTotal = 0;

	do									// Tool의 갯수를 파악하기 위한 구문
	{
		OpenFile.ReadString(buf);
		buf.TrimLeft();
		LengthTotal++;
		if(buf.Left(2)=="T2")
			countTool++;				// TOOL의 갯수 선파악

		else if(buf == "%")
		{
			startEnd++;

			if(startEnd == 2)		break;
		}

	}while(1);


	DeleteDrawBufferMem();
	DrawBuffer = new GbrInfo[LengthTotal*2];

	continue_flag = FALSE;
	OpenFile.Close();


	///////////////    파일 변환 시작		/////////////
	
	if(OpenFile.Open(FileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone) == FALSE)
	{
		AfxMessageBox(L"파일을 불러오는데 실패하였습니다.");	return FALSE;
	}

	DeleteToolInfoMem();
	toolInfo = new TOOLINFO[countTool+1];			// Toolinformation 저장할 구조체 생성
	
	if(COORD_OPEN() == 0)
	{
		AfxMessageBox(L"잘못된 파일을 읽었습니다.");
		return FALSE;
	}

	countTool = 0;		// 초기화

	continue_flag = TRUE;
	OpenFile.ReadString(buf);
	buf.TrimLeft();

	if(buf != "%")
	{
		AfxMessageBox(L"잘못된 파일을 읽었습니다.");
		continue_flag = FALSE;

		return FALSE;
	}

	while(continue_flag)
	{
		OpenFile.ReadString(buf);
		buf.TrimLeft();

		if(buf.Left(1) == "T")		// 모드
		{
			if(buf.GetAt(1) == '0')
			{
				modeProcess = buf.Mid(buf.GetLength()-2, buf.GetLength());
			}
			else if(buf.GetAt(1) == '1')
			{
				// integer와 decimal 값으로 추측, 불필요에 의한 미구현
			}
			else 
			{
				SaveTool();
			}
			
		}
		else if(buf.Left(1) == "X" || buf.Left(1) == "Y")
		{
			Find_Point(buf);
// 			     if(modeProcess == "G0") Find_Point(buf);
// 			else if(modeProcess == "G1") Find_Point(buf);
// 			else if(modeProcess == "G3") Find_Point(buf);
		}
		else if(buf.Left(1) == "P")
		{
			m_iPCode = _ttoi(buf.Mid(0,1));
		}
		else if(buf.Left(1) == "G")
		{
			modeProcess = buf.Mid(1,2);

			Find_Entity(buf);
		}
		else if(buf == "%")
		{
			toolInfo[countTool].countPoint = countPoint;						// 마직막 Tool Point 갯수정보 저장
			continue_flag = FALSE;
		}
			
	}
	//ConvertDxf();

	OpenFile.Close();
	COORD_CLOSE();
	
	return TRUE;
}


void CFileBas::Init_Bas(void)
{
	countPoint = 0;
	basLength = 0;
	tempX = tempY = 0.0;
	countTool = 0;
	DrawBuffer = NULL;
	toolInfo = NULL;
}


int CFileBas::COORD_OPEN(void)
{
	char path[1000];
	CString str;
	
	wsprintfA(path, "%S\\entity.dat", m_strEntityFolder);
	if((m_fCoord = _fsopen(path, "w",_SH_DENYNO))==NULL)
		return 0;
	
	return 1;
}


bool CFileBas::Find_Header(CStdioFile &OpenFile)
{
/*	OpenFile.ReadString(buf);
	buf.TrimLeft();	// 왼쪽 여백 없애기
	if(buf.Mid(1, (buf.FindOneOf(L"I"))) == "ESI")				// 회사이름과 모델명을 찾는다.
	{
		modelName = buf.Mid((buf.FindOneOf(L"I") + 2), (buf.GetLength() - ((buf.FindOneOf(L"I") + 3))) );
	}
	else														// 모델명이 없을경우 지나간다.
	{
		return FALSE;
	}
	*/
	return TRUE;
}

bool CFileBas::Find_Arc(CString strCoord)
{
	return false;
}


bool CFileBas::Find_Circcle(CString strCoord)
{
	return false;
}


bool CFileBas::Find_Line(CString strCoord)
{
	return false;
}

bool CFileBas::Find_Entity(CString strEntity)
{
	int G_pos;				/** G_pos local variable 01*/
	int D_pos;				/** D_pos local variable 02*/
	int X_pos;				/** X_pos local variable 03*/
	int Y_pos;				/** Y_pos local variable 04*/
	int I_pos;				/** I_pos local variable 05*/
	int J_pos;				/** J_pos local variable 06*/

	double dTemp_cX, dTemp_cY, dTemp_cR, dTemp_sA, dTemp_eA;

	G_pos = strEntity.FindOneOf(_T("G"));
	D_pos = strEntity.FindOneOf(_T("D"));
	X_pos = strEntity.FindOneOf(_T("X"));
	Y_pos = strEntity.FindOneOf(_T("Y"));
	I_pos = strEntity.FindOneOf(_T("I"));
	J_pos = strEntity.FindOneOf(_T("J"));

	if(G_pos != -1) //G-CODE 추출
	{
		m_iGCode = Abstract_GCode(strEntity,G_pos,D_pos,X_pos,Y_pos,I_pos,J_pos);
		m_iCurrentGMode = m_iGCode;
	}

	if(D_pos != -1) //D-CODE 추출
	{
		m_iDCode = _ttoi(strEntity.Mid(D_pos+1,strEntity.GetLength()-(D_pos+1)));
		m_iCurrentDMode = m_iDCode;
		if((G_pos != -1)&&(m_iGCode == 54))
		{
			m_strLayerName.Format(_T("%d"),m_iCurrentDMode);
		}
	}

	if(X_pos != -1) //X-CODE 추출
	{
		m_dX = Abstract_XCode(strEntity,X_pos,Y_pos,I_pos,J_pos,D_pos);
		m_dX = ApplySettingX(m_dX);
	}
	// 	else{
	// 		m_dSX = m_dEX;
	// 	}

	if(Y_pos != -1) //Y-CODE 추출
	{
		m_dY = Abstract_YCode(strEntity,Y_pos,I_pos,J_pos,D_pos);
		m_dY = ApplySettingY(m_dY);
	}
	// 	else{
	// 		m_dSY = m_dEY;
	// 	}


	if(I_pos != -1) //I-CODE 추출
	{
		m_dI = Abstract_ICode(strEntity,I_pos,J_pos,D_pos);
		m_dI = ApplySettingX(m_dI);
	}
	else
	{
		m_dI = 0.0000;
	}

	if(J_pos != -1) //J-CODE 추출
	{
		m_dJ = Abstract_JCode(strEntity,J_pos,D_pos);
		m_dJ = ApplySettingY(m_dJ);
	}
	else
	{
		m_dJ = 0.0000;
	}

	switch(m_iCurrentGMode)
	{
	case 0:	//LINE
		if(m_iPCode == 0) // Program Start.
		{
			if(X_pos != -1){
				m_dSX = m_dX;
				m_dEX = m_dX;
			}
			else m_dSX = m_dEX;

			if(Y_pos != -1){
				m_dSY = m_dY;
				m_dEY = m_dY;
			}
			else m_dSY = m_dEY;
		}
		
		m_dCurrentPosX = m_dX;
		m_dCurrentPosY = m_dY;
		
		break;
	case 1:
		if(m_iPCode == 0) // Mark to
		{
			if(X_pos != -1) m_dEX = m_dX;
			else m_dEX = m_dSX;
			if(Y_pos != -1) m_dEY = m_dY;
			else m_dEY = m_dSY;

			WRITE_LINE(m_dSX, m_dSY, m_dEX, m_dEY);

			m_dSX = m_dEX;
			m_dSY = m_dEY;
		}

		m_dCurrentPosX = m_dX;
		m_dCurrentPosY = m_dY;
			
		break;
	case 2:
	case 3:
		if(m_iPCode == 0)
		{
			if(X_pos != -1) m_dEX = m_dX;
			else m_dEX = m_dSX;
			if(Y_pos != -1) m_dEY = m_dY;
			else m_dEY = m_dSY;

			double TempI = m_dX - m_dI;
			double TempJ = m_dY - m_dJ;

			dTemp_cX = m_dI;
			dTemp_cY = m_dJ;

			dTemp_cR = (double)(sqrt(double(TempI*TempI + TempJ*TempJ))); //cal Diameter
			
			if((m_dCurrentPosX == m_dX) && (m_dCurrentPosY == m_dY)) // CW or CCW Circle
			{ 
 				WRITE_CIRCLE(dTemp_cX, dTemp_cY, dTemp_cR);
			}
			else // Arc
			{
				dTemp_sA = Cal_Angle(dTemp_cX,dTemp_cY,m_dSX,m_dSY); // cal Start Angle 
				dTemp_eA = Cal_Angle(dTemp_cX,dTemp_cY,m_dEX,m_dEY); // cal End Angle
 				if(m_iCurrentGMode == 2) WRITE_ARC(dTemp_cX,dTemp_cY,dTemp_cR,dTemp_eA,dTemp_sA); //Inverse CW
     			if(m_iCurrentGMode == 3) WRITE_ARC(dTemp_cX,dTemp_cY,dTemp_cR,dTemp_sA,dTemp_eA); //Inverse CCW
			}

			m_dSX = m_dEX;
			m_dSY = m_dEY;
		}

		m_dCurrentPosX = m_dX;
		m_dCurrentPosY = m_dY;
		break;
	}

	return false;
}

double CFileBas::Cal_Angle(double cx, double cy, double x, double y)
{
	double temp_x;
	double temp_y;
	double angle;

	temp_x = x-cx;
	temp_y = y-cy;

	angle = atan2(temp_y,temp_x) * 180 / PHI;

	return angle;
}

double CFileBas::ApplySettingX(double iNum)
{
	CString Tempstr;				/** Tempstr local variable 01*/
//	int TempNum;					/** TempNum local variable 02*/

	iNum = iNum/(float)(pow((long double)10,m_iXLower));	

// 	if(m_iOmissionMode == LZO) iNum = iNum/(float)(pow((long double)10,m_iXLower));	
// 	else 
// 	{
// 		Tempstr.Format(_T("%d"),iNum);
// 
// 		TempNum = Tempstr.GetLength();
// 
// 		if( TempNum <  m_iXUpper ) iNum = iNum * (10*(m_iXUpper - TempNum));
// 		else if( TempNum == m_iXUpper ) iNum = iNum;
// 		else if( TempNum >  m_iXUpper ) iNum = iNum / (10*(TempNum - m_iXUpper));
// 	}
// 
// 	if(m_fChangeUNIT != 1) 
// 	{
// 		iNum = iNum * m_fChangeUNIT;
// 	}

	return iNum;
}

double CFileBas::ApplySettingY(double iNum)
{
	CString Tempstr;				/** Tempstr local variable 01*/
//	int TempNum;					/** TempNum local variable 02*/

	iNum = iNum/(float)(pow((long double)10,m_iYLower));
// 	if(m_iOmissionMode == LZO) iNum = iNum/(float)(pow((long double)10,m_iYLower));
// 	else 
// 	{
// 		Tempstr.Format(_T("%d"),iNum);
// 		TempNum = Tempstr.GetLength();
// 
// 		if( TempNum <  m_iYUpper ) iNum = iNum * (10*(m_iYUpper - TempNum));
// 		else if( TempNum == m_iYUpper ) iNum = iNum;
// 		else if( TempNum >  m_iYUpper ) iNum = iNum / (10*(TempNum - m_iYUpper));
// 	}
// 
// 	if(m_fChangeUNIT != 1) 
// 	{
// 		iNum = iNum*25.4;
// 	}

	return iNum;
}

double CFileBas::Abstract_XCode(CString strCmd, int X_pos,int Y_pos,int I_pos,int J_pos,int D_pos)
{
	CString coord;			/** coord local variable 01*/

	if(Y_pos != -1) coord=strCmd.Mid(X_pos+1,Y_pos-(X_pos+1));
	else if(I_pos != -1) coord=strCmd.Mid(X_pos+1,I_pos-(X_pos+1));
	else if(J_pos != -1) coord=strCmd.Mid(X_pos+1,J_pos-(X_pos+1));
	else if(D_pos != -1) coord=strCmd.Mid(X_pos+1,D_pos-(X_pos+1));
	else coord=strCmd.Mid(X_pos+1,strCmd.GetLength()-(X_pos+1));

	return _ttof(coord);
}

double CFileBas::Abstract_YCode(CString strCmd, int Y_pos, int I_pos, int J_pos, int D_pos)
{
	CString coord;			/** coord local variable 01*/

	if(I_pos != -1) coord=strCmd.Mid(Y_pos+1,I_pos-(Y_pos+1));
	else if(J_pos != -1) coord=strCmd.Mid(Y_pos+1,J_pos-(Y_pos+1));
	else if(D_pos != -1) coord=strCmd.Mid(Y_pos+1,D_pos-(Y_pos+1));
	else coord=strCmd.Mid(Y_pos+1,strCmd.GetLength()-(Y_pos+1));

	return _ttof(coord);
}

double CFileBas::Abstract_ICode(CString strCmd,int I_pos,int J_pos,int D_pos)
{
	CString coord;			/** coord local variable 01*/

	     if(J_pos != -1) coord=strCmd.Mid(I_pos+1,J_pos-(I_pos+1));
	else if(D_pos != -1) coord=strCmd.Mid(I_pos+1,D_pos-(I_pos+1));
	else                 coord=strCmd.Mid(I_pos+1,strCmd.GetLength()-(I_pos+1));

	return _ttof(coord);
}

double CFileBas::Abstract_JCode(CString strCmd,int J_pos,int D_pos)
{
	CString coord;			/** coord local variable 01*/

	if(D_pos != -1) coord=strCmd.Mid(J_pos+1,D_pos-(J_pos+1));
	else            coord=strCmd.Mid(J_pos+1,strCmd.GetLength()-(J_pos+1));

	return _ttof(coord);
}

int CFileBas::Abstract_GCode(CString strCmd, int G_pos,int D_pos,int X_pos,int Y_pos,int I_pos,int J_pos)
{
	CString G_value;			/** G_value local variable 01*/

	if(X_pos != -1) G_value=strCmd.Mid(G_pos+1,X_pos-(G_pos+1));
	else if(Y_pos != -1) G_value=strCmd.Mid(G_pos+1,Y_pos-(G_pos+1));
	else if(I_pos != -1) G_value=strCmd.Mid(G_pos+1,I_pos-(G_pos+1));
	else if(J_pos != -1) G_value=strCmd.Mid(G_pos+1,J_pos-(G_pos+1));
	else if(D_pos != -1) G_value=strCmd.Mid(G_pos+1,D_pos-(G_pos+1));
	else G_value=strCmd.Mid(G_pos+1,strCmd.GetLength()-1);

	return _ttoi(G_value);
}

bool CFileBas::Find_Point(CString coord)
{
	double x,y;		// 현재라인 위치 저장
	int placeY;		// Y좌표 위치 찾기
	CString tempToolName;

	x = y = 0;
	
	if(coord.Find('Y') == -1)			//Y좌표값이 없을때
	{
		x = Change_Coord(_ttof(coord.Mid(1)) );
		x = x;
		tempX = x;
		y = tempY;
				
	}
	else if(coord.Left(1) != 'X')		// X좌표값이 없을때
	{
		x = tempX;
		y = Change_Coord(_ttof(coord.Mid(1)) );
		y = y;
		tempY = y;

	}
	else
	{
		x = Change_Coord(_ttof(coord.Mid(1)) );
		x = x;
		placeY = coord.Find('Y');							// Y좌표 위치 찾기
		y = Change_Coord(_ttof(coord.Mid(placeY+1)) );		// Y좌표값 대입
		y = y;
		tempX = x;											// X좌표 임시 저장
		tempY = y;											// Y좌표 임시 저장
				
	}

	if(countTool == 0)					// T2를 만나기전 T0의 좌표값을 입력하여 입력전 T0의 구조체 값을 넣어준다.
	{
		//toolInfo[countTool].toolName.Format(L"T0^%.6f",1.);
		//tempToolName = toolInfo[countTool].toolName;
		toolInfo[countTool].toolName.Format(L"0");		
		tempToolName = toolInfo[countTool].toolName;
		toolInfo[countTool].diameter = 0.1;
	}
	else
	{
		tempToolName = toolInfo[countTool].toolName;
	}

	WRITE_POINT(tempToolName, x, y);
	
	return TRUE;
}


void CFileBas::WRITE_POINT(CString tName, double cx, double cy)
{
	CString tempStr;
	int nColor;

	
	nColor = _ttoi(tName);
	nColor += 1;

	fprintf(m_fCoord, "D %S %d %lf %lf\n", tName, nColor, cx, cy);

	DrawBuffer[basLength].entity = "POINT";
	DrawBuffer[basLength].px = cx;		
	DrawBuffer[basLength].py = cy;

	countPoint++;
	basLength++;
}

void CFileBas::WRITE_LINE(double dSX, double dSY, double dEX, double dEY)
{
	CString tempStr;
	int nColor;
	if(countTool == 0)
	{
		toolInfo[countTool].toolName.Format(L"0");		
		tempStr = toolInfo[countTool].toolName;
		toolInfo[countTool].diameter = 0.1;
	}
	else
	{
		tempStr = toolInfo[countTool].toolName;
	}

	DrawBuffer[basLength].entity = "LINE";
	DrawBuffer[basLength].sx = dSX;		
	DrawBuffer[basLength].sy = dSY;
	DrawBuffer[basLength].ex = dEX;		
	DrawBuffer[basLength].ey = dEY;

	nColor = _ttoi(tempStr);
	nColor += 1;
	fprintf(m_fCoord, "L %S %d %lf %lf %lf %lf\n", tempStr, nColor, dSX, dSY, dEX, dEY);

	countPoint++;
	basLength++;
}

void CFileBas::WRITE_CIRCLE(double dCX, double dCY, double dR)
{
	CString tempStr;
	int nColor;

	DrawBuffer[basLength].entity = "CIRCLE";
	DrawBuffer[basLength].cx = dCX;
	DrawBuffer[basLength].cy = dCY;
	DrawBuffer[basLength].r = dR;
	DrawBuffer[basLength].sx = dCX + dR;		
	DrawBuffer[basLength].sy = dCY;
	DrawBuffer[basLength].ex = dCX + dR;		
	DrawBuffer[basLength].ey = dCY;

	if(countTool == 0)
	{
		toolInfo[countTool].toolName.Format(L"0");		
		tempStr = toolInfo[countTool].toolName;
		toolInfo[countTool].diameter = 0.1;
	}
	else
	{
		tempStr = toolInfo[countTool].toolName;
	}

	nColor = _ttoi(tempStr);
	nColor += 1;
	fprintf(m_fCoord, "C %S %d %lf %lf %lf %lf %lf %lf %lf\n", tempStr, nColor, 
		(DrawBuffer[basLength].cx), (DrawBuffer[basLength].cy),(DrawBuffer[basLength].r), 
		(DrawBuffer[basLength].sx), (DrawBuffer[basLength].sy), 
		(DrawBuffer[basLength].ex), (DrawBuffer[basLength].ey));

	countPoint++;
	basLength++;
}

void CFileBas::WRITE_ARC(double dCX, double dCY, double dR, double dSA, double dEA)
{
	double dAng;
	CString tempStr;
	int nColor;

	DrawBuffer[basLength].entity = "ARC";
	DrawBuffer[basLength].cx = dCX;
	DrawBuffer[basLength].cy = dCY;
	DrawBuffer[basLength].r = dR;
	DrawBuffer[basLength].sa = dSA;
	DrawBuffer[basLength].ea = dEA;

	dAng = (dSA * PHI / 180.);
	DrawBuffer[basLength].sx = dCX + (dR * (cos(dAng)));
	DrawBuffer[basLength].sy = dCY + (dR * (sin(dAng)));

	if(dSA > dEA) dEA += 360;

	dAng = (dEA * PHI / 180.);

	DrawBuffer[basLength].ex = dCX + (dR * (cos(dAng)));
	DrawBuffer[basLength].ey = dCY + (dR * (sin(dAng)));

	if(countTool == 0)
	{
		toolInfo[countTool].toolName.Format(L"0");		
		tempStr = toolInfo[countTool].toolName;
		toolInfo[countTool].diameter = 0.1;
	}
	else
	{
		tempStr = toolInfo[countTool].toolName;
	}

	nColor = _ttoi(tempStr);
	nColor += 1;
	fprintf(m_fCoord, "A %S %d %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", 
		tempStr, nColor,
		(DrawBuffer[basLength].cx), (DrawBuffer[basLength].cy), (DrawBuffer[basLength].r), 
		(DrawBuffer[basLength].sa), (DrawBuffer[basLength].ea), (DrawBuffer[basLength].sx), 
		(DrawBuffer[basLength].sy), (DrawBuffer[basLength].ex), (DrawBuffer[basLength].ey));

	countPoint++;
	basLength++;
}

int CFileBas::COORD_CLOSE(void)
{
	fclose(m_fCoord);

	FILE* write;
	CStdioFile read;
	char read_path[1000];
	char write_path[1000];
	CString path,str;
	int revCount;

	wsprintfA(write_path, "%S\\entity.dat", m_strCommonFolder);
	wsprintfA(read_path, "%S\\entity.dat", m_strEntityFolder);
	path = read_path;

	if(read.Open(path,CFile::modeRead|CFile::shareDenyNone|CFile::typeText)==FALSE)
		return FALSE;

	
	if((write=_fsopen(write_path, "w",_SH_DENYNO))==NULL)
		return 0;

	fprintf(write, "[EntityInformation]\n");
	fprintf(write, "InspectionCount %d\n", 0);

	fprintf(write, "EntityLength %d\n",basLength);
	fprintf(write, "ToolCount %d\n",countTool+1);			// 수정요망

	revCount = 0;
	countTool++;
	do
	{
		//fprintf(write, "Tool %d <%s> [%d]\n", revCount, toolInfo[revCount].toolName, toolInfo[revCount].countPoint );
		fprintf(write, "Tool %d %d %d %.4f\n", revCount, revCount, toolInfo[revCount].countPoint, toolInfo[revCount].diameter );
		revCount++;
		countTool--;
	}while(countTool);
	
	fprintf(write, "%%\n");

	while(1)				// Comm 폴더 entity 쓰기
	{
		read.ReadString(str);

		if(str == "") break;

		fprintf(write, "%S\n",str);
	}
	
	read.Close();

	fclose(write);

	return 1;
}


double CFileBas::Round(double initNum)
{
	initNum = floor(10000*(initNum + 0.00005))/10000;		// 소수점 6자리를 유효숫자로 만들어준다.

	return initNum;
}


double CFileBas::Change_Coord(double originNum)
{
	if(modeProcess == "G0") originNum = originNum / pow((long double)10,m_iXLower);	
	
	return originNum;
}


void CFileBas::InitializeFileFolder(void)
{
	CString TempStr;			/** TempStr local variable 01*/
	CString TempStr2;			/** TempStr2 local variable 02*/
	int TempLength;				/** TempLength local variable 03*/
	int TempNumber;				/** TempNumber local variable 04*/
	TCHAR folder[1000];			/** folder local variable 05*/

	////////////////// 실행 디렉토리 만들기 ///////////////////////////
	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr = folder;
	TempLength = TempStr.GetLength();
	TempStr.MakeReverse();
	TempNumber = TempStr.Find(_T("\\"),0);
	TempStr = TempStr.Right(TempLength-TempNumber-1);
	TempStr.MakeReverse();

	m_strCoordFolder	= TempStr+_T("\\Coord");
	m_strCommonFolder	= m_strCoordFolder+_T("\\Comm");
	m_strBasFolder		= m_strCoordFolder+_T("\\Bas");
	m_strDxfFolder		= m_strCoordFolder+_T("\\Dxf");
	m_strEntityFolder	= m_strBasFolder+_T("\\Entity");

	CreateDirectory(m_strCoordFolder,	NULL);
	CreateDirectory(m_strCommonFolder,	NULL);
	CreateDirectory(m_strBasFolder,		NULL);
	CreateDirectory(m_strDxfFolder,		NULL);
	CreateDirectory(m_strEntityFolder,	NULL);
	///////////////////////////////////////////////////////////
}


void CFileBas::SaveTool(void)
{
	double		diaTemp;

	diaTemp = 0;

	toolInfo[countTool].countPoint = countPoint;					// 이전 좌표의 갯수 저장
				
	countPoint = 0;													// 좌표갯수 초기화
	countTool++;													// Tool증가
				
	toolInfo[countTool].toolNum = countTool;						// Tool 번호 저장
				
	diaTemp = _ttof(buf.Mid(3,(buf.GetAllocLength()-3))  );			
	toolInfo[countTool].diameter = Change_Coord(diaTemp);			// 지름 저장

	toolInfo[countTool].toolName.Format(L"%d", countTool);
	
	// T0 나오고 T2가 나오면 새로운 Tool 선언. Diameter 정의.
}


double CFileBas::Return_Diameter(int number)
{
	double LayerDiameter;

	LayerDiameter = toolInfo[number].diameter;

	return LayerDiameter;
}


int CFileBas::ConvertDxf(void)
{
	FILE *out;
	CString SCR,DEST;
	BOOL flag;
	long cnt = 1000;

	SCR = m_strDxfFolder;
	SCR = SCR + L"\\predxf1.dxf";
	DEST = m_strDxfFolder;
	DEST = DEST + L"\\newdxf.dxf";

	wsprintfA(drawing_name, "%S", DEST);

	flag = CopyFile(SCR,DEST,FALSE);

	if( (out=_fsopen(drawing_name, "a",_SH_DENYNO))==NULL)
		return FALSE;

	fprintf(out,"  0\n");
	fprintf(out,"TEXT\n");
	fprintf(out,"  5\n");
	fprintf(out,"56\n");
	fprintf(out,"  8\n");
	fprintf(out,"0\n");
	fprintf(out," 62\n");
	fprintf(out,"    0\n");
	fprintf(out," 10\n");
	fprintf(out,"9.1904761904761898\n");
	fprintf(out," 20\n");
	fprintf(out,"-2.6018528580525522\n");
	fprintf(out," 30\n");
	fprintf(out,"0.0\n");
	fprintf(out," 40\n");
	fprintf(out,"1.0\n");
	fprintf(out,"  1\n");
	fprintf(out,"20\n");
	fprintf(out,"  0\n");
	fprintf(out,"ENDBLK\n");
	fprintf(out,"  5\n");
	fprintf(out,"58\n");
	fprintf(out,"  8\n");
	fprintf(out,"0\n");
	fprintf(out,"  0\n");
	fprintf(out,"ENDSEC\n");
	fprintf(out,"  0\n");
	fprintf(out,"SECTION\n");
	fprintf(out,"  2\n");
	fprintf(out,"ENTITIES\n");
	fprintf(out," 0\n");

	for(unsigned long i=0;i<basLength;i++)
	{
		if(DrawBuffer[i].entity == "LINE")
		{
			fprintf(out,"LINE\n");
			fprintf(out,"  5\n");
			fprintf(out,"%d\n",cnt); cnt++;
			fprintf(out,"  8\n");
			fprintf(out,"0\n");
			fprintf(out," 10\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].sx);
			fprintf(out," 20\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].sy);
			fprintf(out," 30\n");
			fprintf(out,"0.0\n");
			fprintf(out," 11\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].ex);
			fprintf(out," 21\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].ey);
			fprintf(out," 31\n");
			fprintf(out,"0.0\n");
			fprintf(out,"  0\n");
		}
		if(DrawBuffer[i].entity == "CIRCLE")
		{
			fprintf(out,"CIRCLE\n");
			fprintf(out,"  5\n");
			fprintf(out,"%d\n",cnt); cnt++;
			fprintf(out,"  8\n");
			fprintf(out,"0\n");
			fprintf(out," 10\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].cx);
			fprintf(out," 20\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].cy);
			fprintf(out," 30\n");
			fprintf(out,"0.0\n");
			fprintf(out," 40\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].r);
			fprintf(out,"  0\n");
		}
		if(DrawBuffer[i].entity == "ARC")
		{
			fprintf(out,"ARC\n");
			fprintf(out,"  5\n");
			fprintf(out,"%d\n",cnt); cnt++;
			fprintf(out,"  8\n");
			fprintf(out,"0\n");
			fprintf(out," 10\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].cx);
			fprintf(out," 20\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].cy);
			fprintf(out," 30\n");
			fprintf(out,"0.0\n");
			fprintf(out," 40\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].r);
			fprintf(out," 50\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].sa);
			fprintf(out," 51\n");
			fprintf(out,"%.9lf\n",DrawBuffer[i].ea);
			fprintf(out,"  0\n");
		}
		if(DrawBuffer[i].entity == "POINT")
		{
			fprintf(out,"POINT\n");
			fprintf(out,"  5\n");
			fprintf(out,"%d\n",cnt); cnt++;
			fprintf(out,"  8\n");
			fprintf(out,"0\n");
			fprintf(out," 10\n");
			fprintf(out,"%.7lf\n",DrawBuffer[i].px);
			fprintf(out," 20\n");
			fprintf(out,"%.7lf\n",DrawBuffer[i].py);
			fprintf(out," 30\n");
			fprintf(out,"0.0\n");
			fprintf(out," 0\n");
		}
	}

	fprintf(out,"ENDSEC\n");
	fprintf(out,"  0\n");
	fprintf(out,"EOF\n");

	fclose(out);

	cnt = 1000;

	return TRUE;
}


void CFileBas::DeleteDrawBufferMem()
{
	if(DrawBuffer != NULL)
	{
		delete[] DrawBuffer;
		DrawBuffer = NULL;
	}
}

void CFileBas::DeleteToolInfoMem()
{
	if(toolInfo != NULL)
	{
		delete[] toolInfo;
		toolInfo = NULL;
	}
}


void CFileBas::InitEntity(void)
{
	m_dX = 0;
	m_dY = 0;
	m_dSX = 0;
	m_dSY = 0;
	m_dEX = 0;
	m_dEY = 0;
	m_dCurrentPosX = 0;
	m_dCurrentPosY = 0;
}