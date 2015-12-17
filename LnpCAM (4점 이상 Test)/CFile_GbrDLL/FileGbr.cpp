#include "StdAfx.h"
#include "FileGbr.h"
#include <math.h>
#include <locale.h>
#include <share.h>
#include <locale.h>

#define PHI 3.141592653589793238462643383279 

CFileGbr::CFileGbr(void)
{
	setlocale(LC_ALL,"Korean");
	pDrawBuffer = NULL;
	m_iCurrentDMode = 2;
	m_iCurrentGMode = 1;
	m_dCurrentPosX = 0.0;
	m_dCurrentPosY = 0.0;
	m_lGbrLength = 0;
	m_lTrueLength = 0;
	m_iLayerNumber = 0;
	m_iLayerCount = 0;
	m_strLayerName = "0";
	EntityFileOut = NULL;
	GbrFileOut = NULL;
	m_bLoadStopFlag = false;
	m_dI = 0;
	m_dJ = 0;
	m_dX = 0;
	m_dY = 0;
	m_fChangeUNIT = 0;
	m_iCoordMode = 0;
	m_iDCode = 0;
	m_iGCode = 0;
	m_iOmissionMode = 0;
	m_iProgressCount = 0;
	m_iTotalProgress = 0;
	m_iXLower = 0;
	m_iXUpper = 0;
	m_iYLower = 0;
	m_iYUpper = 0;
	m_nInspectionCnt = 0;
	
	InitializeFileFolder();
}

CFileGbr::~CFileGbr(void)
{
	InitializeGbrBuffer();
}

void CFileGbr::InitializeFileFolder(void)
{
	CString TempStr;			/** TempStr local variable 01*/
	CString TempStr2;			/** TempStr2 local variable 02*/
	int TempLength;				/** TempLength local variable 03*/
	int TempNumber;				/** TempNumber local variable 04*/
	TCHAR folder[1000];			/** folder local variable 05*/
	////////////////// Entity 저장 디렉토리 만들기 ///////////////////////////
	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr = folder;
	TempLength = TempStr.GetLength();
	TempStr.MakeReverse();
	TempNumber = TempStr.Find(_T("\\"),0);
	TempStr = TempStr.Right(TempLength-TempNumber-1);
	TempStr.MakeReverse();

	m_strEntityFolder = TempStr+_T("\\Coord");
	CreateDirectory(m_strEntityFolder, NULL);

	m_strEntityFolder = m_strEntityFolder+_T("\\Gbr");
	CreateDirectory(m_strEntityFolder, NULL);

	m_strEntityFolder = m_strEntityFolder+_T("\\Entity");
	CreateDirectory(m_strEntityFolder, NULL);
	///////////////////////////////////////////////////////////

	////////////////// Common 디렉토리 만들기 ///////////////////////////
	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr2 = folder;
	TempLength = TempStr2.GetLength();
	TempStr2.MakeReverse();
	TempNumber = TempStr2.Find(_T("\\"),0);
	TempStr2 = TempStr2.Right(TempLength-TempNumber-1);
	TempStr2.MakeReverse();

	m_strCommonFolder = TempStr2+_T("\\Coord");
	CreateDirectory(m_strCommonFolder, NULL);

	m_strCommonFolder = m_strCommonFolder+_T("\\Comm");
	CreateDirectory(m_strCommonFolder, NULL);
	///////////////////////////////////////////////////////////
}

void CFileGbr::InitializeGbrBuffer(void)
{
	if (pDrawBuffer != NULL) 
	{ 
		delete [] pDrawBuffer; 
		pDrawBuffer = NULL; 
	}

	m_bLoadStopFlag =FALSE;

	m_iCurrentDMode = 2;
	m_iCurrentGMode = 1;
	m_dCurrentPosX = 0.0;
	m_dCurrentPosY = 0.0;
	m_lGbrLength = 0;
	m_lTrueLength = 0;
	m_iLayerCount = 0;
	m_iLayerNumber = 0;
	m_iProgressCount = 0;

	m_dSX = 0;
	m_dEX = 0;
	m_dSY = 0;
	m_dEY = 0;
	m_dX = 0;
	m_dX = 0;
}

