// File_Drl.cpp : implementation file
//

#include "StdAfx.h"
#include "FileDrl.h"
#include <cmath>
#include <share.h>

#define _TTOF(x)	(float)_ttof(x)

CFileDrl::CFileDrl(void)
{
	/////////////////////////////////////////
	/////////HEAD 구조체 초기화 /////////////
	m_Head.cstrPathName = "";
	m_Tool = NULL;
	m_DxfPoint = NULL;
	Init_Folder();
	Init_Head();
	Init_Coord();
	////////////////
}


CFileDrl::~CFileDrl(void)
{
	Delete_Tool();
	Delete_DxfPoint();
}

void CFileDrl::Init_Folder()
{
	CString TempStr;			/** TempStr local variable 01*/
	CString TempStr2;			/** TempStr2 local variable 02*/
	int TempLength;				/** TempLength local variable 03*/
	int TempNumber;				/** TempNumber local variable 04*/
	TCHAR folder[1000];			/** folder local variable 05*/

	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr = folder;
	TempLength = TempStr.GetLength();
	TempStr.MakeReverse();
	TempNumber = TempStr.Find(_T("\\"),0);
	TempStr = TempStr.Right(TempLength-TempNumber-1);
	TempStr.MakeReverse();

	m_strCoordFolder	= TempStr+_T("\\Coord");
	m_strCommonFolder	= m_strCoordFolder+_T("\\Comm");
	m_strDrlFolder		= m_strCoordFolder+_T("\\Drl");
	m_strLogFolder		= m_strDrlFolder+_T("\\Log");
	m_strDxfFolder		= m_strCoordFolder+_T("\\Dxf");
	m_strEntityFolder	= m_strDrlFolder+_T("\\Entity");

	CreateDirectory(m_strCoordFolder,	NULL);
	CreateDirectory(m_strCommonFolder,	NULL);
	CreateDirectory(m_strDrlFolder,		NULL);
	CreateDirectory(m_strDxfFolder,		NULL);
	CreateDirectory(m_strEntityFolder,	NULL);
	CreateDirectory(m_strLogFolder,		NULL);

}

void CFileDrl::Init_Coord()
{
	m_fResultX = 0;
	m_fResultY = 0;
	m_fStartX = 0;
	m_fStartY = 0;
	m_fEndX = 0;
	m_fEndY = 0;
	m_Point.cstrX = "0";
	m_Point.cstrY = "0";
}

void CFileDrl::Init_Head()
{
	m_Head.bInchMetric = 0;
	m_Head.bLzTz = 0;
	m_Head.nFormat = 0;
	m_Head.nPointNumber = 0;
	m_Head.nToolNumber = 0;
	m_Head.nVersion = 0;
	m_Head.nUpper = 3;
	m_Head.nLower = 3;
	m_Head.bAbsoluteIncremental = 0;
	m_Head.bDrillRoute = 0;
	m_nToolIndex = 0;

	m_nLogCount = 0;
}

void CFileDrl::Input_Information(CStdioFile &osf)
{
	CString cstrTemp;
	cstrTemp = "[EntityInformation]\n";
	osf.WriteString(cstrTemp);
	cstrTemp = "InspectionCount 0\n";
	osf.WriteString(cstrTemp);
	cstrTemp.Format(L"EntityLength %d\n", m_Head.nPointNumber);
	osf.WriteString(cstrTemp);
	cstrTemp.Format(L"ToolCount %d\n", m_Head.nToolNumber);
	osf.WriteString(cstrTemp);
	for(int i = 0; i < m_Head.nToolNumber; i++)
	{
		cstrTemp.Format(L"Tool %d %d %d %.4f\n", i, m_Tool[i].nToolName, m_Tool[i].nPointNumber, m_Tool[i].fDiameter);
		osf.WriteString(cstrTemp);
	}
	osf.WriteString(L"%\n");

}

void CFileDrl::Calc_AbsoluteIncremental(STRUCT_POINT &point)
{
	if(m_Head.bAbsoluteIncremental == 1)	// 증분 모드 일때
	{
		m_fResultX = Calc_Coord((float)_ttof(point.cstrX));
		m_fResultY = Calc_Coord((float)_ttof(point.cstrY));
	}
	else // 절대 모드 일때
	{
		m_fResultX = Calc_Coord((float)_ttof(point.cstrX));
		m_fResultY = Calc_Coord((float)_ttof(point.cstrY));
	}
}

void CFileDrl::Calc_DrillRouteMode(float fX, float fY)
{
	if(m_Head.bDrillRoute == 1)   // 라우트 모드 일때
	{
		if(m_bRouteLineStartFlag == 0)
		{
			m_fStartX = fX;
			m_fStartY = fY;
			m_bRouteLineStartFlag = 1;
		}
		else
		{
			m_fEndX = fX;
			m_fEndY = fY;
			m_bRouteLineStartFlag = 0;
		}
	}
}

void CFileDrl::Write_Program(CStdioFile &osf, CString &cstrOutString)
{
	if(m_Head.bDrillRoute == 1 && m_bRouteLineStartFlag == 0)
	{
		cstrOutString.Format(L"L %d %d %f %f %f %f\n",m_Tool[m_nToolIndex].nToolName, m_Tool[m_nToolIndex].nToolName, m_fStartX, m_fStartY, m_fEndX, m_fEndY);
		osf.WriteString(cstrOutString);
	}
	else if(m_Head.bDrillRoute == 1 && m_bRouteCircleStartFlag == 1)
	{
		cstrOutString.Format(L"C %d %d %f %f %f \n",m_Tool[m_nToolIndex].nToolName, m_Tool[m_nToolIndex].nToolName, m_fStartX, m_fStartY, _ttof(m_Point.cstrR));
		osf.WriteString(cstrOutString);
	}
	else
	{
		cstrOutString.Format(L"D %d %d %f %f\n", m_Tool[m_nToolIndex].nToolName, m_Tool[m_nToolIndex].nToolName, m_fResultX, m_fResultY);
		osf.WriteString(cstrOutString);
	}
}
BOOL CFileDrl::Calc_PositionXY(CFileDrl::_STRUCT_POINT &point, CString &buf)
{
	int nPositionX, nPositionY, nPositionR;
	nPositionX = buf.FindOneOf(L"X");
	nPositionY = buf.FindOneOf(L"Y");
	nPositionR = buf.FindOneOf(L"R");
	if(nPositionR != -1)
	{
		m_Point.cstrR = buf.Mid((nPositionR+1), (buf.GetLength()-nPositionR));
		if((nPositionX != -1) && (nPositionY != -1)) //X,Y 둘다 있는 경우
		{
			m_Point.cstrX = buf.Mid((nPositionX+1),nPositionY-(nPositionX+1));
			m_Point.cstrY = buf.Mid((nPositionY+1), (buf.GetLength()-nPositionR));
		}
		else if(nPositionY == -1 && nPositionX != -1)  // Y가 없는 경우
		{
			m_Point.cstrX = buf.Mid((nPositionX+1),buf.GetLength()-1);
		}
		else if(nPositionX == -1 && nPositionY != -1) // X가 없는 경우
		{
			m_Point.cstrY = buf.Mid((nPositionY+1), (buf.GetLength()-nPositionR));
		}
		else
			return FALSE;
		return TRUE;
	}
	if((nPositionX != -1) && (nPositionY != -1)) //X,Y 둘다 있는 경우
	{
		m_Point.cstrX = buf.Mid((nPositionX+1),nPositionY-(nPositionX+1));
		m_Point.cstrY = buf.Mid((nPositionY+1), (buf.GetLength()-nPositionY));
	}
	else if(nPositionY == -1 && nPositionX != -1)  // Y가 없는 경우
	{
		m_Point.cstrX = buf.Mid((nPositionX+1),buf.GetLength()-1);
	}
	else if(nPositionX == -1 && nPositionY != -1) // X가 없는 경우
	{
		m_Point.cstrY = buf.Mid((nPositionY+1), (buf.GetLength()-nPositionY));
	}
	else return FALSE;

	return TRUE;
}