int CFileGbr::GetLine(CString& buf)
{
	char line[1000];		/** line local variable 01*/

	if (fgets(line,100,GbrFileOut) == false) return 0;
	buf=line;
	buf.TrimLeft();
	buf.TrimRight();

	return 1;
}

bool CFileGbr::LoadGbrFile(CString i_strFileName)
{
	InitializeGbrBuffer();

	char filepath[1000];			/** filepath local variable 01*/
	int TempNum;					/** TempNum local variable 02*/
	CString Tempstr;				/** Tempstr local variable 03*/
	wsprintfA(filepath,"%S",i_strFileName);

	/* Open Gbr file */
	if((GbrFileOut =_fsopen(filepath, "r",_SH_DENYNO))==NULL)
		return false;

	do{
		if(m_bLoadStopFlag == TRUE)
		{
			fclose(GbrFileOut);
			return false;
		}
		if(GetLine(m_strCommandLine))
		{
			m_lGbrLength++;
		}
		else
		{
			break;
		}
	}while(1);
	fclose(GbrFileOut);

	/* Create temporary buffer for process */
	pDrawBuffer = new GbrInfo[m_lGbrLength-1];
	m_iTotalProgress = m_lGbrLength*2;

	/* Open Drl file */
	if((GbrFileOut =_fsopen(filepath, "r",_SH_DENYNO))==NULL)
		return false;

	do{
		if(m_bLoadStopFlag == TRUE)
		{
			fclose(GbrFileOut);
			return false;
		}
		if(GetLine(m_strCommandLine))
		{
			m_iProgressCount++;
			MemoryBuf = m_strCommandLine;
			if(MemoryBuf.Left(1) == "%") //analysis header
			{
				TempNum = MemoryBuf.GetLength();

				Tempstr = MemoryBuf.Mid(TempNum-1,1);

				if( Tempstr == "%")
				{
					FindGbrHeader();
				}
			}
			else //analysis command
			{
				TempNum = MemoryBuf.GetLength();

				Tempstr = MemoryBuf.Mid(TempNum-1,1);

				if( Tempstr != "%")
				{	
					FindGbrCommandLine();
				}
			}

		}
		else
			break;

	}while(1);
	fclose(GbrFileOut);

	/* Create Entity file */
	if(WriteEntityFile()==0)
		return false; 

	return true;
}