BOOL CFileDrl::Input_Program(CStdioFile &sf, CStdioFile &osf, CString &buf)
{
	CString cstrOutString;
	sf.ReadString(buf);
	if(buf.Find(L"T",0)==0)
	{
		m_nToolIndex++;
	}
	else if(buf.GetAt(0) == 'G')
	{
		if(buf.Left(3) == "G00") //라우트모드시작
		{
			if(Calc_PositionXY(m_Point, buf))
			{
				m_fStartX = Calc_Coord(_TTOF(m_Point.cstrX));
				m_fStartY = Calc_Coord(_TTOF(m_Point.cstrY));
			}
			m_Head.bDrillRoute = 1;
			m_bRouteLineStartFlag = 1;
		}
		else if(buf.Left(3)=="G05") // 드릴모드시작
		{
			m_bRouteLineStartFlag = 0;
			m_bRouteCircleStartFlag = 0;
			m_Head.bDrillRoute = 0;
		}
		else if(buf.Left(3)=="G01") // 라우트 선분
		{
			if(Calc_PositionXY(m_Point, buf))
			{
				m_fEndX = Calc_Coord(_TTOF(m_Point.cstrX));
				m_fEndY = Calc_Coord(_TTOF(m_Point.cstrY));
				m_bRouteLineStartFlag = 0;
			}
			m_bRouteCircleStartFlag = 0;
		}
		else if(buf.Left(3)=="G02" || buf.Left(3)=="G03")
		{
			if(Calc_PositionXY(m_Point, buf))
			{
				m_fStartX = Calc_Coord(_TTOF(m_Point.cstrX));
				m_fStartY = Calc_Coord(_TTOF(m_Point.cstrY));
				cstrOutString.Format(L"C '%d' %f %f %f \n",m_Tool[m_nToolIndex].nToolName, m_fStartX, m_fStartY, _ttof(m_Point.cstrR));
				osf.WriteString(cstrOutString);
			}
			else
				m_bRouteCircleStartFlag = 1;
		}
		else if(buf == "G91") // 절대좌표 모드
		{
			m_Head.bAbsoluteIncremental = 1;
		}
		else if(buf == "G90") // 상대좌표 모드
		{
			m_Head.bAbsoluteIncremental = 0;
		}
	}
	else 
	{
		if(buf == "M30")
			return TRUE;
		if(Calc_PositionXY(m_Point, buf))
		{
			Calc_AbsoluteIncremental(m_Point);
			Calc_DrillRouteMode(m_fResultX, m_fResultY);
			m_DxfPoint[m_nDxfPointIndex].cstrEntityName = "P";
			m_DxfPoint[m_nDxfPointIndex].fX = m_fResultX;
			m_DxfPoint[m_nDxfPointIndex].fY = m_fResultY;
			m_nDxfPointIndex++;
			Write_Program(osf, cstrOutString);
		}
	}
	return TRUE;
}
float CFileDrl::Calc_Coord(float duCoord)
{
	CString Tempstr;			/** Tempstr local variable 01*/
	int TempNum;				/** TempNum local variable 02*/

	if(m_Head.bLzTz == 1) duCoord = duCoord/pow((float)10,m_Head.nLower);	// Leading zero
	else 
	{
		Tempstr.Format(_T("%d"),int(duCoord)); // Trailing Zero
		TempNum = Tempstr.GetLength();

		float fduCoord = duCoord/pow((float)10,m_Head.nLower);	// Leading zero

		     if( TempNum <  m_Head.nUpper ) duCoord = duCoord * (pow((float)10,(m_Head.nUpper - TempNum)));
		else if( TempNum == m_Head.nUpper ) duCoord = duCoord;
		else if( TempNum >  m_Head.nUpper ) duCoord = duCoord / (pow((float)10,(TempNum - m_Head.nUpper)));
	}

	if(m_Head.bInchMetric == 0) 
		duCoord = (float)(duCoord*25.4);

	return duCoord;
}
double CFileDrl::round(double value, int pos )
{
	double temp;
	temp = value * pow( (double)10, (int)pos );  // 원하는 소수점 자리수만큼 10의 누승을 함
	temp = floor( temp + 0.5 );          // 0.5를 더한후 버림하면 반올림이 됨
	temp *= pow( (double)10, -(int)pos );           // 다시 원래 소수점 자리수로
	return temp;
}
void CFileDrl::Input_Tool(CStdioFile &sf, CString &buf)
{
	sf.ReadString(buf);
	int nCount = 0;
	CString cstrBuffer = L""; 
	CString cstrBuffer2 = L"";
	CString cstrBuffer3 = L"";
	if(buf.Find(L"T",0)==0)
	{
		for(int i = 0; i < buf.GetLength(); i++)
		{
			if(buf.GetAt(i)=='T')
			{ 
				cstrBuffer = buf.GetAt(i);
				cstrBuffer += " ";
			}
			else if(buf.GetAt(i)=='C')
			{ 
				cstrBuffer += buf.GetAt(i);
				cstrBuffer += " ";
				cstrBuffer2 += " ";
			}
			else if(buf.GetAt(i)=='F')
			{ 
				cstrBuffer += buf.GetAt(i);
				cstrBuffer += " ";
				cstrBuffer2 += " ";
			}
			else if(buf.GetAt(i)=='B')
			{ 
				cstrBuffer += buf.GetAt(i);
				cstrBuffer += " ";
				cstrBuffer2 += " ";
			}
			else if(buf.GetAt(i)=='S')
			{ 
				cstrBuffer += buf.GetAt(i);
				cstrBuffer += " ";
				cstrBuffer2 += " ";
			}
			else if(buf.GetAt(i)=='H')
			{ 
				cstrBuffer += buf.GetAt(i);
				cstrBuffer += " ";
				cstrBuffer2 += " ";
			}
			else if(buf.GetAt(i)=='Z')
			{ 
				if((buf.GetAt(i+1)=='C')||
					(buf.GetAt(i+1)=='F')||
					(buf.GetAt(i+1)=='B')||
					(buf.GetAt(i+1)=='S')||
					(buf.GetAt(i+1)=='H'))
					cstrBuffer2 += "0";
				cstrBuffer += buf.GetAt(i);
				cstrBuffer += " ";
				cstrBuffer2 += " ";
			}
			else if(buf.GetAt(i)=='-')
			{ 
				if((buf.GetAt(i+2)=='C')||
					(buf.GetAt(i+2)=='F')||
					(buf.GetAt(i+2)=='B')||
					(buf.GetAt(i+2)=='S')||
					(buf.GetAt(i+2)=='H'))
					cstrBuffer2 += "0";
				cstrBuffer += buf.GetAt(i);
				cstrBuffer += " ";
				cstrBuffer2 += " ";
				i++;
			}
			else
			{
				cstrBuffer2 += buf.GetAt(i);
			}
		}
		cstrBuffer2 += " ";
		int nTemp = cstrBuffer.GetLength();
		int nTemp2 = cstrBuffer2.GetLength();
		for(int k = 0, l = 0; k < nTemp2; k++)
		{	
			nCount++;
			if(cstrBuffer2.GetAt(k)==' ')
			{
				cstrBuffer3 = cstrBuffer2.Mid((k-nCount+1), (nCount-1));
				nCount = 0;
				switch(cstrBuffer.GetAt(l))
				{
				case 'T':
					{
						m_Tool[m_nToolIndex].nToolName = _ttoi(cstrBuffer3); 
						break;
					}
				case 'C':
					m_Tool[m_nToolIndex].fDiameter = (float)_ttof(cstrBuffer3); break;
				case 'F':
					m_Tool[m_nToolIndex].nFeedRate = _ttoi(cstrBuffer3); break;
				case 'S':
					m_Tool[m_nToolIndex].nSpindleRpm = _ttoi(cstrBuffer3); break;
				case 'H':
					m_Tool[m_nToolIndex].nHitMax = _ttoi(cstrBuffer3); break;
				case 'Z':
					m_Tool[m_nToolIndex].fZDepth = (float)_ttof(cstrBuffer3); break;
				case '-':
					m_Tool[m_nToolIndex].fZDepth = (float)_ttof(cstrBuffer3); break;
				default:
					break;
				}
				l+=2;
			}
		}
		m_nToolIndex++;
	}
}
BOOL CFileDrl::Is_Loaded()
{
	return m_bIsLoaded;
}
void CFileDrl::Open_File()
{
	CFileDialog pDlg(TRUE, L"Drl", L"*.drl", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		L"Excellon File(*.drl)|*.drl|", NULL);
	if(pDlg.DoModal()==IDOK)
	{
		CString path = pDlg.GetPathName(); // 초기경로 지정
		path.MakeUpper();
		if(path.Right(3) == _T("DRL"))
		{
			m_bIsLoaded = Load_DrlFile(path);
		}
		if(m_bIsLoaded == FALSE)
		{
			AfxMessageBox(L"파일을 해석할 수가 없어요.");
		}
	}
}
BOOL CFileDrl::Load_DrlFile(LPCTSTR fName)
{
	m_nStartTime = GetTickCount();
	Init_Head();
	CStdioFile sf, osf;
	//char write_path[300] = "";
	CString write_path;

	//wsprintfA(write_path, "%S\\ConvertLog.txt", m_strLogFolder);
	write_path = m_strLogFolder + L"\\ConvertLog.txt";

	if(osf.Open(write_path, CFile::modeCreate | CFile::modeWrite | CFile::typeText) == FALSE) 
		return FALSE;
	CString	buf, comp;
	if(sf.Open(fName, CFile::modeRead|CFile::typeText) == FALSE) return FALSE;
	
	while (1)
	{
		if(sf.ReadString(buf) == false) return FALSE;
		else if(buf == "M48") break;
	}

// 	if(buf != "M48")
// 	{
// 		return FALSE;
// 	}
	do 
	{
		Find_Header(sf, buf);	// 헤더검색 및 헤더 정보 추출
	}while((buf != "M95") && (buf != "%"));
	if(Make_Tool()==FALSE)	// Tool 배열 생성
	{
		return FALSE;
	}
	Init_Tool(); 
	m_nToolIndex = 0;
	do 
	{
		Find_PointNumber(sf, buf, osf);	// tool, point count
	}while((buf != "M30") && (buf != "%"));
	Init_Point(); 
	Init_DxfPoint();
	sf.Close();
	osf.Close();
	////////// 다시 시작 /////////////
	if(sf.Open(fName, CFile::modeRead|CFile::typeText) == FALSE) return FALSE;
//	sf.ReadString(buf);
	m_nToolIndex = 0;
// 	if(buf != "M48")
// 	{
// 		return FALSE;
// 	}

	while (1)
	{
		if(sf.ReadString(buf) == false) return FALSE;
		else if(buf == "M48") break;
	}
	do 
	{
		Input_Tool(sf, buf);	
	}while((buf != "M95") && (buf != "%"));

	write_path =  m_strEntityFolder + L"\\entity.dat" ;
	if(osf.Open(write_path, CFile::modeCreate | CFile::modeWrite | CFile::typeText) == FALSE) return FALSE;
	Input_Information(osf);
	m_nToolIndex = -1;
	while((buf != "M30"))
	{
// 		if(m_nDxfPointIndex > m_Head.nPointNumber)
// 			break;
		Input_Program(sf, osf, buf);
	}
	osf.Close();
	sf.Close();
	m_nEndTime = GetTickCount();
	//Convert_Dxf();
	MakeCommonEntityFile();
	return TRUE;
}
void CFileDrl::Init_Point()
{
	m_Point.cstrX = "0";
	m_Point.cstrY = "0";
	m_Point.cstrR = "0";
}
BOOL CFileDrl::Find_Header(CStdioFile &sf, CString &buf)
{
	CString			comp;
	int				length,find;
	sf.ReadString(buf);
	length = buf.GetLength();
	find=buf.Find(L",",0);
	comp = buf.Mid(0,find);
	if(comp == "METRIC")
	{
		m_Head.bInchMetric = 1; 
		find = (length - find) -1;
		comp = buf.Right(find);
		if(comp == "TZ")
		{
			//m_Head.bLzTz = 0; // 임시로 전부 LZ 로 해석
			m_Head.bLzTz = 1;
			return TRUE;
		}
		else if(comp == "LZ")
		{
			m_Head.bLzTz = 1;
			return TRUE;
		}
	}
	else if(comp == "INCH")
	{
		m_Head.bInchMetric = 0;
		find = (length - find) -1;
		comp = buf.Right(find);
		if(comp == "TZ")
		{
			m_Head.bLzTz = 1;
			return TRUE;
		}
		else if(comp == "LZ")
		{
			m_Head.bLzTz = 1; // 임시로 전부 LZ 로 해석
			//m_Head.bLzTz = 0;
			return TRUE;
		}
	}
	else if(comp == "VER")
	{
		find = (length - find) -1;
		comp = buf.Right(find);
		m_Head.nVersion = _ttoi(comp);
		return TRUE;
	}
	else if(comp == "FMAT")
	{
		find = (length - find) -1;
		comp = buf.Right(find);
		m_Head.nFormat = _ttoi(comp);
		return TRUE;
	}
	else if((buf.Find(L"T",0)) == 0)
	{
		if(buf == "TCST")
		{
			return TRUE;
		}
		// 도구 찾았음
		m_Head.nToolNumber++;
		return TRUE;
	}
	return TRUE;
}
BOOL CFileDrl::Find_PointNumber(CStdioFile &sf, CString &buf, CStdioFile &osf)
{
	sf.ReadString(buf);
	if(buf.Find(L"T",0)==0)
	{
		m_Tool[m_nToolIndex].nIndex = m_nToolIndex;
		m_nToolIndex++;
	}
	else if((buf.Find(L"X",0) == 0) || (buf.Find(L"Y",0)==0))
	{
		m_Tool[m_nToolIndex-1].nPointNumber++;
		m_Head.nPointNumber++;
	}
	if((buf.FindOneOf(L"G") != -1) || (buf.FindOneOf(L"M") != -1)||(buf.FindOneOf(L"R") != -1)||(buf.FindOneOf(L"$") != -1))
	{
		osf.WriteString(buf);
		m_nLogCount++;
	}
	return TRUE;
}
BOOL CFileDrl::Make_Tool()
{
	Delete_Tool();
	if(m_Head.nToolNumber == 0)
	{
		return FALSE;
	}
	m_Tool = new STRUCT_TOOL[m_Head.nToolNumber+1];
	return TRUE;
}
void CFileDrl::Init_Tool()
{

	for(int i =0; i < m_Head.nToolNumber; i++)
	{
		m_Tool[i].fDiameter = 0;
		m_Tool[i].fZDepth = 0;
		m_Tool[i].nFeedRate = 0;
		m_Tool[i].nHitMax = 0;
		m_Tool[i].nIndex = 0;
		m_Tool[i].nPointNumber = 0;
		m_Tool[i].nSpindleRpm = 0;
		m_Tool[i].nRetractRate = 0;
	}
}
int CFileDrl::Get_ToolNumber()
{
	return m_Head.nToolNumber;
}
int CFileDrl::Get_PointNumber()
{
	return m_Head.nPointNumber;
}
CString CFileDrl::Get_ToolName(int nIndexTool)
{
	CString cstrTemp;
	cstrTemp.Format(L"'%d'",m_Tool[nIndexTool].nToolName);
	return cstrTemp;
}
float CFileDrl::Get_ToolDiameter(int nIndexTool)
{
	return m_Tool[nIndexTool].fDiameter;
}
int CFileDrl::Get_Format()
{
	return m_Head.nFormat;
}
void CFileDrl::Set_UpperLower(int nUpper, int nLower)
{
	m_Head.nUpper = nUpper;
	m_Head.nLower = nLower;
}