void CFileGbr::FindGbrHeader(void)
{
	CString tempstr;				/** tempstr local variable 01*/
	CString strCmd1;				/** strCmd1 local variable 02*/
	CString strCmd2;				/** strCmd2 local variable 03*/
	
	if(MemoryBuf == "%") return;

	strCmd1 = MemoryBuf.GetAt(1);
	strCmd2 = MemoryBuf.GetAt(2);
	tempstr = strCmd1+strCmd2;

	if(tempstr == "FS")
	{
		tempstr = MemoryBuf.GetAt(3);
		if(tempstr == "L") 
			 m_iOmissionMode = LZO;
		else m_iOmissionMode = TZO;

		tempstr = MemoryBuf.GetAt(4);
		if(tempstr == "A") 
			 m_iCoordMode = ABS;
		else m_iCoordMode = INC;

		tempstr = MemoryBuf.GetAt(6);
		m_iXUpper = _ttoi(tempstr);
		
		tempstr = MemoryBuf.GetAt(7);
		m_iXLower = _ttoi(tempstr);
		
		tempstr = MemoryBuf.GetAt(9);
		m_iYUpper = _ttoi(tempstr);
		
		tempstr = MemoryBuf.GetAt(10);
		m_iYLower = _ttoi(tempstr);

		return;
	}

	if(tempstr == "MO")
	{
		tempstr = MemoryBuf.GetAt(3);
		if(tempstr == "I") 
			m_fChangeUNIT = (float)(25.4);
		else 
			m_fChangeUNIT = 1;

		return;
	}
	if(tempstr == "AD")
	{
		tempstr = MemoryBuf.GetAt(3);
		if(tempstr !="*")
		{
			double temp_r = 0.03937;					/** temp_r local variable 04*/
			int k1 = MemoryBuf.GetLength();				/** k1 local variable 05*/
			int k2 = MemoryBuf.FindOneOf(_T(","));		/** k2 local variable 06*/
			int k3 = MemoryBuf.FindOneOf(_T("X"));		/** k2 local variable 06*/

			if(k2 == -1) return;

			strCmd1 = MemoryBuf.Mid(4,k2-5);
			strCmd2 = MemoryBuf.Mid(k2+1,k1-k2-3);

			m_pLayer[m_iLayerCount].strLayerName = strCmd1;
			if(m_fChangeUNIT == 1)
				m_pLayer[m_iLayerCount].fHoleSize = (float)(_ttof(strCmd2));
			else
				m_pLayer[m_iLayerCount].fHoleSize = (float)(_ttof(strCmd2)/temp_r);

			tempstr = MemoryBuf.GetAt(k2-1);

			if(tempstr == "R")
				m_pLayer[m_iLayerCount].bRectFlag = true;
			else
				m_pLayer[m_iLayerCount].bRectFlag = false;

			m_pLayer[m_iLayerCount].nEntityCnt = 0;
			m_iLayerCount++;
		}
	}
}