void CFileDrl::Delete_Tool()
{
	if(m_Tool != NULL)
	{
		delete [] m_Tool;
		m_Tool = NULL;
	}
}

int CFileDrl::Convert_Dxf()
{

	FILE *out;
	CString SCR,DEST;
	BOOL flag;
	long cnt = 1000;
	char temp[1000];

	SCR = m_strEntityFolder;
	SCR = SCR + L"\\predxf1.dxf";
	DEST = m_strEntityFolder;
	DEST = DEST+ L"\\newdxf.dxf";

	wsprintfA(temp, "%S", DEST);

	flag = CopyFile(SCR,DEST,FALSE);

	if((out = _fsopen(temp, "w", _SH_DENYNO))==NULL)
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

	for(int i=0; i< m_Head.nPointNumber ; i++)
	{
		if(m_DxfPoint[i].cstrEntityName == "P")
			fprintf(out,"POINT\n");
		fprintf(out,"  5\n");
		fprintf(out,"%d\n",cnt); cnt++;
		fprintf(out,"  8\n");
		fprintf(out,"0\n");
		fprintf(out," 10\n");
		fprintf(out,"%f\n",m_DxfPoint[i].fX);
		fprintf(out," 20\n");
		fprintf(out,"%f\n",m_DxfPoint[i].fY);
		fprintf(out," 30\n");
		fprintf(out,"0.0\n");
		fprintf(out,"0\n");
	}
	fprintf(out,"ENDSEC\n");
	fprintf(out,"  0\n");
	fprintf(out,"EOF\n");

	fclose(out);

	cnt = 1000;


	return TRUE;
}
void CFileDrl::Init_DxfPoint()
{
	Delete_DxfPoint();

	m_DxfPoint = new STRUCT_DXFPOINT[m_Head.nPointNumber+1];
	m_nDxfPointIndex = 0;
}
void CFileDrl::Delete_DxfPoint()
{
	if(m_DxfPoint != NULL)
	{
		delete [] m_DxfPoint;
		m_DxfPoint = NULL;
	} 
}

int CFileDrl::MakeCommonEntityFile(void)
{
	int nFileLength;
	int nCnt;
	char read_path[1000];		/** read_path local variable 01*/
	char write_path[1000];		/** write_path local variable 02*/
	CString path;				/** path local variable 03*/
	CString str;				/** str local variable 04*/

	wsprintfA(write_path, "%S\\entity.dat", m_strCommonFolder); 
	wsprintfA(read_path, "%S\\entity.dat", m_strEntityFolder); 
	path = read_path;

	FILE *write;
	CStdioFile read;			/** read local variable 05*/

	if(read.Open(path,CFile::modeRead|CFile::shareDenyNone|CFile::typeText)== false)
		return 0;

	if((write =_fsopen(write_path, "w",_SH_DENYNO))==NULL)
	{
		read.Close();
		return 0;
	}

	nCnt= 0;
	nFileLength = (int)read.GetLength();

	while(1)
	{
		if(nCnt >= nFileLength) 
			break;

		if(read.ReadString(str) == FALSE) 
			break;

		nCnt++;

		fprintf(write, "%S\n",str);
	}

	read.Close();

	fclose(write);

	return 1;
}