void CFileGbr::FindGbrCommandLine(void)
{
	int G_pos;				/** G_pos local variable 01*/
	int D_pos;				/** D_pos local variable 02*/
	int X_pos;				/** X_pos local variable 03*/
	int Y_pos;				/** Y_pos local variable 04*/
	int I_pos;				/** I_pos local variable 05*/
	int J_pos;				/** J_pos local variable 06*/
	int nLayerNum;
	double dTemp_cX, dTemp_cY, dTemp_cR, dTemp_sA, dTemp_eA;

	if(MemoryBuf.FindOneOf(_T("*")) == -1) return;

	MemoryBuf.TrimRight(_T("*"));

	if((MemoryBuf == _T(""))||(MemoryBuf == _T("G75"))||
		(MemoryBuf == _T("G74"))||(MemoryBuf == _T("M2"))||(MemoryBuf == _T("M02"))) return;

	G_pos = MemoryBuf.FindOneOf(_T("G"));
	D_pos = MemoryBuf.FindOneOf(_T("D"));
	X_pos = MemoryBuf.FindOneOf(_T("X"));
	Y_pos = MemoryBuf.FindOneOf(_T("Y"));
	I_pos = MemoryBuf.FindOneOf(_T("I"));
	J_pos = MemoryBuf.FindOneOf(_T("J"));

	if(G_pos != -1) //G-CODE 추출
	{
		m_iGCode = Abstract_GCode(MemoryBuf,G_pos,D_pos,X_pos,Y_pos,I_pos,J_pos);
		m_iCurrentGMode = m_iGCode;
		if(m_iGCode == 54)
		{
			m_strLayerName.Format(_T("0"));
			m_iLayerNumber++;
		}
	}

	if(D_pos != -1) //D-CODE 추출
	{
		m_iDCode = _ttoi(MemoryBuf.Mid(D_pos+1,MemoryBuf.GetLength()-(D_pos+1)));
		m_iCurrentDMode = m_iDCode;
		if((G_pos != -1)&&(m_iGCode == 54))
		{
			m_strLayerName.Format(_T("%d"),m_iCurrentDMode);
		}
	}

	if(X_pos != -1) //X-CODE 추출
	{
		m_dX = Abstract_XCode(MemoryBuf,X_pos,Y_pos,I_pos,J_pos,D_pos);
		m_dX = ApplySettingX(m_dX);
	}
// 	else{
// 		m_dSX = m_dEX;
// 	}

	if(Y_pos != -1) //Y-CODE 추출
	{
		m_dY = Abstract_YCode(MemoryBuf,Y_pos,I_pos,J_pos,D_pos);
		m_dY = ApplySettingY(m_dY);
	}
// 	else{
// 		m_dSY = m_dEY;
// 	}


	if(I_pos != -1) //I-CODE 추출
	{
		m_dI = Abstract_ICode(MemoryBuf,I_pos,J_pos,D_pos);
		m_dI = ApplySettingX(m_dI);
	}
	else
	{
		m_dI = 0.0000;
	}

	if(J_pos != -1) //J-CODE 추출
	{
		m_dJ = Abstract_JCode(MemoryBuf,J_pos,D_pos);
		m_dJ = ApplySettingY(m_dJ);
	}
	else
	{
		m_dJ = 0.0000;
	}

	switch(m_iCurrentGMode)
	{
		case 0:	//LINE
		case 75:
		case 54:
			if(m_iCurrentDMode == 3) // Drill Mode.
			{
				nLayerNum = FindLayerNumber(m_strLayerName);
				if(m_pLayer[nLayerNum].bRectFlag == false){
					if(X_pos != -1) m_dEX = m_dX;
					else m_dEX = m_dSX;
					if(Y_pos != -1) m_dEY = m_dY;
					else m_dEY = m_dSY;
					InsertBufferDrill(m_dX, m_dY, m_pLayer[nLayerNum].fHoleSize);
					m_pLayer[nLayerNum].nEntityCnt++;
				}
				else
				{
					InsertBufferLine(m_dX - m_pLayer[nLayerNum].fHoleSize/2, m_dY - m_pLayer[nLayerNum].fHoleSize/2,
						m_dX + m_pLayer[nLayerNum].fHoleSize/2, m_dY - m_pLayer[nLayerNum].fHoleSize/2);
					m_pLayer[nLayerNum].nEntityCnt++;

					InsertBufferLine(m_dX + m_pLayer[nLayerNum].fHoleSize/2, m_dY - m_pLayer[nLayerNum].fHoleSize/2,
						m_dX + m_pLayer[nLayerNum].fHoleSize/2, m_dY+m_pLayer[nLayerNum].fHoleSize/2);
					m_pLayer[nLayerNum].nEntityCnt++;

					InsertBufferLine(m_dX + m_pLayer[nLayerNum].fHoleSize/2, m_dY + m_pLayer[nLayerNum].fHoleSize/2,
						m_dX - m_pLayer[nLayerNum].fHoleSize/2, m_dY+m_pLayer[nLayerNum].fHoleSize/2);
					m_pLayer[nLayerNum].nEntityCnt++;

					InsertBufferLine(m_dX - m_pLayer[nLayerNum].fHoleSize/2, m_dY + m_pLayer[nLayerNum].fHoleSize/2,
						m_dX - m_pLayer[nLayerNum].fHoleSize/2, m_dY-m_pLayer[nLayerNum].fHoleSize/2);
					m_pLayer[nLayerNum].nEntityCnt++;
				}
			}
			else if(m_iCurrentDMode == 2) // Move To
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

				m_dCurrentPosX = m_dX;
				m_dCurrentPosY = m_dY;
			}
			else
			{
				m_dCurrentPosX = m_dX;
				m_dCurrentPosY = m_dY;
			}

			break;
		case 1:
			{
				if(m_iCurrentDMode == 1) // Mark to
				{
					nLayerNum = FindLayerNumber(m_strLayerName);

					if(X_pos != -1) m_dEX = m_dX;
					else m_dEX = m_dSX;
					if(Y_pos != -1) m_dEY = m_dY;
					else m_dEY = m_dSY;

					InsertBufferLine(m_dSX, m_dSY, m_dEX, m_dEY);

					m_dSX = m_dEX;
					m_dSY = m_dEY;

					m_pLayer[nLayerNum].nEntityCnt++;
				}
				else if(m_iCurrentDMode == 2) // Move To
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

					m_dCurrentPosX = m_dX;
					m_dCurrentPosY = m_dY;
				}
				else
				{
					m_dCurrentPosX = m_dX;
					m_dCurrentPosY = m_dY;
				}
			}		
			break;
		case 2:
		case 3:
			if(m_iCurrentDMode == 1)
			{
				if(X_pos != -1) m_dEX = m_dX;
				else m_dEX = m_dSX;
				if(Y_pos != -1) m_dEY = m_dY;
				else m_dEY = m_dSY;

				nLayerNum = FindLayerNumber(m_strLayerName);

				if((m_dCurrentPosX == m_dX) && (m_dCurrentPosY == m_dY)) // CW or CCW Circle
				{
					dTemp_cX = m_dCurrentPosX + m_dI;
					dTemp_cY = m_dCurrentPosY + m_dJ;
					dTemp_cR = (float)(sqrt(float(m_dI*m_dI + m_dJ*m_dJ))); //cal Diameter

					InsertBufferCircle(dTemp_cX, dTemp_cY, dTemp_cR);
				}
				else // Arc
				{
					dTemp_cX = m_dCurrentPosX + m_dI;
					dTemp_cY = m_dCurrentPosY + m_dJ;
					dTemp_cR = (float)(sqrt(float(m_dI*m_dI + m_dJ*m_dJ))); //cal Diameter

					dTemp_sA = Cal_Angle(dTemp_cX,dTemp_cY,m_dSX,m_dSY); // cal Start Angle 
					dTemp_eA = Cal_Angle(dTemp_cX,dTemp_cY,m_dEX,m_dEY); // cal End Angle
					if(m_iCurrentGMode == 2) InsertBufferArc(dTemp_cX,dTemp_cY,dTemp_cR,dTemp_eA,dTemp_sA); //Inverse CW
					if(m_iCurrentGMode == 3) InsertBufferArc(dTemp_cX,dTemp_cY,dTemp_cR,dTemp_sA,dTemp_eA); //Inverse CCW
				}

				m_dSX = m_dEX;
				m_dSY = m_dEY;

				m_dCurrentPosX = m_dEX;
				m_dCurrentPosY = m_dEY;

				m_pLayer[nLayerNum].nEntityCnt++;
			}
			else if(m_iCurrentDMode == 2) // Move To
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

				m_dCurrentPosX = m_dEX;
				m_dCurrentPosY = m_dEY;
			}
			else
			{
				m_dCurrentPosX = m_dX;
				m_dCurrentPosY = m_dY;
			}
			break;
	}
}

double CFileGbr::Cal_Angle(double cx, double cy, double x, double y)
{
	double temp_x;
	double temp_y;
	double angle;

	temp_x = x-cx;
	temp_y = y-cy;

	angle = atan2(temp_y,temp_x) * 180 / PHI;

	return angle;
}

bool CFileGbr::WriteEntityFile(void)
{
	char read_path[1000];			/** read_path local variable 01*/
	char write_path[1000];			/** write_path local variable 02*/
	BOOL flag;						/** flag local variable 03*/

	wsprintfA(write_path, "%S\\entity.dat", m_strCommonFolder); 
	wsprintfA(read_path, "%S\\entity.dat", m_strEntityFolder); 

	if( (EntityFileOut =_fsopen(read_path, "w",_SH_DENYNO))==NULL)
		return false;

	fprintf(EntityFileOut, "[EntityInformation]\n");

	fprintf(EntityFileOut, "InspectionCount %d\n", m_nInspectionCnt);
	for(int i = 0; i < m_nInspectionCnt; i++)
	{
		fprintf(EntityFileOut, "InspectionPoint%d X: %.4lf Y: %.4lf\n", i+1, dInspPosX[i], dInspPosY[i]);
	}

	fprintf(EntityFileOut, "EntityLength %d\n",m_lTrueLength);

	int nRealLayerCnt = 0;
	int nRealLayerList[100];

	for(int nLayer=0; nLayer < m_iLayerCount; nLayer++)
	{
		if(m_pLayer[nLayer].nEntityCnt != 0){
			nRealLayerList[nRealLayerCnt] = nLayer;
			nRealLayerCnt++;
		}
	}

	fprintf(EntityFileOut, "ToolCount %d\n",nRealLayerCnt);
	for(int i=0; i<nRealLayerCnt; i++)
	{
		fprintf(EntityFileOut, "Tool %d D%S %d %.8f\n",i, 
			m_pLayer[nRealLayerList[i]].strLayerName, 
			m_pLayer[nRealLayerList[i]].nEntityCnt,
			m_pLayer[nRealLayerList[i]].fHoleSize);
	}

	fprintf(EntityFileOut, "%%\n");

	for(unsigned long i=0;i<(unsigned long)m_lTrueLength;i++)	/** i local variable 01*/
	{
		if(m_bLoadStopFlag == TRUE)
		{
			fclose(EntityFileOut);
			return false;
		}

		m_iProgressCount++;
		if(pDrawBuffer[i].strEntity == "DRILL")
		{
			fprintf(EntityFileOut, "D D%S %d %lf %lf\n", pDrawBuffer[i].strLayer, pDrawBuffer[i].nColor,
				(pDrawBuffer[i].dCX),(pDrawBuffer[i].dCY));
		}
		else if(pDrawBuffer[i].strEntity == "CIRCLE")
		{
			fprintf(EntityFileOut, "C D%S %d %lf %lf %lf %lf %lf %lf %lf\n", pDrawBuffer[i].strLayer, pDrawBuffer[i].nColor,
				(pDrawBuffer[i].dCX),(pDrawBuffer[i].dCY),(pDrawBuffer[i].dR), (pDrawBuffer[i].dSX), (pDrawBuffer[i].dSY), (pDrawBuffer[i].dEX), (pDrawBuffer[i].dEY));
		}
		else if(pDrawBuffer[i].strEntity == "ARC")
		{
			fprintf(EntityFileOut, "A D%S %d %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", pDrawBuffer[i].strLayer, pDrawBuffer[i].nColor,
				(pDrawBuffer[i].dCX),(pDrawBuffer[i].dCY),(pDrawBuffer[i].dR), (pDrawBuffer[i].dSA), (pDrawBuffer[i].dEA), (pDrawBuffer[i].dSX), (pDrawBuffer[i].dSY), (pDrawBuffer[i].dEX), (pDrawBuffer[i].dEY));
		}
		else if(pDrawBuffer[i].strEntity == "LINE")
		{
			fprintf(EntityFileOut, "L D%S %d %lf %lf %lf %lf\n", pDrawBuffer[i].strLayer, pDrawBuffer[i].nColor,
				(pDrawBuffer[i].dSX),(pDrawBuffer[i].dSY),
				(pDrawBuffer[i].dEX),(pDrawBuffer[i].dEY));
		}
	}

	fclose(EntityFileOut);

	CString strEntityPath;			/** strEntityPath local variable 04*/
	CString strCommonPath;			/** strCommonPath local variable 05*/

	strEntityPath.Format(_T("%S"),read_path);
	strCommonPath.Format(_T("%S"),write_path);

	flag = CopyFile(strEntityPath,strCommonPath,FALSE);

	return true;
}

double CFileGbr::Abstract_XCode(CString strCmd, int X_pos,int Y_pos,int I_pos,int J_pos,int D_pos)
{
	CString coord;			/** coord local variable 01*/

	if(Y_pos != -1) coord=strCmd.Mid(X_pos+1,Y_pos-(X_pos+1));
	else if(I_pos != -1) coord=strCmd.Mid(X_pos+1,I_pos-(X_pos+1));
	else if(J_pos != -1) coord=strCmd.Mid(X_pos+1,J_pos-(X_pos+1));
	else if(D_pos != -1) coord=strCmd.Mid(X_pos+1,D_pos-(X_pos+1));
	else coord=strCmd.Mid(X_pos+1,strCmd.GetLength()-(X_pos+1));

	return _ttof(coord);
}

double CFileGbr::Abstract_YCode(CString strCmd, int Y_pos, int I_pos, int J_pos, int D_pos)
{
	CString coord;			/** coord local variable 01*/

	if(I_pos != -1) coord=strCmd.Mid(Y_pos+1,I_pos-(Y_pos+1));
	else if(J_pos != -1) coord=strCmd.Mid(Y_pos+1,J_pos-(Y_pos+1));
	else if(D_pos != -1) coord=strCmd.Mid(Y_pos+1,D_pos-(Y_pos+1));
	else coord=strCmd.Mid(Y_pos+1,strCmd.GetLength()-(Y_pos+1));

	return _ttof(coord);
}

double CFileGbr::Abstract_ICode(CString strCmd,int I_pos,int J_pos,int D_pos)
{
	CString coord;			/** coord local variable 01*/

	if(J_pos != -1) coord=strCmd.Mid(I_pos+1,J_pos-(I_pos+1));
	else if(D_pos != -1) coord=strCmd.Mid(I_pos+1,D_pos-(I_pos+1));
	else coord=strCmd.Mid(I_pos+1,strCmd.GetLength()-(I_pos+1));

	return _ttof(coord);
}

double CFileGbr::Abstract_JCode(CString strCmd,int J_pos,int D_pos)
{
	CString coord;			/** coord local variable 01*/

	if(D_pos != -1) coord=strCmd.Mid(J_pos+1,D_pos-(J_pos+1));
	else coord=strCmd.Mid(J_pos+1,strCmd.GetLength()-(J_pos+1));

	return _ttof(coord);
}

int CFileGbr::Abstract_GCode(CString strCmd, int G_pos,int D_pos,int X_pos,int Y_pos,int I_pos,int J_pos)
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

double CFileGbr::ApplySettingX(double iNum)
{
	CString Tempstr;				/** Tempstr local variable 01*/
	int TempNum;					/** TempNum local variable 02*/

	if(m_iOmissionMode == LZO) iNum = iNum/(float)(pow((long double)10,m_iXLower));	
	else 
	{
		Tempstr.Format(_T("%d"),iNum);

		TempNum = Tempstr.GetLength();

		     if( TempNum <  m_iXUpper ) iNum = iNum * (10*(m_iXUpper - TempNum));
		else if( TempNum == m_iXUpper ) iNum = iNum;
		else if( TempNum >  m_iXUpper ) iNum = iNum / (10*(TempNum - m_iXUpper));
	}

	if(m_fChangeUNIT != 1) 
	{
		iNum = iNum * m_fChangeUNIT;
	}

	return iNum;
}

double CFileGbr::ApplySettingY(double iNum)
{
	CString Tempstr;				/** Tempstr local variable 01*/
	int TempNum;					/** TempNum local variable 02*/

	if(m_iOmissionMode == LZO) iNum = iNum/(float)(pow((long double)10,m_iYLower));
	else 
	{
		Tempstr.Format(_T("%d"),iNum);
		TempNum = Tempstr.GetLength();
		
		     if( TempNum <  m_iYUpper ) iNum = iNum * (10*(m_iYUpper - TempNum));
		else if( TempNum == m_iYUpper ) iNum = iNum;
		else if( TempNum >  m_iYUpper ) iNum = iNum / (10*(TempNum - m_iYUpper));
	}

	if(m_fChangeUNIT != 1) 
	{
		iNum = iNum*25.4;
	}

	return iNum;
}

int CFileGbr::FindLayerNumber(CString i_strLayerName)
{
	int i;			/** i local variable 01*/
	for(i=0;i<m_iLayerCount;i++)
	{
		if(i_strLayerName == m_pLayer[i].strLayerName)
		{
			return i;
		}
	}
	return 0;
}

double CFileGbr::DoRound(double i_dNumber)
{
	i_dNumber =  floor(10000.*(i_dNumber + 0.00005))/10000. ;
	return i_dNumber;
}

void CFileGbr::InsertBufferDrill(double dCX, double dCY, double dHoleSize)
{
	pDrawBuffer[m_lTrueLength].nColor = m_iLayerNumber;
	pDrawBuffer[m_lTrueLength].strEntity = "DRILL";
	pDrawBuffer[m_lTrueLength].strLayer = m_strLayerName;
	pDrawBuffer[m_lTrueLength].dCX = dCX;
	pDrawBuffer[m_lTrueLength].dCY = dCY;
	pDrawBuffer[m_lTrueLength].dR = dHoleSize;

	m_lTrueLength++;
}

void CFileGbr::InsertBufferCircle(double dCX, double dCY, double dR)
{
	pDrawBuffer[m_lTrueLength].nColor = m_iLayerNumber;
	pDrawBuffer[m_lTrueLength].strEntity = "CIRCLE";
	pDrawBuffer[m_lTrueLength].strLayer = m_strLayerName;
	pDrawBuffer[m_lTrueLength].dCX = dCX;
	pDrawBuffer[m_lTrueLength].dCY = dCY;
	pDrawBuffer[m_lTrueLength].dR = dR;
	pDrawBuffer[m_lTrueLength].dSX = dCX + dR;
	pDrawBuffer[m_lTrueLength].dSY = dCY;
	pDrawBuffer[m_lTrueLength].dEX = dCX + dR;
	pDrawBuffer[m_lTrueLength].dEY = dCY;

	m_lTrueLength++;
}

void CFileGbr::InsertBufferArc(double dCX, double dCY, double dR, double dSA, double dEA)
{
	double dAng;

	pDrawBuffer[m_lTrueLength].nColor = m_iLayerNumber;
	pDrawBuffer[m_lTrueLength].strEntity = "ARC";
	pDrawBuffer[m_lTrueLength].strLayer = m_strLayerName;
	pDrawBuffer[m_lTrueLength].dCX = dCX;
	pDrawBuffer[m_lTrueLength].dCY = dCY;
	pDrawBuffer[m_lTrueLength].dR  = dR;
	pDrawBuffer[m_lTrueLength].dSA = dSA;
	pDrawBuffer[m_lTrueLength].dEA = dEA;
	
	dAng = (dSA * PHI / 180.);
	pDrawBuffer[m_lTrueLength].dSX = dCX + (dR * (cos(dAng)));
	pDrawBuffer[m_lTrueLength].dSY = dCY + (dR * (sin(dAng)));

	if(dSA > dEA) dEA += 360;

	dAng = (dEA * PHI / 180.);

	pDrawBuffer[m_lTrueLength].dEX = dCX + (dR * (cos(dAng)));
	pDrawBuffer[m_lTrueLength].dEY = dCY + (dR * (sin(dAng)));

	m_lTrueLength++;
}

void CFileGbr::InsertBufferLine(double dSX, double dSY, double dEX, double dEY)
{
	pDrawBuffer[m_lTrueLength].nColor = m_iLayerNumber;
	pDrawBuffer[m_lTrueLength].strEntity = "LINE";
	pDrawBuffer[m_lTrueLength].strLayer = m_strLayerName;
	pDrawBuffer[m_lTrueLength].dSX = dSX;
	pDrawBuffer[m_lTrueLength].dSY = dSY;
	pDrawBuffer[m_lTrueLength].dEX = dEX;
	pDrawBuffer[m_lTrueLength].dEY = dEY;

	m_lTrueLength++;
}


void CFileGbr::SetLoadStop(BOOL bStopFlag)
{
	m_bLoadStopFlag = bStopFlag;
}


void CFileGbr::SetProgress(int iNum)
{
	m_iTotalProgress = iNum;
}


int CFileGbr::GetProgressTotal(void)
{
	return m_iTotalProgress;
}


int CFileGbr::GetProgress(void)
{
	return m_iProgressCount;
}
